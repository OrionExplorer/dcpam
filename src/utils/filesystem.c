#include "../include/shared.h"
#include "../include/utils/filesystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifndef _WIN32
#include <unistd.h>
#endif


char* get_app_path( void ) {
    static char buf[MAX_PATH_LENGTH];
    if( getcwd( buf, MAX_PATH_LENGTH ) ) {
        return strncat( buf, "", MAX_PATH_LENGTH - strlen( buf ) );
    } else {
        return "";
    }
}


short directory_exists( const char *path ) {
    return chdir( path ) == 0 ? 1 : 0;
}
