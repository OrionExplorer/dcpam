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

#include "../core/cache.h"
#include "../utils/log.h"

typedef enum {
   SQL_COND_UNDEF = 0,
   SQL_COND_LIMIT,
   SQL_COND_WHERE
} SQL_CONDITION;

SQL_CONDITION SQL_PARSER_supported_conditions( const char *sql );
int SQL_PARSER_collect_data( D_CACHE *cache, D_SUB_CACHE *sub_cache, const char *sql, SQL_CONDITION sql_condition, LOG_OBJECT *log );
