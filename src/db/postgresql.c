#include "../include/db/postgresql.h"
#include "../include/utils/time.h"
#include "../include/utils/log.h"
#include "../include/utils/memory.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>


static pthread_mutex_t      db_exec_mutex = PTHREAD_MUTEX_INITIALIZER;


void PG_disconnect( PG_CONNECTION* db_connection ) {
    if( db_connection != NULL ) {
        LOG_print( "[%s]\tPG_disconnect( <'%s'> ).\n", TIME_get_gmt(), db_connection->id != NULL ? db_connection->id : "x" );

        if( db_connection->connection != NULL ) {
            PQfinish( db_connection->connection );  
            db_connection->connection = NULL;
        }
        if( db_connection->id != NULL ) {
            free( db_connection->id );
            db_connection->id = NULL;
        }
        db_connection->active = 0;

        LOG_print( "[%s]\tPG_disconnect.\n", TIME_get_gmt() );
    }
}


int PG_connect(
    PG_CONNECTION*  db_connection,
    const char*     host,
    const int       port,
    const char*     dbname,
    const char*     user,
    const char*     password,
    const char*     connection_string,
    const char*     name
) {
    char        conn_str[ 1024 ];

    db_connection->id = ( char * )SAFECALLOC( 1024, sizeof( char ), __FILE__, __LINE__ );
    if( connection_string ) {
        snprintf( conn_str, 1024, connection_string, dbname, host, port, user, password );
    } else {
        snprintf( conn_str, 1024, "dbname=%s host=%s port=%d user=%s password=%s", dbname, host, port, user, password );
    }
    
    snprintf( db_connection->id, 1024, "%s@%s[db=%s] (%s)", user, host, dbname, name );

    db_connection->connection = PQconnectdb( conn_str );

    if ( PQstatus( db_connection->connection ) == CONNECTION_BAD ) {
        db_connection->active = 0;
        free( db_connection->id ); db_connection->id = NULL;
        LOG_print( "Error. Message: \"%s\"", PQerrorMessage( db_connection->connection ) );
        return 0;
    }

    db_connection->active = 1;

    LOG_print( "ok.\n" );

    return 1;
}


int PG_exec(
    PG_CONNECTION       *db_connection,
    const char          *sql,
    size_t              sql_length, 
    DB_QUERY            *dst_result,
    const char* const   *param_values,
    const int           params_count,
    const int           *param_lengths,
    const int           *param_formats,
    Oid                 *param_types,
    qec                 *query_exec_callback,
    void                *data_ptr1,
    void                *data_ptr2
) {
    PGresult        *pg_result = NULL;

    LOG_print( "[%s]\tPG_exec( <'%s'>, \"%s\", ... ).\n", TIME_get_gmt(), db_connection->id, sql );
    pthread_mutex_lock( &db_exec_mutex );

    if( dst_result ) {
        dst_result->row_count = 0;
        dst_result->field_count = 0;

        dst_result->sql = ( char* )SAFECALLOC( sql_length + 1, sizeof( char ), __FILE__, __LINE__ );
        for( size_t l = 0; l < sql_length; l++ ) {
            *( dst_result->sql + l ) = sql[ l ];
        }
    }

    if( db_connection->connection ) {
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
        LOG_print( "[%s][fail] PG_exec. Message: \"%s\"\n", TIME_get_gmt(), PQerrorMessage( db_connection->connection ) );
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
                    strncpy( record->fields[ j ].label, PQfname( pg_result, j ), MAX_COLUMN_NAME_LEN );

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
                ( *query_exec_callback )( record, data_ptr1, data_ptr2 );
            }
        }

        if( dst_result ) {

            dst_result->records = ( DB_RECORD* )SAFEMALLOC( row_count * sizeof( DB_RECORD ), __FILE__, __LINE__ );

            dst_result->field_count = field_count;

            for( int i = 0; i < row_count; i++ ) {
                dst_result->records[ i ].fields = ( DB_FIELD* )SAFEMALLOC( field_count * sizeof( DB_FIELD ), __FILE__, __LINE__ );
                dst_result->records[ i ].field_count = field_count;

                for( int j = 0; j < field_count; j++ ) {
                    strncpy( dst_result->records[ i ].fields[ j ].label, PQfname( pg_result, j ), MAX_COLUMN_NAME_LEN );
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
    LOG_print( "[%s]\tPG_exec.\n", TIME_get_gmt() );
    PQclear( pg_result );
    pthread_mutex_unlock( &db_exec_mutex );

    return 1;
}
