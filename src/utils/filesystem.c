#include "../include/shared.h"
#include "../include/utils/filesystem.h"
#include "../include/utils/memory.h"
#include "../include/utils/strings.h"
#include "../include/core/network/http.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifndef _WIN32
#include <unistd.h>
#endif

FILE* FILE_open( const char *filename, const char *r_mode, const char *w_mode, LOG_OBJECT *log ) {

    if( filename == NULL ) {
        LOG_print( log, "[%s] FILE_open fatal error: filename argument is missing.\n", TIME_get_gmt() );
        return NULL;
    }

    /* HTTP protocol */
    if( strstr( filename, "http://" ) ) {
        LOG_print( log, "[%s] FILE_open error: HTTP protocol is not yet supported.\n", TIME_get_gmt() );

        char    host[ 100 ];
        int     port = 80;
        char    path[ 1024 ];

        if( sscanf( filename, "http://%99[^:]:%99d/%1023[^\n]", host, &port, path ) != 3 ) {
            LOG_print( log, "[%s] FILE_open error: URL must contain host, port number and path.\n", TIME_get_gmt() );
            return NULL;
        }

        /* Download file */
        HTTP_CLIENT *http_c = SAFEMALLOC( sizeof( HTTP_CLIENT ), __FILE__, __LINE__ );
        size_t f_content_len = 0;
        char *f_content = HTTP_CLIENT_get_content( http_c, host, path, port, &f_content_len, log );

        if( f_content ) {
            /* Create temporary file */
            char *tmp_file_name = mkrndstr( 16 );
            LOG_print( log, "[%s] Temporary file name: %s\n", TIME_get_gmt(), tmp_file_name );
            FILE *tmp_f = fopen( tmp_file_name, w_mode );
            size_t data_saved = fwrite( f_content, f_content_len, 1, tmp_f );
            if( data_saved != 1 ) {
                LOG_print( log, "[%s] FILE_open fatal error: unable to save requested file.\n", TIME_get_gmt() );
                free( http_c ); http_c = NULL;
                free( f_content ); f_content = NULL;
                return NULL;
            }

            free( http_c ); http_c = NULL;
            free( f_content ); f_content = NULL;

            if( fclose( tmp_f ) == 0 ) {
                return fopen( tmp_file_name, r_mode );
            } else {
                return NULL;
            }

        } else {
            LOG_print( log, "[%s] FILE_open fatal error: requested URL \"%s\" returned no data.\n", TIME_get_gmt(), filename );
            free( http_c ); http_c = NULL;
            return NULL;
        }

        free( http_c ); http_c = NULL;

        return NULL;
    }

    LOG_print( log, "[%s] FILE_open( %s )...\n", TIME_get_gmt(), filename );

    /* Local file */
    return fopen( filename, r_mode );
}


char* get_app_path( void ) {
    static char		buf[ MAX_PATH_LENGTH ];

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
    FILE        *fp = NULL;
    char        *content = NULL;

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
