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

#ifndef STRINGS_H
#define STRINGS_H

#include <string.h>

char* mkrndstr( size_t len );
char* rtrim( char* string, char junk );
int strpos( char *haystack, char *needle );
size_t strlcat( char *dst, const char *src, size_t size );
size_t strlcpy( char *dst, const char *src, size_t size );

#endif
