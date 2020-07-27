#ifndef DB_SYSTEM_SCHEMA_H
#define DB_SYSTEM_SCHEMA_H

#include "../../db/postgresql.h"
#include "../../db/mysql.h"
#include "../../db/mariadb.h"
#include "../../db/odbc.h"
#include "../../db/oracle.h"
#include "../../db/sqlite.h"

typedef union {
    PG_CONNECTION           pgsql_conn;
    MYSQL_CONNECTION        mysql_conn;
    MARIADB_CONNECTION      mariadb_conn;
    ODBC_CONNECTION         odbc_conn;
    ORACLE_CONNECTION       oracle_conn;
    SQLITE_CONNECTION       sqlite_conn;
} DB_CONN;

typedef enum {
    D_POSTGRESQL = 1,
    D_MYSQL,
    D_MARIADB,
    D_ODBC,
    D_ORACLE,
    D_SQLITE
} DB_DRIVER;

typedef struct DATABASE_SYSTEM_DB {
    char                    *ip;
    int                     port;
    DB_DRIVER               driver;
    char                    *user;
    char                    *password;
    char                    *connection_string;
    char                    *db;
    DB_CONN                 db_conn;
    char                    *name;
} DATABASE_SYSTEM_DB;

typedef struct DATABASE_SYSTEM_QUERY {
    char                    *name;
    DB_SYSTEM_CDC           change_data_capture;
    char                    data_types[ MAX_CDC_COLUMNS ][ 32 ];
    int                     data_types_len;
} DATABASE_SYSTEM_QUERY;

typedef struct DATABASE_SYSTEM {
    char                    *name;
    DATABASE_SYSTEM_DB      system_db;
    DATABASE_SYSTEM_DB      dcpam_db;
    DATABASE_SYSTEM_DB      *staging_db;
    DATABASE_SYSTEM_QUERY   queries[ MAX_SYSTEM_QUERIES ];
    int                     queries_len;
} DATABASE_SYSTEM;

#endif
