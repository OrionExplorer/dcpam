#include "../include/db/mariadb.h"
#include "../include/utils/time.h"
#include "../include/utils/log.h"
#include "../include/utils/memory.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

static pthread_mutex_t      db_exec_mutex = PTHREAD_MUTEX_INITIALIZER;


void MARIADB_disconnect( MARIADB_CONNECTION* db_connection ) {
    LOG_print( "[%s]\tMARIADB_disconnect( <'%s'> ).\n", TIME_get_gmt(), db_connection->id ? db_connection->id : "" );

    if( db_connection->connection ) {
        mysql_close( db_connection->connection );
        db_connection->connection = NULL;
    }

    if( db_connection->id ) {
        free( db_connection->id );
        db_connection->id = NULL;
    }

    LOG_print( "[%s]\tMARIADB_disconnect.\n", TIME_get_gmt() );
}


int MARIADB_connect(
    MARIADB_CONNECTION* db_connection,
    const char* host,
    const int           port,
    const char* dbname,
    const char* user,
    const char* password,
    const char* connection_string
) {

    db_connection->id = ( char* )SAFECALLOC( 1024, sizeof( char ), __FILE__, __LINE__ );
    snprintf( db_connection->id, 1024, "%s@%s[db=%s]", user, host, dbname );

    db_connection->connection = mysql_init( NULL );

    //mysql_options( db_connection->connection, MYSQL_SET_CHARSET_NAME, "cp1250" );
    //mysql_options( db_connection->connection, MYSQL_SET_CHARSET_NAME, MYSQL_AUTODETECT_CHARSET_NAME );
    //printf("MySQL character set is: %s.\n", mysql_character_set_name( db_connection->connection ) );

    if( mysql_real_connect( db_connection->connection, host, user, password, dbname, 0, NULL, 0 ) == NULL ) {
        db_connection->active = 0;
        free( db_connection->id ); db_connection->id = NULL;
        LOG_print( "error. Message: \"%s\".\n", mysql_error( db_connection->connection ) );
        return 0;
    }

    db_connection->active = 1;

    LOG_print( "ok.\n" );

    return 1;
}


int MARIADB_exec(
    MARIADB_CONNECTION* db_connection,
    const char* sql,
    size_t       sql_length,
    DB_QUERY* dst_result,
    const char* const* param_values,
    const int           params_count,
    const int* param_lengths,
    const int* param_formats,
    const char* const* param_types,
    qec* query_exec_callback,
    void* data_ptr1,
    void* data_ptr2
) {
    MYSQL_RES* mysql_result = NULL;
    MYSQL_ROW       mysql_row = NULL;
    MYSQL_FIELD* mysql_field = NULL;
    MYSQL_STMT* stmt = NULL;
    MYSQL_BIND* bind_param = NULL;
    unsigned long* lengths = NULL;
    int             query_result = -1;
    unsigned long   l = 0;

    LOG_print( "[%s]\tMARIADB_exec( <'%s'>, \"%s\", ... ).\n", TIME_get_gmt(), db_connection->id, sql );
    pthread_mutex_lock( &db_exec_mutex );

    dst_result->row_count = 0;
    dst_result->field_count = 0;

    dst_result->sql = ( char* )SAFECALLOC( sql_length + 1, sizeof( char ), __FILE__, __LINE__ );

    for( l = 0; l < sql_length; l++ ) {
        *( dst_result->sql + l ) = sql[ l ];
    }

    if( db_connection->connection ) {
        /* Simple query without binding parameters */
        if( param_values == NULL || params_count == 0 ) {

            int row_count = 0;
            int field_count = 0;
            query_result = mysql_real_query( db_connection->connection, dst_result->sql, ( unsigned long )sql_length );

            if( query_result == 0 ) {
                mysql_result = mysql_store_result( db_connection->connection );
            } else {
                LOG_print( "[%s][ERROR]\tMARIADB_exec: #%d, %s (\"%s\", len=%d)\n", TIME_get_gmt(), query_result, mysql_error( db_connection->connection ), dst_result->sql, sql_length );
                pthread_mutex_unlock( &db_exec_mutex );
                return 0;
            }

            if( mysql_result ) {
                field_count = ( int )mysql_num_fields( mysql_result );
                dst_result->field_count = field_count;
                row_count = ( int )mysql_num_rows( mysql_result );
            }

            if( row_count > 0 ) {
                unsigned long val_length = 0;
                dst_result->row_count = row_count;
                dst_result->records = ( DB_RECORD* )SAFEMALLOC( row_count * sizeof( DB_RECORD ), __FILE__, __LINE__ );

                for( int i = 0; i < row_count; i++ ) {
                    mysql_row = mysql_fetch_row( mysql_result );
                    lengths = mysql_fetch_lengths( mysql_result );
                    dst_result->records[ i ].fields = ( DB_FIELD* )SAFEMALLOC( field_count * sizeof( DB_FIELD ), __FILE__, __LINE__ );
                    dst_result->records[ i ].field_count = field_count;

                    for( int j = 0; j < field_count; j++ ) {
                        val_length = lengths[ j ];
                        if( val_length > 0 ) {
                            dst_result->records[ i ].fields[ j ].size = val_length;
                            dst_result->records[ i ].fields[ j ].value = SAFECALLOC( val_length + 1, sizeof( char ), __FILE__, __LINE__ );
                            for( unsigned long k = 0; k < val_length; k++ ) {
                                *( k + dst_result->records[ i ].fields[ j ].value ) = mysql_row[ j ][ k ];
                            }
                        } else {
                            dst_result->records[ i ].fields[ j ].value = NULL;
                            dst_result->records[ i ].fields[ j ].size = 0;
                        }
                    }
                }

                for( int i = 0; i < field_count; i++ ) {
                    mysql_field = mysql_fetch_field( mysql_result );
                    for( int j = 0; j < row_count; j++ ) {
                        strncpy( dst_result->records[ j ].fields[ i ].label, mysql_field->name, MAX_COLUMN_NAME_LEN );
                    }
                }
            }

            mysql_free_result( mysql_result );

        } else if( params_count > 0 ) {
            /* Query with bind parameters */
            stmt = mysql_stmt_init( db_connection->connection );
            if( !stmt ) {
                LOG_print( "[%s][ERROR]\tMARIADB_exec: mysql_stmt_init() out of memory.\n", TIME_get_gmt() );
                mysql_stmt_close( stmt );
                pthread_mutex_unlock( &db_exec_mutex );
                return 0;
            }
            if( mysql_stmt_prepare( stmt, dst_result->sql, ( unsigned long )sql_length ) ) {
                LOG_print( "[%s][ERROR]\tMARIADB_exec: mysql_stmt_prepare() SQL failed: %s\n", TIME_get_gmt(), mysql_stmt_error( stmt ) );
                mysql_stmt_close( stmt );
                pthread_mutex_unlock( &db_exec_mutex );
                return 0;
            }

            int internal_params_count = mysql_stmt_param_count( stmt );

            if( internal_params_count != params_count ) {
                LOG_print( "[%s][ERROR]\tMARIADB_exec: invalid parameter count returned by MySQL.\n", TIME_get_gmt(), mysql_stmt_error( stmt ) );
                mysql_stmt_close( stmt );
                pthread_mutex_unlock( &db_exec_mutex );
                return 0;
            }

            bind_param = SAFEMALLOC( internal_params_count * sizeof( MYSQL_BIND ), __FILE__, __LINE__ );
            memset( bind_param, 0, internal_params_count * sizeof( MYSQL_BIND ) );

            for( int i = 0; i < internal_params_count; i++ ) {
                bind_param[ i ].buffer_type = MYSQL_TYPE_STRING;
                bind_param[ i ].buffer = ( char* )param_values[ i ];
                bind_param[ i ].buffer_length = param_lengths[ i ];
                bind_param[ i ].is_null = 0;
                bind_param[ i ].length = 0;
            }

            if( mysql_stmt_bind_param( stmt, bind_param ) ) {
                LOG_print( "[%s][ERROR]\tMARIADB_exec: mysql_stmt_bind_param() failed\n" );
                LOG_print( " %s\n", mysql_stmt_error( stmt ) );
                free( bind_param ); bind_param = NULL;
                mysql_stmt_close( stmt );
                pthread_mutex_unlock( &db_exec_mutex );
                return 0;
            }

            if( mysql_stmt_execute( stmt ) ) {
                LOG_print( "[%s][ERROR]\tMARIADB_exec: mysql_stmt_execute() failed\n" );
                LOG_print( " %s\n", mysql_stmt_error( stmt ) );
                free( bind_param ); bind_param = NULL;
                mysql_stmt_close( stmt );
                pthread_mutex_unlock( &db_exec_mutex );
                return 0;
            }

            /****
            * TODO: mysql_stmt_fetch (eg. https://docs.oracle.com/cd/E17952_01/mysql-5.5-en/c-api-prepared-call-statements.html)
            ****/

            free( bind_param ); bind_param = NULL;
            mysql_stmt_free_result( stmt );
            mysql_stmt_close( stmt );

            pthread_mutex_unlock( &db_exec_mutex );
        }

    } else {
        LOG_print( "[%s][ERROR]\tMARIADB_exec: #%d, %s.\n", TIME_get_gmt(), query_result, mysql_error( db_connection->connection ) );
    }

    LOG_print( "[%s]\tMARIADB_exec.\n", TIME_get_gmt() );
    pthread_mutex_unlock( &db_exec_mutex );

    return 1;
}
