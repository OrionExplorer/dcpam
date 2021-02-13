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

#ifndef CACHE_H
#define CACHE_H

#include "../db/db.h"
#include "../core/db/system_schema.h"
#include "../utils/log.h"
#include "../utils/time.h"

typedef struct {
    DB_QUERY                *query;
    DATABASE_SYSTEM_DB      *db;
    size_t                  size;
    double                  ttl;
    long int                generate_time;
} D_CACHE;

typedef struct {
  D_CACHE                   *src;
  char                      *sql;
  size_t                    indices_len;
  long                      *indices;
  long int                  generate_time;
} D_SUB_CACHE;

int DB_CACHE_init( D_CACHE *dst, DATABASE_SYSTEM_DB *db, const char *sql, double cache_ttl, LOG_OBJECT *log );
void DB_CACHE_free( D_CACHE* dst, LOG_OBJECT *log );
void DB_CACHE_get( const char* sql, DB_QUERY** dst, D_SUB_CACHE** s_dst );
void DB_CACHE_print( D_CACHE *dst, LOG_OBJECT *log );
char* DB_CACHE_get_usage_str( void );

int DB_SUB_CACHE_init( D_CACHE *cache, D_SUB_CACHE *sub_cache, const char *sql, LOG_OBJECT *log );
void DB_SUB_CACHE_free( D_SUB_CACHE *dst, LOG_OBJECT *log );

#endif
