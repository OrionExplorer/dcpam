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

void* LCS_WORKER_watcher( void* p );

int LCS_WORKER_shutdown( LOG_OBJECT* log ) {
    LOG_print( log, "[%s] LCS_WORKER_shutdown (app_terminated=%d).\n", TIME_get_gmt(), app_terminated );
    return TRUE;
}

void* LCS_WORKER_watcher( void* p ) {
    LOG_OBJECT      log;

    LOG_init( &log, "lcs_worker", 65535 );

    while( app_terminated == 0 ) {
        for( int i = 0; i < L_APP.COMPONENTS_len; i++ ) {
            if( L_APP.COMPONENTS[ i ] ) {

                LOG_print( &log, "[%s] Component check: \"%s %s (%s:%d)\"...\n", TIME_get_gmt(), L_APP.COMPONENTS[ i ]->name, L_APP.COMPONENTS[ i ]->version, L_APP.COMPONENTS[ i ]->ip, L_APP.COMPONENTS[ i ]->port );

                int res = COMPONENT_check( L_APP.COMPONENTS[ i ], &log );
                strncpy( L_APP.COMPONENTS[ i ]->timestamp, TIME_get_gmt(), 20 );
                if( res == 0 ) {
                    LOG_print( &log, "[%s] FAILURE Component check: \"%s %s (%s:%d)\" is not responding!\n", TIME_get_gmt(), L_APP.COMPONENTS[ i ]->name, L_APP.COMPONENTS[ i ]->version, L_APP.COMPONENTS[ i ]->ip, L_APP.COMPONENTS[ i ]->port );
                    L_APP.COMPONENTS[ i ]->active = 0;
                } else {
                    LOG_print( &log, "[%s] SUCCESS Component check for \"%s %s (%s:%d)\".\n", TIME_get_gmt(), L_APP.COMPONENTS[ i ]->name, L_APP.COMPONENTS[ i ]->version, L_APP.COMPONENTS[ i ]->ip, L_APP.COMPONENTS[ i ]->port );
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
    pthread_t   lcs_worker_pid;

    LOG_print( log, "[%s] LCS_WORKER_init...", TIME_get_gmt() );

    if( pthread_create( &lcs_worker_pid, NULL, LCS_WORKER_watcher, NULL ) == 0 ) {
        pthread_join( lcs_worker_pid, NULL );
    } else {
        return 0;
    }

    return 1;
}
