#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include "../../include/core/db/worker.h"
#include "../../include/utils/time.h"
#include "../../include/core/schema.h"
#include "../../include/utils/log.h"
#include "../../include/core/db/system.h"
#include "../../include/core/db/cdc/extract.h"
#include "../../include/core/db/cdc/stage.h"
#include "../../include/core/db/cdc/transform.h"
#include "../../include/core/db/cdc/load.h"

pthread_t                   w_watcher_thread[ MAX_DATA_SYSTEMS ];
static pthread_mutex_t      watcher_mutex = PTHREAD_MUTEX_INITIALIZER;

int                         worker_save_counter = 0;
extern int                  app_terminated;

extern int                  DATABASE_SYSTEMS_COUNT;
extern DATABASE_SYSTEM      DATABASE_SYSTEMS[ MAX_DATA_SYSTEMS ];   /* Database-based systems | config.json => "system" */
extern DCPAM_APP            APP;                                    /* Main application object | config.json => "app" */

typedef struct WORKER_DATA {
    DATABASE_SYSTEM     *DATA_SYSTEM;
    int                 thread_id;
} WORKER_DATA;


void* DB_WORKER_watcher( void* src_WORKER_DATA );


int DB_WORKER_init( void ) {
    int             thread_s[ MAX_DATA_SYSTEMS ];
    WORKER_DATA     t_worker_data[ MAX_DATA_SYSTEMS ];
    int             i = 0;
    pthread_attr_t  attrs;


    mysql_library_init( 0, NULL, NULL );
    /* Connect to dcpam database */
    LOG_print( "Init main database connection:\n" );
    if( DATABASE_SYSTEM_DB_init( &APP.DB ) == FALSE ) {
        DATABASE_SYSTEM_DB_free( &APP.DB );
        mysql_library_end();
        return FALSE;
    }

    /* Connect to each database-based system */
    pthread_attr_init( &attrs );
    pthread_attr_setdetachstate( &attrs, PTHREAD_CREATE_JOINABLE );

    for( i = 0; i < DATABASE_SYSTEMS_COUNT; i++ ) {
        t_worker_data[ i ].DATA_SYSTEM = &DATABASE_SYSTEMS[ i ];
        t_worker_data[ i ].thread_id = i;
        
        thread_s[ i ] = pthread_create( &w_watcher_thread[ i ], &attrs, DB_WORKER_watcher, ( void* )&t_worker_data[ i ] );
        if( thread_s[ i ] != 0 ) {
            LOG_print( "[%s] WORKER_init( ) failed to create DB_WORKER_watcher thread for \"%s\". Error: %d.\n", TIME_get_gmt(), DATABASE_SYSTEMS[ i ].name, thread_s[ i ] );
            mysql_library_end();
            return 0;
        }       
        Sleep(10);
    }

    for( i = 0; i < DATABASE_SYSTEMS_COUNT; i++ ) {
        pthread_join( w_watcher_thread[ i ], NULL );
    }


    mysql_library_end();
    LOG_print( "All threads are terminated.\n" );

    return TRUE;
}


void* DB_WORKER_watcher( void* src_WORKER_DATA ) {
    int                 i = 0;
    DB_QUERY            inserted_data;
    DB_QUERY            deleted_data;
    DB_QUERY            modified_data;
    WORKER_DATA         *t_worker_data = ( WORKER_DATA* )src_WORKER_DATA;
    DATABASE_SYSTEM     *DATA_SYSTEM = t_worker_data->DATA_SYSTEM;

    LOG_print( "Init database connection thread for \"%s\":\n", DATA_SYSTEM->name );
    if( DATABASE_SYSTEM_DB_init( &DATA_SYSTEM->DB ) == FALSE ) {
        pthread_cancel( w_watcher_thread[ t_worker_data->thread_id ] );
        return FALSE;
    }

    /* Init query structs */
    DB_QUERY_init( &inserted_data );
    DB_QUERY_init( &deleted_data );
    DB_QUERY_init( &modified_data );

    while( app_terminated == 0 ) {
        pthread_mutex_lock( &watcher_mutex );

        worker_save_counter += WORKER_WATCHER_SLEEP;

        LOG_print("\nDB_WORKER_watcher for \"%s\" started...\n", DATA_SYSTEM->name );

        for( i = 0; i < DATA_SYSTEM->queries_len; i++ ) {
            LOG_print( "\t· Query #%d: %s\n", i + 1, DATA_SYSTEM->queries[ i ].name );


            DB_CDC_ExtractInserted( &DATA_SYSTEM->queries[ i ].change_data_capture.extract, &DATA_SYSTEM->DB, &inserted_data );
            DB_CDC_StageInserted( &DATA_SYSTEM->queries[ i ].change_data_capture.stage, &DATA_SYSTEM->DB, &inserted_data );
            DB_CDC_TransformInserted( &DATA_SYSTEM->queries[ i ].change_data_capture.transform, &DATA_SYSTEM->DB, &inserted_data );
            DB_CDC_LoadInserted( &DATA_SYSTEM->queries[ i ].change_data_capture.load, &DATA_SYSTEM->DB, &inserted_data );
            DB_QUERY_free( &inserted_data );

            DB_CDC_ExtractDeleted( &DATA_SYSTEM->queries[ i ].change_data_capture.extract, &DATA_SYSTEM->DB, &deleted_data );
            DB_CDC_StageDeleted( &DATA_SYSTEM->queries[ i ].change_data_capture.stage, &DATA_SYSTEM->DB, &deleted_data );
            DB_CDC_TransformDeleted( &DATA_SYSTEM->queries[ i ].change_data_capture.transform, &DATA_SYSTEM->DB, &deleted_data );
            DB_CDC_LoadDeleted( &DATA_SYSTEM->queries[ i ].change_data_capture.load, &DATA_SYSTEM->DB, &deleted_data );
            DB_QUERY_free( &deleted_data );

            DB_CDC_ExtractModified( &DATA_SYSTEM->queries[ i ].change_data_capture.extract, &DATA_SYSTEM->DB, &modified_data );
            DB_CDC_StageModified( &DATA_SYSTEM->queries[ i ].change_data_capture.stage, &DATA_SYSTEM->DB, &modified_data );
            DB_CDC_TransformModified( &DATA_SYSTEM->queries[ i ].change_data_capture.transform, &DATA_SYSTEM->DB, &modified_data );
            //DB_CDC_LoadModified( &DATA_SYSTEM->queries[ i ].change_data_capture.load, &DATA_SYSTEM->DB, &modified_data );
            DB_QUERY_free( &modified_data );
        }

        LOG_print("DB_WORKER_watcher for \"%s\" finished.\n", DATA_SYSTEM->name );
        pthread_mutex_unlock( &watcher_mutex );
        Sleep( WORKER_WATCHER_SLEEP );
        if( app_terminated == 1 ) {
            LOG_print( "[%s]\t- Thread exit handler executed for \"%s\".\n", TIME_get_gmt(), DATA_SYSTEM->name );
            DATABASE_SYSTEM_close( DATA_SYSTEM );
            pthread_exit( NULL );
        }
    }

    if( DATABASE_SYSTEMS->name != NULL ) {
        DATABASE_SYSTEM_close( DATA_SYSTEM );
    }
    pthread_exit( NULL );
    return FALSE;
}


int DB_WORKER_shutdown( void ) {
    int         i = 0;

    LOG_print( "[%s] WORKER_shutdown (app_terminated=%d):\n", TIME_get_gmt(), app_terminated );
    for( i = 0; i < DATABASE_SYSTEMS_COUNT; i++ ) {
        LOG_print( "\t- \"%s\"\n", DATABASE_SYSTEMS[ i ].name );
        LOG_print( "[%s] DB_WORKER_watcher for \"%s\" set to termination.\n", TIME_get_gmt(), DATABASE_SYSTEMS[ i ].name );
    }

    return TRUE;
}
