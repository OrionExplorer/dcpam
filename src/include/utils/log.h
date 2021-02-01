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

#ifndef LOG_H
#define LOG_H

#include "../shared.h"

typedef struct LOG_OBJECT {
    char        filename[ MAX_PATH_LENGTH ];
    char        *buffer;
} LOG_OBJECT;

void LOG_print( LOG_OBJECT *log, char *fmt, ... );
void LOG_save( LOG_OBJECT *log, const char *app_path );
void LOG_init( LOG_OBJECT *log, const char *prefix, const size_t buffer_size );
void LOG_free( LOG_OBJECT *log );

#endif
