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

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "log.h"
#include "../core/network/client.h"

#ifdef _WIN32
#define getcwd _getcwd
#endif // _WIN32

int FILE_download( const char* src, const char* dst, HTTP_DATA* http_data, const char* w_mode, LOG_OBJECT* log );
FILE* FILE_open( const char *filename, HTTP_DATA* http_data, const char *r_mode, const char *w_mode, LOG_OBJECT *log );
char* get_app_path( void );
short directory_exists( const char *path );
char* file_get_content( const char *filename );


#endif
