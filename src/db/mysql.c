#include "../include/db/mysql.h"
#include "../include/utils/time.h"
#include "../include/utils/log.h"
#include "../include/utils/memory.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

static pthread_mutex_t      db_exec_mutex = PTHREAD_MUTEX_INITIALIZER;


void MYSQL_disconnect( MYSQL_CONNECTION* db_connection, LOG_OBJECT *log ) {
    LOG_print( log, "[%s]\tMYSQL_disconnect( <'%s'> ).\n", TIME_get_gmt(), db_connection->id ? db_connection->id : "" );

    if( db_connection->connection ) {
        mysql_close( db_connection->connection );
        db_connection->connection = NULL;
    }

    if( db_connection->id ) {
        free( db_connection->id );
        db_connection->id = NULL;
    }

    LOG_print( log, "[%s]\tMYSQL_disconnect.\n", TIME_get_gmt() );
}


int MYSQL_connect(
    MYSQL_CONNECTION* db_connection,
    const char* host,
    const int           port,
    const char* dbname,
    const char* user,
    const char* password,
    const char* connection_string,
    const char* name,
    LOG_OBJECT *log
) {

    db_connection->id = ( char* )SAFECALLOC( 1024, sizeof( char ), __FILE__, __LINE__ );
    snprintf( db_connection->id, 1024, "%s@%s[db=%s] (%s)", user, host, dbname, name );

    db_connection->connection = mysql_init( NULL );

    //mysql_options( db_connection->connection, MYSQL_SET_CHARSET_NAME, "cp1250" );
    //mysql_options( db_connection->connection, MYSQL_SET_CHARSET_NAME, MYSQL_AUTODETECT_CHARSET_NAME );
    //printf("MySQL character set is: %s.\n", mysql_character_set_name( db_connection->connection ) );

    if( mysql_real_connect( db_connection->connection, host, user, password, dbname, 0, NULL, 0 ) == NULL ) {
        db_connection->active = 0;
        free( db_connection->id ); db_connection->id = NULL;
        LOG_print( log, "error. Message: \"%s\".\n", mysql_error( db_connection->connection ) );
        return 0;
    }

    db_connection->active = 1;

    LOG_print( log, "ok.\n" );

    return 1;
}


int MYSQL_exec(
    MYSQL_CONNECTION* db_connection,
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
    void* data_ptr2,
    LOG_OBJECT *log
) {
    MYSQL_RES* mysql_result = NULL;
    MYSQL_ROW       mysql_row = NULL;
    MYSQL_FIELD* mysql_field = NULL;
    MYSQL_STMT* stmt = NULL;
    MYSQL_BIND* bind_param = NULL;
    unsigned long* lengths = NULL;
    int             query_result = -1;

    LOG_print( log, "[%s]\tMYSQL_exec( <'%s'>, \"%s\", ... ).\n", TIME_get_gmt(), db_connection->id, sql );
    pthread_mutex_lock( &db_exec_mutex );


    if( dst_result ) {

        unsigned long   l = 0;

        dst_result->row_count = 0;
        dst_result->field_count = 0;

        dst_result->sql = ( char* )SAFECALLOC( sql_length + 1, sizeof( char ), __FILE__, __LINE__ );

        for( l = 0; l < sql_length; l++ ) {
            *( dst_result->sql + l ) = sql[ l ];
        }
    }

    if( db_connection->connection ) {
        /* Simple query without binding parameters */
        if( param_values == NULL || params_count == 0 ) {

            int row_count = 0;
            int field_count = 0;
            query_result = mysql_real_query( db_connection->connection, sql, ( unsigned long )sql_length );

            if( query_result == 0 ) {
                mysql_result = mysql_store_result( db_connection->connection );
            } else {
                LOG_print( log, "[%s][ERROR]\tMYSQL_exec: #%d, %s (\"%s\", len=%d)\n", TIME_get_gmt(), query_result, mysql_error( db_connection->connection ), sql, sql_length );
                pthread_mutex_unlock( &db_exec_mutex );
                return 0;
            }

            if( mysql_result ) {
                field_count = ( int )mysql_num_fields( mysql_result );
                if( dst_result ) {
                    dst_result->field_count = field_count;
                }
                
                row_count = ( int )mysql_num_rows( mysql_result );
            }

            if( row_count > 0 ) {
                unsigned long val_length = 0;

                if( query_exec_callback ) {

                    char *table_fields[ MAX_COLUMNS ];

                    for( int i = 0; i < field_count; i++ ) {
                        table_fields[ i ] = SAFECALLOC( MAX_COLUMN_NAME_LEN + 1, sizeof( char ), __FILE__, __LINE__ );
                        mysql_field = mysql_fetch_field( mysql_result );
                        strlcpy( table_fields[ i ], mysql_field->name, MAX_COLUMN_NAME_LEN );
                    }

                    for( int i = 0; i < row_count; i++ ) {
                        DB_RECORD* record = SAFEMALLOC( 1 * sizeof( DB_RECORD ), __FILE__, __LINE__ );

                        record->field_count = field_count;
                        record->fields = ( DB_FIELD* )SAFEMALLOC( field_count * sizeof( DB_FIELD ), __FILE__, __LINE__ );

                        mysql_row = mysql_fetch_row( mysql_result );
                        lengths = mysql_fetch_lengths( mysql_result );

                        for( int j = 0; j < field_count; j++ ) {
                            val_length = lengths[ j ];
                            if( val_length > 0 ) {
                                record->fields[ j ].size = val_length;
                                record->fields[ j ].value = SAFECALLOC( val_length + 1, sizeof( char ), __FILE__, __LINE__ );
                                for( unsigned long k = 0; k < val_length; k++ ) {
                                    *( k + record->fields[ j ].value ) = mysql_row[ j ][ k ];
                                }
                            } else {
                                record->fields[ j ].value = NULL;
                                record->fields[ j ].size = 0;
                            }

                            strlcpy( record->fields[ j ].label, table_fields[ j ], MAX_COLUMN_NAME_LEN );
                        }

                        pthread_mutex_unlock( &db_exec_mutex );
                        ( *query_exec_callback )( record, data_ptr1, data_ptr2, log );
                    }

                    for( int i = 0; i < field_count; i++ ) {
                        free( table_fields[ i ] ); table_fields[ i ] = NULL;
                    }
                }

                if( dst_result ) {
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
                            strlcpy( dst_result->records[ j ].fields[ i ].label, mysql_field->name, MAX_COLUMN_NAME_LEN );
                        }
                    }
                }
            }

            mysql_free_result( mysql_result );

        } else if( params_count > 0 ) {
            /* Query with bind parameters */
            LOG_print( log, "[%s] Warning: not yet implemented!\n", TIME_get_gmt() );
        }

    } else {
        LOG_print( log, "[%s][ERROR]\tMYSQL_exec: #%d, %s.\n", TIME_get_gmt(), query_result, mysql_error( db_connection->connection ) );
    }

    LOG_print( log, "[%s]\tMYSQL_exec.\n", TIME_get_gmt() );
    pthread_mutex_unlock( &db_exec_mutex );

    return 1;
}
