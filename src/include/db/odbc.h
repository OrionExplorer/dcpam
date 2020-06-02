#ifndef ODBC_H
#define ODBC_H

#ifdef _WIN32
#include <Windows.h>
#endif // _WIN32

#include <sql.h>
#include <sqltypes.h>
#include <sqlext.h>
#include "db.h"


typedef struct {
    SQLHENV   sqlenvhandle;
    SQLHDBC     connection;
    char        *id;
    int         active;
} ODBC_CONNECTION;


int ODBC_connect( ODBC_CONNECTION* db_connection, const char* host, const int port, const char* dbname, const char* user, const char* password, const char* connection_string );
int  ODBC_exec( ODBC_CONNECTION* db_connection, const char* sql, unsigned long sql_length, DB_QUERY* dst_result, const char* const *param_values, const int params_count, const int *param_lengths, const int *param_formats, const char* const *param_types );
void ODBC_disconnect( ODBC_CONNECTION* db_connection );

#endif
