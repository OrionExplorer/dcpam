#ifndef MARIADB_H
#define MARIADB_H

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
} MARIADB_CONNECTION;


int MARIADB_connect(
    MARIADB_CONNECTION* db_connection,
    const char* host,
    const int port,
    const char* dbname,
    const char* user,
    const char* password,
    const char* connection_string,
    const char* name,
    LOG_OBJECT* log
);

int MARIADB_exec(
    MARIADB_CONNECTION* db_connection,
    const char* sql,
    size_t sql_length,
    DB_QUERY* dst_result,
    const char* const *param_values,
    const int params_count,
    const int *param_lengths,
    const int *param_formats,
    const char* const *param_types,
    qec* query_exec_callback,
    void* data_ptr1,
    void* data_ptr2,
    LOG_OBJECT* log
);

void MARIADB_disconnect( MARIADB_CONNECTION* db_connection, LOG_OBJECT* log );

#endif
