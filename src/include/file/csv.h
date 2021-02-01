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

#ifndef CSV_H
#define CSV_H

#include "../shared.h"
#include "../utils/log.h"
#include "../core/network/client.h"

typedef struct {
    char            label[ MAX_COLUMN_NAME_LEN ];
    char            *value;
    unsigned long   size;
    char            type[ 16 ];
} CSV_FIELD;

typedef struct {
    CSV_FIELD       *fields;
    int             field_count;
} CSV_RECORD;

typedef struct {
    int             row_count;
    int             field_count;
    CSV_RECORD      *records;
    char            *file_name;
    int             loaded;
    char            delimiter[1];
} CSV_FILE;

typedef void ( *clc )( CSV_RECORD*, void* data_ptr1, void* data_ptr2, LOG_OBJECT* log ); /* CSV Load Callback */


int CSV_FILE_load( CSV_FILE* dst, const char* filename, HTTP_DATA* http_data, clc* csv_load_callback, void *data_ptr1, void *data_ptr2, LOG_OBJECT* log );


#endif
