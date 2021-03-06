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

#include "../include/shared.h"
#include "../include/utils/filesystem.h"
#include "../include/utils/memory.h"
#include "../include/utils/strings.h"
#include "../include/utils/time.h"
#include "../include/core/network/http.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifndef _WIN32
#include <unistd.h>
#endif

int FILE_download( const char* src, const char* dst, HTTP_DATA* http_data, const char* w_mode, LOG_OBJECT* log ) {
    char    host[ 100 ];
    int     port = 80;
    char    path[ 1024 ];
    int     secure = 0;

    if( sscanf( src, "%*[^:]%*[:/]%99[^:]:%99d%1023s", host, &port, path ) != 3 ) {
        LOG_print( log, "[%s] FILE_download error: URL must contain host, port number and path.\n", TIME_get_gmt() );
        return 0;
    }

    if( strstr( src, "https://" ) ) {
        secure = 1;
    }

    /* Download file */
    HTTP_CLIENT* http_c = SAFEMALLOC( sizeof( HTTP_CLIENT ), __FILE__, __LINE__ );
    size_t f_content_len = 0;
    char* f_content = HTTP_CLIENT_get_content( http_c, host, path, port, secure, http_data, &f_content_len, log );

    if( f_content ) {
        FILE* tmp_f = fopen( dst, w_mode );
        if( tmp_f ) {
            size_t data_saved = fwrite( f_content, f_content_len, 1, tmp_f );
            if( data_saved != 1 ) {
                LOG_print( log, "[%s] FILE_download fatal error: unable to save requested file.\n", TIME_get_gmt() );
                fclose( tmp_f );
                free( http_c ); http_c = NULL;
                free( f_content ); f_content = NULL;
                return 0;
            }
        }

        free( http_c ); http_c = NULL;
        free( f_content ); f_content = NULL;

        if( tmp_f ) {
            return fclose( tmp_f ) == 0 ? 1 : 0;
        } else {
            return 0;
        }

    } else {
        LOG_print( log, "[%s] FILE_download fatal error: requested URL \"%s\" returned no data.\n", TIME_get_gmt(), src );
        free( http_c ); http_c = NULL;
        return 0;
    }

    free( http_c ); http_c = NULL;

    return 0;
}

FILE* FILE_open( const char *filename, HTTP_DATA* http_data, const char *r_mode, const char *w_mode, LOG_OBJECT *log ) {

    if( filename == NULL ) {
        LOG_print( log, "[%s] FILE_open fatal error: filename argument is missing.\n", TIME_get_gmt() );
        return NULL;
    }

    /* HTTP protocol */
    if( strstr( filename, "http://" ) || strstr( filename, "https://" ) ) {
        char* tmp_random_name = mkrndstr( 16 );
        /* 16 + "." + "dcpam_temp" + \0 */
        char* tmp_file_name = SAFECALLOC( 16 + 1 + 10 + 1, sizeof( char ), __FILE__, __LINE__ );
        strncpy( tmp_file_name, tmp_random_name, 16 + 1 + 10 + 1 );
        strlcat( tmp_file_name, ".dcpam_temp", 16 + 1 + 10 + 1 );
        LOG_print( log, "[%s] Temporary file name: %s.\n", TIME_get_gmt(), tmp_file_name );

        if( FILE_download( filename, tmp_file_name, http_data, w_mode, log ) == 1 ) {
            FILE *fp = fopen( tmp_file_name, r_mode );
            free( tmp_file_name ); tmp_file_name = NULL;
            free( tmp_random_name ); tmp_random_name = NULL;
            return fp;
        } else {
            free( tmp_file_name ); tmp_file_name = NULL;
            free( tmp_random_name ); tmp_random_name = NULL;
            return NULL;
        }
    }

    LOG_print( log, "[%s] FILE_open( %s )...\n", TIME_get_gmt(), filename );

    /* Local file */
    return fopen( filename, r_mode );
}


char* get_app_path( void ) {
    static char		buf[ MAX_PATH_LENGTH ];

    if( getcwd( buf, MAX_PATH_LENGTH ) ) {
        strlcat( buf, "", MAX_PATH_LENGTH - strlen( buf ) );
        return buf;
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
