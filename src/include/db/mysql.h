#ifndef MYSQL_H
#define MYSQL_H

#ifdef _WIN32
    #include <mysql.h>
#else
    #include <mysql/mysql.h>
#endif
#include "db.h"


typedef struct {
    MYSQL       *connection;
    char        *id;
    int         active;
} MYSQL_CONNECTION;


int MYSQL_connect( MYSQL_CONNECTION* db_connection, const char* host, const int port, const char* dbname, const char* user, const char* password, const char* connection_string );
int MYSQL_exec( MYSQL_CONNECTION* db_connection, const char* sql, size_t sql_length, DB_QUERY* dst_result, const char* const *param_values, const int params_count, const int *param_lengths, const int *param_formats, const char* const *param_types );
void MYSQL_disconnect( MYSQL_CONNECTION* db_connection );

#endif
