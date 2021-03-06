/* Copyright (C) 2020-2021 Marcin Kelar

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335 USA */

#ifndef DB_SYSTEM_SCHEMA_H
#define DB_SYSTEM_SCHEMA_H

#include "../../db/postgresql.h"
#include "../../db/mysql.h"
#include "../../db/mariadb.h"
#include "../../db/odbc.h"
#include "../../db/oracle.h"
#include "../../db/sqlite.h"
#include "../../db/mongodb.h"
#include "../../db/ldap.h"
#include "../../file/csv.h"
#include "../../file/json.h"
#include "etl_schema.h"
#include "../network/http.h"

#define MAX_SYSTEM_QUERIES      32

typedef union {
    PG_CONNECTION               pgsql_conn;
    MYSQL_CONNECTION            mysql_conn;
    MARIADB_CONNECTION          mariadb_conn;
    ODBC_CONNECTION             odbc_conn;
    ORACLE_CONNECTION           oracle_conn;
    SQLITE_CONNECTION           sqlite_conn;
    MONGODB_CONNECTION          mongodb_conn;
    LDAP_CONNECTION             ldap_conn;
} DB_CONN;


typedef enum {
    D_POSTGRESQL = 1,
    D_MYSQL,
    D_MARIADB,
    D_ODBC,
    D_ORACLE,
    D_SQLITE,
    D_MONGODB,
    D_LDAP
} DB_DRIVER;

typedef enum {
    FFT_CSV = 1,
    FFT_JSON,
    FFT_TSV,
    FFT_PSV,
    FFT_XML
} FLAT_FILE_TYPE;

typedef struct DATABASE_SYSTEM_DB {
    char                        *ip;
    int                         port;
    DB_DRIVER                   driver;
    char                        *user;
    char                        *password;
    char                        *connection_string;
    char                        *db;
    DB_CONN                     db_conn;
    char                        *name;
} DATABASE_SYSTEM_DB;

typedef struct DATABASE_SYSTEM_QUERY {
    char                        *name;
    DB_SYSTEM_MODE              mode;
    DB_SYSTEM_ETL               etl_config;
} DATABASE_SYSTEM_QUERY;

typedef struct DATABASE_SYSTEM_FLAT_FILE {
    char                        *preprocessor;
    char                        *name;
    char                        **columns;
    int                         columns_len;
    char                        *load_sql;
    char                        delimiter[1];
    CSV_FILE                    *csv_file;
    JSON_FILE                   *json_file;
    FLAT_FILE_TYPE              type;
    HTTP_DATA                   http;
} DATABASE_SYSTEM_FLAT_FILE;

typedef struct DATABASE_SYSTEM {
    char                        *name;
    DATABASE_SYSTEM_DB          system_db;
    DATABASE_SYSTEM_DB          dcpam_db;
    DATABASE_SYSTEM_DB          *staging_db;
    DATABASE_SYSTEM_QUERY       queries[ MAX_SYSTEM_QUERIES ];
    int                         queries_len;
    DATABASE_SYSTEM_FLAT_FILE   *flat_file;
    int                         failure;
} DATABASE_SYSTEM;

#endif
