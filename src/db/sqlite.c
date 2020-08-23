#include "../include/db/sqlite.h"
#include "../include/utils/time.h"
#include "../include/utils/log.h"
#include "../include/utils/memory.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>


static pthread_mutex_t      db_exec_mutex = PTHREAD_MUTEX_INITIALIZER;


void SQLITE_disconnect( SQLITE_CONNECTION* db_connection, LOG_OBJECT *log ) {
    if( db_connection != NULL ) {
        LOG_print( log, "[%s]\tSQLITE_disconnect( <'%s'> ).\n", TIME_get_gmt(), db_connection->id != NULL ? db_connection->id : "x" );

        if( db_connection->connection != NULL ) {
            sqlite3_close( db_connection->connection );
            db_connection->connection = NULL;
        }
        if( db_connection->id != NULL ) {
            free( db_connection->id );
            db_connection->id = NULL;
        }
        db_connection->active = 0;

        LOG_print( log, "[%s]\tSQLITE_disconnect.\n", TIME_get_gmt() );
    }
}


int SQLITE_connect(
    SQLITE_CONNECTION* db_connection,
    const char* filename,
    const char* name,
    LOG_OBJECT* log
) {

    db_connection->id = ( char* )SAFECALLOC( 1024, sizeof( char ), __FILE__, __LINE__ );
    snprintf( db_connection->id, 1024, "[db=%s] (%s)", filename, name );

    int result = sqlite3_open( filename, &db_connection->connection );

    if( result != SQLITE_OK ) {
        db_connection->active = 0;
        free( db_connection->id ); db_connection->id = NULL;
        LOG_print( log, "Error. Message: \"%s\"", sqlite3_errmsg( db_connection->connection ) );
        sqlite3_close( db_connection->connection );
    }

    db_connection->active = 1;

    LOG_print( log, "ok.\n" );

    return 1;
}

int SQLITE_exec(
    SQLITE_CONNECTION* db_connection,
    const char* sql,
    size_t              sql_length,
    DB_QUERY* dst_result,
    const char* const* param_values,
    const int           params_count,
    const int* param_lengths,
    const int* param_formats,
    qec* query_exec_callback,
    void* data_ptr1,
    void* data_ptr2,
    LOG_OBJECT* log
) {

    LOG_print( log, "[%s]\tSQLITE_exec( <'%s'>, \"%s\", ... ).\n", TIME_get_gmt(), db_connection->id, sql );
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
        if( param_values == NULL || params_count == 0 ) {

            sqlite3_stmt* stmt;

            int result = sqlite3_prepare_v2( db_connection->connection, sql, ( int )sql_length, &stmt, NULL );
            if( result != SQLITE_OK ) {
                LOG_print( log, "Error: sqlite_prepare_v2 (%d).\n", result );
                return 0;
            }

            if( DB_QUERY_get_type( sql ) == DQT_SELECT ) {

                int row_count = 0;
                int field_count = 0;

                while( sqlite3_step( stmt ) != SQLITE_DONE ) {
                    int i = 0;
                    field_count = sqlite3_column_count( stmt );

                    if( query_exec_callback ) {
                        DB_RECORD* record = SAFEMALLOC( 1 * sizeof( DB_RECORD ), __FILE__, __LINE__ );

                        record->field_count = field_count;
                        record->fields = ( DB_FIELD* )SAFEMALLOC( field_count * sizeof( DB_FIELD ), __FILE__, __LINE__ );

                        for( int j = 0; j < field_count; j++ ) {
                            int data_size = sqlite3_column_bytes( stmt, i );
                            strncpy( record->fields[ i ].label, sqlite3_column_name( stmt, i ), MAX_COLUMN_NAME_LEN );
                            record->fields[ i ].size = ( unsigned long )data_size;
                            record->fields[ i ].value = SAFECALLOC( ( data_size + 1 ), sizeof( char ), __FILE__, __LINE__ );
                            memcpy( record->fields[ i ].value, sqlite3_column_blob( stmt, i ), data_size );
                        }

                        pthread_mutex_unlock( &db_exec_mutex );
                        ( *query_exec_callback )( record, data_ptr1, data_ptr2, log );
                    }

                    if( dst_result ) {
                        dst_result->records = realloc( dst_result->records, ( row_count + 1 ) * sizeof( DB_RECORD ) );
                        dst_result->records[ row_count ].fields = ( DB_FIELD* )SAFEMALLOC( field_count * sizeof( DB_FIELD ), __FILE__, __LINE__ );
                        dst_result->records[ row_count ].field_count = field_count;

                        for( i = 0; i < field_count; i++ ) {
                            int data_size = sqlite3_column_bytes( stmt, i );
                            strncpy( dst_result->records[ row_count ].fields[ i ].label, sqlite3_column_name( stmt, i ), MAX_COLUMN_NAME_LEN );
                            dst_result->records[ row_count ].fields[ i ].size = ( unsigned long )data_size;
                            dst_result->records[ row_count ].fields[ i ].value = SAFECALLOC( ( data_size + 1 ), sizeof( char ), __FILE__, __LINE__ );
                            memcpy( dst_result->records[ row_count ].fields[ i ].value, sqlite3_column_blob( stmt, i ), data_size );
                        }

                        row_count++;
                    }
                    
                }

                if( dst_result ) {
                    dst_result->field_count = field_count;
                    dst_result->row_count = row_count;
                }
            } else {
                if( sqlite3_step( stmt ) != SQLITE_DONE ) {
                    LOG_print( log, "[%s][fail] SQLITE_exec. Message: \"%s\"\n", TIME_get_gmt(), sqlite3_errmsg( db_connection->connection ) );
                    sqlite3_finalize( stmt );
                    pthread_mutex_unlock( &db_exec_mutex );
                    return 0;
                }
            }

            sqlite3_finalize( stmt );


        } else {
            /* Query with bind parameters */
            LOG_print( log, "[%s] Warning: not yet implemented!\n", TIME_get_gmt() );
        }
    }

    LOG_print( log, "[%s]\tSQLITE_exec.\n", TIME_get_gmt() );
    pthread_mutex_unlock( &db_exec_mutex );

    return 1;
}
