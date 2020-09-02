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
