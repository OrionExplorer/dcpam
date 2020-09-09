#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include "../../include/core/db/worker.h"
#include "../../include/utils/time.h"
#include "../../include/core/schema.h"
#include "../../include/utils/memory.h"
#include "../../include/utils/log.h"
#include "../../include/core/file/preload.h"
#include "../../include/core/db/system.h"
#include "../../include/core/db/etl/extract.h"
#include "../../include/core/db/etl/stage.h"
#include "../../include/core/db/etl/transform.h"
#include "../../include/core/db/etl/load.h"
#include "../../include/core/lcs_report.h"

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
    LOG_OBJECT          *log;
} WORKER_DATA;


void* DB_WORKER_watcher( void* src_WORKER_DATA );


int DB_WORKER_init( LOG_OBJECT *log ) {
    int                     thread_s[ MAX_DATA_SYSTEMS ];
    WORKER_DATA             t_worker_data[ MAX_DATA_SYSTEMS ];
    int                     i = 0;
    pthread_attr_t          attrs;

    mysql_library_init( 0, NULL, NULL );

    pthread_attr_init( &attrs );
    pthread_attr_setdetachstate( &attrs, PTHREAD_CREATE_JOINABLE );

    /* Init log files */
    LOG_OBJECT **log_object;

    log_object = SAFEMALLOC( DATABASE_SYSTEMS_COUNT * sizeof * log_object, __FILE__, __LINE__ );

    for( i = 0; i < DATABASE_SYSTEMS_COUNT; i++ ) {
        log_object[ i ] = SAFEMALLOC( sizeof( LOG_OBJECT ), __FILE__, __LINE__ );
        LOG_print( log, "[%s] Initializing log file: %s...\n", TIME_get_gmt(), DATABASE_SYSTEMS[ i ].name );
        LOG_init( log_object[ i ], DATABASE_SYSTEMS[ i ].name, 65535 );
    }

    while( app_terminated == 0 ) {

        /*
            Spawn all Extract processes.
        */
        for( i = 0; i < DATABASE_SYSTEMS_COUNT; i++ ) {

            if( DATABASE_SYSTEMS[ i ].failure == 0 ) {

                t_worker_data[ i ].DATA_SYSTEM = &DATABASE_SYSTEMS[ i ];
                t_worker_data[ i ].thread_id = i;
                t_worker_data[ i ].step = ETL_EXTRACT;
                t_worker_data[ i ].log = log_object[ i ];

                LOG_print( log, "===========================================================\n[%s] DB_WORKER_init: spawning EXTRACT thread %d/%d...\n", TIME_get_gmt(), i + 1, DATABASE_SYSTEMS_COUNT );
                thread_s[ i ] = pthread_create( &w_watcher_thread[ i ], &attrs, DB_WORKER_watcher, ( void* )&t_worker_data[ i ] );
                if( thread_s[ i ] != 0 ) {
                    LOG_print( log, "[%s] DB_WORKER_init( ) failed to create DB_WORKER_watcher thread for \"%s\". Error: %d.\n", TIME_get_gmt(), DATABASE_SYSTEMS[ i ].name, thread_s[ i ] );
                    mysql_library_end();
                    return 0;
                }
                Sleep( 10 );
            } else {
                LOG_print( log, "===========================================================\n[%s] DB_WORKER_init: not spawning EXTRACT thread %d/%d due to previous failure.\n", TIME_get_gmt(), i + 1, DATABASE_SYSTEMS_COUNT );
            }
        }

        for( i = 0; i < DATABASE_SYSTEMS_COUNT; i++ ) {
            if( DATABASE_SYSTEMS[ i ].failure == 0 ) {
                pthread_join( w_watcher_thread[ i ], NULL );
            }
        }
        LOG_print( log, "[%s] DB_WORKER_init: all EXTRACT threads are completed.\n", TIME_get_gmt() );

        /*
            Spawn all Transform processes.
        */
        for( i = 0; i < DATABASE_SYSTEMS_COUNT; i++ ) {

            if( DATABASE_SYSTEMS[ i ].failure == 0 ) {
                t_worker_data[ i ].DATA_SYSTEM = &DATABASE_SYSTEMS[ i ];
                t_worker_data[ i ].thread_id = i;
                t_worker_data[ i ].step = ETL_TRANSFORM;
                t_worker_data[ i ].log = log_object[ i ];

                LOG_print( log, "===========================================================\n[%s] DB_WORKER_init: spawning TRANSFORM/LOAD thread %d/%d...\n", TIME_get_gmt(), i + 1, DATABASE_SYSTEMS_COUNT );
                thread_s[ i ] = pthread_create( &w_watcher_thread[ i ], &attrs, DB_WORKER_watcher, ( void* )&t_worker_data[ i ] );
                if( thread_s[ i ] != 0 ) {
                    LOG_print( log, "[%s] WORKER_init( ) failed to create DB_WORKER_watcher thread for \"%s\". Error: %d.\n", TIME_get_gmt(), DATABASE_SYSTEMS[ i ].name, thread_s[ i ] );
                    mysql_library_end();
                    return 0;
                }
                Sleep( 10 );
            } else {
                LOG_print( log, "===========================================================\n[%s] DB_WORKER_init: not spawning TRANSFORM/LOAD thread %d/%d due to previous failure.\n", TIME_get_gmt(), i + 1, DATABASE_SYSTEMS_COUNT );
            }
        }

        for( i = 0; i < DATABASE_SYSTEMS_COUNT; i++ ) {
            if( DATABASE_SYSTEMS[ i ].failure == 0 ) {
                pthread_join( w_watcher_thread[ i ], NULL );
            }
        }
        LOG_print( log, "[%s] DB_WORKER_init: all TRANSFORM/LOAD threads are completed.\n", TIME_get_gmt() );

        /*
            Spawn all Load processes.
        */
        for( i = 0; i < DATABASE_SYSTEMS_COUNT; i++ ) {
            if( DATABASE_SYSTEMS[ i ].failure == 0 ) {
                t_worker_data[ i ].DATA_SYSTEM = &DATABASE_SYSTEMS[ i ];
                t_worker_data[ i ].thread_id = i;
                t_worker_data[ i ].step = ETL_LOAD;
                t_worker_data[ i ].log = log_object[ i ];

                LOG_print( log, "===========================================================\n[%s] DB_WORKER_init: spawning LOAD/TRANSFORM thread %d/%d...\n", TIME_get_gmt(), i + 1, DATABASE_SYSTEMS_COUNT );
                thread_s[ i ] = pthread_create( &w_watcher_thread[ i ], &attrs, DB_WORKER_watcher, ( void* )&t_worker_data[ i ] );
                if( thread_s[ i ] != 0 ) {
                    LOG_print( log, "[%s] WORKER_init( ) failed to create DB_WORKER_watcher thread for \"%s\". Error: %d.\n", TIME_get_gmt(), DATABASE_SYSTEMS[ i ].name, thread_s[ i ] );
                    mysql_library_end();
                    return 0;
                }
                Sleep( 10 );
            } else {
                LOG_print( log, "===========================================================\n[%s] DB_WORKER_init: not spawning LOAD/TRANSFORM thread %d/%d due to previous failure.\n", TIME_get_gmt(), i + 1, DATABASE_SYSTEMS_COUNT );
            }
        }

        for( i = 0; i < DATABASE_SYSTEMS_COUNT; i++ ) {
            if( DATABASE_SYSTEMS[ i ].failure == 0 ) {
                pthread_join( w_watcher_thread[ i ], NULL );
            }
        }
        LOG_print( log, "[%s] DB_WORKER_init: all LOAD/TRANSFORM threads are completed.\n", TIME_get_gmt() );
    }

    mysql_library_end();

    for( i = 0; i < DATABASE_SYSTEMS_COUNT; i++ ) {
        LOG_free( log_object[ i ] );
        free( log_object[ i ] ); log_object[ i ] = NULL;
    }
    free( log_object ); log_object = NULL;

    LOG_print( log, "All threads are terminated.\n" );

    return TRUE;
}

void* DB_WORKER_watcher( void* src_WORKER_DATA ) {
    int                     i = 0;
    WORKER_DATA             *t_worker_data = ( WORKER_DATA* )src_WORKER_DATA;
    DATABASE_SYSTEM         *DATA_SYSTEM = t_worker_data->DATA_SYSTEM;
    DB_SYSTEM_ETL_STEP      curr_etl_step = t_worker_data->step;
    LOG_OBJECT              *log = t_worker_data->log;

    LOG_print( log, "\nDB_WORKER_watcher for \"%s\" (%s) started...\n",
        DATA_SYSTEM->name,
        curr_etl_step == ETL_EXTRACT ? "EXTRACT" : curr_etl_step == ETL_TRANSFORM ? "TRANSFORM/LOAD" : curr_etl_step == ETL_LOAD ? "LOAD/TRANSFORM" : "UNKNOWN"
    );

    if( DATA_SYSTEM->failure == 1 ) {
        LOG_print( log, "[%s] DB_WORKER_watcher put on hold due to previous problems with data processing.\n", TIME_get_gmt() );
        pthread_exit( NULL );
        return FALSE;

    }

    LOG_print( log, "Init DCPAM database connection for \"%s\":\n", DATA_SYSTEM->name );
    if( DATABASE_SYSTEM_DB_init( &DATA_SYSTEM->dcpam_db, log ) == FALSE ) {
        pthread_cancel( w_watcher_thread[ t_worker_data->thread_id ] );
        return FALSE;
    }

    LOG_print( log, "Init \"%s\" database connection...\n", DATA_SYSTEM->name );
    if( DATABASE_SYSTEM_DB_init( &DATA_SYSTEM->system_db, log ) == FALSE ) {
        pthread_cancel( w_watcher_thread[ t_worker_data->thread_id ] );
        return FALSE;
    }

    if( DATA_SYSTEM->staging_db ) {
        LOG_print( log, "Init Staging Area database connection for \"%s\":\n", DATA_SYSTEM->name );
        if( DATABASE_SYSTEM_DB_init( DATA_SYSTEM->staging_db, log ) == FALSE ) {
            pthread_cancel( w_watcher_thread[ t_worker_data->thread_id ] );
            return FALSE;
        }
    } else {
        LOG_print( log, "Staging Area is local.\n" );
    }

    pthread_mutex_lock( &watcher_mutex );

    worker_save_counter += WORKER_WATCHER_SLEEP;

    if( curr_etl_step == ETL_EXTRACT ) {

        LCS_REPORT_send( &APP.lcs_report, "PreETL", DCT_START );

        /* Run PreETL actions. */
        LOG_print( log, "[%s] Started run of PreETL Actions.\n", TIME_get_gmt() );
        for( i = 0; i < DATA_SYSTEM->queries_len; i++ ) {
            if( DATA_SYSTEM->queries[ i ].etl_config.pre_actions ) {
                for( int k = 0; k < DATA_SYSTEM->queries[ i ].etl_config.pre_actions_count; k++ ) {
                    DB_exec(
                        &DATA_SYSTEM->dcpam_db,
                        DATA_SYSTEM->queries[ i ].etl_config.pre_actions[ k ]->sql,
                        strlen( DATA_SYSTEM->queries[ i ].etl_config.pre_actions[ k ]->sql ),
                        NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL,
                        log
                    );
                }
            }
        }
        LOG_print( log, "[%s] Finished run of PreETL Actions.\n", TIME_get_gmt() );

        /* If configured, preload flat file to the DB table. */
        if( DATA_SYSTEM->flat_file ) {
            LOG_print( log, "[%s] Loading data from file %s...\n", TIME_get_gmt(), DATA_SYSTEM->flat_file->name );
            DATA_SYSTEM->failure = FILE_ETL_preload( DATA_SYSTEM, DATA_SYSTEM->flat_file->name, log ) == 0 ? 1 : 0;
            if( DATA_SYSTEM->failure == 0 ) {
                LOG_print( log, "[%s] File %s loaded.\n", TIME_get_gmt(), DATA_SYSTEM->flat_file->name );
            } else {
                LOG_print( log, "[%s] Fatal error: file %s could not be loaded.\n", TIME_get_gmt(), DATA_SYSTEM->flat_file->name );
            }
        }

        if( DATA_SYSTEM->failure == 0 ) {

            /* Extract and store data in the Staging Area / target tables. */
            for( i = 0; i < DATA_SYSTEM->queries_len; i++ ) {

                /* Init query structs */
                qec extract_inserted_callback = DATA_SYSTEM->queries[ i ].etl_config.stage ? ( qec )&_ExtractInserted_callback : ( qec )&_LoadInserted_callback;
                qec extract_deleted_callback = DATA_SYSTEM->queries[ i ].etl_config.stage ? ( qec )&_ExtractDeleted_callback : ( qec )&_LoadDeleted_callback;
                qec extract_modified_callback = DATA_SYSTEM->queries[ i ].etl_config.stage ? ( qec )&_ExtractModified_callback : ( qec )&_LoadModified_callback;

                LOG_print( log, "\t· [EXTRACT] Query #%d: %s\n", i + 1, DATA_SYSTEM->queries[ i ].name );

                int ei_res = DB_CDC_ExtractInserted(
                    &DATA_SYSTEM->queries[ i ].etl_config.extract,
                    &DATA_SYSTEM->system_db,
                    &DATA_SYSTEM->dcpam_db,
                    &extract_inserted_callback,
                    DATA_SYSTEM->queries[ i ].etl_config.stage ? ( void* )DATA_SYSTEM->queries[ i ].etl_config.stage : ( void* )&DATA_SYSTEM->queries[ i ].etl_config.load,
                    DATA_SYSTEM->staging_db ? ( void* )DATA_SYSTEM->staging_db : ( void* )&DATA_SYSTEM->dcpam_db,
                    log
                );
                if( ei_res == 0 ) {
                    LOG_print( log, "[%s] Fatal error: process exited with failure. %s processing is put on hold.\n", TIME_get_gmt(), DATA_SYSTEM->name );
                    DATA_SYSTEM->failure = 1;
                    continue;
                }

                int ed_res = DB_CDC_ExtractDeleted(
                    &DATA_SYSTEM->queries[ i ].etl_config.extract,
                    &DATA_SYSTEM->system_db,
                    &DATA_SYSTEM->dcpam_db,
                    &extract_deleted_callback,
                    DATA_SYSTEM->queries[ i ].etl_config.stage ? ( void* )DATA_SYSTEM->queries[ i ].etl_config.stage : ( void* )&DATA_SYSTEM->queries[ i ].etl_config.load,
                    DATA_SYSTEM->staging_db ? ( void* )DATA_SYSTEM->staging_db : ( void* )&DATA_SYSTEM->dcpam_db,
                    log
                );
                if( ed_res == 0 ) {
                    LOG_print( log, "[%s] Fatal error: process exited with failure. %s processing is put on hold.\n", TIME_get_gmt(), DATA_SYSTEM->name );
                    DATA_SYSTEM->failure = 1;
                    continue;
                }

                int em_res = DB_CDC_ExtractModified(
                    &DATA_SYSTEM->queries[ i ].etl_config.extract,
                    &DATA_SYSTEM->system_db,
                    &DATA_SYSTEM->dcpam_db,
                    &extract_modified_callback,
                    DATA_SYSTEM->queries[ i ].etl_config.stage ? ( void* )DATA_SYSTEM->queries[ i ].etl_config.stage : ( void* )&DATA_SYSTEM->queries[ i ].etl_config.load,
                    DATA_SYSTEM->staging_db ? ( void* )DATA_SYSTEM->staging_db : ( void* )&DATA_SYSTEM->dcpam_db,
                    log
                );
                if( em_res == 0 ) {
                    LOG_print( log, "[%s] Fatal error: process exited with failure. %s processing is put on hold.\n", TIME_get_gmt(), DATA_SYSTEM->name );
                    DATA_SYSTEM->failure = 1;
                    continue;
                }
            }
        }
    }

    else {
        for( i = 0; i < DATA_SYSTEM->queries_len; i++ ) {
            if( 
                ( DATA_SYSTEM->queries[ i ].mode == M_ETL && curr_etl_step == ETL_TRANSFORM ) ||
                ( DATA_SYSTEM->queries[ i ].mode == M_ELT && curr_etl_step == ETL_LOAD )
            ) {
                if( DATA_SYSTEM->queries[ i ].etl_config.transform ) {
                    LOG_print( log, "\t· [TRANSFORM] Query #%d: %s\n", i + 1, DATA_SYSTEM->queries[ i ].name );

                    int ti_res = DB_CDC_TransformInserted(
                        DATA_SYSTEM->queries[ i ].etl_config.transform->inserted,
                        DATA_SYSTEM->queries[ i ].etl_config.transform->inserted_count,
                        &DATA_SYSTEM->dcpam_db,
                        &DATA_SYSTEM->system_db,
                        log
                    );
                    if( ti_res == 0 ) {
                        LOG_print( log, "[%s] Fatal error: process exited with failure. %s processing is put on hold.\n", TIME_get_gmt(), DATA_SYSTEM->name );
                        DATA_SYSTEM->failure = 1;
                        continue;
                    }

                    int td_res = DB_CDC_TransformDeleted(
                        DATA_SYSTEM->queries[ i ].etl_config.transform->deleted,
                        DATA_SYSTEM->queries[ i ].etl_config.transform->deleted_count,
                        &DATA_SYSTEM->dcpam_db,
                        &DATA_SYSTEM->system_db,
                        log
                    );
                    if( td_res == 0 ) {
                        LOG_print( log, "[%s] Fatal error: process exited with failure. %s processing is put on hold.\n", TIME_get_gmt(), DATA_SYSTEM->name );
                        DATA_SYSTEM->failure = 1;
                        continue;
                    }

                    int tm_res = DB_CDC_TransformModified(
                        DATA_SYSTEM->queries[ i ].etl_config.transform->modified,
                        DATA_SYSTEM->queries[ i ].etl_config.transform->modified_count,
                        &DATA_SYSTEM->dcpam_db,
                        &DATA_SYSTEM->system_db,
                        log
                    );
                    if( tm_res == 0 ) {
                        LOG_print( log, "[%s] Fatal error: process exited with failure. %s processing is put on hold.\n", TIME_get_gmt(), DATA_SYSTEM->name );
                        DATA_SYSTEM->failure = 1;
                        continue;
                    }
                }
            } else if( 
                ( DATA_SYSTEM->queries[ i ].mode == M_ETL && curr_etl_step == ETL_LOAD ) ||
                ( DATA_SYSTEM->queries[ i ].mode == M_ELT && curr_etl_step == ETL_TRANSFORM )
            ) {
                LOG_print( log, "\t· [LOAD] Query #%d: %s\n", i + 1, DATA_SYSTEM->queries[ i ].name );

                int li_res = DB_CDC_LoadInserted(
                    &DATA_SYSTEM->queries[ i ].etl_config.load,
                    DATA_SYSTEM->staging_db ? DATA_SYSTEM->staging_db : &DATA_SYSTEM->dcpam_db,
                    &DATA_SYSTEM->dcpam_db,
                    log
                );
                if( li_res == 0 ) {
                    LOG_print( log, "[%s] Fatal error: process exited with failure. %s processing is put on hold.\n", TIME_get_gmt(), DATA_SYSTEM->name );
                    DATA_SYSTEM->failure = 1;
                    continue;
                }

                int ld_res = DB_CDC_LoadDeleted(
                    &DATA_SYSTEM->queries[ i ].etl_config.load,
                    DATA_SYSTEM->staging_db ? DATA_SYSTEM->staging_db : &DATA_SYSTEM->dcpam_db,
                    &DATA_SYSTEM->dcpam_db,
                    log
                );
                if( ld_res == 0 ) {
                    LOG_print( log, "[%s] Fatal error: process exited with failure. %s processing is put on hold.\n", TIME_get_gmt(), DATA_SYSTEM->name );
                    DATA_SYSTEM->failure = 1;
                    continue;
                }

                int lm_res = DB_CDC_LoadModified(
                    &DATA_SYSTEM->queries[ i ].etl_config.load,
                    DATA_SYSTEM->staging_db ? DATA_SYSTEM->staging_db : &DATA_SYSTEM->dcpam_db,
                    &DATA_SYSTEM->dcpam_db,
                    log
                );
                if( lm_res == 0 ) {
                    LOG_print( log, "[%s] Fatal error: process exited with failure. %s processing is put on hold.\n", TIME_get_gmt(), DATA_SYSTEM->name );
                    DATA_SYSTEM->failure = 1;
                    continue;
                }

                /* Run PostETL actions. */
                LOG_print( log, "[%s] Started run of PostETL Actions.\n", TIME_get_gmt() );
                for( int j = 0; j < DATA_SYSTEM->queries_len; j++ ) {
                    if( DATA_SYSTEM->queries[ j ].etl_config.post_actions ) {
                        for( int k = 0; k < DATA_SYSTEM->queries[ j ].etl_config.post_actions_count; k++ ) {
                            DB_exec(
                                &DATA_SYSTEM->dcpam_db,
                                DATA_SYSTEM->queries[ j ].etl_config.post_actions[ k ]->sql,
                                strlen( DATA_SYSTEM->queries[ j ].etl_config.post_actions[ k ]->sql ),
                                NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL,
                                log
                            );
                        }
                    }
                }
                LOG_print( log, "[%s] Finished run of PostETL Actions.\n", TIME_get_gmt() );
            }
        }
    }

    LOG_print( log, "DB_WORKER_watcher for \"%s\" with process type \"%s\" finished.\n",
                DATA_SYSTEM->name,
                curr_etl_step == ETL_EXTRACT ? "EXTRACT" : curr_etl_step == ETL_TRANSFORM ? "TRANSFORM/LOAD" : curr_etl_step == ETL_LOAD ? "LOAD/TRANSFORM" : "UNKNOWN"
    );

    pthread_mutex_unlock( &watcher_mutex );
    if( curr_etl_step == ETL_LOAD ) {
        if( APP.run_once == 1 ) {
            app_terminated = 1;
        } else {
            LOG_print( log, "[%s] Workflow finished.\n", TIME_get_gmt() );
            Sleep( WORKER_WATCHER_SLEEP );
        }
    }

    if( app_terminated == 1 ) {
        LOG_print( log, "[%s]\t- Thread exit handler executed for \"%s\".\n", TIME_get_gmt(), DATA_SYSTEM->name );
        DATABASE_SYSTEM_close( DATA_SYSTEM, log );
        pthread_exit( NULL );
    }

    if( DATA_SYSTEM->name != NULL ) {
        DATABASE_SYSTEM_DB_close( &DATA_SYSTEM->dcpam_db, log );
        DATABASE_SYSTEM_DB_close( &DATA_SYSTEM->system_db, log );

        if( DATA_SYSTEM->staging_db ) {
            DATABASE_SYSTEM_DB_close( DATA_SYSTEM->staging_db, log );
        }
    }
    pthread_exit( NULL );
    return FALSE;
}


int DB_WORKER_shutdown( LOG_OBJECT *log ) {
    int         i = 0;

    LOG_print( log, "[%s] WORKER_shutdown (app_terminated=%d):\n", TIME_get_gmt(), app_terminated );
    for( i = 0; i < DATABASE_SYSTEMS_COUNT; i++ ) {
        LOG_print( log, "\t- \"%s\"\n", DATABASE_SYSTEMS[ i ].name );
        LOG_print( log, "[%s] DB_WORKER_watcher for \"%s\" set to termination.\n", TIME_get_gmt(), DATABASE_SYSTEMS[ i ].name );
    }

    return TRUE;
}
