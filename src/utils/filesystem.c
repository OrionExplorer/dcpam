#include "../include/shared.h"
#include "../include/utils/filesystem.h"
#include "../include/utils/memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifndef _WIN32
#include <unistd.h>
#endif


char* get_app_path( void ) {
    static char		buf[MAX_PATH_LENGTH];

    if( getcwd( buf, MAX_PATH_LENGTH ) ) {
        return strncat( buf, "", MAX_PATH_LENGTH - strlen( buf ) );
    } else {
        return "";
    }
}


short directory_exists( const char *path ) {
    return chdir( path ) == 0 ? 1 : 0;
}


char* file_get_content( const char *filename ) {
	FILE 		*fp = NULL;
	char 		*content = NULL;

	fp = fopen( filename, "rb" );
	if( fp ) {

		if( fseek( fp, 0, SEEK_END ) == 0 ) {

			long len = ftell( fp );

			if( len > 0 && fseek( fp, 0, SEEK_SET ) == 0 ) {
				content = SAFEMALLOC( ( size_t )len + sizeof( "" ), __FILE__, __LINE__ );

				if( content ) {
					size_t read_content = fread( content, sizeof( char ), ( size_t )len, fp );

					if( ( long )read_content == len ) {
						content[ read_content ] = '\0';
					} else {
						free( content ); content = NULL;
					}
				}
			}
		}

		fclose( fp );
	}

	return content;
}
