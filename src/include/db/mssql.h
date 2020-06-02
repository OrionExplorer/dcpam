#ifndef MSSQL_H
#define MSSQL_H

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
} MSSQL_CONNECTION;


int MSSQL_connect( MSSQL_CONNECTION* db_connection, const char* host, const int port, const char* dbname, const char* user, const char* password, const char* connection_string );
int  MSSQL_exec( MSSQL_CONNECTION* db_connection, const char* sql, unsigned long sql_length, DB_QUERY* dst_result, const char* const *param_values, const int params_count, const int *param_lengths, const int *param_formats, const char* const *param_types );
void MSSQL_disconnect( MSSQL_CONNECTION* db_connection );

#endif
