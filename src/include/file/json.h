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

#ifndef JSON_H
#define JSON_H

#include "../shared.h"
#include "../utils/log.h"
#include "../core/network/client.h"

typedef struct {
    char            label[ MAX_COLUMN_NAME_LEN ];
    char            *value;
    unsigned long   size;
    char            type[ 16 ];
} JSON_FIELD;

typedef struct {
    JSON_FIELD       *fields;
    int             field_count;
} JSON_RECORD;

typedef struct {
    int             row_count;
    int             field_count;
    JSON_RECORD      *records;
    char            *file_name;
    int             loaded;
} JSON_FILE;

typedef void ( *jlc )( JSON_RECORD*, void* data_ptr1, void* data_ptr2, LOG_OBJECT* log ); /* JSON Load Callback */


int JSON_FILE_load( JSON_FILE* dst, const char* filename, HTTP_DATA* http_data, jlc* json_load_callback, void *data_ptr1, void *data_ptr2, LOG_OBJECT* log );


#endif
