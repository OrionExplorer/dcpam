#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#ifdef _WIN32
#define getcwd _getcwd
#endif // _WIN32


char* get_app_path( void );
short directory_exists( const char *path );
char* file_get_content( const char *filename );


#endif
