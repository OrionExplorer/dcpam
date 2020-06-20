#ifndef SQLITE_H
#define SQLITE_H

#include "../third-party/sqlite3.h"
#include "db.h"


typedef struct {
    sqlite3 	*connection;
    char        *id;
    int         active;
} SQLITE_CONNECTION;


int SQLITE_connect( SQLITE_CONNECTION* db_connection, const char* filename );
int SQLITE_exec( SQLITE_CONNECTION* db_connection, const char* sql, size_t sql_length, DB_QUERY* dst_result, const char* const *param_values, const int params_count, const int *param_lengths, const int *param_formats );
void SQLITE_disconnect( SQLITE_CONNECTION* db_connection );

#endif
