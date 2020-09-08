#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "../include/utils/memory.h"
#include "../include/DCPAM_LCS/lcs_worker.h"
#include "../include/core/schema.h"
#include "../include/core/component.h"
#include "../include/utils/time.h"

static pthread_mutex_t      watcher_mutex = PTHREAD_MUTEX_INITIALIZER;

int                         worker_save_counter = 0;
extern int                  app_terminated;
extern L_DCPAM_APP          L_APP;                                    /* Main application object | config.json => "app" */

typedef struct WORKER_DATA {
    int                 thread_id;
    LOG_OBJECT          *log;
} WORKER_DATA;

void* LCS_WORKER_watcher( void* src_WORKER_DATA );

int LCS_WORKER_shutdown( LOG_OBJECT* log ) {
    int         i = 0;

    LOG_print( log, "[%s] LCS_WORKER_shutdown (app_terminated=%d).\n", TIME_get_gmt(), app_terminated );

    return TRUE;
}

void* LCS_WORKER_watcher( void* src_WORKER_DATA ) {
    LOG_OBJECT      log;
    WORKER_DATA     *t_worker_data = ( WORKER_DATA* )src_WORKER_DATA;

    LOG_init( &log, "lcs_worker", 65535 );

    while( app_terminated == 0 ) {
        for( int i = 0; i < L_APP.COMPONENTS_len; i++ ) {
            if( L_APP.COMPONENTS[ i ] ) {

                LOG_print( &log, "[%s] Component check: \"%s %s (%s:%d)\"...\n", TIME_get_gmt(), L_APP.COMPONENTS[ i ]->name, L_APP.COMPONENTS[ i ]->version, L_APP.COMPONENTS[ i ]->ip, L_APP.COMPONENTS[ i ]->port );

                int res = COMPONENT_check( L_APP.COMPONENTS[ i ], &log );
                strncpy( L_APP.COMPONENTS[ i ]->timestamp, TIME_get_gmt(), 20 );
                if( res == 0 ) {
                    LOG_print( &log, "[%s] Component check failure: \"%s %s (%s:%d)\" is not responding!\n", TIME_get_gmt(), L_APP.COMPONENTS[ i ]->name, L_APP.COMPONENTS[ i ]->version, L_APP.COMPONENTS[ i ]->ip, L_APP.COMPONENTS[ i ]->port );
                    L_APP.COMPONENTS[ i ]->active = 0;
                } else {
                    LOG_print( &log, "[%s] Component check succes for \"%s %s (%s:%d)\".\n", TIME_get_gmt(), L_APP.COMPONENTS[ i ]->name, L_APP.COMPONENTS[ i ]->version, L_APP.COMPONENTS[ i ]->ip, L_APP.COMPONENTS[ i ]->port );
                    L_APP.COMPONENTS[ i ]->active = 1;
                }

                Sleep( WORKER_WATCHER_SLEEP );
            }
        }
    }

    LOG_free( &log );

    pthread_exit( NULL );
}

int LCS_WORKER_init( LOG_OBJECT *log ) {
    int         lcs_worker_thread = 0;
    pthread_t   lcs_worker_pid;
    WORKER_DATA t_worker_data;


    LOG_print( log, "[%s] LCS_WORKER_init...", TIME_get_gmt() );

    lcs_worker_thread = pthread_create( &lcs_worker_pid, NULL, LCS_WORKER_watcher, ( void* )&t_worker_data );

    pthread_join( lcs_worker_pid, NULL );

    return 1;
}
