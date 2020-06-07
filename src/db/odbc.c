#include "../include/db/odbc.h"
#include "../include/utils/time.h"
#include "../include/utils/log.h"
#include "../include/utils/memory.h"
#include <stdlib.h>
#include <string.h>
#include <sql.h>
#include <sqlext.h>
#include <pthread.h>

static pthread_mutex_t      db_exec_mutex = PTHREAD_MUTEX_INITIALIZER;


#define CHECK_ERROR(e, s, h, t, r) {if (e!=SQL_SUCCESS && e != SQL_SUCCESS_WITH_INFO) { ODBC_get_error(s, h, t); if( r == TRUE ) { return 0; } } }

void ODBC_get_error( char* fn, SQLHANDLE handle, SQLSMALLINT type ) {
    SQLINTEGER i = 0;
    SQLINTEGER native_err;
    SQLCHAR sql_state[ 7 ];
    SQLCHAR message[ 256 ];
    SQLSMALLINT text_len;
    SQLRETURN ret;

    LOG_print( "Error: Driver reported the following error %s\n", fn );
    do
    {
        ret = SQLGetDiagRec( type, handle, ++i, sql_state, &native_err,
            message, sizeof( message ), &text_len );
        if( SQL_SUCCEEDED( ret ) ) {
            LOG_print( "%s:%ld:%ld:%s\n",
                sql_state, ( long )i, ( long )native_err, message );
        }
    } while( ret == SQL_SUCCESS );
}

void ODBC_disconnect( ODBC_CONNECTION* db_connection ) {
    printf( "[%s]\tODBC_disconnect( <'%s'> ).\n", TIME_get_gmt(), db_connection->id ? db_connection->id : "" );

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

    printf( "[%s]\tODBC_disconnect.\n", TIME_get_gmt() );
}


int ODBC_connect(
    ODBC_CONNECTION* db_connection,
    const char* host,
    const int           port,
    const char* dbname,
    const char* user,
    const char* password,
    const char* connection_string
) {
    SQLWCHAR        sqlstate[ 1024 ];
    SQLWCHAR        message[ 1024 ];
    SQLWCHAR        w_connection_str[ 1024 ];
    SQLRETURN       retcode;


    retcode = SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &db_connection->sqlenvhandle );
    CHECK_ERROR( retcode, "SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &db_connection->sqlenvhandle )", db_connection->sqlenvhandle, SQL_HANDLE_ENV, TRUE );


    retcode = SQLSetEnvAttr( db_connection->sqlenvhandle, SQL_ATTR_ODBC_VERSION, ( void* )SQL_OV_ODBC3, 0 );
    CHECK_ERROR( retcode, "SQLSetEnvAttr( db_connection->sqlenvhandle, SQL_ATTR_ODBC_VERSION, ( void* )SQL_OV_ODBC3, 0 )", db_connection->sqlenvhandle, SQL_HANDLE_ENV, TRUE );


    retcode = SQLAllocHandle( SQL_HANDLE_DBC, db_connection->sqlenvhandle, &db_connection->connection );
    CHECK_ERROR( retcode, "SQLAllocHandle( SQL_HANDLE_DBC, db_connection->sqlenvhandle, &db_connection->connection )", db_connection->connection, SQL_HANDLE_DBC, TRUE );

    if( connection_string ) {
        db_connection->id = SAFECALLOC( strlen( connection_string ) + 1, sizeof( char ), __FILE__, __LINE__ );
        if( db_connection->id ) {
            strncpy(
                db_connection->id,
                connection_string,
                strlen( connection_string )
            );
        }
    } else {
        LOG_print( "error. Message: no \"connection_string\" provided in config.json\n" );
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
    CHECK_ERROR( retcode, "SQLDriverConnect()", db_connection->connection, SQL_HANDLE_DBC, TRUE );

    db_connection->active = 1;

    LOG_print( "ok.\n" );

    return 1;
}


int ODBC_exec(
    ODBC_CONNECTION        *db_connection,
    const char             *sql,
    unsigned long          sql_length,
    DB_QUERY               *dst_result,
    const char* const      *param_values,
    const int              params_count,
    const int              *param_lengths,
    const int              *param_formats,
    const char* const      *param_types
) {
    SQLHSTMT        stmt;
    int             i, j, k, l;
    SQLLEN          indicator;
    SQLRETURN       retcode;
    SQLWCHAR        sqlstate[ 1024 ];
    SQLWCHAR        message[ 1024 ];
    int             row_count = 0, field_count = 0;
    SQLCHAR         *column_name[ MAX_COLUMNS ];
    SQLWCHAR        *column_data[ MAX_COLUMNS ];
    SQLULEN         column_data_size[ MAX_COLUMNS ];
    SQLLEN          column_data_len[ MAX_COLUMNS ];
    SQLSMALLINT     column_data_nullable[ MAX_COLUMNS ];
    SQLSMALLINT     num_columns;
    DB_RECORD       *tmp_records = NULL;

    if( db_connection->active == 0 || db_connection->id == NULL ) {
        return 0;
    }

    LOG_print( "[%s]\tODBC_exec( <'%s'>, \"%s\", ... ).\n", TIME_get_gmt(), db_connection->id, sql );
    pthread_mutex_lock( &db_exec_mutex );

    dst_result->row_count = 0;
    dst_result->field_count = 0;

    dst_result->sql = SAFECALLOC( sql_length + 1, sizeof( char ), __FILE__, __LINE__ );
    for( l = 0; l < sql_length; l++ ) {
        *( dst_result->sql + l ) = sql[ l ];
    }

    if( db_connection->connection ) {
        retcode = SQLAllocHandle( SQL_HANDLE_STMT, db_connection->connection, &stmt );
        CHECK_ERROR( retcode, "SQLAllocHandle()", stmt, SQL_HANDLE_STMT, TRUE );

        /*retcode = SQLPrepare( stmt, sql, sql_length );
        CHECK_ERROR( retcode, "SQLPrepare()", stmt, SQL_HANDLE_STMT );*/

        /* Simple query without binding parameters */
        if( param_values == NULL || params_count == 0 ) {

            for( i = 0; i < MAX_COLUMNS; i++ ) {
                column_name[ i ] = NULL;
                column_data[ i ] = NULL;
            }

            retcode = SQLPrepare( stmt, ( SQLCHAR* )sql, sql_length );
            CHECK_ERROR( retcode, "SQLPrepare()", stmt, SQL_HANDLE_STMT, TRUE );

            retcode = SQLNumResultCols( stmt, &num_columns );
            CHECK_ERROR( retcode, "SQLNumResultCols()", stmt, SQL_HANDLE_STMT, TRUE );

            for( i = 0; i < num_columns; i++ ) {
                column_name[ i ] = SAFECALLOC( MAX_COLUMN_NAME_LEN + 1, sizeof( SQLCHAR ), __FILE__, __LINE__ );
                retcode = SQLDescribeCol(
                    stmt,
                    i + 1,
                    column_name[ i ],
                    MAX_COLUMN_NAME_LEN,
                    NULL,
                    NULL,
                    &column_data_size[ i ],
                    NULL,
                    &column_data_nullable[ i ]
                );
                CHECK_ERROR( retcode, "SQLDescribeCol()", stmt, SQL_HANDLE_STMT, TRUE );

                column_data[ i ] = SAFECALLOC( column_data_size[ i ] + 1, sizeof( SQLCHAR ), __FILE__, __LINE__ );

                retcode = SQLBindCol(
                    stmt,
                    i + 1,
                    SQL_C_CHAR,
                    column_data[ i ],
                    column_data_size[ i ],
                    &column_data_len[ i ]
                );
                CHECK_ERROR( retcode, "SQLBindCol()", stmt, SQL_HANDLE_STMT, TRUE );
            }

            retcode = SQLExecute( stmt );
            CHECK_ERROR( retcode, "SQLExecute()", stmt, SQL_HANDLE_STMT, FALSE );

            if( DB_QUERY_get_type( dst_result->sql ) == DQT_SELECT ) {
                while( TRUE ) {
                    retcode = SQLFetch( stmt );

                    if( retcode == SQL_NO_DATA ) {
                        break;
                    }

                    CHECK_ERROR( retcode, "SQLFetch()", stmt, SQL_HANDLE_STMT, TRUE );

                    dst_result->records = ( DB_RECORD* )realloc( tmp_records, ( row_count + 1 ) * sizeof( DB_RECORD ) );
                    if( dst_result->records != NULL ) {
                        tmp_records = dst_result->records;
                        dst_result->records[ row_count ].fields = ( DB_FIELD* )SAFEMALLOC( num_columns * sizeof( DB_FIELD ), __FILE__, __LINE__ );
                        for( k = 0; k < num_columns; k++ ) {
                            strncpy( dst_result->records[ row_count ].fields[ k ].label, column_name[ k ], MAX_COLUMN_NAME_LEN );

                            dst_result->records[ row_count ].fields[ k ].size = ( int )column_data_len[ k ];
                            dst_result->records[ row_count ].fields[ k ].value = SAFECALLOC( ( int )column_data_len[ k ] + 1, sizeof( char ), __FILE__, __LINE__ );
                            memcpy( dst_result->records[ row_count ].fields[ k ].value, ( char* )column_data[ k ], ( int )column_data_len[ k ] );
                        }
                    }

                    row_count++;
                }

                retcode = SQLCloseCursor( stmt );
                CHECK_ERROR( retcode, "SQLCloseCursor()", stmt, SQL_HANDLE_DBC, TRUE );
                tmp_records = NULL;
                dst_result->row_count = row_count;
                dst_result->field_count = num_columns;

                for( i = 0; i < num_columns; i++ ) {
                    free( column_name[ i ] ); column_name[ i ] = NULL;
                    free( column_data[ i ] ); column_data[ i ] = NULL;
                }
            }
        } else {
            /* Query with bind parameters */
            LOG_print( "[%s] Warning: not yet implemented!\n", TIME_get_gmt() );
        }

        SQLFreeHandle( SQL_HANDLE_STMT, stmt );
    }
    LOG_print( "[%s]\tODBC_exec.\n", TIME_get_gmt() );
    pthread_mutex_unlock( &db_exec_mutex );

    return 1;
}
