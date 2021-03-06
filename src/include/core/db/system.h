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

#ifndef DB_SYSTEM_H
#define DB_SYSTEM_H

#include "../../shared.h"
#include "../../core/schema.h"

void DATABASE_SYSTEM_add(
            const char              *name,
            DATABASE_SYSTEM_DB      *source_db,
            DATABASE_SYSTEM_DB      *dcpam_db,
            DATABASE_SYSTEM_DB      *staging_db,
            DATABASE_SYSTEM_FLAT_FILE* flat_file,
            DATABASE_SYSTEM_QUERY   queries[ MAX_SYSTEM_QUERIES ],
            const int               queries_len,
            short                   verbose,
            LOG_OBJECT              *log
);


void DATABASE_SYSTEM_DB_add(
            const char              *ip,
            const int               port,
            const DB_DRIVER         driver,
            const char              *user,
            const char              *password,
            const char              *db,
            const char              *connection_string,
            DATABASE_SYSTEM_DB      *dst,
            const char              *name,
            short                   verbose,
            LOG_OBJECT              *log
);


int DATABASE_SYSTEM_DB_init (
            DATABASE_SYSTEM_DB      *db,
            LOG_OBJECT              *log
);


void DATABASE_SYSTEM_DB_free(
            DATABASE_SYSTEM_DB      *db,
            LOG_OBJECT              *log
);

void DATABASE_SYSTEM_DB_close(
            DATABASE_SYSTEM_DB      *db,
            LOG_OBJECT              *log
);

void DATABASE_SYSTEM_close(
            DATABASE_SYSTEM         *system,
            LOG_OBJECT              *log
);


void DATABASE_SYSTEM_QUERY_add(
            const char              *name,
            DB_SYSTEM_MODE          mode,
            DB_SYSTEM_ETL           etl,
            DATABASE_SYSTEM_QUERY   *dst,
            short                   verbose,
            LOG_OBJECT              *log
);

void SYSTEM_QUERY_free(
      DATABASE_SYSTEM_QUERY         *dst
);


void SYSTEM_ETL_CONFIG_free(
      DB_SYSTEM_ETL                 *dst
);


int DB_exec(
            DATABASE_SYSTEM_DB      *db,
            const char              *sql_template,
            size_t                  sql_length,
            DB_QUERY                *dst_result,
            const char* const       *param_values,
            const int               params_count,
            const int               *param_lengths,
            const int               *param_formats,
            const char              *param_types,
            qec                     *query_exec_callback,
            void                    *data_ptr1,
            void                    *data_ptr2,
            LOG_OBJECT              *log
);

DATABASE_SYSTEM_DB *DATABASE_SYSTEM_DB_get(
            const char              *name
);

#endif
