#ifndef DB_SYSTEM_SCHEMA_H
#define DB_SYSTEM_SCHEMA_H

#include "../../db/postgresql.h"
#include "../../db/mysql.h"
#include "../../db/mssql.h"

typedef union {
    PG_CONNECTION           pgsql_conn;
    MYSQL_CONNECTION        mysql_conn;
    MSSQL_CONNECTION        mssql_conn;
} DB_CONN;

typedef enum {
    D_POSTGRESQL = 1,
    D_MYSQL,
    D_MSSQL
} DB_DRIVER;

typedef struct DATABASE_SYSTEM_DB {
    char                    *ip;
    int                     port;
    DB_DRIVER               driver;
    char                    *user;
    char                    *password;
    char                    *db;
    DB_CONN                 db_conn;
} DATABASE_SYSTEM_DB;

typedef struct DATABASE_SYSTEM_QUERY {
    char                    *name;
    DB_SYSTEM_CDC           change_data_capture;
    char                    data_types[ MAX_CDC_COLUMNS ][ 32 ];
    int                     data_types_len;
} DATABASE_SYSTEM_QUERY;

typedef struct DATABASE_SYSTEM {
    char                    *name;
    DATABASE_SYSTEM_DB      DB;
    DATABASE_SYSTEM_QUERY   queries[ MAX_SYSTEM_QUERIES ];
    int                     queries_len;
} DATABASE_SYSTEM;

#endif
