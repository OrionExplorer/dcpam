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

static pthread_mutex_t      printf_mutex = PTHREAD_MUTEX_INITIALIZER;

void LOG_prepare( LOG_OBJECT *log, const char *prefix, const char *app_path ) {
    memset( log->buffer, '\0', LARGE_BUFF_SIZE );
    snprintf( log->filename, MAX_PATH_LENGTH, "%s%s%s-log.txt", app_path, LOGS_PATH, prefix ? prefix : "dcpam" );
    LOG_print( log, "\n[%s] Session start.\n", TIME_get_gmt() );

}


static void LOG_validate_paths( LOG_OBJECT *log, const char *prefix, const char *app_path ) {
    char *log_path = malloc( MAX_PATH_LENGTH_CHAR + 1 );

    if( log_path ) {
        snprintf( log_path, MAX_PATH_LENGTH, "%s%s", app_path, LOGS_PATH );
        if( directory_exists( log_path ) == 0 ) {
            if( mkdir( log_path, 0777 ) != 0 ) {
                LOG_print( log, "Error creating path!\n" );
            }
            if( chdir( app_path ) != 0 ) {
                LOG_print( log, "Error: chdir().\n" );
                return;
            }
        }
        LOG_prepare( log, prefix, app_path );

        free( log_path );
        log_path = NULL;
    } else {
        LOG_print( log, "LOG_validate_paths error: unable to allocate memory.");
    }
    
}

void LOG_init( LOG_OBJECT *log, const char *prefix) {
    char _app_path[ MAX_PATH_LENGTH ];
    snprintf( _app_path, MAX_PATH_LENGTH, get_app_path() );
    /*strncpy( app_path, get_app_path(), MAX_PATH_LENGTH );*/

    ( void )LOG_validate_paths( log, prefix, _app_path );

    LOG_print( log, "[%s] Start path: \"%s\".\n", TIME_get_gmt(), _app_path );

    LOG_save( log, _app_path );

}

void LOG_print( LOG_OBJECT *log, char *fmt, ... ) {
    char *output_text;
    FILE *f_LOG;
    va_list args;

    pthread_mutex_lock( &printf_mutex );
    output_text = ( char* )calloc( LARGE_BUFF_SIZE + 1, sizeof( char ) );

    va_start( args, fmt );
    vsnprintf( output_text, LARGE_BUFF_SIZE, fmt, args );
    va_end( args );

    strncat( log->buffer, output_text, LARGE_BUFF_SIZE );
    printf( "%s", output_text );

    free( output_text );
    output_text = NULL;

    f_LOG = fopen( log->filename, "a+" );
    if( f_LOG ) {
        fseek( f_LOG, 0, SEEK_END );
        fwrite( log->buffer, strlen( log->buffer ), 1, f_LOG );
        if( fclose( f_LOG ) == EOF ) {
            LOG_print( log, "Error: unable to save log object!\n" );
        }
    }

    memset( log->buffer, '\0', LARGE_BUFF_SIZE );
    //if( chdir( app_path ) != 0 ) {
    //    LOG_print( log, "Error: unable to perform chdir( %s ).\n", app_path );
    //}
    pthread_mutex_unlock( &printf_mutex );
}

void LOG_save( LOG_OBJECT *log, const char *app_path ) {
    FILE *f_LOG;

    f_LOG = fopen( log->filename, "a+" );
    if( f_LOG ) {
        fseek( f_LOG, 0, SEEK_END );
        fwrite( log->buffer, strlen( log->buffer ), 1, f_LOG );
        if( fclose( f_LOG ) == EOF ) {
            LOG_print( log, "Error: unable to save log object!\n" );
        }
    } else {
        LOG_print( log, "Error: unable to open log file.\n" );
        exit( EXIT_FAILURE );
    }

    memset( log->buffer, 0, LARGE_BUFF_SIZE );
    if( chdir( app_path ) != 0 ) {
        LOG_print( log, "Error: unable to perform chdir( %s ).\n", app_path );
    }
}
