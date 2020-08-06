#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include "../include/utils/log.h"
#include "../include/core/db/worker.h"
#include "../include/core/schema.h"
#include "../include/third-party/cJSON.h"
#include "../include/utils/memory.h"
#include "../include/utils/time.h"
#include "../include/utils/strings.h"
#include "../include/utils/filesystem.h"
#include "../include/core/db/system.h"

#pragma warning( disable : 6031 )

char                    app_path[ MAX_PATH_LENGTH + 1 ];
char                    LOG_filename[ MAX_PATH_LENGTH ];
extern int              app_terminated = 0;

extern DATABASE_SYSTEM  DATABASE_SYSTEMS[ MAX_DATA_SYSTEMS ];
extern int              DATABASE_SYSTEMS_COUNT;
extern DCPAM_APP       APP;

void DCPAM_free_configuration( void );


void app_terminate( void ) {
    LOG_print( "\r\n[%s] Received termination signal.\n", TIME_get_gmt() );
    if( app_terminated == 0 ) {
        app_terminated = 1;
        printf( "\r" );
        DB_WORKER_shutdown();
        DCPAM_free_configuration();
        LOG_print( "[%s] DCPAM graceful shutdown finished. Waiting for all threads to terminate...\n", TIME_get_gmt() );
    }

    return;
}

void DCPAM_free_configuration( void ) {

    DATABASE_SYSTEM_DB_free( &APP.DB );
    if( APP.STAGING ) {
        DATABASE_SYSTEM_DB_free( APP.STAGING );
        free( APP.STAGING ); APP.STAGING = NULL;
    }

    if( APP.version != NULL ) { free( APP.version ); APP.version = NULL; }
    if( APP.name != NULL ) { free( APP.name ); APP.name = NULL; }

    for( int i = 0; i < APP.DATA_len; i++ ) {
        if( APP.DATA[ i ].id != NULL ) { free( APP.DATA[ i ].id ); APP.DATA[ i ].id = NULL; }
        if( APP.DATA[ i ].name != NULL ) { free( APP.DATA[ i ].name ); APP.DATA[ i ].name = NULL; }
        if( APP.DATA[ i ].db_table_name != NULL ) { free( APP.DATA[ i ].db_table_name ); APP.DATA[ i ].db_table_name = NULL; }
        if( APP.DATA[ i ].description != NULL ) { free( APP.DATA[ i ].description ); APP.DATA[ i ].description = NULL; }

        for( int j = 0; j < APP.DATA[ i ].actions_len; j++ ) {
            if( APP.DATA[ i ].actions[ j ].name != NULL ) { free( APP.DATA[ i ].actions[ j ].name ); APP.DATA[ i ].actions[ j ].name = NULL; }
            if( APP.DATA[ i ].actions[ j ].description != NULL ) { free( APP.DATA[ i ].actions[ j ].description ); APP.DATA[ i ].actions[ j ].description = NULL; }
            if( APP.DATA[ i ].actions[ j ].condition != NULL ) { free( APP.DATA[ i ].actions[ j ].condition ); APP.DATA[ i ].actions[ j ].condition = NULL; }
            if( APP.DATA[ i ].actions[ j ].sql != NULL ) { free( APP.DATA[ i ].actions[ j ].sql ); APP.DATA[ i ].actions[ j ].sql = NULL; }
        }
    }
}

int DCPAM_load_configuration( const char* filename ) {
    cJSON* config_json = NULL;

    cJSON* cfg_app = NULL;
    cJSON* cfg_app_version = NULL;
    cJSON* cfg_app_name = NULL;
    cJSON* cfg_app_run_once = NULL;
    cJSON* cfg_app_db = NULL;
    cJSON* cfg_app_db_ip = NULL;
    cJSON* cfg_app_db_port = NULL;
    cJSON* cfg_app_db_driver = NULL;
    cJSON* cfg_app_db_user = NULL;
    cJSON* cfg_app_db_password = NULL;
    cJSON* cfg_app_db_connection_string = NULL;
    cJSON* cfg_app_db_db = NULL;
    cJSON* cfg_app_db_name = NULL;

    cJSON* cfg_app_sa = NULL;
    cJSON* cfg_app_sa_ip = NULL;
    cJSON* cfg_app_sa_port = NULL;
    cJSON* cfg_app_sa_driver = NULL;
    cJSON* cfg_app_sa_user = NULL;
    cJSON* cfg_app_sa_password = NULL;
    cJSON* cfg_app_sa_connection_string = NULL;
    cJSON* cfg_app_sa_db = NULL;
    cJSON* cfg_app_sa_name = NULL;

    cJSON* cfg_system_array = NULL;
    cJSON* cfg_system_enabled = NULL;
    cJSON* cfg_system_name = NULL;
    cJSON* cfg_system_minute_ratio = NULL;
    cJSON* cfg_system_ip = NULL;
    cJSON* cfg_system_port = NULL;
    cJSON* cfg_system_driver = NULL;
    cJSON* cfg_system_user = NULL;
    cJSON* cfg_system_db = NULL;
    cJSON* cfg_system_password = NULL;
    cJSON* cfg_system_connection_string = NULL;
    cJSON* cfg_system_db_name = NULL;
    cJSON* cfg_system_queries_array = NULL;
    cJSON* cfg_system_query_item = NULL;
    cJSON* cfg_system_query_item_name = NULL;
    cJSON* cfg_system_query_item_mode = NULL;

    cJSON* cfg_system_query_item_etl = NULL;
    cJSON* cfg_system_query_item_etl_pre_actions = NULL;
    cJSON* cfg_system_query_item_etl_pre_action_item = NULL;
    cJSON* cfg_system_query_item_etl_extract = NULL;
    cJSON* cfg_system_query_item_etl_extract_inserted = NULL;
    cJSON* cfg_system_query_item_etl_extract_inserted_primary_db = NULL;
    cJSON* cfg_system_query_item_etl_extract_inserted_primary_db_sql = NULL;
    cJSON* cfg_system_query_item_etl_extract_inserted_secondary_db = NULL;
    cJSON* cfg_system_query_item_etl_extract_inserted_secondary_db_sql = NULL;
    cJSON* cfg_system_query_item_etl_extract_deleted = NULL;
    cJSON* cfg_system_query_item_etl_extract_deleted_primary_db_sql = NULL;
    cJSON* cfg_system_query_item_etl_extract_deleted_primary_db = NULL;
    cJSON* cfg_system_query_item_etl_extract_deleted_secondary_db_sql = NULL;
    cJSON* cfg_system_query_item_etl_extract_deleted_secondary_db = NULL;
    cJSON* cfg_system_query_item_etl_extract_modified = NULL;
    cJSON* cfg_system_query_item_etl_extract_modified_primary_db_sql = NULL;
    cJSON* cfg_system_query_item_etl_extract_modified_primary_db = NULL;
    cJSON* cfg_system_query_item_etl_extract_modified_secondary_db_sql = NULL;
    cJSON* cfg_system_query_item_etl_extract_modified_secondary_db = NULL;

    cJSON* cfg_system_query_item_etl_stage = NULL;
    cJSON* cfg_system_query_item_etl_stage_inserted = NULL;
    cJSON* cfg_system_query_item_etl_stage_inserted_sql = NULL;
    cJSON* cfg_system_query_item_etl_stage_inserted_extracted_values_array = NULL;
    cJSON* cfg_system_query_item_etl_stage_inserted_extracted_values_item = NULL;
    cJSON* cfg_system_query_item_etl_stage_deleted = NULL;
    cJSON* cfg_system_query_item_etl_stage_deleted_sql = NULL;
    cJSON* cfg_system_query_item_etl_stage_deleted_extracted_values_array = NULL;
    cJSON* cfg_system_query_item_etl_stage_deleted_extracted_values_item = NULL;
    cJSON* cfg_system_query_item_etl_stage_modified = NULL;
    cJSON* cfg_system_query_item_etl_stage_modified_sql = NULL;
    cJSON* cfg_system_query_item_etl_stage_modified_extracted_values_array = NULL;
    cJSON* cfg_system_query_item_etl_stage_modified_extracted_values_item = NULL;
    cJSON* cfg_system_query_item_etl_stage_reset = NULL;
    
    cJSON* cfg_system_query_item_etl_transform = NULL;
    cJSON* cfg_system_query_item_etl_transform_inserted_array = NULL;
    cJSON* cfg_system_query_item_etl_transform_inserted_item = NULL;
    cJSON* cfg_system_query_item_etl_transform_inserted_module = NULL;
    cJSON* cfg_system_query_item_etl_transform_inserted_staged_data = NULL;
    cJSON* cfg_system_query_item_etl_transform_inserted_source_system_update = NULL;
    cJSON* cfg_system_query_item_etl_transform_deleted_array = NULL;
    cJSON* cfg_system_query_item_etl_transform_deleted_item = NULL;
    cJSON* cfg_system_query_item_etl_transform_deleted_module = NULL;
    cJSON* cfg_system_query_item_etl_transform_deleted_staged_data = NULL;
    cJSON* cfg_system_query_item_etl_transform_deleted_source_system_update = NULL;
    cJSON* cfg_system_query_item_etl_transform_modified_array = NULL;
    cJSON* cfg_system_query_item_etl_transform_modified_item = NULL;
    cJSON* cfg_system_query_item_etl_transform_modified_module = NULL;
    cJSON* cfg_system_query_item_etl_transform_modified_staged_data = NULL;
    cJSON* cfg_system_query_item_etl_transform_modified_source_system_update = NULL;
    
    cJSON* cfg_system_query_item_etl_load = NULL;
    cJSON* cfg_system_query_item_etl_load_inserted = NULL;
    cJSON* cfg_system_query_item_etl_load_inserted_input_data_sql = NULL;
    cJSON* cfg_system_query_item_etl_load_inserted_output_data_sql = NULL;
    cJSON* cfg_system_query_item_etl_load_inserted_extracted_values_array = NULL;
    cJSON* cfg_system_query_item_etl_load_inserted_extracted_values_item = NULL;
    cJSON* cfg_system_query_item_etl_load_deleted = NULL;
    cJSON* cfg_system_query_item_etl_load_deleted_input_data_sql = NULL;
    cJSON* cfg_system_query_item_etl_load_deleted_output_data_sql = NULL;
    cJSON* cfg_system_query_item_etl_load_deleted_extracted_values_array = NULL;
    cJSON* cfg_system_query_item_etl_load_deleted_extracted_values_item = NULL;
    cJSON* cfg_system_query_item_etl_load_modified = NULL;
    cJSON* cfg_system_query_item_etl_load_modified_input_data_sql = NULL;
    cJSON* cfg_system_query_item_etl_load_modified_output_data_sql = NULL;
    cJSON* cfg_system_query_item_etl_load_modified_extracted_values_array = NULL;
    cJSON* cfg_system_query_item_etl_load_modified_extracted_values_item = NULL;
    cJSON* cfg_system_query_item_etl_post_actions = NULL;
    cJSON* cfg_system_query_item_etl_post_action_item = NULL;

    cJSON* cfg_system_query_item_columns = NULL;
    cJSON* cfg_system_query_item_columns_name = NULL;

    cJSON* cfg_system_info = NULL;

    cJSON* array_value = NULL;
    int                         result = 0;


    char*                       config_string = NULL;


    LOG_print( "[%s] DCPAM_load_configuration( %s ).\n", TIME_get_gmt(), filename );

    config_string = file_get_content( filename );

    if( config_string ) {
        config_json = cJSON_Parse( config_string );
        if( config_json ) {

            DATABASE_SYSTEM_QUERY **tmp_queries = SAFEMALLOC( MAX_SYSTEM_QUERIES * sizeof * tmp_queries, __FILE__, __LINE__ );

            cfg_app = cJSON_GetObjectItem( config_json, "app" );
            if( cfg_app ) {

                cfg_app_name = cJSON_GetObjectItem( cfg_app, "name" );
                if( cfg_app_name ) {
                    size_t str_len = strlen( cfg_app_name->valuestring );
                    APP.name = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                    snprintf( APP.name, str_len + 1, cfg_app_name->valuestring );
                } else {
                    LOG_print( "ERROR: \"app.name\" key not found.\n" );
                    cJSON_Delete( config_json );
                    free( config_string ); config_string = NULL;
                    return FALSE;
                }

                cfg_app_run_once = cJSON_GetObjectItem( cfg_app, "run_once" );
                if( cfg_app_run_once ) {
                    APP.run_once = cfg_app_run_once->valueint;
                    LOG_print( "NOTICE: DCPAM is in %s run mode.\n", APP.run_once == 1 ? "one-time" : "persistent" );
                } else {
                    LOG_print( "NOTICE: \"run_once\" key not found. DCPAM is in persistent run mode.\n" );
                    APP.run_once = 0;
                }

                cfg_app_version = cJSON_GetObjectItem( cfg_app, "version" );
                if( cfg_app_version ) {
                    size_t str_len = strlen( cfg_app_version->valuestring );
                    APP.version = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                    snprintf( APP.version, str_len + 1, cfg_app_version->valuestring );
                    LOG_print( "v%s.\n", APP.version );
                } else {
                    LOG_print( "ERROR: \"app.version\" key not found.\n" );
                    cJSON_Delete( config_json );
                    free( config_string ); config_string = NULL;
                    return FALSE;
                }

                cfg_app_db = cJSON_GetObjectItem( cfg_app, "DB" );
                if( cfg_app_db ) {

                    cfg_app_db_ip = cJSON_GetObjectItem( cfg_app_db, "ip" );
                    if( cfg_app_db_ip == NULL ) {
                        LOG_print( "ERROR: \"app.DB.ip\" key not found.\n" );
                    }

                    cfg_app_db_port = cJSON_GetObjectItem( cfg_app_db, "port" );
                    if( cfg_app_db_port == NULL ) {
                        LOG_print( "ERROR: \"app.DB.port\" key not found.\n" );
                    }

                    cfg_app_db_driver = cJSON_GetObjectItem( cfg_app_db, "driver" );
                    if( cfg_app_db_driver == NULL ) {
                        LOG_print( "ERROR: \"app.DB.driver\" key not found.\n" );
                    }

                    cfg_app_db_user = cJSON_GetObjectItem( cfg_app_db, "user" );
                    if( cfg_app_db_user == NULL ) {
                        LOG_print( "ERROR: \"app.DB.user\" key not found.\n" );
                    }

                    cfg_app_db_password = cJSON_GetObjectItem( cfg_app_db, "password" );
                    if( cfg_app_db_password == NULL ) {
                        LOG_print( "ERROR: \"app.DB.password\" key not found.\n" );
                    }

                    cfg_app_db_connection_string = cJSON_GetObjectItem( cfg_app_db, "connection_string" );
                    if( cfg_app_db_connection_string == NULL ) {
                        LOG_print( "ERROR: \"app.DB.connection_string\" key not found.\n" );
                    }

                    cfg_app_db_db = cJSON_GetObjectItem( cfg_app_db, "db" );
                    if( cfg_app_db_db == NULL ) {
                        LOG_print( "ERROR: \"app.DB.db\" key not found.\n" );
                    }

                    cfg_app_db_name = cJSON_GetObjectItem( cfg_app_db, "name" );
                    if( cfg_app_db_name == NULL ) {
                        LOG_print( "Error: \"app.DB.name\" key not found.\n" );
                    }

                    DATABASE_SYSTEM_DB_add(
                        cfg_app_db_ip->valuestring,
                        cfg_app_db_port->valueint,
                        cfg_app_db_driver->valueint,
                        cfg_app_db_user->valuestring,
                        cfg_app_db_password->valuestring,
                        cfg_app_db_db->valuestring,
                        cfg_app_db_connection_string->valuestring,
                        &APP.DB,
                        cfg_app_db_name->valuestring,
                        TRUE
                    );
                } else {
                    LOG_print( "ERROR: \"app.DB\" key not found.\n" );
                    cJSON_Delete( config_json );
                    free( config_string ); config_string = NULL;
                    return FALSE;
                }

                /* Staging Area - optional */
                cfg_app_sa = cJSON_GetObjectItem( cfg_app, "STAGING" );
                if( cfg_app_sa ) {

                    APP.STAGING = SAFEMALLOC( sizeof( DATABASE_SYSTEM_DB ), __FILE__, __LINE__ );

                    cfg_app_sa_ip = cJSON_GetObjectItem( cfg_app_sa, "ip" );
                    if( cfg_app_sa_ip == NULL ) {
                        LOG_print( "ERROR: \"app.STAGING.ip\" key not found.\n" );
                    }

                    cfg_app_sa_port = cJSON_GetObjectItem( cfg_app_sa, "port" );
                    if( cfg_app_sa_port == NULL ) {
                        LOG_print( "ERROR: \"app.STAGING.port\" key not found.\n" );
                    }

                    cfg_app_sa_driver = cJSON_GetObjectItem( cfg_app_sa, "driver" );
                    if( cfg_app_sa_driver == NULL ) {
                        LOG_print( "ERROR: \"app.STAGING.driver\" key not found.\n" );
                    }

                    cfg_app_sa_user = cJSON_GetObjectItem( cfg_app_sa, "user" );
                    if( cfg_app_sa_user == NULL ) {
                        LOG_print( "ERROR: \"app.STAGING.user\" key not found.\n" );
                    }

                    cfg_app_sa_password = cJSON_GetObjectItem( cfg_app_sa, "password" );
                    if( cfg_app_sa_password == NULL ) {
                        LOG_print( "ERROR: \"app.STAGING.password\" key not found.\n" );
                    }

                    cfg_app_sa_connection_string = cJSON_GetObjectItem( cfg_app_sa, "connection_string" );
                    if( cfg_app_sa_connection_string == NULL ) {
                        LOG_print( "ERROR: \"app.STAGING.connection_string\" key not found.\n" );
                    }

                    cfg_app_sa_db = cJSON_GetObjectItem( cfg_app_sa, "db" );
                    if( cfg_app_sa_db == NULL ) {
                        LOG_print( "ERROR: \"app.STAGING.db\" key not found.\n" );
                    }

                    cfg_app_sa_name = cJSON_GetObjectItem( cfg_app_sa, "name" );
                    if( cfg_app_sa_name == NULL ) {
                        LOG_print( "ERROR: \"app.STAGING.name\" key not found.\n" );
                    }

                    DATABASE_SYSTEM_DB_add(
                        cfg_app_sa_ip->valuestring,
                        cfg_app_sa_port->valueint,
                        cfg_app_sa_driver->valueint,
                        cfg_app_sa_user->valuestring,
                        cfg_app_sa_password->valuestring,
                        cfg_app_sa_db->valuestring,
                        cfg_app_sa_connection_string->valuestring,
                        APP.STAGING,
                        cfg_app_sa_name->valuestring,
                        TRUE
                    );
                } else {
                    APP.STAGING = NULL;
                    LOG_print( "NOTICE: \"app.STAGING\" key not found. Staging Area is local.\n" );
                    /*cJSON_Delete( config_json );
                    free( config_string ); config_string = NULL;
                    return FALSE;*/
                }
            } else {
                LOG_print( "ERROR: \"app\" key not found.\n " );
                cJSON_Delete( config_json );
                free( config_string ); config_string = NULL;
                return FALSE;
            }

            cfg_system_array = cJSON_GetObjectItem( config_json, "system" );
            if( cfg_system_array ) {
                for( int i = 0; i < cJSON_GetArraySize( cfg_system_array ); i++ ) {

                    array_value = cJSON_GetArrayItem( cfg_system_array, i );

                    cfg_system_enabled = cJSON_GetObjectItem( array_value, "enabled" );
                    if( cfg_system_enabled == NULL ) {
                        LOG_print( "ERROR: \"system[%d].enabled\" key not found.\n", i );
                        cJSON_Delete( config_json );
                        free( config_string ); config_string = NULL;
                        return FALSE;
                    } else {
                        if( cfg_system_enabled->valueint == 0 ) {
                            continue;
                        }
                    }

                    cfg_system_name = cJSON_GetObjectItem( array_value, "name" );
                    if( cfg_system_name == NULL ) {
                        LOG_print( "ERROR: \"system[%d].name\" key not found.\n", i );
                        cJSON_Delete( config_json );
                        free( config_string ); config_string = NULL;
                        return FALSE;
                    }
                    LOG_print( "\n[%s] Loading system data: %s.\n", TIME_get_gmt(), cfg_system_name->valuestring );

                    cfg_system_info = cJSON_GetObjectItem( array_value, "DB" );
                    if( cfg_system_info == NULL ) {
                        LOG_print( "ERROR: \"system[%d].DB\" key not found.\n", i );
                        cJSON_Delete( config_json );
                        free( config_string ); config_string = NULL;
                        return FALSE;
                    }

                    cfg_system_ip = cJSON_GetObjectItem( cfg_system_info, "ip" );
                    if( cfg_system_ip == NULL ) {
                        LOG_print( "ERROR: \"system[%d].DB.ip\" key not found.\n", i );
                        cJSON_Delete( config_json );
                        free( config_string ); config_string = NULL;
                        return FALSE;
                    }
                    cfg_system_port = cJSON_GetObjectItem( cfg_system_info, "port" );
                    if( cfg_system_port == NULL ) {
                        LOG_print( "ERROR: \"system[%d].DB.port\" key not found.\n", i );
                        cJSON_Delete( config_json );
                        free( config_string ); config_string = NULL;
                        return FALSE;
                    }
                    cfg_system_driver = cJSON_GetObjectItem( cfg_system_info, "driver" );
                    if( cfg_system_driver == NULL ) {
                        LOG_print( "ERROR: \"system[%d].DB.driver\" key not found.\n", i );
                        cJSON_Delete( config_json );
                        free( config_string ); config_string = NULL;
                        return FALSE;
                    }
                    cfg_system_user = cJSON_GetObjectItem( cfg_system_info, "user" );
                    if( cfg_system_user == NULL ) {
                        LOG_print( "ERROR: \"system[%d].DB.user\" key not found.\n", i );
                        cJSON_Delete( config_json );
                        free( config_string ); config_string = NULL;
                        return FALSE;
                    }
                    cfg_system_db = cJSON_GetObjectItem( cfg_system_info, "db" );
                    if( cfg_system_db == NULL ) {
                        LOG_print( "ERROR: \"system[%d].DB.db\" key not found.\n", i );
                        cJSON_Delete( config_json );
                        free( config_string ); config_string = NULL;
                        return FALSE;
                    }
                    cfg_system_password = cJSON_GetObjectItem( cfg_system_info, "password" );
                    if( cfg_system_password == NULL ) {
                        LOG_print( "ERROR: \"system[%d].DB.password\" key not found.\n", i );
                        cJSON_Delete( config_json );
                        free( config_string ); config_string = NULL;
                        return FALSE;
                    }
                    cfg_system_connection_string = cJSON_GetObjectItem( cfg_system_info, "connection_string" );
                    if( cfg_system_connection_string == NULL ) {
                        LOG_print( "ERROR: \"system[%d].DB.connection_string\" key not found.\n", i );
                        cJSON_Delete( config_json );
                        free( config_string ); config_string = NULL;
                        return FALSE;
                    }

                    cfg_system_db_name = cJSON_GetObjectItem( cfg_system_info, "name" );
                    if( cfg_system_db_name == NULL ) {
                        LOG_print( "ERROR: \"system[%d].DB.name\" key not found.\n", i );
                        cJSON_Delete( config_json );
                        free( config_string ); config_string = NULL;
                        return FALSE;
                    }

                    int tmp_queries_count = 0;
                    int tmp_data_types_len = 0;

                    cfg_system_queries_array = cJSON_GetObjectItem( array_value, "queries" );
                    if( cfg_system_queries_array == NULL ) {
                        LOG_print( "ERROR: \"system[%d].queries\" key not found.\n", i );
                        cJSON_Delete( config_json );
                        free( config_string ); config_string = NULL;
                        return FALSE;
                    }
                    for( int j = 0; j < cJSON_GetArraySize( cfg_system_queries_array ); j++ ) {
                        cfg_system_query_item = cJSON_GetArrayItem( cfg_system_queries_array, j );

                        DB_SYSTEM_ETL *tmp_cdc = SAFEMALLOC( sizeof(DB_SYSTEM_ETL), __FILE__, __LINE__ );

                        cfg_system_query_item_name = cJSON_GetObjectItem( cfg_system_query_item, "name" );
                        if( cfg_system_query_item_name == NULL ) {
                            LOG_print( "ERROR: \"system[%d].queries[%d].name\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }

                        cfg_system_query_item_mode = cJSON_GetObjectItem( cfg_system_query_item, "mode" );
                        if( cfg_system_query_item_mode == NULL ) {
                            LOG_print( "NOTICE: \"system[%d].queries[%d].mode\" key not found. Default mode is \"ETL\" (1).\n", i, j );
                        }

                        cfg_system_query_item_etl = cJSON_GetObjectItem( cfg_system_query_item, "etl" );
                        if( cfg_system_query_item_etl == NULL ) {
                            LOG_print( "ERROR: \"system[%d].queries[%d].etl\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        /*
                            etl.pre_actions
                        */
                        cfg_system_query_item_etl_pre_actions = cJSON_GetObjectItem( cfg_system_query_item_etl, "pre_actions" );
                        if( cfg_system_query_item_etl_pre_actions == NULL ) {
                            LOG_print( "NOTICE: No PreCDC actions defined.\n" );
                            tmp_cdc->pre_actions = NULL;
                            tmp_cdc->pre_actions_count = 0;
                        } else {
                            tmp_cdc->pre_actions_count = cJSON_GetArraySize( cfg_system_query_item_etl_pre_actions );
                            tmp_cdc->pre_actions = SAFEMALLOC( tmp_cdc->pre_actions_count * sizeof * tmp_cdc->pre_actions, __FILE__, __LINE__ );

                            for( int k = 0; k < tmp_cdc->pre_actions_count; k++ ) {
                                cfg_system_query_item_etl_pre_action_item = cJSON_GetArrayItem( cfg_system_query_item_etl_pre_actions, k );
                                if( cfg_system_query_item_etl_pre_action_item == NULL ) {
                                    LOG_print( "WARNING: element at position %d in PreCDC actions is invalid!\n", k );
                                } else {
                                    tmp_cdc->pre_actions[ k ] = SAFEMALLOC( sizeof( DB_SYSTEM_ETL_PRE ), __FILE__, __LINE__ );
                                    size_t str_len = strlen( cfg_system_query_item_etl_pre_action_item->valuestring );
                                    tmp_cdc->pre_actions[ k ]->sql = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                                    strncpy(
                                        tmp_cdc->pre_actions[ k ]->sql,
                                        cfg_system_query_item_etl_pre_action_item->valuestring,
                                        str_len
                                    );
                                }
                            }
                        }

                        /*
                            etl.post_actions
                        */
                        cfg_system_query_item_etl_post_actions = cJSON_GetObjectItem( cfg_system_query_item_etl, "post_actions" );
                        if( cfg_system_query_item_etl_post_actions == NULL ) {
                            LOG_print( "NOTICE: No PostCDC actions defined.\n" );
                            tmp_cdc->post_actions = NULL;
                            tmp_cdc->post_actions_count = 0;
                        } else {
                            tmp_cdc->post_actions_count = cJSON_GetArraySize( cfg_system_query_item_etl_post_actions );
                            tmp_cdc->post_actions = SAFEMALLOC( tmp_cdc->post_actions_count * sizeof * tmp_cdc->post_actions, __FILE__, __LINE__ );

                            for( int k = 0; k < tmp_cdc->post_actions_count; k++ ) {
                                cfg_system_query_item_etl_post_action_item = cJSON_GetArrayItem( cfg_system_query_item_etl_post_actions, k );
                                if( cfg_system_query_item_etl_post_action_item == NULL ) {
                                    LOG_print( "WARNING: element at position %d in PreCDC actions is invalid!\n", k );
                                } else {
                                    tmp_cdc->post_actions[ k ] = SAFEMALLOC( sizeof( DB_SYSTEM_ETL_POST ), __FILE__, __LINE__ );
                                    size_t str_len = strlen( cfg_system_query_item_etl_post_action_item->valuestring );
                                    tmp_cdc->post_actions[ k ]->sql = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                                    strncpy(
                                        tmp_cdc->post_actions[ k ]->sql,
                                        cfg_system_query_item_etl_post_action_item->valuestring,
                                        str_len
                                    );
                                }
                            }
                        }
                        /*
                            etl.extract
                        */
                        cfg_system_query_item_etl_extract = cJSON_GetObjectItem( cfg_system_query_item_etl, "extract" );
                        if( cfg_system_query_item_etl_extract == NULL ) {
                            LOG_print( "ERROR: \"system[%d].queries[%d].etl.extract\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        /*
                            etl.extract.inserted
                        */
                        cfg_system_query_item_etl_extract_inserted = cJSON_GetObjectItem( cfg_system_query_item_etl_extract, "inserted" );
                        if( cfg_system_query_item_etl_extract_inserted == NULL ) {
                            LOG_print( "ERROR: \"system[%d].queries[%d].etl.extract.inserted\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        /*
                            etl.extract.inserted.primary_db
                        */
                        cfg_system_query_item_etl_extract_inserted_primary_db = cJSON_GetObjectItem( cfg_system_query_item_etl_extract_inserted, "primary_db" );
                        if( cfg_system_query_item_etl_extract_inserted_primary_db == NULL ) {
                            LOG_print( "ERROR: \"system[%d].queries[%d].etl.extract.inserted.primary_db\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        size_t str_len = strlen( cfg_system_query_item_etl_extract_inserted_primary_db->valuestring );
                        tmp_cdc->extract.inserted.primary_db = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                        snprintf( tmp_cdc->extract.inserted.primary_db, str_len + 1, cfg_system_query_item_etl_extract_inserted_primary_db->valuestring );
                        /*strncpy(
                            tmp_cdc->extract.inserted.primary_db,
                            cfg_system_query_item_etl_extract_inserted_primary_db->valuestring,
                            str_len
                        );*/

                        /*
                            etl.extract.inserted.primary_db_sql
                        */
                        cfg_system_query_item_etl_extract_inserted_primary_db_sql = cJSON_GetObjectItem( cfg_system_query_item_etl_extract_inserted, "primary_db_sql" );
                        if( cfg_system_query_item_etl_extract_inserted_primary_db_sql == NULL ) {
                            LOG_print( "ERROR: \"system[%d].queries[%d].etl.extract.inserted.primary_db_sql\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        size_t str_len2 = strlen( cfg_system_query_item_etl_extract_inserted_primary_db_sql->valuestring );
                        tmp_cdc->extract.inserted.primary_db_sql_len = str_len2;
                        tmp_cdc->extract.inserted.primary_db_sql = SAFECALLOC( str_len2 + 1, sizeof( char ), __FILE__, __LINE__ );
                        strncpy(
                            tmp_cdc->extract.inserted.primary_db_sql,
                            cfg_system_query_item_etl_extract_inserted_primary_db_sql->valuestring,
                            str_len2
                        );
                        /*
                            etl.extract.secondary_db
                        */
                        cfg_system_query_item_etl_extract_inserted_secondary_db = cJSON_GetObjectItem( cfg_system_query_item_etl_extract_inserted, "secondary_db" );
                        if( cfg_system_query_item_etl_extract_inserted_secondary_db == NULL ) {
                            LOG_print( "ERROR: \"system[%d].queries[%d].etl.extract.inserted.secondary_db\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        size_t str_len3 = strlen( cfg_system_query_item_etl_extract_inserted_secondary_db->valuestring );
                        tmp_cdc->extract.inserted.secondary_db = SAFECALLOC( str_len3 + 1, sizeof( char ), __FILE__, __LINE__ );
                        snprintf( tmp_cdc->extract.inserted.secondary_db, str_len3+1, cfg_system_query_item_etl_extract_inserted_secondary_db->valuestring );
                        /*strncpy(
                            tmp_cdc->extract.inserted.secondary_db,
                            cfg_system_query_item_etl_extract_inserted_secondary_db->valuestring,
                            str_len
                        );*/

                        /*
                            etl.extract.secondary_db_sql
                        */
                        cfg_system_query_item_etl_extract_inserted_secondary_db_sql = cJSON_GetObjectItem( cfg_system_query_item_etl_extract_inserted, "secondary_db_sql" );
                        if( cfg_system_query_item_etl_extract_inserted_secondary_db_sql == NULL ) {
                            LOG_print( "ERROR: \"system[%d].queries[%d].etl.extract.inserted.secondary_db_sql\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        size_t str_len4 = strlen( cfg_system_query_item_etl_extract_inserted_secondary_db_sql->valuestring );
                        tmp_cdc->extract.inserted.secondary_db_sql_len = str_len4;
                        tmp_cdc->extract.inserted.secondary_db_sql = SAFECALLOC( str_len4 + 1, sizeof( char ), __FILE__, __LINE__ );
                        strncpy(
                            tmp_cdc->extract.inserted.secondary_db_sql,
                            cfg_system_query_item_etl_extract_inserted_secondary_db_sql->valuestring,
                            str_len4
                        );

                        /*
                            etl.extract.modified
                        */
                        cfg_system_query_item_etl_extract_modified = cJSON_GetObjectItem( cfg_system_query_item_etl_extract, "modified" );
                        if( cfg_system_query_item_etl_extract_modified == NULL ) {
                            LOG_print( "ERROR: \"system[%d].queries[%d].etl.extract.modified\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        /*
                            etl.extract.modified.primary_db
                        */
                        cfg_system_query_item_etl_extract_modified_primary_db = cJSON_GetObjectItem( cfg_system_query_item_etl_extract_modified, "primary_db" );
                        if( cfg_system_query_item_etl_extract_modified_primary_db == NULL ) {
                            LOG_print( "ERROR: \"system[%d].queries[%d].etl.extract.modified.primary_db\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        size_t str_len5 = strlen( cfg_system_query_item_etl_extract_modified_primary_db->valuestring );
                        tmp_cdc->extract.modified.primary_db = SAFECALLOC( str_len5 + 1, sizeof( char ), __FILE__, __LINE__ );
                        snprintf( tmp_cdc->extract.modified.primary_db, str_len5+1, cfg_system_query_item_etl_extract_modified_primary_db->valuestring );
                        /*strncpy(
                            tmp_cdc->extract.modified.primary_db,
                            cfg_system_query_item_etl_extract_modified_primary_db->valuestring,
                            str_len
                        );*/

                        /*
                            etl.extract.modified.primary_db_sql
                        */
                        cfg_system_query_item_etl_extract_modified_primary_db_sql = cJSON_GetObjectItem( cfg_system_query_item_etl_extract_modified, "primary_db_sql" );
                        if( cfg_system_query_item_etl_extract_modified_primary_db_sql == NULL ) {
                            LOG_print( "ERROR: \"system[%d].queries[%d].etl.extract.modified.primary_db_sql\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        size_t str_len6 = strlen( cfg_system_query_item_etl_extract_modified_primary_db_sql->valuestring );
                        tmp_cdc->extract.modified.primary_db_sql_len = str_len6;
                        tmp_cdc->extract.modified.primary_db_sql = SAFECALLOC( str_len6 + 1, sizeof( char ), __FILE__, __LINE__ );
                        strncpy(
                            tmp_cdc->extract.modified.primary_db_sql,
                            cfg_system_query_item_etl_extract_modified_primary_db_sql->valuestring,
                            str_len6
                        );

                        /*
                            etl.extract.modified.secondary_db
                        */
                        cfg_system_query_item_etl_extract_modified_secondary_db = cJSON_GetObjectItem( cfg_system_query_item_etl_extract_modified, "secondary_db" );
                        if( cfg_system_query_item_etl_extract_modified_secondary_db == NULL ) {
                            LOG_print( "ERROR: \"system[%d].queries[%d].etl.extract.modified.secondary_db\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        size_t str_len7 = strlen( cfg_system_query_item_etl_extract_modified_secondary_db->valuestring );
                        tmp_cdc->extract.modified.secondary_db = SAFECALLOC( str_len7 + 1, sizeof( char ), __FILE__, __LINE__ );
                        snprintf( tmp_cdc->extract.modified.secondary_db, str_len7+1, cfg_system_query_item_etl_extract_modified_secondary_db->valuestring );
                        /*strncpy(
                            tmp_cdc->extract.modified.secondary_db,
                            cfg_system_query_item_etl_extract_modified_secondary_db->valuestring,
                            str_len
                        );*/

                        /*
                            etl.extract.modified.secondary_db_sql
                        */
                        cfg_system_query_item_etl_extract_modified_secondary_db_sql = cJSON_GetObjectItem( cfg_system_query_item_etl_extract_modified, "secondary_db_sql" );
                        if( cfg_system_query_item_etl_extract_modified_secondary_db_sql == NULL ) {
                            LOG_print( "ERROR: \"system[%d].queries[%d].etl.extract.modified.secondary_db_sql\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        size_t str_len8 = strlen( cfg_system_query_item_etl_extract_modified_secondary_db_sql->valuestring );
                        tmp_cdc->extract.modified.secondary_db_sql_len = str_len8;
                        tmp_cdc->extract.modified.secondary_db_sql = SAFECALLOC( str_len8 + 1, sizeof( char ), __FILE__, __LINE__ );
                        strncpy(
                            tmp_cdc->extract.modified.secondary_db_sql,
                            cfg_system_query_item_etl_extract_modified_secondary_db_sql->valuestring,
                            str_len8
                        );

                        /*
                            etl.extract.deleted
                        */
                        cfg_system_query_item_etl_extract_deleted = cJSON_GetObjectItem( cfg_system_query_item_etl_extract, "deleted" );
                        if( cfg_system_query_item_etl_extract_deleted == NULL ) {
                            LOG_print( "ERROR: \"system[%d].queries[%d].etl.extract.deleted\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        /*
                            etl.extract.deleted.primary_db
                        */
                        cfg_system_query_item_etl_extract_deleted_primary_db = cJSON_GetObjectItem( cfg_system_query_item_etl_extract_deleted, "primary_db" );
                        if( cfg_system_query_item_etl_extract_deleted_primary_db == NULL ) {
                            LOG_print( "ERROR: \"system[%d].queries[%d].etl.extract.deleted.primary_db\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        size_t str_len9 = strlen( cfg_system_query_item_etl_extract_deleted_primary_db->valuestring );
                        tmp_cdc->extract.deleted.primary_db = SAFECALLOC( str_len9 + 1, sizeof( char ), __FILE__, __LINE__ );
                        snprintf( tmp_cdc->extract.deleted.primary_db, str_len9+1, cfg_system_query_item_etl_extract_deleted_primary_db->valuestring );
                        /*strncpy(
                            tmp_cdc->extract.deleted.primary_db,
                            cfg_system_query_item_etl_extract_deleted_primary_db->valuestring,
                            str_len
                        );*/
                        /*
                            etl.extract.deleted.primary_db_sql
                        */
                        cfg_system_query_item_etl_extract_deleted_primary_db_sql = cJSON_GetObjectItem( cfg_system_query_item_etl_extract_deleted, "primary_db_sql" );
                        if( cfg_system_query_item_etl_extract_deleted_primary_db_sql == NULL ) {
                            LOG_print( "ERROR: \"system[%d].queries[%d].etl.extract.deleted.primary_db_sql\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        size_t str_len10 = strlen( cfg_system_query_item_etl_extract_deleted_primary_db_sql->valuestring );
                        tmp_cdc->extract.deleted.primary_db_sql_len = str_len10;
                        tmp_cdc->extract.deleted.primary_db_sql = SAFECALLOC( str_len10 + 1, sizeof( char ), __FILE__, __LINE__ );
                        strncpy(
                            tmp_cdc->extract.deleted.primary_db_sql,
                            cfg_system_query_item_etl_extract_deleted_primary_db_sql->valuestring,
                            str_len10
                        );
                        /*
                            etl.extract.deleted.secondary_db
                        */
                        cfg_system_query_item_etl_extract_deleted_secondary_db = cJSON_GetObjectItem( cfg_system_query_item_etl_extract_deleted, "secondary_db" );
                        if( cfg_system_query_item_etl_extract_deleted_secondary_db == NULL ) {
                            LOG_print( "ERROR: \"system[%d].queries[%d].etl.extract.deleted.secondary_db\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        size_t str_len11 = strlen( cfg_system_query_item_etl_extract_deleted_secondary_db->valuestring );
                        tmp_cdc->extract.deleted.secondary_db = SAFECALLOC( str_len11 + 1, sizeof( char ), __FILE__, __LINE__ );
                        snprintf( tmp_cdc->extract.deleted.secondary_db, str_len11+1, cfg_system_query_item_etl_extract_deleted_secondary_db->valuestring );
                        /*strncpy(
                            tmp_cdc->extract.deleted.secondary_db,
                            cfg_system_query_item_etl_extract_deleted_secondary_db->valuestring,
                            str_len
                        );*/

                        /*
                            etl.extract.deleted.secondary_db_sql
                        */
                        cfg_system_query_item_etl_extract_deleted_secondary_db_sql = cJSON_GetObjectItem( cfg_system_query_item_etl_extract_deleted, "secondary_db_sql" );
                        if( cfg_system_query_item_etl_extract_deleted_secondary_db_sql == NULL ) {
                            LOG_print( "ERROR: \"system[%d].queries[%d].etl.extract.deleted.secondary_db_sql\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        size_t str_len12 = strlen( cfg_system_query_item_etl_extract_deleted_secondary_db_sql->valuestring );
                        tmp_cdc->extract.deleted.secondary_db_sql_len = str_len12;
                        tmp_cdc->extract.deleted.secondary_db_sql = SAFECALLOC( str_len12 + 1, sizeof( char ), __FILE__, __LINE__ );
                        strncpy(
                            tmp_cdc->extract.deleted.secondary_db_sql,
                            cfg_system_query_item_etl_extract_deleted_secondary_db_sql->valuestring,
                            str_len12
                        );

                        /**********************************************/
                        /*
                            etl.stage
                            WARNING: Staging is optional. 
                        */
                        cfg_system_query_item_etl_stage = cJSON_GetObjectItem( cfg_system_query_item_etl, "stage" );
                        if( cfg_system_query_item_etl_stage == NULL ) {
                            LOG_print( "NOTICE: \"system[%d].queries[%d].etl.stage\" key not found.\n", i, j );
                            tmp_cdc->stage = NULL;
                        } else {
                            tmp_cdc->stage = SAFEMALLOC( sizeof( DB_SYSTEM_ETL_STAGE ), __FILE__, __LINE__ );
                            /*
                                etl.stage.inserted
                            */
                            cfg_system_query_item_etl_stage_inserted = cJSON_GetObjectItem( cfg_system_query_item_etl_stage, "inserted" );
                            if( cfg_system_query_item_etl_stage_inserted == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].etl.stage.inserted\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                return FALSE;
                            }
                            /*
                                etl.stage.inserted.sql
                            */
                            cfg_system_query_item_etl_stage_inserted_sql = cJSON_GetObjectItem( cfg_system_query_item_etl_stage_inserted, "sql" );
                            if( cfg_system_query_item_etl_stage_inserted_sql == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].etl.stage.inserted.sql\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                return FALSE;
                            }
                            size_t str_len14 = strlen( cfg_system_query_item_etl_stage_inserted_sql->valuestring );
                            tmp_cdc->stage->inserted.sql_len = str_len14;
                            tmp_cdc->stage->inserted.sql = SAFECALLOC( str_len14 + 1, sizeof( char ), __FILE__, __LINE__ );
                            strncpy(
                                tmp_cdc->stage->inserted.sql,
                                cfg_system_query_item_etl_stage_inserted_sql->valuestring,
                                str_len14
                            );
                            /*
                                etl.stage.inserted.extracted_values
                            */

                            for( int k = 0; k < MAX_ETL_COLUMNS; k++ ) {
                                memset( tmp_cdc->stage->inserted.extracted_values[ k ], 0, MAX_COLUMN_NAME_LEN );
                            }
                            tmp_cdc->stage->inserted.extracted_values_len = 0;
                            cfg_system_query_item_etl_stage_inserted_extracted_values_array = cJSON_GetObjectItem( cfg_system_query_item_etl_stage_inserted, "extracted_values" );
                            if( cfg_system_query_item_etl_stage_inserted_extracted_values_array == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].etl.stage.inserted.extracted_values\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                return FALSE;
                            }
                            for( int k = 0; k < cJSON_GetArraySize( cfg_system_query_item_etl_stage_inserted_extracted_values_array ); k++ ) {
                                cfg_system_query_item_etl_stage_inserted_extracted_values_item = cJSON_GetArrayItem( cfg_system_query_item_etl_stage_inserted_extracted_values_array, k );
                                snprintf( tmp_cdc->stage->inserted.extracted_values[ k ], MAX_COLUMN_NAME_LEN, "%s", cfg_system_query_item_etl_stage_inserted_extracted_values_item->valuestring );
                                tmp_cdc->stage->inserted.extracted_values_len++;
                            }
                            /*
                                etl.stage.deleted
                            */
                            cfg_system_query_item_etl_stage_deleted = cJSON_GetObjectItem( cfg_system_query_item_etl_stage, "deleted" );
                            if( cfg_system_query_item_etl_stage_deleted == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].etl.stage.deleted\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                return FALSE;
                            }
                            /*
                                etl.stage.deleted.sql
                            */
                            cfg_system_query_item_etl_stage_deleted_sql = cJSON_GetObjectItem( cfg_system_query_item_etl_stage_deleted, "sql" );
                            if( cfg_system_query_item_etl_stage_deleted_sql == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].etl.stage.deleted.sql\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                return FALSE;
                            }
                            size_t str_len = strlen( cfg_system_query_item_etl_stage_deleted_sql->valuestring );
                            tmp_cdc->stage->deleted.sql_len = str_len;
                            tmp_cdc->stage->deleted.sql = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                            strncpy(
                                tmp_cdc->stage->deleted.sql,
                                cfg_system_query_item_etl_stage_deleted_sql->valuestring,
                                str_len
                            );
                            /*
                                etl.stage.deleted.extracted_values
                            */
                            cfg_system_query_item_etl_stage_deleted_extracted_values_array = cJSON_GetObjectItem( cfg_system_query_item_etl_stage_deleted, "extracted_values" );
                            if( cfg_system_query_item_etl_stage_deleted_extracted_values_array == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].etl.stage.deleted.extracted_values\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                return FALSE;
                            }
                            for( int k = 0; k < MAX_ETL_COLUMNS; k++ ) {
                                memset( tmp_cdc->stage->deleted.extracted_values[ k ], 0, MAX_COLUMN_NAME_LEN );
                            }
                            tmp_cdc->stage->deleted.extracted_values_len = 0;
                            for( int k = 0; k < cJSON_GetArraySize( cfg_system_query_item_etl_stage_deleted_extracted_values_array ); k++ ) {
                                cfg_system_query_item_etl_stage_deleted_extracted_values_item = cJSON_GetArrayItem( cfg_system_query_item_etl_stage_deleted_extracted_values_array, k );
                                snprintf( tmp_cdc->stage->deleted.extracted_values[ k ], MAX_COLUMN_NAME_LEN, "%s", cfg_system_query_item_etl_stage_deleted_extracted_values_item->valuestring );
                                tmp_cdc->stage->deleted.extracted_values_len++;
                            }
                            /*
                                etl.stage.modified
                            */
                            cfg_system_query_item_etl_stage_modified = cJSON_GetObjectItem( cfg_system_query_item_etl_stage, "modified" );
                            if( cfg_system_query_item_etl_stage_modified == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].etl.stage.modified\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                return FALSE;
                            }
                            /*
                                etl.stage.modified.sql
                            */
                            cfg_system_query_item_etl_stage_modified_sql = cJSON_GetObjectItem( cfg_system_query_item_etl_stage_modified, "sql" );
                            if( cfg_system_query_item_etl_stage_modified_sql == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].etl.stage.modified.sql\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                return FALSE;
                            }
                            size_t str_len2 = strlen( cfg_system_query_item_etl_stage_modified_sql->valuestring );
                            tmp_cdc->stage->modified.sql_len = str_len2;
                            tmp_cdc->stage->modified.sql = SAFECALLOC( str_len2 + 1, sizeof( char ), __FILE__, __LINE__ );
                            strncpy(
                                tmp_cdc->stage->modified.sql,
                                cfg_system_query_item_etl_stage_modified_sql->valuestring,
                                str_len2
                            );
                            /*
                                etl.stage.modified.extracted_values
                            */
                            cfg_system_query_item_etl_stage_modified_extracted_values_array = cJSON_GetObjectItem( cfg_system_query_item_etl_stage_modified, "extracted_values" );
                            if( cfg_system_query_item_etl_stage_modified_extracted_values_array == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].etl.stage.modified.extracted_values\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                return FALSE;
                            }
                            for( int k = 0; k < MAX_ETL_COLUMNS; k++ ) {
                                memset( tmp_cdc->stage->modified.extracted_values[ k ], 0, MAX_COLUMN_NAME_LEN );
                            }
                            tmp_cdc->stage->modified.extracted_values_len = 0;
                            for( int k = 0; k < cJSON_GetArraySize( cfg_system_query_item_etl_stage_modified_extracted_values_array ); k++ ) {
                                cfg_system_query_item_etl_stage_modified_extracted_values_item = cJSON_GetArrayItem( cfg_system_query_item_etl_stage_modified_extracted_values_array, k );
                                snprintf( tmp_cdc->stage->modified.extracted_values[ k ], MAX_COLUMN_NAME_LEN, "%s", cfg_system_query_item_etl_stage_modified_extracted_values_item->valuestring );
                                tmp_cdc->stage->modified.extracted_values_len++;
                            }
                        }
                        
                        /********************************************/

                        /*
                            etl.transform
                            WARNING: Transformation is optional.
                        */
                        cfg_system_query_item_etl_transform = cJSON_GetObjectItem( cfg_system_query_item_etl, "transform" );
                        if( cfg_system_query_item_etl_transform == NULL ) {
                            LOG_print( "NOTICE: \"system[%d].queries[%d].etl.transform\" key not found.\n", i, j );
                            tmp_cdc->transform = NULL;
                        } else {
                            tmp_cdc->transform = SAFEMALLOC( sizeof( DB_SYSTEM_ETL_TRANSFORM ), __FILE__, __LINE__ );

                            /*
                                etl.transform.inserted
                            */
                            cfg_system_query_item_etl_transform_inserted_array = cJSON_GetObjectItem( cfg_system_query_item_etl_transform, "inserted" );
                            if( cfg_system_query_item_etl_transform_inserted_array == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].etl.transform.inserted\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                return FALSE;
                            }
                            tmp_cdc->transform->inserted_count = cJSON_GetArraySize( cfg_system_query_item_etl_transform_inserted_array );
                            tmp_cdc->transform->inserted = SAFEMALLOC( tmp_cdc->transform->inserted_count * sizeof * tmp_cdc->transform->inserted, __FILE__, __LINE__ );
                            for( int k = 0; k < tmp_cdc->transform->inserted_count; k++ ) {
                                cfg_system_query_item_etl_transform_inserted_item = cJSON_GetArrayItem( cfg_system_query_item_etl_transform_inserted_array, k );
                                tmp_cdc->transform->inserted[ k ] = SAFEMALLOC( sizeof( DB_SYSTEM_ETL_TRANSFORM_QUERY ), __FILE__, __LINE__ );

                                cfg_system_query_item_etl_transform_inserted_module = cJSON_GetObjectItem( cfg_system_query_item_etl_transform_inserted_item, "module" );
                                size_t str_len = strlen( cfg_system_query_item_etl_transform_inserted_module->valuestring );
                                tmp_cdc->transform->inserted[ k ]->module = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                                snprintf( tmp_cdc->transform->inserted[ k ]->module, str_len + 1, cfg_system_query_item_etl_transform_inserted_module->valuestring );
                                /*strncpy(
                                    tmp_cdc->transform->inserted[ k ]->module,
                                    cfg_system_query_item_etl_transform_inserted_module->valuestring,
                                    str_len
                                );*/

                                cfg_system_query_item_etl_transform_inserted_staged_data = cJSON_GetObjectItem( cfg_system_query_item_etl_transform_inserted_item, "staged_data" );
                                size_t str_len2 = strlen( cfg_system_query_item_etl_transform_inserted_staged_data->valuestring );
                                tmp_cdc->transform->inserted[ k ]->staged_data = SAFECALLOC( str_len2 + 1, sizeof( char ), __FILE__, __LINE__ );
                                strncpy(
                                    tmp_cdc->transform->inserted[ k ]->staged_data,
                                    cfg_system_query_item_etl_transform_inserted_staged_data->valuestring,
                                    str_len2
                                );

                                cfg_system_query_item_etl_transform_inserted_source_system_update = cJSON_GetObjectItem( cfg_system_query_item_etl_transform_inserted_item, "source_system_update" );
                                size_t str_len3 = strlen( cfg_system_query_item_etl_transform_inserted_source_system_update->valuestring );
                                tmp_cdc->transform->inserted[ k ]->source_system_update = SAFECALLOC( str_len3 + 1, sizeof( char ), __FILE__, __LINE__ );
                                strncpy(
                                    tmp_cdc->transform->inserted[ k ]->source_system_update,
                                    cfg_system_query_item_etl_transform_inserted_source_system_update->valuestring,
                                    str_len
                                );
                            }

                            /*
                                etl.transform.deleted
                            */
                            cfg_system_query_item_etl_transform_deleted_array = cJSON_GetObjectItem( cfg_system_query_item_etl_transform, "deleted" );
                            if( cfg_system_query_item_etl_transform_deleted_array == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].etl.transform.deleted\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                return FALSE;
                            }
                            tmp_cdc->transform->deleted_count = cJSON_GetArraySize( cfg_system_query_item_etl_transform_deleted_array );
                            tmp_cdc->transform->deleted = SAFEMALLOC( tmp_cdc->transform->deleted_count * sizeof * tmp_cdc->transform->deleted, __FILE__, __LINE__ );
                            for( int k = 0; k < tmp_cdc->transform->deleted_count; k++ ) {
                                cfg_system_query_item_etl_transform_deleted_item = cJSON_GetArrayItem( cfg_system_query_item_etl_transform_deleted_array, k );

                                tmp_cdc->transform->deleted[ k ] = SAFEMALLOC( sizeof( DB_SYSTEM_ETL_TRANSFORM_QUERY ), __FILE__, __LINE__ );

                                cfg_system_query_item_etl_transform_deleted_module = cJSON_GetObjectItem( cfg_system_query_item_etl_transform_deleted_item, "module" );
                                size_t str_len = strlen( cfg_system_query_item_etl_transform_deleted_module->valuestring );
                                tmp_cdc->transform->deleted[ k ]->module = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                                snprintf( tmp_cdc->transform->deleted[ k ]->module, str_len + 1, cfg_system_query_item_etl_transform_deleted_module->valuestring );
                                /*strncpy(
                                    tmp_cdc->transform->deleted[ k ]->module,
                                    cfg_system_query_item_etl_transform_deleted_module->valuestring,
                                    str_len
                                );*/

                                cfg_system_query_item_etl_transform_deleted_staged_data = cJSON_GetObjectItem( cfg_system_query_item_etl_transform_deleted_item, "staged_data" );
                                size_t str_len2 = strlen( cfg_system_query_item_etl_transform_deleted_staged_data->valuestring );
                                tmp_cdc->transform->deleted[ k ]->staged_data = SAFECALLOC( str_len2 + 1, sizeof( char ), __FILE__, __LINE__ );
                                strncpy(
                                    tmp_cdc->transform->deleted[ k ]->staged_data,
                                    cfg_system_query_item_etl_transform_deleted_staged_data->valuestring,
                                    str_len2
                                );

                                cfg_system_query_item_etl_transform_deleted_source_system_update = cJSON_GetObjectItem( cfg_system_query_item_etl_transform_deleted_item, "source_system_update" );
                                size_t str_len3 = strlen( cfg_system_query_item_etl_transform_deleted_source_system_update->valuestring );
                                tmp_cdc->transform->deleted[ k ]->source_system_update = SAFECALLOC( str_len3 + 1, sizeof( char ), __FILE__, __LINE__ );
                                strncpy(
                                    tmp_cdc->transform->deleted[ k ]->source_system_update,
                                    cfg_system_query_item_etl_transform_deleted_source_system_update->valuestring,
                                    str_len3
                                );
                            }

                            /*
                                etl.transform.modified
                            */
                            cfg_system_query_item_etl_transform_modified_array = cJSON_GetObjectItem( cfg_system_query_item_etl_transform, "modified" );
                            if( cfg_system_query_item_etl_transform_modified_array == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].etl.transform.modified\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                return FALSE;
                            }
                            tmp_cdc->transform->modified_count = cJSON_GetArraySize( cfg_system_query_item_etl_transform_modified_array );
                            tmp_cdc->transform->modified = SAFEMALLOC( tmp_cdc->transform->modified_count * sizeof * tmp_cdc->transform->modified, __FILE__, __LINE__ );
                            for( int k = 0; k < tmp_cdc->transform->modified_count; k++ ) {
                                cfg_system_query_item_etl_transform_modified_item = cJSON_GetArrayItem( cfg_system_query_item_etl_transform_modified_array, k );

                                tmp_cdc->transform->modified[ k ] = SAFEMALLOC( sizeof( DB_SYSTEM_ETL_TRANSFORM_QUERY ), __FILE__, __LINE__ );

                                cfg_system_query_item_etl_transform_modified_module = cJSON_GetObjectItem( cfg_system_query_item_etl_transform_modified_item, "module" );
                                size_t str_len = strlen( cfg_system_query_item_etl_transform_modified_module->valuestring );
                                tmp_cdc->transform->modified[ k ]->module = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                                snprintf( tmp_cdc->transform->modified[ k ]->module, str_len + 1, cfg_system_query_item_etl_transform_modified_module->valuestring );
                                /*strncpy(
                                    tmp_cdc->transform->modified[ k ]->module,
                                    cfg_system_query_item_etl_transform_modified_module->valuestring,
                                    str_len
                                );*/

                                cfg_system_query_item_etl_transform_modified_staged_data = cJSON_GetObjectItem( cfg_system_query_item_etl_transform_modified_item, "staged_data" );
                                size_t str_len2 = strlen( cfg_system_query_item_etl_transform_modified_staged_data->valuestring );
                                tmp_cdc->transform->modified[ k ]->staged_data = SAFECALLOC( str_len2 + 1, sizeof( char ), __FILE__, __LINE__ );
                                strncpy(
                                    tmp_cdc->transform->modified[ k ]->staged_data,
                                    cfg_system_query_item_etl_transform_modified_staged_data->valuestring,
                                    str_len2
                                );

                                cfg_system_query_item_etl_transform_modified_source_system_update = cJSON_GetObjectItem( cfg_system_query_item_etl_transform_modified_item, "source_system_update" );
                                size_t str_len3 = strlen( cfg_system_query_item_etl_transform_modified_source_system_update->valuestring );
                                tmp_cdc->transform->modified[ k ]->source_system_update = SAFECALLOC( str_len3 + 1, sizeof( char ), __FILE__, __LINE__ );
                                strncpy(
                                    tmp_cdc->transform->modified[ k ]->source_system_update,
                                    cfg_system_query_item_etl_transform_modified_source_system_update->valuestring,
                                    str_len3
                                );
                            }
                        }

                        /*
                            etl.load
                        */
                        cfg_system_query_item_etl_load = cJSON_GetObjectItem( cfg_system_query_item_etl, "load" );
                        if( cfg_system_query_item_etl_load == NULL ) {
                            LOG_print( "ERROR: \"system[%d].queries[%d].etl.load\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        /*
                            etl.load.inserted
                        */
                        cfg_system_query_item_etl_load_inserted = cJSON_GetObjectItem( cfg_system_query_item_etl_load, "inserted" );
                        if( cfg_system_query_item_etl_load_inserted == NULL ) {
                            LOG_print( "ERROR: \"system[%d].queries[%d].etl.load.inserted\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        /*
                            etl.load.inserted.input_data_sql
                        */
                        cfg_system_query_item_etl_load_inserted_input_data_sql = cJSON_GetObjectItem( cfg_system_query_item_etl_load_inserted, "input_data_sql" );
                        if( cfg_system_query_item_etl_load_inserted_input_data_sql == NULL ) {
                            LOG_print( "ERROR: \"system[%d].queries[%d].etl.load.inserted.input_data_sql\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        size_t str_len14 = strlen( cfg_system_query_item_etl_load_inserted_input_data_sql->valuestring );
                        tmp_cdc->load.inserted.input_data_sql_len = str_len14;
                        tmp_cdc->load.inserted.input_data_sql = SAFECALLOC( str_len14 + 1, sizeof( char ), __FILE__, __LINE__ );
                        strncpy(
                            tmp_cdc->load.inserted.input_data_sql,
                            cfg_system_query_item_etl_load_inserted_input_data_sql->valuestring,
                            str_len14
                        );

                        /*
                            etl.load.inserted.output_data_sql
                        */
                        cfg_system_query_item_etl_load_inserted_output_data_sql = cJSON_GetObjectItem( cfg_system_query_item_etl_load_inserted, "output_data_sql" );
                        if( cfg_system_query_item_etl_load_inserted_output_data_sql == NULL ) {
                            LOG_print( "ERROR: \"system[%d].queries[%d].etl.load.inserted.output_data_sql\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        size_t str_len15 = strlen( cfg_system_query_item_etl_load_inserted_output_data_sql->valuestring );
                        tmp_cdc->load.inserted.output_data_sql_len = str_len15;
                        tmp_cdc->load.inserted.output_data_sql = SAFECALLOC( str_len15 + 1, sizeof( char ), __FILE__, __LINE__ );
                        strncpy(
                            tmp_cdc->load.inserted.output_data_sql,
                            cfg_system_query_item_etl_load_inserted_output_data_sql->valuestring,
                            str_len15
                        );

                        /*
                            etl.load.inserted.extracted_values
                        */

                        for( int k = 0; k < MAX_ETL_COLUMNS; k++ ) {
                            memset( tmp_cdc->load.inserted.extracted_values[ k ], 0, MAX_COLUMN_NAME_LEN );
                        }
                        tmp_cdc->load.inserted.extracted_values_len = 0;
                        cfg_system_query_item_etl_load_inserted_extracted_values_array = cJSON_GetObjectItem( cfg_system_query_item_etl_load_inserted, "extracted_values" );
                        if( cfg_system_query_item_etl_load_inserted_extracted_values_array == NULL ) {
                            LOG_print( "ERROR: \"system[%d].queries[%d].etl.load.inserted.extracted_values\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        for( int k = 0; k < cJSON_GetArraySize( cfg_system_query_item_etl_load_inserted_extracted_values_array ); k++ ) {
                            cfg_system_query_item_etl_load_inserted_extracted_values_item = cJSON_GetArrayItem( cfg_system_query_item_etl_load_inserted_extracted_values_array, k );
                            snprintf( tmp_cdc->load.inserted.extracted_values[ k ], MAX_COLUMN_NAME_LEN, "%s", cfg_system_query_item_etl_load_inserted_extracted_values_item->valuestring );
                            tmp_cdc->load.inserted.extracted_values_len++;
                        }
                        /*
                            etl.load.deleted
                        */
                        cfg_system_query_item_etl_load_deleted = cJSON_GetObjectItem( cfg_system_query_item_etl_load, "deleted" );
                        if( cfg_system_query_item_etl_load_deleted == NULL ) {
                            LOG_print( "ERROR: \"system[%d].queries[%d].etl.load.deleted\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        /*
                            etl.load.deleted.input_data_sql
                        */
                        cfg_system_query_item_etl_load_deleted_input_data_sql = cJSON_GetObjectItem( cfg_system_query_item_etl_load_deleted, "input_data_sql" );
                        if( cfg_system_query_item_etl_load_deleted_input_data_sql == NULL ) {
                            LOG_print( "ERROR: \"system[%d].queries[%d].etl.load.deleted.input_data_sql\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        size_t str_len16 = strlen( cfg_system_query_item_etl_load_deleted_input_data_sql->valuestring );
                        tmp_cdc->load.deleted.input_data_sql_len = str_len16;
                        tmp_cdc->load.deleted.input_data_sql = SAFECALLOC( str_len16 + 1, sizeof( char ), __FILE__, __LINE__ );
                        strncpy(
                            tmp_cdc->load.deleted.input_data_sql,
                            cfg_system_query_item_etl_load_deleted_input_data_sql->valuestring,
                            str_len16
                        );
                        /*
                            etl.load.deleted.output_data_sql
                        */
                        cfg_system_query_item_etl_load_deleted_output_data_sql = cJSON_GetObjectItem( cfg_system_query_item_etl_load_deleted, "output_data_sql" );
                        if( cfg_system_query_item_etl_load_deleted_output_data_sql == NULL ) {
                            LOG_print( "ERROR: \"system[%d].queries[%d].etl.load.deleted.output_data_sql\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        size_t str_len17 = strlen( cfg_system_query_item_etl_load_deleted_output_data_sql->valuestring );
                        tmp_cdc->load.deleted.output_data_sql_len = str_len17;
                        tmp_cdc->load.deleted.output_data_sql = SAFECALLOC( str_len17 + 1, sizeof( char ), __FILE__, __LINE__ );
                        strncpy(
                            tmp_cdc->load.deleted.output_data_sql,
                            cfg_system_query_item_etl_load_deleted_output_data_sql->valuestring,
                            str_len17
                        );
                        /*
                            etl.load.deleted.extracted_values
                        */
                        cfg_system_query_item_etl_load_deleted_extracted_values_array = cJSON_GetObjectItem( cfg_system_query_item_etl_load_deleted, "extracted_values" );
                        if( cfg_system_query_item_etl_load_deleted_extracted_values_array == NULL ) {
                            LOG_print( "ERROR: \"system[%d].queries[%d].etl.load.deleted.extracted_values\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        for( int k = 0; k < MAX_ETL_COLUMNS; k++ ) {
                            memset( tmp_cdc->load.deleted.extracted_values[ k ], 0, MAX_COLUMN_NAME_LEN );
                        }
                        tmp_cdc->load.deleted.extracted_values_len = 0;
                        for( int k = 0; k < cJSON_GetArraySize( cfg_system_query_item_etl_load_deleted_extracted_values_array ); k++ ) {
                            cfg_system_query_item_etl_load_deleted_extracted_values_item = cJSON_GetArrayItem( cfg_system_query_item_etl_load_deleted_extracted_values_array, k );
                            snprintf( tmp_cdc->load.deleted.extracted_values[ k ], MAX_COLUMN_NAME_LEN, "%s", cfg_system_query_item_etl_load_deleted_extracted_values_item->valuestring );
                            tmp_cdc->load.deleted.extracted_values_len++;
                        }
                        /*
                            etl.load.modified
                        */
                        cfg_system_query_item_etl_load_modified = cJSON_GetObjectItem( cfg_system_query_item_etl_load, "modified" );
                        if( cfg_system_query_item_etl_load_modified == NULL ) {
                            LOG_print( "ERROR: \"system[%d].queries[%d].etl.load.modified\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        /*
                            etl.load.modified.input_data_sql
                        */
                        cfg_system_query_item_etl_load_modified_input_data_sql = cJSON_GetObjectItem( cfg_system_query_item_etl_load_modified, "input_data_sql" );
                        if( cfg_system_query_item_etl_load_modified_input_data_sql == NULL ) {
                            LOG_print( "ERROR: \"system[%d].queries[%d].etl.load.modified.input_data_sql\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        size_t str_len18 = strlen( cfg_system_query_item_etl_load_modified_input_data_sql->valuestring );
                        tmp_cdc->load.modified.input_data_sql_len = str_len18;
                        tmp_cdc->load.modified.input_data_sql = SAFECALLOC( str_len18 + 1, sizeof( char ), __FILE__, __LINE__ );
                        strncpy(
                            tmp_cdc->load.modified.input_data_sql,
                            cfg_system_query_item_etl_load_modified_input_data_sql->valuestring,
                            str_len18
                        );
                        /*
                            etl.load.modified.output_data_sql
                        */
                        cfg_system_query_item_etl_load_modified_output_data_sql = cJSON_GetObjectItem( cfg_system_query_item_etl_load_modified, "output_data_sql" );
                        if( cfg_system_query_item_etl_load_modified_output_data_sql == NULL ) {
                            LOG_print( "ERROR: \"system[%d].queries[%d].etl.load.modified.output_data_sql\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        size_t str_len19 = strlen( cfg_system_query_item_etl_load_modified_output_data_sql->valuestring );
                        tmp_cdc->load.modified.output_data_sql_len = str_len19;
                        tmp_cdc->load.modified.output_data_sql = SAFECALLOC( str_len19 + 1, sizeof( char ), __FILE__, __LINE__ );
                        strncpy(
                            tmp_cdc->load.modified.output_data_sql,
                            cfg_system_query_item_etl_load_modified_output_data_sql->valuestring,
                            str_len19
                        );
                        /*
                            etl.load.modified.extracted_values
                        */
                        cfg_system_query_item_etl_load_modified_extracted_values_array = cJSON_GetObjectItem( cfg_system_query_item_etl_load_modified, "extracted_values" );
                        if( cfg_system_query_item_etl_load_modified_extracted_values_array == NULL ) {
                            LOG_print( "ERROR: \"system[%d].queries[%d].etl.load.modified.extracted_values\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        for( int k = 0; k < MAX_ETL_COLUMNS; k++ ) {
                            memset( tmp_cdc->load.modified.extracted_values[ k ], 0, MAX_COLUMN_NAME_LEN );
                        }
                        tmp_cdc->load.modified.extracted_values_len = 0;
                        for( int k = 0; k < cJSON_GetArraySize( cfg_system_query_item_etl_load_modified_extracted_values_array ); k++ ) {
                            cfg_system_query_item_etl_load_modified_extracted_values_item = cJSON_GetArrayItem( cfg_system_query_item_etl_load_modified_extracted_values_array, k );
                            snprintf( tmp_cdc->load.modified.extracted_values[ k ], MAX_COLUMN_NAME_LEN, "%s", cfg_system_query_item_etl_load_modified_extracted_values_item->valuestring );
                            tmp_cdc->load.modified.extracted_values_len++;
                        }

                        tmp_queries[ tmp_queries_count ] = SAFEMALLOC( sizeof( DATABASE_SYSTEM_QUERY ), __FILE__, __LINE__ );

                        DATABASE_SYSTEM_QUERY_add(
                            cfg_system_query_item_name->valuestring,
                            cfg_system_query_item_mode ? cfg_system_query_item_mode->valueint : M_ETL,
                            *tmp_cdc,
                            tmp_queries[ tmp_queries_count ],
                            FALSE
                        );

                        tmp_queries_count++;

                        free( tmp_cdc ); tmp_cdc = NULL;
                    }
                    /* Create temporary DATABASE_SYSTEM_DB for future use in DATABASE_SYSTEM_add */
                    DATABASE_SYSTEM_DB *tmp_db = SAFEMALLOC( sizeof( DATABASE_SYSTEM_DB ), __FILE__, __LINE__ );
                    DATABASE_SYSTEM_DB_add(
                        cfg_system_ip->valuestring,
                        cfg_system_port->valueint,
                        cfg_system_driver->valueint,
                        cfg_system_user->valuestring,
                        cfg_system_password->valuestring,
                        cfg_system_db->valuestring,
                        cfg_system_connection_string->valuestring,
                        tmp_db,
                        cfg_system_db_name->valuestring,
                        TRUE
                    );

                    if( tmp_queries ) {
                        DATABASE_SYSTEM_add(
                            cfg_system_name->valuestring,
                            tmp_db,
                            &APP.DB,
                            APP.STAGING ? APP.STAGING : NULL,
                            *tmp_queries,
                            tmp_queries_count,
                            TRUE
                        );

                        for( int i = 0; i < tmp_queries_count; i++ ) {
                            free( tmp_queries[ i ] ); tmp_queries[ i ] = NULL;
                        }
                    }

                    free( tmp_queries ); tmp_queries = NULL;

                    free( tmp_db );
                    LOG_print( "[%s] Finished loading system %s.\n\n", TIME_get_gmt(), cfg_system_name->valuestring );
                }
            } else {
                LOG_print( "ERROR: \"system\" key not found.\n" );
                cJSON_Delete( config_json );
                return FALSE;
            }

            cJSON_Delete( config_json );
        }
        free( config_string );
        config_string = NULL;

        result = 1;
    } else {
        LOG_print( "[%s] Fatal error: unable to open config file \"%s\"!\n", TIME_get_gmt(), filename );
    }

    return result;
}


int main( int argc, char** argv ) {
    char        config_file[ MAX_PATH_LENGTH + 1 ];

    signal( SIGINT, (__sighandler_t)&app_terminate );
#ifndef _WIN32
    signal( SIGPIPE, SIG_IGN );
#endif
    signal( SIGABRT, (__sighandler_t)&app_terminate );
    signal( SIGTERM, (__sighandler_t)&app_terminate );

    LOG_init( "dcpam-etl" );

    memset( config_file, '\0', MAX_PATH_LENGTH );
    if( argc <= 1 ) {
        snprintf( config_file, MAX_PATH_LENGTH, "etl_config.json" );
    } else {
        if( strlen( argv[ 1 ] ) > MAX_PATH_LENGTH ) {
            LOG_print( "[%s] Notice: \"%s\" is not valid config file name.\n", TIME_get_gmt(), argv[ 1 ] );
            snprintf( config_file, MAX_PATH_LENGTH, "etl_config.json" );
        } else {
            snprintf( config_file, MAX_PATH_LENGTH, "%s", argv[ 1 ] );
        }
    }

    if( DCPAM_load_configuration( config_file ) == 1 ) {
        if( DB_WORKER_init() == 1 ) {
            //while( 1 );
        }
    }

    DCPAM_free_configuration();

    return 0;
}
