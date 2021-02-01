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

#ifndef SHARED_H
#define SHARED_H

#include "portable.h"
#include <stdio.h>
#include <time.h>

#define MAX_BUFFER                          65535
#define MAX_BUFFER_CHAR                     65535*sizeof( char )
#define UPLOAD_BUFFER                       16384
#define UPLOAD_BUFFER_CHAR                  16384*sizeof( char )
#define LOG_BUFFER                          512
#define LOG_BUFFER_CHAR                     512*sizeof( char )
#define LARGE_BUFF_SIZE                     8192
#define LARGE_BUFF_SIZE_CHAR                8192*sizeof( char )
#define BIG_BUFF_SIZE                       2048
#define BIG_BUFF_SIZE_CHAR                  2048*sizeof( char )
#define MEDIUM_BUFF_SIZE                    1024
#define MEDIUM_BUFF_SIZE_CHAR               1024*sizeof( char )
#define STD_BUFF_SIZE                       256
#define STD_BUFF_SIZE_CHAR                  256*sizeof( char )
#define TIME_BUFF_SIZE                      30
#define TIME_BUFF_SIZE_CHAR                 30*sizeof( char )
#define SMALL_BUFF_SIZE                     32
#define SMALL_BUFF_SIZE_CHAR                32*sizeof( char )
#define TINY_BUFF_SIZE                      17
#define TINY_BUFF_SIZE_CHAR                 16*sizeof( char )
#define PROTO_BUFF_SIZE                     10
#define PROTO_BUFF_SIZE_CHAR                10*sizeof( char )
#define MICRO_BUFF_SIZE                     8
#define MICRO_BUFF_SIZE_CHAR                8*sizeof( char )
#define EXT_LEN                             8
#define EXT_LEN_CHAR                        8*sizeof( char )

#define MAX_PATH_LENGTH                     2048
#define MAX_PATH_LENGTH_CHAR                2048*sizeof( char )
#define MAX_CLIENTS                         FD_SETSIZE
#define DEFAULT_PORT                        9091
#define MAX_COLUMNS							64
#define MAX_COLUMN_NAME_LEN					256

#define LOGS_PATH                           ""SLASH
#define RFC1123FMT                          "%a, %d %b %Y %H:%M:%S GMT"
#define DATETIME                            "%d-%m-%Y %H:%M:%S"

#define WORKER_WATCHER_SLEEP                5000
#define TICKET_WATCHER_SLEEP                1000
#define SAVE_DATA_INTERVAL                  10000

#define TRUE                                1
#define FALSE                               0

#define MAX_DCPAM_DATA_ITEMS                5
#define MAX_DCPAM_DATA_ACTIONS              8

#define MAX_DATA_SYSTEMS                    4


typedef struct DCPAM_ALLOWED_HOST {
    char* ip;
    char* api_key;
} DCPAM_ALLOWED_HOST;

typedef enum ACTION_TYPE {
    AT_READ = 1,
    AT_WRITE
} ACTION_TYPE;

#endif
