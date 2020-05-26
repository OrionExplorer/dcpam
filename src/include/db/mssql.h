#ifndef MSSQL_H
#define MSSQL_H

#include <sqltypes.h>
#include <sql.h>
#include <sqlext.h>
#include "db.h"


typedef struct {
    SQLHANDLE   sqlenvhandle;
    SQLHANDLE   connection;
    char        *id;
    int         active;
} MSSQL_CONNECTION;


int MSSQL_connect( MSSQL_CONNECTION* db_connection, const char* host, const int port, const char* dbname, const char* user, const char* password );
int  MSSQL_exec( MSSQL_CONNECTION* db_connection, const char* sql, unsigned long sql_length, DB_QUERY* dst_result, const char* const *param_values, const int params_count, const int *param_lengths, const int *param_formats, const char* const *param_types );
void MSSQL_disconnect( MSSQL_CONNECTION* db_connection );

#endif
