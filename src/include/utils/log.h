#ifndef LOG_H
#define LOG_H

#include "../shared.h"

extern char     LOG_filename[MAX_PATH_LENGTH];

void LOG_print( char *fmt, ... );
void LOG_save( void );
void LOG_init( void );

#endif
