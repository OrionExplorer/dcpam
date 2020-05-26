#ifndef MYSQL_H
#define MYSQL_H

#include <mysql/mysql.h>
#include "db.h"


typedef struct {
    MYSQL       *connection;
    char        *id;
    int         active;
} MYSQL_CONNECTION;


int MYSQL_connect( MYSQL_CONNECTION* db_connection, const char* host, const int port, const char* dbname, const char* user, const char* password );
int MYSQL_exec( MYSQL_CONNECTION* db_connection, const char* sql, unsigned long sql_length, DB_QUERY* dst_result, const char* const *param_values, const int params_count, const int *param_lengths, const int *param_formats, const char* const *param_types );
void MYSQL_disconnect( MYSQL_CONNECTION* db_connection );

#endif
