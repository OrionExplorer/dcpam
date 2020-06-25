#ifndef POSTGRESQL_H
#define POSTGRESQL_H

#ifdef _WIN32
    #include <libpq-fe.h>
#else
    #include <postgresql/libpq-fe.h>
#endif
#include "db.h"


typedef struct {
    PGconn      *connection;
    char        *id;
    int         active;
} PG_CONNECTION;


int PG_connect(
    PG_CONNECTION* db_connection,
    const char* host,
    const int port,
    const char* dbname,
    const char* user,
    const char* password,
    const char* connection_string
);

int PG_exec(
    PG_CONNECTION* db_connection,
    const char* sql,
    size_t sql_length,
    DB_QUERY* dst_result,
    const char* const *param_values,
    const int params_count,
    const int *param_lengths,
    const int *param_formats,
    Oid *param_types,
    qec *query_exec_callback,
    void *data_ptr1,
    void *data_ptr2
);
void PG_disconnect( PG_CONNECTION* db_connection );

#endif
