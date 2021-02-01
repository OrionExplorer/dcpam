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

#ifndef MEMORY_H
#define MEMORY_H

void* safe_malloc( size_t n, const char* filename, int line );
void* safe_calloc( size_t n, size_t m, const char* filename, int line );

#define SAFEMALLOC( n, f, l )         safe_malloc( n, f, l )
#define SAFECALLOC( n, m, f, l )      safe_calloc( n, m, f, l )

#endif
