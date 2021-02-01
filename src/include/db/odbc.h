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

#ifndef ODBC_H
#define ODBC_H

#ifdef _WIN32
#include <Windows.h>
#endif // _WIN32

#include <sql.h>
#include <sqltypes.h>
#include <sqlext.h>
#include "db.h"


typedef struct {
    SQLHENV     sqlenvhandle;
    SQLHDBC     connection;
    char        *id;
    int         active;
} ODBC_CONNECTION;


int ODBC_connect(
    ODBC_CONNECTION* db_connection,
    const char* host,
    const int port,
    const char* dbname,
    const char* user,
    const char* password,
    const char* connection_string,
    const char* name,
    LOG_OBJECT* log
);

int  ODBC_exec(
    ODBC_CONNECTION* db_connection,
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

void ODBC_disconnect( ODBC_CONNECTION* db_connection, LOG_OBJECT* log );

#endif
