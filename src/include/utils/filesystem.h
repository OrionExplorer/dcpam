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
