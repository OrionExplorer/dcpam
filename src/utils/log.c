#include "../include/shared.h"
#include "../include/utils/log.h"
#include "../include/utils/time.h"
#include "../include/utils/filesystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>

static char                 log_object[LOG_BUFFER + STD_BUFF_SIZE];
char                        LOG_filename[MAX_PATH_LENGTH];
static pthread_mutex_t      printf_mutex = PTHREAD_MUTEX_INITIALIZER;

static void LOG_prepare( void ) {

    snprintf( LOG_filename, MAX_PATH_LENGTH, "%s%slog.txt", app_path, LOGS_PATH );
    LOG_print( "\n[%s] Session start.\n", TIME_get_gmt() );

}


static void LOG_validate_paths( void ) {
    char *log_path = malloc( MAX_PATH_LENGTH_CHAR + 1 );

    if( log_path ) {
        snprintf( log_path, MAX_PATH_LENGTH, "%s%s", app_path, LOGS_PATH );
        if( directory_exists( log_path ) == 0 ) {
            if( mkdir( log_path, 0777 ) != 0 ) {
                LOG_print( "Error creating path!\n" );
            }
            if( chdir( app_path ) != 0 ) {
                LOG_print( "Error: chdir().\n" );
                return;
            }
        }

        LOG_prepare();

        free( log_path );
        log_path = NULL;
    } else {
        LOG_print( "LOG_validate_paths error: unable to allocate memory.");
    }
    
}

void LOG_init( void ) {
    snprintf( app_path, MAX_PATH_LENGTH, "%s", get_app_path() );
    /*strncpy( app_path, get_app_path(), MAX_PATH_LENGTH );*/

    ( void )LOG_validate_paths();

    printf( "[%s] Start path: \"%s\".\n", TIME_get_gmt(), app_path );

    LOG_save();

}

void LOG_print( char *fmt, ... ) {
    char *output_text;
    FILE *f_LOG;
    va_list args;

    pthread_mutex_lock( &printf_mutex );
    output_text = ( char* )calloc( LARGE_BUFF_SIZE + 1, sizeof( char ) );

    va_start( args, fmt );
    vsnprintf( output_text, LARGE_BUFF_SIZE, fmt, args );
    va_end( args );

    strncat( log_object, output_text, LOG_BUFFER );
    printf( "%s", output_text );

    free( output_text );
    output_text = NULL;

    f_LOG = fopen( LOG_filename, "a+" );
    if( f_LOG ) {
        fseek( f_LOG, 0, SEEK_END );
        fwrite( log_object, strlen( log_object ), 1, f_LOG );
        if( fclose( f_LOG ) == EOF ) {
            LOG_print( "Error: unable to save log object!\n" );
        }
    }

    memset( log_object, '\0', LOG_BUFFER );
    if( chdir( app_path ) != 0 ) {
        LOG_print( "Error: unable to perform chdir( %s ).\n", app_path );
    }
    pthread_mutex_unlock( &printf_mutex );
}

void LOG_save( void ) {
    FILE *f_LOG;

    f_LOG = fopen( LOG_filename, "a+" );
    if( f_LOG ) {
        fseek( f_LOG, 0, SEEK_END );
        fwrite( log_object, strlen( log_object ), 1, f_LOG );
        if( fclose( f_LOG ) == EOF ) {
            LOG_print( "Error: unable to save log object!\n" );
        }
    } else {
        LOG_print( "Error: unable to open log file.\n" );
        exit( EXIT_FAILURE );
    }

    memset( log_object, 0, LOG_BUFFER );
    if( chdir( app_path ) != 0 ) {
        LOG_print( "Error: unable to perform chdir( %s ).\n", app_path );
    }
}
