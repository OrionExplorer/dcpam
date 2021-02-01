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

#ifndef SQLITE_H
#define SQLITE_H

#include "../third-party/sqlite3.h"
#include "db.h"


typedef struct {
    sqlite3 	*connection;
    char        *id;
    int         active;
} SQLITE_CONNECTION;


int SQLITE_connect(
    SQLITE_CONNECTION* db_connection,
    const char* filename,
    const char* name,
    LOG_OBJECT* log
);

int SQLITE_exec(
    SQLITE_CONNECTION* db_connection,
    const char* sql,
    size_t sql_length,
    DB_QUERY* dst_result,
    const char* const *param_values,
    const int params_count,
    const int *param_lengths,
    const int *param_formats,
    qec* query_exec_callback,
    void* data_ptr1,
    void* data_ptr2,
    LOG_OBJECT* log
);

void SQLITE_disconnect( SQLITE_CONNECTION* db_connection, LOG_OBJECT* log );

#endif
