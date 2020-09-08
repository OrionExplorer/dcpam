#include "../include/shared.h"
#include "../include/utils/log.h"
#include "../include/utils/time.h"
#include "../include/utils/filesystem.h"
#include "../include/utils/memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>

static pthread_mutex_t      printf_mutex = PTHREAD_MUTEX_INITIALIZER;

void LOG_prepare( LOG_OBJECT *log, const char *prefix, const char *app_path ) {
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

void LOG_init( LOG_OBJECT* log, const char* prefix, const size_t buffer_size ) {
    char _app_path[ MAX_PATH_LENGTH ];
    snprintf( _app_path, MAX_PATH_LENGTH, get_app_path() );

    log->buffer = SAFECALLOC( buffer_size, sizeof( char ), __FILE__, __LINE__ );

    ( void )LOG_validate_paths( log, prefix, _app_path );

    LOG_print( log, "[%s] Start path: \"%s\".\n", TIME_get_gmt(), _app_path );

    LOG_save( log, _app_path );
}

void LOG_free( LOG_OBJECT* log ) {
    if( log ) {
        free( log->buffer ); log->buffer = NULL;
    }
}

void LOG_print( LOG_OBJECT *log, char *fmt, ... ) {
    char            *output_text;
    va_list         args;
    int             buf_len = 0;
    const size_t    FORMATTED_TEXT_LEN = 1024;
    const size_t    BUFFER_SIZE = 32768;

    pthread_mutex_lock( &printf_mutex );

    output_text = SAFEMALLOC( FORMATTED_TEXT_LEN, __FILE__, __LINE__ );

    va_start( args,fmt );
    vsnprintf( output_text, FORMATTED_TEXT_LEN, fmt, args );
    va_end( args );

    strncat( log->buffer, output_text, BUFFER_SIZE );
    printf( "%s", output_text );

    free( output_text );
    output_text = NULL;

    buf_len = strlen( log->buffer );
    if( buf_len >= BUFFER_SIZE ) {
        int     len = 0;

        if( buf_len > BUFFER_SIZE ) {
            len = buf_len - BUFFER_SIZE;
            char *tmp_buf = SAFEMALLOC( len, __FILE__, __LINE__ );

            strncpy( tmp_buf, log->buffer+BUFFER_SIZE, len );
            strncat( log->buffer, tmp_buf, len );
            free( tmp_buf );
            tmp_buf = NULL;
        }

        FILE *f_LOG = fopen( log->filename, "a+" );
        if( f_LOG ) {
            fseek( f_LOG, 0, SEEK_END );
            fwrite( log->buffer, BUFFER_SIZE + len, 1, f_LOG );
            if( fclose( f_LOG ) == EOF ) {
                LOG_print( log, "Error: unable to save log object!\n" );
            }
        } else {
            printf( "[%s] Fatal error: unable to save log file %s.\n", TIME_get_gmt(), log->filename );
        }

        memset( log->buffer, 0, BUFFER_SIZE );
    }

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
    /*if( chdir( app_path ) != 0 ) {
        LOG_print( log, "Error: unable to perform chdir( %s ).\n", app_path );
    }*/
}
