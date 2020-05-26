#include "../include/db/mssql.h"
#include "../include/utils/time.h"
#include "../include/utils/log.h"
#include "../include/utils/memory.h"
#include <stdlib.h>
#include <string.h>


void MSSQL_disconnect( MSSQL_CONNECTION* db_connection ) {
    printf( "[%s]\tMSSQL_disconnect( <'%s'> ).\n", TIME_get_gmt(), db_connection->id ? db_connection->id : "" );

    if( db_connection->connection ) {
        SQLDisconnect( db_connection->connection );
        SQLFreeHandle( SQL_HANDLE_DBC, db_connection->connection );
        SQLFreeHandle( SQL_HANDLE_ENV, db_connection->sqlenvhandle );
        db_connection->connection = NULL;
    }

    if( db_connection->id ) {
        free( db_connection->id );
        db_connection->id = NULL;
    }

    printf( "[%s]\tMSSQL_disconnect.\n", TIME_get_gmt() );
}


int MSSQL_connect( MSSQL_CONNECTION* db_connection, const char* host, const int port, const char* dbname, const char* user, const char* password ) {
    char        conn_str[512];
    SQLWCHAR        retconstring[1024];
    SQLWCHAR        sqlstate[1024];
    SQLWCHAR        message[1024];
    SQLWCHAR        db_connection_str[1024];
    SQLRETURN   retcode;


    db_connection->id = ( char * )SAFECALLOC( strlen( user ) + strlen( host ) + strlen( dbname ) + 7, sizeof( char ) );
    sprintf( conn_str, "dbname=%s host=%s port=%d user=%s password=%s", dbname, host, port, user, password );
    sprintf( db_connection->id, "%s@%s[db=%s]", user, host, dbname );
    //printf( "ID: %s\n", db_connection->id );

    if( SQL_SUCCESS != SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &db_connection->sqlenvhandle ) ) {
        db_connection->active = 0;
    } else {
        db_connection->active = 1;
    }

    if( SQL_SUCCESS != SQLSetEnvAttr( db_connection->sqlenvhandle, SQL_ATTR_ODBC_VERSION, ( SQLPOINTER )SQL_OV_ODBC3, 0 ) ) {
        db_connection->active = 0;
    } else {
        db_connection->active = 1;
    }

    if( SQL_SUCCESS != SQLAllocHandle( SQL_HANDLE_DBC, db_connection->sqlenvhandle, &db_connection->connection ) ) {
        db_connection->active = 0;
    } else {
        db_connection->active = 1;
    }

    if( db_connection->active == 0 ) {
        if( SQL_SUCCESS == SQLGetDiagRec( SQL_HANDLE_DBC, db_connection->connection, 1, sqlstate, NULL, message, 1024, NULL ) ) {
            free( db_connection->id ); db_connection->id = NULL;
            printf( "error. Message: \"%s\". SQLSTATE: \"%s\".\n", ( char * )message, ( char* )sqlstate );
            return 0;
        }
    }

    swprintf( db_connection_str, 1024, L"DRIVER={SQL Server};SERVER=%s, %d;DATABASE=%s;UID=%s;PWD=%s;", host, port, dbname, user, password );
    //swprintf( db_connection_str, 1024, L"DRIVER={SQL Server};SERVER=%hs, 1433;DATABASE=%hs;UID=%hs;PWD=%hs;charset=cp852", host, dbname, user, password );
    //swprintf( db_connection_str, 1024, L"DRIVER={SQL Server};SERVER=%hs, 1433;DATABASE=%hs;UID=%hs;PWD=%hs;charset=latin1", host, dbname, user, password );

    switch( SQLDriverConnect( db_connection->connection, NULL,
        /*( SQLWCHAR* )L"DRIVER={SQL Server};SERVER=localhost, 1433;DATABASE=servicedesk;UID=otrs;PWD=otrs321;charset=cp1250",*/
        db_connection_str,
        SQL_NTS, retconstring, 1024, NULL, SQL_DRIVER_NOPROMPT ) ) {
    case SQL_SUCCESS_WITH_INFO:
        db_connection->active = 1;
        break;
    case SQL_INVALID_HANDLE:
    case SQL_ERROR:
        db_connection->active = 0;
        retcode = -1;
        break;
    default:
        break;
    }

    if( db_connection->active == 0 ) {
        if( SQL_SUCCESS == SQLGetDiagRec( SQL_HANDLE_DBC, db_connection->connection, 1, sqlstate, NULL, message, 1024, NULL ) ) {
            free( db_connection->id ); db_connection->id = NULL;
            printf( "error. Message: \"%s\". SQLSTATE: \"%s\".\n", ( char * )message, ( char * )sqlstate );
            return 0;
        }
    }
    db_connection->active = 1;

    return 1;
}


int MSSQL_exec(
    MSSQL_CONNECTION    *db_connection,
    const char          *sql,
    unsigned long       sql_length,
    DB_QUERY            *dst_result,
    const char* const   *param_values,
    const int           params_count,
    const int           *param_lengths,
    const int           *param_formats,
    const char* const   *param_types 
) {
    /*SQLHANDLE   stmt;
    SQLUSMALLINT i;
    SQLLEN      indicator;
    SQLRETURN   ret;
    SQLWCHAR    sqlstate[1024];
    SQLWCHAR    message[1024];
    int         row_count = 0, field_count = 0;
    char        name[128];
    char        columns[256][64];
    DB_RECORD   *tmp_records = NULL;
    wchar_t     *w_sql = NULL;
    int         sql_len = ( int )strlen( sql );
    wchar_t     *w_table = NULL;
    int         table_len = ( int )strlen( table );*/

    if( db_connection->active == 0 || db_connection->id == NULL ) {
        return 0;
    }

    /*LOG_print( "\n[%s]\tMSSQL_exec( [%d] <'%s'>, \"%s\", ... ).\n", TIME_get_gmt(), sql_len, db_connection->id, sql );

    dst_result->sql = ( char* )SAFECALLOC( sql_len + 1, sizeof( char ) );

    w_sql = ( wchar_t* )SAFECALLOC( sql_len + 1, sizeof( wchar_t ) );
    strncpy( dst_result->sql, sql, sql_len );
    swprintf( w_sql, sql_len + 1, L"%hs", sql );

    w_table = ( wchar_t* )SAFECALLOC( table_len + 1, sizeof( wchar_t ) );
    swprintf( w_table, table_len + 1, L"%hs", table );
    memset( name, '\0', 128 );

    if( db_connection->connection ) {
        if( SQL_SUCCESS != SQLAllocHandle( SQL_HANDLE_STMT, db_connection->connection, &stmt ) ) {
            LOG_print( "error. SQLAllocHandle(SQL_HANDLE_STMT, ... ) failed.\n" );
            LOG_print( "[%s][ERROR#3]\tMSSQL_exec.\n", TIME_get_gmt() );
            return 0;
        }

        SQLColumns( stmt, NULL, 0, NULL, 0, ( SQLWCHAR* )w_table, SQL_NTS, NULL, 0 );
        SQLBindCol( stmt, 4, SQL_C_CHAR, name, 128, &indicator );

        while( SQLFetch( stmt ) == SQL_SUCCESS ) {
            strncpy( columns[field_count], name, 64 );
            field_count++;
        }

        dst_result->field_count = field_count;
        SQLFreeHandle( SQL_HANDLE_STMT, stmt );
        if( SQL_SUCCESS != SQLAllocHandle( SQL_HANDLE_STMT, db_connection->connection, &stmt ) ) {
            LOG_print( "error. SQLAllocHandle(SQL_HANDLE_STMT, ... ) failed.\n" );
            LOG_print( "[%s][ERROR#2]\tMSSQL_exec.\n", TIME_get_gmt() );
            return 0;
        }
        ret = SQLExecDirect( stmt, ( SQLWCHAR* )w_sql, SQL_NTS );
        if( ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO ) {
            if( SQL_SUCCESS == SQLGetDiagRec( SQL_HANDLE_STMT, stmt, 1, sqlstate, NULL, message, 1024, NULL ) ) {
                wprintf( L"\nERROR. Message: \"%s\". SQLSTATE: \"%s\".\n", message, sqlstate );
                wprintf( L"\nSQL:\n=========\n%s\n=========\n", w_sql );
                LOG_print("ERROR. Message: \"%s\"", message );
                return 0;
            }
            LOG_print( "[%s][ERROR#1]\tMSSQL_exec.\n", TIME_get_gmt() );
            return 0;
        }

        while( SQLFetch( stmt ) == SQL_SUCCESS ) {
            dst_result->records = ( DB_RECORD* )realloc( tmp_records, ( row_count + 1 ) * sizeof( DB_RECORD ) );
            if( dst_result->records != NULL ) {
                tmp_records = dst_result->records;
                dst_result->records[row_count].fields = ( DB_FIELD* )SAFEMALLOC( field_count * sizeof( DB_FIELD ) );
                for( i = 0; i < field_count; i++ ) {
                    ret = SQLGetData( stmt, i + 1, SQL_C_CHAR, name, 128, NULL );
                    strncpy( dst_result->records[row_count].fields[i].label, columns[i], 64 );
                    if( ret == SQL_SUCCESS ) {
                        dst_result->records[row_count].fields[i].value = ( char* )SAFECALLOC( strlen( name ) + 1, sizeof( char ) );
                        strncpy( dst_result->records[row_count].fields[i].value, name, strlen( name ) );
                    } else {
                        dst_result->records[row_count].fields[i].value = ( char* )SAFECALLOC( 1, sizeof( char ) );
                        dst_result->records[row_count].fields[i].value[0] = 0;
                    }
                }
            }
            row_count++;
        }
        tmp_records = NULL;
        dst_result->row_count = row_count;
        SQLFreeHandle( SQL_HANDLE_STMT, stmt );
    }*/
    LOG_print( "[%s]\tMSSQL_exec.\n", TIME_get_gmt() );

    return 1;
}
