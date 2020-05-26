#ifndef POSTGRESQL_H
#define POSTGRESQL_H

#include <postgresql/libpq-fe.h>
#include "db.h"


typedef struct {
    PGconn      *connection;
    char        *id;
    int         active;
} PG_CONNECTION;


int PG_connect( PG_CONNECTION* db_connection, const char* host, const int port, const char* dbname, const char* user, const char* password );
int PG_exec( PG_CONNECTION* db_connection, const char* sql, unsigned long sql_length, DB_QUERY* dst_result, const char* const *param_values, const int params_count, const int *param_lengths, const int *param_formats, Oid *param_types );
void PG_disconnect( PG_CONNECTION* db_connection );

#endif
