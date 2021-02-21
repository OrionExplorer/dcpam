/* Copyright (C) 2020-2021 Marcin Kelar

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335 USA */

#include "../include/db/mongodb.h"
#include "../include/utils/time.h"
#include "../include/utils/log.h"
#include "../include/utils/memory.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>


static pthread_mutex_t      db_exec_mutex = PTHREAD_MUTEX_INITIALIZER;


void MONGODB_disconnect( MONGODB_CONNECTION* db_connection, LOG_OBJECT *log ) {
    if( db_connection != NULL ) {
        LOG_print( log, "[%s]\tMONGODB_disconnect( <'%s'> ).\n", TIME_get_gmt(), db_connection->id != NULL ? db_connection->id : "x" );

        if( db_connection->database ) {
            mongoc_database_destroy( db_connection->database );
        }
        if( db_connection->connection ) {
            mongoc_uri_destroy( db_connection->uri );
        }
        if( db_connection->connection ) {
            mongoc_client_destroy( db_connection->connection );
        }
        if( db_connection->id != NULL ) {
            free( db_connection->id );
            db_connection->id = NULL;
        }
        db_connection->active = 0;

        LOG_print( log, "[%s]\tMONGODB_disconnect.\n", TIME_get_gmt() );
    }
}


int MONGODB_connect(
    MONGODB_CONNECTION*  db_connection,
    const char*     host,
    const int       port,
    const char*     dbname,
    const char*     user,
    const char*     password,
    const char*     connection_string,
    const char*     name,
    LOG_OBJECT*     log
) {
    char            conn_str[ 1024 ];
    bson_error_t    error;
    char            *str;
    int             retval;

    db_connection->id = ( char * )SAFECALLOC( 1024, sizeof( char ), __FILE__, __LINE__ );
    if( connection_string ) {
        snprintf( conn_str, 1024, connection_string, dbname, host, port, user, password );
    } else {
        snprintf( conn_str, 1024, "dbname=%s host=%s port=%d user=%s password=%s", dbname, host, port, user, password );
    }

    snprintf( db_connection->id, 1024, "%s@%s[db=%s] (%s)", user, host, dbname, name );

    db_connection->uri = mongoc_uri_new_with_error( connection_string, &error );

    if( !db_connection->uri ) {
        LOG_print( log, "Error: Failed to parse URI \"%s\". Message: %s\n", connection_string, error.message );
        return 0;
    }

    db_connection->connection = mongoc_client_new_from_uri( db_connection->uri );

    if( !db_connection->connection ) {
        db_connection->active = 0;
        free( db_connection->id ); db_connection->id = NULL;
        LOG_print( log, "Error: unable to create client connection.\n" );
        return 0;
    }

    db_connection->database = mongoc_client_get_database( db_connection->connection, dbname );
    mongoc_client_set_appname( db_connection->connection, "DCPAM" );

    db_connection->active = 1;

    LOG_print( log, "ok.\n" );

    return 1;
}


int MONGODB_exec(
    MONGODB_CONNECTION       *db_connection,
    const char          *sql,
    size_t              sql_length, 
    DB_QUERY            *dst_result,
    const char* const   *param_values,
    const int           params_count,
    const int           *param_lengths,
    const int           *param_formats,
    const char* const   *param_types,
    qec                 *query_exec_callback,
    void                *data_ptr1,
    void                *data_ptr2,
    LOG_OBJECT          *log
) {
    bson_error_t        error;
    bson_t              *command;
    const bson_t        *reply;
    mongoc_cursor_t     *cursor;
    char                *str;

    LOG_print( log, "[%s]\tMONGODB_exec( <'%s'>, \"%s\", ... ).\n", TIME_get_gmt(), db_connection->id, sql );
    pthread_mutex_lock( &db_exec_mutex );

    if( dst_result ) {
        dst_result->row_count = 0;
        dst_result->field_count = 0;

        dst_result->sql = ( char* )SAFECALLOC( sql_length + 1, sizeof( char ), __FILE__, __LINE__ );
        for( size_t l = 0; l < sql_length; l++ ) {
            *( dst_result->sql + l ) = sql[ l ];
        }
    }

    if( db_connection->connection && db_connection->database) {
        command = bson_new_from_json( ( const uint8_t* )sql, sql_length, &error);
        if( command ) {
            cursor = mongoc_database_command( db_connection->database, MONGOC_QUERY_NONE, 0, 0, 0, command, NULL, NULL);
            while( mongoc_cursor_next( cursor, &reply ) ) {
              str = bson_as_json(reply, NULL);
              LOG_print( log, "%s\n", str);
              bson_free(str);
              bson_destroy( command );
           }

           if (mongoc_cursor_error(cursor, &error)) {
              LOG_print( log, "Fetch failure: %s\n", error.message);
              mongoc_cursor_destroy( cursor );
              return 0;
           }
           mongoc_cursor_destroy( cursor );
        } else {
            LOG_print( log, "Command error: %s\n", error.message);
            return 0;
        }
    } else {
        bson_free(str);
        return 0;
    }

    /*if( db_connection->connection ) {
        if( param_values == NULL || params_count == 0) {
            pg_result = PQexec( db_connection->connection, sql );
        } else {
            pg_result = PQexecParams( db_connection->connection,
                sql,
                params_count,
                param_types,
                (const char * const *)param_values,
                param_lengths,
                param_formats,
                1
            );
        }
    } else {
        PQclear( pg_result );
    }

    if ( PQresultStatus( pg_result ) != PGRES_TUPLES_OK && PQresultStatus( pg_result ) != PGRES_COMMAND_OK ) {
        PQclear( pg_result );
        LOG_print( log, "[%s][fail] MONGODB_exec. Message: \"%s\"\n", TIME_get_gmt(), PQerrorMessage( db_connection->connection ) );
        pthread_mutex_unlock( &db_exec_mutex );
        return 0;
    }
    
    int row_count = PQntuples( pg_result );
 
    if( dst_result ) {
        dst_result->row_count = row_count;
    }

    if( row_count > 0 ) {

        int field_count = PQnfields( pg_result );

        if( query_exec_callback ) {

            for( int i = 0; i < row_count; i++ ) {

                DB_RECORD   *record = SAFEMALLOC( 1 * sizeof( DB_RECORD ), __FILE__, __LINE__ );

                record->field_count = field_count;
                record->fields = ( DB_FIELD* )SAFEMALLOC( field_count * sizeof( DB_FIELD ), __FILE__, __LINE__ );

                for( int j = 0; j < field_count; j++ ) {
                    strlcpy( record->fields[ j ].label, PQfname( pg_result, j ), MAX_COLUMN_NAME_LEN );

                    int val_length = PQgetlength( pg_result, i, j );

                    if( val_length > 0 ) {

                        record->fields[ j ].value = SAFECALLOC( ( val_length + 1 ), sizeof( char ), __FILE__, __LINE__ );
                        char* tmp_res = PQgetvalue( pg_result, i, j );

                        record->fields[ j ].size = val_length;

                        for( int l = 0; l < val_length; l++ ) {
                            record->fields[ j ].value[ l ] = tmp_res[ l ];
                        }
                    } else {
                        record->fields[ j ].size = 0;
                        record->fields[ j ].value = NULL;
                    }
                }
                pthread_mutex_unlock( &db_exec_mutex );
                ( *query_exec_callback )( record, data_ptr1, data_ptr2, log );
            }
        }

        if( dst_result ) {

            dst_result->records = ( DB_RECORD* )SAFEMALLOC( row_count * sizeof( DB_RECORD ), __FILE__, __LINE__ );

            dst_result->field_count = field_count;

            for( int i = 0; i < row_count; i++ ) {
                dst_result->records[ i ].fields = ( DB_FIELD* )SAFEMALLOC( field_count * sizeof( DB_FIELD ), __FILE__, __LINE__ );
                dst_result->records[ i ].field_count = field_count;

                for( int j = 0; j < field_count; j++ ) {
                    strlcpy( dst_result->records[ i ].fields[ j ].label, PQfname( pg_result, j ), MAX_COLUMN_NAME_LEN );
                    int val_length = PQgetlength( pg_result, i, j );
                    if( val_length > 0 ) {
                        dst_result->records[ i ].fields[ j ].value = SAFECALLOC( ( val_length + 1 ), sizeof( char ), __FILE__, __LINE__ );
                        char* tmp_res = PQgetvalue( pg_result, i, j );

                        dst_result->records[ i ].fields[ j ].size = val_length;
                        for( int l = 0; l < val_length; l++ ) {
                            dst_result->records[ i ].fields[ j ].value[ l ] = tmp_res[ l ];
                        }
                    } else {
                        dst_result->records[ i ].fields[ j ].size = 0;
                        dst_result->records[ i ].fields[ j ].value = NULL;
                    }
                }
            }
        }
    }
    LOG_print( log, "[%s]\tMONGODB_exec.\n", TIME_get_gmt() );
    PQclear( pg_result );*/
    pthread_mutex_unlock( &db_exec_mutex );

    return 1;
}
