#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include "../../include/core/db/worker.h"
#include "../../include/utils/time.h"
#include "../../include/core/schema.h"
#include "../../include/utils/log.h"
#include "../../include/core/db/system.h"
#include "../../include/core/db/etl/extract.h"
#include "../../include/core/db/etl/stage.h"
#include "../../include/core/db/etl/transform.h"
#include "../../include/core/db/etl/load.h"

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
    DB_SYSTEM_ETL_STEP  step;
} WORKER_DATA;


void* DB_WORKER_watcher( void* src_WORKER_DATA );


int DB_WORKER_init( void ) {
    int                     thread_s[ MAX_DATA_SYSTEMS ];
    WORKER_DATA             t_worker_data[ MAX_DATA_SYSTEMS ];
    int                     i = 0;
    pthread_attr_t          attrs;
    DB_SYSTEM_ETL_STEP      curr_etl_step = ETL_EXTRACT;
    DB_SYSTEM_ETL_STEP      prev_etl_step = ETL_EXTRACT;


    mysql_library_init( 0, NULL, NULL );

    pthread_attr_init( &attrs );
    pthread_attr_setdetachstate( &attrs, PTHREAD_CREATE_JOINABLE );

    /*
        Spawn all Extract processes.
    */
    for( i = 0; i < DATABASE_SYSTEMS_COUNT; i++ ) {
        t_worker_data[ i ].DATA_SYSTEM = &DATABASE_SYSTEMS[ i ];
        t_worker_data[ i ].thread_id = i;
        t_worker_data[ i ].step = ETL_EXTRACT;

        LOG_print("===========================================================\n[%s] DB_WORKER_init: spawning EXTRACT thread %d/%d...\n", TIME_get_gmt(), i + 1, DATABASE_SYSTEMS_COUNT );
        thread_s[ i ] = pthread_create( &w_watcher_thread[ i ], &attrs, DB_WORKER_watcher, ( void* )&t_worker_data[ i ] );
        if( thread_s[ i ] != 0 ) {
            LOG_print( "[%s] DB_WORKER_init( ) failed to create DB_WORKER_watcher thread for \"%s\". Error: %d.\n", TIME_get_gmt(), DATABASE_SYSTEMS[ i ].name, thread_s[ i ] );
            mysql_library_end();
            return 0;
        }
        Sleep( 10 );
    }

    for( i = 0; i < DATABASE_SYSTEMS_COUNT; i++ ) {
        pthread_join( w_watcher_thread[ i ], NULL );
    }
    LOG_print("[%s] DB_WORKER_init: all EXTRACT threads are completed.\n", TIME_get_gmt() );

    /*
        Spawn all Transform processes.
    */
    for( i = 0; i < DATABASE_SYSTEMS_COUNT; i++ ) {
        t_worker_data[ i ].DATA_SYSTEM = &DATABASE_SYSTEMS[ i ];
        t_worker_data[ i ].thread_id = i;
        t_worker_data[ i ].step = ETL_TRANSFORM;

        LOG_print( "===========================================================\n[%s] DB_WORKER_init: spawning TRANSFORM thread %d/%d...\n", TIME_get_gmt(), i + 1, DATABASE_SYSTEMS_COUNT );
        thread_s[ i ] = pthread_create( &w_watcher_thread[ i ], &attrs, DB_WORKER_watcher, ( void* )&t_worker_data[ i ] );
        if( thread_s[ i ] != 0 ) {
            LOG_print( "[%s] WORKER_init( ) failed to create DB_WORKER_watcher thread for \"%s\". Error: %d.\n", TIME_get_gmt(), DATABASE_SYSTEMS[ i ].name, thread_s[ i ] );
            mysql_library_end();
            return 0;
        }
        Sleep( 10 );
    }

    for( i = 0; i < DATABASE_SYSTEMS_COUNT; i++ ) {
        pthread_join( w_watcher_thread[ i ], NULL );
    }
    LOG_print( "[%s] DB_WORKER_init: all TRANSFORM threads are completed.\n", TIME_get_gmt() );

    /*
        Spawn all Load processes.
    */
    for( i = 0; i < DATABASE_SYSTEMS_COUNT; i++ ) {
        t_worker_data[ i ].DATA_SYSTEM = &DATABASE_SYSTEMS[ i ];
        t_worker_data[ i ].thread_id = i;
        t_worker_data[ i ].step = ETL_LOAD;

        LOG_print( "===========================================================\n[%s] DB_WORKER_init: spawning LOAD thread %d/%d...\n", TIME_get_gmt(), i + 1, DATABASE_SYSTEMS_COUNT );
        thread_s[ i ] = pthread_create( &w_watcher_thread[ i ], &attrs, DB_WORKER_watcher, ( void* )&t_worker_data[ i ] );
        if( thread_s[ i ] != 0 ) {
            LOG_print( "[%s] WORKER_init( ) failed to create DB_WORKER_watcher thread for \"%s\". Error: %d.\n", TIME_get_gmt(), DATABASE_SYSTEMS[ i ].name, thread_s[ i ] );
            mysql_library_end();
            return 0;
        }
        Sleep( 10 );
    }

    for( i = 0; i < DATABASE_SYSTEMS_COUNT; i++ ) {
        pthread_join( w_watcher_thread[ i ], NULL );
    }
    LOG_print( "[%s] DB_WORKER_init: all LOAD threads are completed.\n", TIME_get_gmt() );

    mysql_library_end();
    LOG_print( "All threads are terminated.\n" );

    return TRUE;
}

void* DB_WORKER_watcher( void* src_WORKER_DATA ) {
    int                     i = 0;
    WORKER_DATA             *t_worker_data = ( WORKER_DATA* )src_WORKER_DATA;
    DATABASE_SYSTEM         *DATA_SYSTEM = t_worker_data->DATA_SYSTEM;
    DB_SYSTEM_ETL_STEP      curr_etl_step = t_worker_data->step;

    LOG_print( "\nDB_WORKER_watcher for \"%s\" (%s) started...\n",
        DATA_SYSTEM->name,
        curr_etl_step == ETL_EXTRACT ? "EXTRACT" : curr_etl_step == ETL_TRANSFORM ? "TRANSFORM" : curr_etl_step == ETL_LOAD ? "LOAD" : "UNKNOWN"
    );

    LOG_print( "Init DCPAM database connection for \"%s\":\n", DATA_SYSTEM->name );
    if( DATABASE_SYSTEM_DB_init( &DATA_SYSTEM->dcpam_db) == FALSE ) {
        pthread_cancel( w_watcher_thread[ t_worker_data->thread_id ] );
        return FALSE;
    }

    LOG_print( "Init source system database connection for \"%s\":\n", DATA_SYSTEM->name );
    if( DATABASE_SYSTEM_DB_init( &DATA_SYSTEM->system_db ) == FALSE ) {
        pthread_cancel( w_watcher_thread[ t_worker_data->thread_id ] );
        return FALSE;
    }

    if( DATA_SYSTEM->staging_db ) {
        LOG_print( "Init Staging Area database connection for \"%s\":\n", DATA_SYSTEM->name );
        if( DATABASE_SYSTEM_DB_init( DATA_SYSTEM->staging_db ) == FALSE ) {
            pthread_cancel( w_watcher_thread[ t_worker_data->thread_id ] );
            return FALSE;
        }
    } else {
        LOG_print( "Staging Area is local.\n" );
    }

    //while( app_terminated == 0 ) {
        pthread_mutex_lock( &watcher_mutex );

        worker_save_counter += WORKER_WATCHER_SLEEP;

        if( curr_etl_step == ETL_EXTRACT ) {
            /* 0: Run PreCDC actions. */
            LOG_print( "[%s] Started run of PreCDC Actions.\n", TIME_get_gmt() );
            for( i = 0; i < DATA_SYSTEM->queries_len; i++ ) {
                if( DATA_SYSTEM->queries[ i ].etl_config.pre_actions ) {
                    for( int k = 0; k < DATA_SYSTEM->queries[ i ].etl_config.pre_actions_count; k++ ) {
                        DB_exec(
                            &DATA_SYSTEM->dcpam_db,
                            DATA_SYSTEM->queries[ i ].etl_config.pre_actions[ k ]->sql,
                            strlen( DATA_SYSTEM->queries[ i ].etl_config.pre_actions[ k ]->sql ),
                            NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL
                        );
                    }
                }
            }
            LOG_print( "[%s] Finished run of PreCDC Actions.\n", TIME_get_gmt() );

            /* 1st: Extract and store data in the Staging Area */
            for( i = 0; i < DATA_SYSTEM->queries_len; i++ ) {
                /* Init query structs */
                qec extract_inserted_callback = DATA_SYSTEM->queries[ i ].etl_config.stage ? ( qec )&_ExtractInserted_callback : ( qec )&_LoadInserted_callback;
                qec extract_deleted_callback = DATA_SYSTEM->queries[ i ].etl_config.stage ? ( qec )&_ExtractDeleted_callback : ( qec )&_LoadDeleted_callback;
                qec extract_modified_callback = DATA_SYSTEM->queries[ i ].etl_config.stage ? ( qec )&_ExtractModified_callback : ( qec )&_LoadModified_callback;

                LOG_print( "\t· [EXTRACT] Query #%d: %s\n", i + 1, DATA_SYSTEM->queries[ i ].name );

                DB_CDC_ExtractInserted(
                    &DATA_SYSTEM->queries[ i ].etl_config.extract,
                    &DATA_SYSTEM->system_db,
                    &DATA_SYSTEM->dcpam_db,
                    &extract_inserted_callback,
                    DATA_SYSTEM->queries[ i ].etl_config.stage ? ( void* )DATA_SYSTEM->queries[ i ].etl_config.stage : ( void* )&DATA_SYSTEM->queries[ i ].etl_config.load,
                    DATA_SYSTEM->staging_db ? ( void* )DATA_SYSTEM->staging_db : ( void* )&DATA_SYSTEM->dcpam_db
                );
                DB_CDC_ExtractDeleted(
                    &DATA_SYSTEM->queries[ i ].etl_config.extract,
                    &DATA_SYSTEM->system_db,
                    &DATA_SYSTEM->dcpam_db,
                    &extract_deleted_callback,
                    DATA_SYSTEM->queries[ i ].etl_config.stage ? ( void* )DATA_SYSTEM->queries[ i ].etl_config.stage : ( void* )&DATA_SYSTEM->queries[ i ].etl_config.load,
                    DATA_SYSTEM->staging_db ? ( void* )DATA_SYSTEM->staging_db : ( void* )&DATA_SYSTEM->dcpam_db
                );
                DB_CDC_ExtractModified(
                    &DATA_SYSTEM->queries[ i ].etl_config.extract,
                    &DATA_SYSTEM->system_db,
                    &DATA_SYSTEM->dcpam_db,
                    &extract_modified_callback,
                    DATA_SYSTEM->queries[ i ].etl_config.stage ? ( void* )DATA_SYSTEM->queries[ i ].etl_config.stage : ( void* )&DATA_SYSTEM->queries[ i ].etl_config.load,
                    DATA_SYSTEM->staging_db ? ( void* )DATA_SYSTEM->staging_db : ( void* )&DATA_SYSTEM->dcpam_db
                );
            }
        }

        else {
            for( i = 0; i < DATA_SYSTEM->queries_len; i++ ) {
                if( 
                    ( DATA_SYSTEM->queries[ i ].mode == M_ETL && curr_etl_step == ETL_TRANSFORM ) ||
                    ( DATA_SYSTEM->queries[ i ].mode == M_ELT && curr_etl_step == ETL_LOAD )
                ) {
                    if( DATA_SYSTEM->queries[ i ].etl_config.transform ) {
                        LOG_print( "\t· [TRANSFORM] Query #%d: %s\n", i + 1, DATA_SYSTEM->queries[ i ].name );

                        //DB_CDC_TransformInserted( &DATA_SYSTEM->queries[ i ].change_data_capture.transform, &DATA_SYSTEM->DB, &inserted_data );
                        //DB_CDC_TransformDeleted( &DATA_SYSTEM->queries[ i ].change_data_capture.transform, &DATA_SYSTEM->DB, &deleted_data );
                        //DB_CDC_TransformModified( &DATA_SYSTEM->queries[ i ].change_data_capture.transform, &DATA_SYSTEM->DB, &modified_data );
                    }
                } else if( 
                    ( DATA_SYSTEM->queries[ i ].mode == M_ETL && curr_etl_step == ETL_LOAD ) ||
                    ( DATA_SYSTEM->queries[ i ].mode == M_ELT && curr_etl_step == ETL_TRANSFORM )
                ) {
                    LOG_print( "\t· [LOAD] Query #%d: %s\n", i + 1, DATA_SYSTEM->queries[ i ].name );

                    DB_CDC_LoadInserted(
                        &DATA_SYSTEM->queries[ i ].etl_config.load,
                        DATA_SYSTEM->staging_db ? DATA_SYSTEM->staging_db : &DATA_SYSTEM->dcpam_db,
                        &DATA_SYSTEM->dcpam_db
                    );
                    DB_CDC_LoadDeleted(
                        &DATA_SYSTEM->queries[ i ].etl_config.load,
                        DATA_SYSTEM->staging_db ? DATA_SYSTEM->staging_db : &DATA_SYSTEM->dcpam_db,
                        &DATA_SYSTEM->dcpam_db
                    );
                    DB_CDC_LoadModified(
                        &DATA_SYSTEM->queries[ i ].etl_config.load,
                        DATA_SYSTEM->staging_db ? DATA_SYSTEM->staging_db : &DATA_SYSTEM->dcpam_db,
                        &DATA_SYSTEM->dcpam_db
                    );

                    /* 4rd: Run PostCDC actions. */
                    LOG_print( "[%s] Started run of PostCDC Actions.\n", TIME_get_gmt() );
                    for( int j = 0; j < DATA_SYSTEM->queries_len; j++ ) {
                        if( DATA_SYSTEM->queries[ j ].etl_config.post_actions ) {
                            for( int k = 0; k < DATA_SYSTEM->queries[ j ].etl_config.post_actions_count; k++ ) {
                                DB_exec(
                                    &DATA_SYSTEM->dcpam_db,
                                    DATA_SYSTEM->queries[ j ].etl_config.post_actions[ k ]->sql,
                                    strlen( DATA_SYSTEM->queries[ j ].etl_config.post_actions[ k ]->sql ),
                                    NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL
                                );
                            }
                        }
                    }
                    LOG_print( "[%s] Finished run of PostCDC Actions.\n", TIME_get_gmt() );
                } else {
                    LOG_print("WTF: curr_etl_step = %d, mode = %d\n", curr_etl_step, DATA_SYSTEM->queries[ i ].mode );
                }
            }
        }
        //else if( curr_etl_step == ETL_TRANSFORM ) {
        //    /* 2nd: Transform data. */
        //    for( i = 0; i < DATA_SYSTEM->queries_len; i++ ) {

        //        if( DATA_SYSTEM->queries[ i ].etl_config.transform ) {
        //            LOG_print( "\t· [TRANSFORM] Query #%d: %s\n", i + 1, DATA_SYSTEM->queries[ i ].name );

        //            //DB_CDC_TransformInserted( &DATA_SYSTEM->queries[ i ].change_data_capture.transform, &DATA_SYSTEM->DB, &inserted_data );
        //            //DB_CDC_TransformDeleted( &DATA_SYSTEM->queries[ i ].change_data_capture.transform, &DATA_SYSTEM->DB, &deleted_data );
        //            //DB_CDC_TransformModified( &DATA_SYSTEM->queries[ i ].change_data_capture.transform, &DATA_SYSTEM->DB, &modified_data );
        //        }
        //    }
        //}

        //else if( curr_etl_step == ETL_LOAD ) {
        //    /* 3rd: Load data. */
        //    for( i = 0; i < DATA_SYSTEM->queries_len; i++ ) {
        //        LOG_print( "\t· [LOAD] Query #%d: %s\n", i + 1, DATA_SYSTEM->queries[ i ].name );

        //        DB_CDC_LoadInserted(
        //            &DATA_SYSTEM->queries[ i ].etl_config.load,
        //            DATA_SYSTEM->staging_db ? DATA_SYSTEM->staging_db : &DATA_SYSTEM->dcpam_db,
        //            &DATA_SYSTEM->dcpam_db
        //        );
        //        DB_CDC_LoadDeleted(
        //            &DATA_SYSTEM->queries[ i ].etl_config.load,
        //            DATA_SYSTEM->staging_db ? DATA_SYSTEM->staging_db : &DATA_SYSTEM->dcpam_db,
        //            &DATA_SYSTEM->dcpam_db
        //        );
        //        DB_CDC_LoadModified(
        //            &DATA_SYSTEM->queries[ i ].etl_config.load,
        //            DATA_SYSTEM->staging_db ? DATA_SYSTEM->staging_db : &DATA_SYSTEM->dcpam_db,
        //            &DATA_SYSTEM->dcpam_db
        //        );
        //    }

        //    /* 4rd: Run PostCDC actions. */
        //    LOG_print( "[%s] Started run of PostCDC Actions.\n", TIME_get_gmt() );
        //    for( i = 0; i < DATA_SYSTEM->queries_len; i++ ) {
        //        if( DATA_SYSTEM->queries[ i ].etl_config.post_actions ) {
        //            for( int k = 0; k < DATA_SYSTEM->queries[ i ].etl_config.post_actions_count; k++ ) {
        //                DB_exec(
        //                    &DATA_SYSTEM->dcpam_db,
        //                    DATA_SYSTEM->queries[ i ].etl_config.post_actions[ k ]->sql,
        //                    strlen( DATA_SYSTEM->queries[ i ].etl_config.post_actions[ k ]->sql ),
        //                    NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL
        //                );
        //            }
        //        }
        //    }
        //    LOG_print( "[%s] Finished run of PostCDC Actions.\n", TIME_get_gmt() );
        //}


        LOG_print("DB_WORKER_watcher for \"%s\" with process type \"%s\" finished.\n",
                    DATA_SYSTEM->name,
                    curr_etl_step == ETL_EXTRACT ? "EXTRACT" : curr_etl_step == ETL_TRANSFORM ? "TRANSFORM" : curr_etl_step == ETL_LOAD ? "LOAD" : "UNKNOWN"
        );

        pthread_mutex_unlock( &watcher_mutex );
        if( curr_etl_step == ETL_LOAD ) {
            if( APP.run_once == 1 ) {
                app_terminated = 1;
            } else {
                Sleep( WORKER_WATCHER_SLEEP );
            }
        }

        if( app_terminated == 1 ) {
            LOG_print( "[%s]\t- Thread exit handler executed for \"%s\".\n", TIME_get_gmt(), DATA_SYSTEM->name );
            DATABASE_SYSTEM_close( DATA_SYSTEM );
            pthread_exit( NULL );
        }
    //}

    if( DATA_SYSTEM->name != NULL ) {
        DATABASE_SYSTEM_DB_close( &DATA_SYSTEM->dcpam_db );
        DATABASE_SYSTEM_DB_close( &DATA_SYSTEM->system_db );
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
