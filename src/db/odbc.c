#include "../include/db/odbc.h"
#include "../include/utils/time.h"
#include "../include/utils/log.h"
#include "../include/utils/memory.h"
#include "../include/utils/strings.h"
#include <stdlib.h>
#include <string.h>
#include <sql.h>
#include <sqlext.h>
#include <pthread.h>

static pthread_mutex_t      db_exec_mutex = PTHREAD_MUTEX_INITIALIZER;


#define CHECK_ERROR(e, s, h, t, r, l) {if (e!=SQL_SUCCESS && e != SQL_SUCCESS_WITH_INFO) { ODBC_get_error(s, h, t, l); if( r == TRUE ) { return 0; } } }

void ODBC_get_error( char* fn, SQLHANDLE handle, SQLSMALLINT type, LOG_OBJECT *log ) {
    SQLSMALLINT i = 0;
    SQLINTEGER native_err;
    SQLCHAR sql_state[ 7 ];
    SQLCHAR message[ 256 ];
    SQLSMALLINT text_len;
    SQLRETURN ret;

    LOG_print( log, "Error: Driver reported the following error - [%s]:\n", fn );
    do
    {
        ret = SQLGetDiagRec( type, handle, ++i, sql_state, &native_err,
            message, sizeof( message ), &text_len );
        if( SQL_SUCCEEDED( ret ) ) {
            LOG_print( log, "%s:%ld:%ld:%s\n",
                sql_state, ( long )i, ( long )native_err, message );
        }
    } while( ret == SQL_SUCCESS );
}

void ODBC_disconnect( ODBC_CONNECTION* db_connection, LOG_OBJECT *log ) {
    LOG_print( log, "[%s]\tODBC_disconnect( <'%s'> ).\n", TIME_get_gmt(), db_connection->id ? db_connection->id : "" );

    if( db_connection->connection != SQL_NULL_HDBC ) {
        SQLDisconnect( db_connection->connection );
        SQLFreeHandle( SQL_HANDLE_DBC, db_connection->connection );
        db_connection->connection = NULL;
    }

    if( db_connection->sqlenvhandle != SQL_NULL_HENV ) {
        SQLFreeHandle( SQL_HANDLE_ENV, db_connection->sqlenvhandle );
    }

    if( db_connection->id ) {
        free( db_connection->id );
        db_connection->id = NULL;
    }

    LOG_print( log, "[%s]\tODBC_disconnect.\n", TIME_get_gmt() );
}


int ODBC_connect(
    ODBC_CONNECTION* db_connection,
    const char* host,
    const int           port,
    const char* dbname,
    const char* user,
    const char* password,
    const char* connection_string,
    const char* name,
    LOG_OBJECT *log
) {
    SQLRETURN       retcode;

    retcode = SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &db_connection->sqlenvhandle );
    CHECK_ERROR( retcode, "SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &db_connection->sqlenvhandle )", db_connection->sqlenvhandle, SQL_HANDLE_ENV, TRUE, log );


    retcode = SQLSetEnvAttr( db_connection->sqlenvhandle, SQL_ATTR_ODBC_VERSION, ( void* )SQL_OV_ODBC3, 0 );
    CHECK_ERROR( retcode, "SQLSetEnvAttr( db_connection->sqlenvhandle, SQL_ATTR_ODBC_VERSION, ( void* )SQL_OV_ODBC3, 0 )", db_connection->sqlenvhandle, SQL_HANDLE_ENV, TRUE, log );


    retcode = SQLAllocHandle( SQL_HANDLE_DBC, db_connection->sqlenvhandle, &db_connection->connection );
    CHECK_ERROR( retcode, "SQLAllocHandle( SQL_HANDLE_DBC, db_connection->sqlenvhandle, &db_connection->connection )", db_connection->connection, SQL_HANDLE_DBC, TRUE, log );

    if( connection_string ) {
        db_connection->id = SAFECALLOC( 1025, sizeof( char ), __FILE__, __LINE__ );
        if( db_connection->id ) {
            snprintf( db_connection->id, 1024, "%s (%s)", connection_string, name );
            /*strncpy(
                db_connection->id,
                connection_string,
                strlen( connection_string )
            );*/
        }
    } else {
        LOG_print( log, "error. Message: no \"connection_string\" provided in config.json\n" );
        return 0;
    }

    retcode = SQLDriverConnect( db_connection->connection, NULL,
        ( SQLCHAR* )connection_string,
        SQL_NTS,
        NULL,
        0,
        NULL,
        SQL_DRIVER_NOPROMPT
    );
    CHECK_ERROR( retcode, "SQLDriverConnect()", db_connection->connection, SQL_HANDLE_DBC, TRUE, log );

    db_connection->active = 1;

    LOG_print( log, "ok.\n" );

    return 1;
}


int ODBC_exec(
    ODBC_CONNECTION        *db_connection,
    const char             *sql,
    size_t                 sql_length,
    DB_QUERY               *dst_result,
    const char* const      *param_values,
    const int              params_count,
    const int              *param_lengths,
    const int              *param_formats,
    const char* const      *param_types,
    qec* query_exec_callback,
    void* data_ptr1,
    void* data_ptr2,
    LOG_OBJECT *log
) {
    SQLHSTMT        stmt;
    SQLRETURN       retcode;
    SQLCHAR         *column_name[ MAX_COLUMNS ];
    SQLWCHAR        *column_data[ MAX_COLUMNS ];
    SQLULEN         column_data_size[ MAX_COLUMNS ];
    SQLLEN          column_data_len[ MAX_COLUMNS ];
    SQLSMALLINT     column_data_type[ MAX_COLUMNS ];
    SQLSMALLINT     column_data_nullable[ MAX_COLUMNS ];
    SQLSMALLINT     num_columns = 0;
    DB_RECORD       *tmp_records = NULL;

    if( db_connection->active == 0 || db_connection->id == NULL ) {
        return 0;
    }

    LOG_print( log, "[%s]\tODBC_exec( <'%s'>, \"%s\", ... ).\n", TIME_get_gmt(), db_connection->id, sql );
    pthread_mutex_lock( &db_exec_mutex );

    if( dst_result ) {
        dst_result->row_count = 0;
        dst_result->field_count = 0;

        dst_result->sql = SAFECALLOC( sql_length + 1, sizeof( char ), __FILE__, __LINE__ );
        for( unsigned long l = 0; l < sql_length; l++ ) {
            *( dst_result->sql + l ) = sql[ l ];
        }
    }

    if( db_connection->connection ) {
        retcode = SQLAllocHandle( SQL_HANDLE_STMT, db_connection->connection, &stmt );
        CHECK_ERROR( retcode, "SQLAllocHandle()", stmt, SQL_HANDLE_STMT, TRUE, log );

        SQLSetStmtAttr( &stmt, SQL_ATTR_CURSOR_TYPE, ( SQLPOINTER )SQL_CURSOR_DYNAMIC, 0 );

        /* Simple query without binding parameters */
        if( param_values == NULL || params_count == 0 ) {

            for( int i = 0; i < MAX_COLUMNS; i++ ) {
                column_name[ i ] = NULL;
                column_data[ i ] = NULL;
            }

            retcode = SQLPrepare( stmt, ( SQLCHAR* )sql, ( SQLINTEGER )sql_length );
            CHECK_ERROR( retcode, "SQLPrepare()", stmt, SQL_HANDLE_STMT, TRUE, log );

            if( DB_QUERY_get_type( sql ) == DQT_SELECT ) {
                retcode = SQLNumResultCols( stmt, &num_columns );
                if( ( retcode != SQL_SUCCESS ) && ( retcode != SQL_SUCCESS_WITH_INFO ) ) {
                    ODBC_get_error( "SQLNumResultCols()", stmt, SQL_HANDLE_STMT, log );
                    SQLFreeHandle( SQL_HANDLE_STMT, stmt );

                    LOG_print( log, "[%s]\tODBC_exec.\n", TIME_get_gmt() );
                    pthread_mutex_unlock( &db_exec_mutex );
                    return 0;
                }

                for( int i = 0; i < num_columns; i++ ) {
                    column_name[ i ] = SAFECALLOC( MAX_COLUMN_NAME_LEN + 1, sizeof( SQLCHAR ), __FILE__, __LINE__ );
                    retcode = SQLDescribeCol(
                        stmt,
                        i + 1,
                        column_name[ i ],
                        MAX_COLUMN_NAME_LEN,
                        NULL,
                        &column_data_type[ i ],
                        &column_data_size[ i ],
                        NULL,
                        &column_data_nullable[ i ]
                    );
                    CHECK_ERROR( retcode, "SQLDescribeCol()", stmt, SQL_HANDLE_STMT, TRUE, log );

                    column_data[ i ] = SAFECALLOC( column_data_size[ i ] + 1, sizeof( SQLCHAR ), __FILE__, __LINE__ );

                    retcode = SQLBindCol(
                        stmt,
                        i + 1,
                        SQL_C_CHAR,
                        column_data[ i ],
                        column_data_size[ i ],
                        &column_data_len[ i ]
                    );
                    CHECK_ERROR( retcode, "SQLBindCol()", stmt, SQL_HANDLE_STMT, TRUE, log );
                }
            }

            retcode = SQLExecute( stmt );
            if( ( retcode != SQL_SUCCESS ) && ( retcode != SQL_SUCCESS_WITH_INFO ) ) {
                ODBC_get_error( "SQLExecute()", stmt, SQL_HANDLE_STMT, log );
                SQLFreeHandle( SQL_HANDLE_STMT, stmt );

                for( int i = 0; i < MAX_COLUMNS; i++ ) {
                    free( column_name[ i ] ); column_name[ i ] = NULL;
                    free( column_data[ i ] ); column_data[ i ] = NULL;
                    column_data_len[ i ] = 0;
                    column_data_size[ i ] = 0;
                }

                LOG_print( log, "[%s]\tODBC_exec.\n", TIME_get_gmt() );
                pthread_mutex_unlock( &db_exec_mutex );
                return 0;
            }

            if( DB_QUERY_get_type( sql ) == DQT_SELECT && num_columns > 0 ) {

                int row_count = 0;

                /*if( dst_result && dst_result->records ) {
                    free( dst_result->records ); dst_result->records = NULL;
                }*/
                while( TRUE ) {
                    retcode = SQLFetch( stmt );
                    if( retcode == SQL_NO_DATA ) {
                        break;
                    }

                    CHECK_ERROR( retcode, "SQLFetch()", stmt, SQL_HANDLE_STMT, TRUE, log );

                    if( query_exec_callback ) {

                        DB_RECORD   *record = SAFEMALLOC( 1 * sizeof( DB_RECORD ), __FILE__, __LINE__ );

                        record->field_count = num_columns;
                        record->fields = ( DB_FIELD* )SAFEMALLOC( num_columns * sizeof( DB_FIELD ), __FILE__, __LINE__ );

                        for( int k = 0; k < num_columns; k++ ) {
                            if( column_data_len[ k ] != SQL_NULL_DATA ) {
                                strncpy( record->fields[ k ].label, ( const char* )column_name[ k ], MAX_COLUMN_NAME_LEN );
                                record->fields[ k ].value = SAFECALLOC( ( size_t )column_data_len[ k ] + 1, sizeof( char ), __FILE__, __LINE__ );
                                memcpy( record->fields[ k ].value, ( char* )column_data[ k ], ( size_t )column_data_len[ k ] + 1);
                                record->fields[ k ].size = ( unsigned long )column_data_len[ k ];
                                /* Special case, where datetime value length is larger by 1 than real date string size */
                                if( column_data_type[ k ] == SQL_TYPE_TIMESTAMP ) {
                                    memcpy( record->fields[ k ].value, rtrim( record->fields[ k ].value, ' ' ), ( size_t )column_data_len[ k ] - 1 );
                                    record->fields[ k ].size = ( unsigned long )column_data_len[ k ] - 1;
                                }
                            } else {
                                strncpy( record->fields[ k ].label, ( const char* )column_name[ k ], MAX_COLUMN_NAME_LEN );
                                record->fields[ k ].size = 0;
                                record->fields[ k ].value = NULL;
                            }
                        }

                        pthread_mutex_unlock( &db_exec_mutex );
                        ( *query_exec_callback )( record, data_ptr1, data_ptr2, log );

                    }

                    if( dst_result ) {
                        dst_result->records = ( DB_RECORD* )realloc( tmp_records, ( row_count + 1 ) * sizeof( DB_RECORD ) );
                        if( dst_result->records != NULL ) {
                            tmp_records = dst_result->records;
                            dst_result->records[ row_count ].fields = ( DB_FIELD* )SAFEMALLOC( num_columns * sizeof( DB_FIELD ), __FILE__, __LINE__ );
                            dst_result->records[ row_count ].field_count = num_columns;

                            for( int k = 0; k < num_columns; k++ ) {

                                if( column_data_len[ k ] != SQL_NULL_DATA ) {
                                    strncpy( dst_result->records[ row_count ].fields[ k ].label, ( const char* )column_name[ k ], MAX_COLUMN_NAME_LEN );
                                    dst_result->records[ row_count ].fields[ k ].value = SAFECALLOC( ( size_t )(column_data_len[ k ] + 1),  sizeof( char ), __FILE__, __LINE__ );
                                    memcpy( dst_result->records[ row_count ].fields[ k ].value, ( char* )column_data[ k ], ( size_t )column_data_len[ k ] + 1);
                                    dst_result->records[ row_count ].fields[ k ].size = ( unsigned long )column_data_len[ k ];
                                    /* Special case, where datetime value length is larger by 1 than real date string size */
                                    if( column_data_type[ k ] == SQL_TYPE_TIMESTAMP ) {
                                        memcpy( dst_result->records[ row_count ].fields[ k ].value, rtrim( dst_result->records[ row_count ].fields[ k ].value, ' ' ), ( size_t )column_data_len[ k ] - 1 );
                                        dst_result->records[ row_count ].fields[ k ].size = ( unsigned long )column_data_len[ k ] - 1;
                                    }
                                    
                                } else {
                                    strncpy( dst_result->records[ row_count ].fields[ k ].label, ( const char* )column_name[ k ], MAX_COLUMN_NAME_LEN );
                                    dst_result->records[ row_count ].fields[ k ].size = 0;
                                    dst_result->records[ row_count ].fields[ k ].value = NULL;
                                }
                            }
                        }
                        row_count++;
                    }
                }

                //retcode = SQLCloseCursor( stmt );
                //CHECK_ERROR( retcode, "SQLCloseCursor()", stmt, SQL_HANDLE_DBC, TRUE ); 
                if( dst_result ) {
                    tmp_records = NULL;
                    dst_result->row_count = row_count;
                    dst_result->field_count = num_columns;
                }

                for( int i = 0; i < num_columns; i++ ) {
                    free( column_name[ i ] ); column_name[ i ] = NULL;
                    free( column_data[ i ] ); column_data[ i ] = NULL;
                    column_data_len[ i ] = 0;
                    column_data_size[ i ] = 0;
                }
            }
        } else {
            /* Query with bind parameters */
            LOG_print( log, "[%s] Warning: not yet implemented!\n", TIME_get_gmt() );
        }

        SQLFreeHandle( SQL_HANDLE_STMT, stmt );
    }
    LOG_print( log, "[%s]\tODBC_exec.\n", TIME_get_gmt() );
    pthread_mutex_unlock( &db_exec_mutex );

    return 1;
}
