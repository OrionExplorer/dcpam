#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "log.h"

#ifdef _WIN32
#define getcwd _getcwd
#endif // _WIN32

FILE* FILE_open( const char *filename, const char *r_mode, const char *w_mode, LOG_OBJECT *log );
char* get_app_path( void );
short directory_exists( const char *path );
char* file_get_content( const char *filename );


#endif
