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

#ifndef DB_H
#define DB_H

#include "../shared.h"
#include "../utils/log.h"

typedef enum {
    DQT_SELECT,
    DQT_INSERT,
    DQT_UPDATE,
    DQT_DELETE,
    DQT_ALTER,
    DQT_DROP,
    DQT_CREATE,
    DQT_USE,
    DQT_SHOW,
    DQT_UNKNOWN
} DB_QUERY_TYPE;

typedef struct {
    char            label[ MAX_COLUMN_NAME_LEN ];
    char            *value;
    unsigned long   size;
    char            type[ 16 ];
} DB_FIELD;

typedef struct {
    DB_FIELD        *fields;
    int             field_count;
} DB_RECORD;

typedef struct {
    int             row_count;
    int             field_count;
    DB_RECORD       *records;
    char            *sql;
} DB_QUERY;

typedef void ( *qec )( DB_RECORD*, void *data_ptr1, void *data_ptr2, LOG_OBJECT *log ); /* Query Exec Callback */


void            DB_QUERY_init( DB_QUERY *db_query );
void            DB_QUERY_free( DB_QUERY *db_query );
void            DB_QUERY_record_free( DB_RECORD *record );
int             DB_QUERY_format( const char *src, char **dst, size_t *dst_length, const char* const* param_values, const int params_count, const int *param_lengths, LOG_OBJECT *log );
DB_QUERY_TYPE   DB_QUERY_get_type( const char *sql );
int             DB_QUERY_internal_replace_str_( char* src, const char* search, const char*replace, size_t *dst_len );

#endif
