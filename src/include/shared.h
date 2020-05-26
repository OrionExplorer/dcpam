#ifndef SHARED_H
#define SHARED_H

#include "portable.h"
#include <stdio.h>
#include <time.h>

#define MAX_BUFFER                          65535
#define MAX_BUFFER_CHAR                     65535*sizeof( char )
#define UPLOAD_BUFFER                       16384
#define UPLOAD_BUFFER_CHAR                  16384*sizeof( char )
#define LOG_BUFFER                          128
#define LOG_BUFFER_CHAR                     128*sizeof( char )
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

#define MAX_PATH_LENGTH                     1024
#define MAX_PATH_LENGTH_CHAR                1024*sizeof( char )
#define MAX_CLIENTS                         FD_SETSIZE
#define DEFAULT_PORT                        1212

#define LOGS_PATH                           ""SLASH
#define RFC1123FMT                          "%a, %d %b %Y %H:%M:%S GMT"
#define DATETIME                            "%d-%m-%Y %H:%M:%S"

#define WORKER_WATCHER_SLEEP                5000
#define TICKET_WATCHER_SLEEP                1000
#define SAVE_DATA_INTERVAL                  10000

#define TRUE                                1
#define FALSE                               0



typedef enum ACTION_TYPE {
    AT_READ = 1,
    AT_WRITE
} ACTION_TYPE;


extern char                     app_path[ MAX_PATH_LENGTH ];



#endif
