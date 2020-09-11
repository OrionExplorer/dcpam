#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include "../include/utils/log.h"
#include "../include/core/db/worker.h"
#include "../include/core/schema.h"
#include "../include/third-party/cJSON.h"
#include "../include/utils/memory.h"
#include "../include/utils/time.h"
#include "../include/utils/strings.h"
#include "../include/utils/filesystem.h"
#include "../include/core/network/socket_io.h"
#include "../include/core/db/system.h"
#include "../include/core/lcs_report.h"

#pragma warning( disable : 6031 )

char                    app_path[ MAX_PATH_LENGTH + 1 ];
LOG_OBJECT              dcpam_etl_log;
LOG_OBJECT              dcpam_etl_lcs_log;
extern int              app_terminated = 0;

extern DATABASE_SYSTEM  DATABASE_SYSTEMS[ MAX_DATA_SYSTEMS ];
extern int              DATABASE_SYSTEMS_COUNT;
extern DCPAM_APP        APP;

void DCPAM_free_configuration( void );


void app_terminate( void ) {
    LOG_print( &dcpam_etl_log, "\r\n[%s] Received termination signal.\n", TIME_get_gmt() );
    if( app_terminated == 0 ) {
        app_terminated = 1;
        printf( "\r" );
        DB_WORKER_shutdown( &dcpam_etl_log );
        DCPAM_free_configuration();
        LOG_print( &dcpam_etl_log, "[%s] DCPAM graceful shutdown finished. Waiting for all threads to terminate...\n", TIME_get_gmt() );
        LOG_free( &dcpam_etl_lcs_log );
    }

    return;
}


void DCPAM_free_configuration( void ) {

    DATABASE_SYSTEM_DB_free( &APP.DB, & dcpam_etl_log );
    if( APP.STAGING ) {
        DATABASE_SYSTEM_DB_free( APP.STAGING, &dcpam_etl_log );
        free( APP.STAGING ); APP.STAGING = NULL;
    }

    LCS_REPORT_free( &APP.lcs_report );

    if( APP.version != NULL ) { free( APP.version ); APP.version = NULL; }
    if( APP.name != NULL ) { free( APP.name ); APP.name = NULL; }

    for( int i = 0; i < APP.DATA_len; i++ ) {
        if( APP.DATA[ i ].id != NULL ) { free( APP.DATA[ i ].id ); APP.DATA[ i ].id = NULL; }
        if( APP.DATA[ i ].name != NULL ) { free( APP.DATA[ i ].name ); APP.DATA[ i ].name = NULL; }
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

    cJSON* cfg_lcs = NULL;
    cJSON* cfg_lcs_address = NULL;
    cJSON* cfg_lcs_port = NULL;
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
    cJSON* cfg_system_flat_file = NULL;
    cJSON* cfg_system_flat_file_name = NULL;
    cJSON* cfg_system_flat_file_columns_array = NULL;
    cJSON* cfg_system_flat_file_columns_item = NULL;
    cJSON* cfg_system_flat_file_delimiter = NULL;
    cJSON* cfg_system_flat_file_load_sql = NULL;

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
    cJSON* cfg_system_query_item_etl_transform_inserted_api_key = NULL;
    cJSON* cfg_system_query_item_etl_transform_deleted_array = NULL;
    cJSON* cfg_system_query_item_etl_transform_deleted_item = NULL;
    cJSON* cfg_system_query_item_etl_transform_deleted_module = NULL;
    cJSON* cfg_system_query_item_etl_transform_deleted_staged_data = NULL;
    cJSON* cfg_system_query_item_etl_transform_deleted_api_key = NULL;
    cJSON* cfg_system_query_item_etl_transform_deleted_source_system_update = NULL;
    cJSON* cfg_system_query_item_etl_transform_modified_array = NULL;
    cJSON* cfg_system_query_item_etl_transform_modified_item = NULL;
    cJSON* cfg_system_query_item_etl_transform_modified_module = NULL;
    cJSON* cfg_system_query_item_etl_transform_modified_staged_data = NULL;
    cJSON* cfg_system_query_item_etl_transform_modified_api_key = NULL;
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


    LOG_print( &dcpam_etl_log, "[%s] DCPAM_load_configuration( %s ).\n", TIME_get_gmt(), filename );

    config_string = file_get_content( filename );

    if( config_string ) {
        config_json = cJSON_Parse( config_string );
        if( config_json ) {

            DATABASE_SYSTEM_QUERY       **tmp_queries = SAFEMALLOC( MAX_SYSTEM_QUERIES * sizeof * tmp_queries, __FILE__, __LINE__ );

            cfg_app = cJSON_GetObjectItem( config_json, "app" );
            if( cfg_app ) {

                cfg_app_name = cJSON_GetObjectItem( cfg_app, "name" );
                if( cfg_app_name ) {
                    size_t str_len = strlen( cfg_app_name->valuestring );
                    APP.name = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                    snprintf( APP.name, str_len + 1, cfg_app_name->valuestring );
                } else {
                    LOG_print( &dcpam_etl_log, "ERROR: \"app.name\" key not found.\n" );
                    cJSON_Delete( config_json );
                    free( config_string ); config_string = NULL;
                    return FALSE;
                }

                cfg_app_run_once = cJSON_GetObjectItem( cfg_app, "run_once" );
                if( cfg_app_run_once ) {
                    APP.run_once = cfg_app_run_once->valueint;
                    LOG_print( &dcpam_etl_log, "NOTICE: DCPAM is in %s run mode.\n", APP.run_once == 1 ? "one-time" : "persistent" );
                } else {
                    LOG_print( &dcpam_etl_log, "NOTICE: \"run_once\" key not found. DCPAM is in persistent run mode.\n" );
                    APP.run_once = 0;
                }

                cfg_app_version = cJSON_GetObjectItem( cfg_app, "version" );
                if( cfg_app_version ) {
                    size_t str_len = strlen( cfg_app_version->valuestring );
                    APP.version = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                    snprintf( APP.version, str_len + 1, cfg_app_version->valuestring );
                    LOG_print( &dcpam_etl_log, "%s v%s.\n", APP.name, APP.version );
                } else {
                    LOG_print( &dcpam_etl_log, "ERROR: \"app.version\" key not found.\n" );
                    cJSON_Delete( config_json );
                    free( config_string ); config_string = NULL;
                    return FALSE;
                }

                cfg_app_db = cJSON_GetObjectItem( cfg_app, "DB" );
                if( cfg_app_db ) {

                    cfg_app_db_ip = cJSON_GetObjectItem( cfg_app_db, "ip" );
                    if( cfg_app_db_ip == NULL ) {
                        LOG_print( &dcpam_etl_log, "ERROR: \"app.DB.ip\" key not found.\n" );
                    }

                    cfg_app_db_port = cJSON_GetObjectItem( cfg_app_db, "port" );
                    if( cfg_app_db_port == NULL ) {
                        LOG_print( &dcpam_etl_log, "ERROR: \"app.DB.port\" key not found.\n" );
                    }

                    cfg_app_db_driver = cJSON_GetObjectItem( cfg_app_db, "driver" );
                    if( cfg_app_db_driver == NULL ) {
                        LOG_print( &dcpam_etl_log, "ERROR: \"app.DB.driver\" key not found.\n" );
                    }

                    cfg_app_db_user = cJSON_GetObjectItem( cfg_app_db, "user" );
                    if( cfg_app_db_user == NULL ) {
                        LOG_print( &dcpam_etl_log, "ERROR: \"app.DB.user\" key not found.\n" );
                    }

                    cfg_app_db_password = cJSON_GetObjectItem( cfg_app_db, "password" );
                    if( cfg_app_db_password == NULL ) {
                        LOG_print( &dcpam_etl_log, "ERROR: \"app.DB.password\" key not found.\n" );
                    }

                    cfg_app_db_connection_string = cJSON_GetObjectItem( cfg_app_db, "connection_string" );
                    if( cfg_app_db_connection_string == NULL ) {
                        LOG_print( &dcpam_etl_log, "ERROR: \"app.DB.connection_string\" key not found.\n" );
                    }

                    cfg_app_db_db = cJSON_GetObjectItem( cfg_app_db, "db" );
                    if( cfg_app_db_db == NULL ) {
                        LOG_print( &dcpam_etl_log, "ERROR: \"app.DB.db\" key not found.\n" );
                    }

                    cfg_app_db_name = cJSON_GetObjectItem( cfg_app_db, "name" );
                    if( cfg_app_db_name == NULL ) {
                        LOG_print( &dcpam_etl_log, "Error: \"app.DB.name\" key not found.\n" );
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
                        TRUE,
                        &dcpam_etl_log
                    );
                } else {
                    LOG_print( &dcpam_etl_log, "ERROR: \"app.DB\" key not found.\n" );
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
                        LOG_print( &dcpam_etl_log, "ERROR: \"app.STAGING.ip\" key not found.\n" );
                    }

                    cfg_app_sa_port = cJSON_GetObjectItem( cfg_app_sa, "port" );
                    if( cfg_app_sa_port == NULL ) {
                        LOG_print( &dcpam_etl_log, "ERROR: \"app.STAGING.port\" key not found.\n" );
                    }

                    cfg_app_sa_driver = cJSON_GetObjectItem( cfg_app_sa, "driver" );
                    if( cfg_app_sa_driver == NULL ) {
                        LOG_print( &dcpam_etl_log, "ERROR: \"app.STAGING.driver\" key not found.\n" );
                    }

                    cfg_app_sa_user = cJSON_GetObjectItem( cfg_app_sa, "user" );
                    if( cfg_app_sa_user == NULL ) {
                        LOG_print( &dcpam_etl_log, "ERROR: \"app.STAGING.user\" key not found.\n" );
                    }

                    cfg_app_sa_password = cJSON_GetObjectItem( cfg_app_sa, "password" );
                    if( cfg_app_sa_password == NULL ) {
                        LOG_print( &dcpam_etl_log, "ERROR: \"app.STAGING.password\" key not found.\n" );
                    }

                    cfg_app_sa_connection_string = cJSON_GetObjectItem( cfg_app_sa, "connection_string" );
                    if( cfg_app_sa_connection_string == NULL ) {
                        LOG_print( &dcpam_etl_log, "ERROR: \"app.STAGING.connection_string\" key not found.\n" );
                    }

                    cfg_app_sa_db = cJSON_GetObjectItem( cfg_app_sa, "db" );
                    if( cfg_app_sa_db == NULL ) {
                        LOG_print( &dcpam_etl_log, "ERROR: \"app.STAGING.db\" key not found.\n" );
                    }

                    cfg_app_sa_name = cJSON_GetObjectItem( cfg_app_sa, "name" );
                    if( cfg_app_sa_name == NULL ) {
                        LOG_print( &dcpam_etl_log, "ERROR: \"app.STAGING.name\" key not found.\n" );
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
                        TRUE,
                        &dcpam_etl_log
                    );
                } else {
                    APP.STAGING = NULL;
                    LOG_print( &dcpam_etl_log, "NOTICE: \"app.STAGING\" key not found. Staging Area is local.\n" );
                    /*cJSON_Delete( config_json );
                    free( config_string ); config_string = NULL;
                    return FALSE;*/
                }
            } else {
                LOG_print( &dcpam_etl_log, "ERROR: \"app\" key not found.\n " );
                cJSON_Delete( config_json );
                free( config_string ); config_string = NULL;
                return FALSE;
            }

            cfg_lcs = cJSON_GetObjectItem( config_json, "LCS" );
            if( cfg_lcs ) {

                cfg_lcs_address = cJSON_GetObjectItem( cfg_lcs, "address" );

                if( cfg_lcs_address ) {
                    size_t address_len = strlen( cfg_lcs_address->valuestring );
                    APP.lcs_report.address = SAFECALLOC( address_len + 1, sizeof( char ), __FILE__, __LINE__ );
                    snprintf( APP.lcs_report.address, address_len + 1, cfg_lcs_address->valuestring );
                    if( LCS_REPORT_init( &APP.lcs_report, APP.lcs_report.address, APP.name, APP.version, &dcpam_etl_log ) == 0 ) {
                        LOG_print( &dcpam_etl_log, "ERROR: unable to connect to Live Component State host at %s.\n", APP.lcs_report.address );
                        free( APP.lcs_report.address ); APP.lcs_report.address = NULL;
                        free( APP.lcs_report.conn ); APP.lcs_report.conn = NULL;
                        cJSON_Delete( config_json );
                        free( config_string ); config_string = NULL;
                        return FALSE;
                    } else {
                        LOG_print( &dcpam_etl_log, "[%s] Initialized LCS report module.\n", TIME_get_gmt() );
                    }
                } else {
                    LOG_print( &dcpam_etl_log, "ERROR: \"LCS.address\" key not found.\n " );
                    cJSON_Delete( config_json );
                    free( config_string ); config_string = NULL;
                    return FALSE;
                }

                cfg_lcs_port = cJSON_GetObjectItem( cfg_lcs, "port" );

                if( cfg_lcs_port ) {
                    APP.lcs_report.port = cfg_lcs_port->valueint;
                } else {
                    APP.lcs_report.port = 7777;
                }

            } else {
                LOG_print( &dcpam_etl_log, "ERROR: \"LCS\" key not found.\n " );
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
                        LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].enabled\" key not found.\n", i );
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
                        LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].name\" key not found.\n", i );
                        cJSON_Delete( config_json );
                        free( config_string ); config_string = NULL;
                        return FALSE;
                    }
                    LOG_print( &dcpam_etl_log, "\n[%s] Loading system data: %s.\n", TIME_get_gmt(), cfg_system_name->valuestring );

                    cfg_system_info = cJSON_GetObjectItem( array_value, "DB" );
                    if( cfg_system_info == NULL ) {
                        LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].DB\" key not found.\n", i );
                        cJSON_Delete( config_json );
                        free( config_string ); config_string = NULL;
                        return FALSE;
                    }

                    cfg_system_ip = cJSON_GetObjectItem( cfg_system_info, "ip" );
                    if( cfg_system_ip == NULL ) {
                        LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].DB.ip\" key not found.\n", i );
                        cJSON_Delete( config_json );
                        free( config_string ); config_string = NULL;
                        return FALSE;
                    }
                    cfg_system_port = cJSON_GetObjectItem( cfg_system_info, "port" );
                    if( cfg_system_port == NULL ) {
                        LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].DB.port\" key not found.\n", i );
                        cJSON_Delete( config_json );
                        free( config_string ); config_string = NULL;
                        return FALSE;
                    }
                    cfg_system_driver = cJSON_GetObjectItem( cfg_system_info, "driver" );
                    if( cfg_system_driver == NULL ) {
                        LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].DB.driver\" key not found.\n", i );
                        cJSON_Delete( config_json );
                        free( config_string ); config_string = NULL;
                        return FALSE;
                    }
                    cfg_system_user = cJSON_GetObjectItem( cfg_system_info, "user" );
                    if( cfg_system_user == NULL ) {
                        LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].DB.user\" key not found.\n", i );
                        cJSON_Delete( config_json );
                        free( config_string ); config_string = NULL;
                        return FALSE;
                    }
                    cfg_system_db = cJSON_GetObjectItem( cfg_system_info, "db" );
                    if( cfg_system_db == NULL ) {
                        LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].DB.db\" key not found.\n", i );
                        cJSON_Delete( config_json );
                        free( config_string ); config_string = NULL;
                        return FALSE;
                    }
                    cfg_system_password = cJSON_GetObjectItem( cfg_system_info, "password" );
                    if( cfg_system_password == NULL ) {
                        LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].DB.password\" key not found.\n", i );
                        cJSON_Delete( config_json );
                        free( config_string ); config_string = NULL;
                        return FALSE;
                    }
                    cfg_system_connection_string = cJSON_GetObjectItem( cfg_system_info, "connection_string" );
                    if( cfg_system_connection_string == NULL ) {
                        LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].DB.connection_string\" key not found.\n", i );
                        cJSON_Delete( config_json );
                        free( config_string ); config_string = NULL;
                        return FALSE;
                    }

                    cfg_system_db_name = cJSON_GetObjectItem( cfg_system_info, "name" );
                    if( cfg_system_db_name == NULL ) {
                        LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].DB.name\" key not found.\n", i );
                        cJSON_Delete( config_json );
                        free( config_string ); config_string = NULL;
                        return FALSE;
                    }

                    int tmp_queries_count = 0;
                    int tmp_data_types_len = 0;

                    cfg_system_queries_array = cJSON_GetObjectItem( array_value, "queries" );
                    if( cfg_system_queries_array == NULL ) {
                        LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries\" key not found.\n", i );
                        cJSON_Delete( config_json );
                        free( config_string ); config_string = NULL;
                        return FALSE;
                    }
                    for( int j = 0; j < cJSON_GetArraySize( cfg_system_queries_array ); j++ ) {
                        cfg_system_query_item = cJSON_GetArrayItem( cfg_system_queries_array, j );

                        DB_SYSTEM_ETL *tmp_cdc = SAFEMALLOC( sizeof(DB_SYSTEM_ETL), __FILE__, __LINE__ );

                        cfg_system_query_item_name = cJSON_GetObjectItem( cfg_system_query_item, "name" );
                        if( cfg_system_query_item_name == NULL ) {
                            LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].name\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }

                        cfg_system_query_item_mode = cJSON_GetObjectItem( cfg_system_query_item, "mode" );
                        if( cfg_system_query_item_mode == NULL ) {
                            LOG_print( &dcpam_etl_log, "NOTICE: \"system[%d].queries[%d].mode\" key not found. Default mode is \"ETL\" (1).\n", i, j );
                        }

                        cfg_system_query_item_etl = cJSON_GetObjectItem( cfg_system_query_item, "etl" );
                        if( cfg_system_query_item_etl == NULL ) {
                            LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        /*
                            etl.pre_actions
                        */
                        cfg_system_query_item_etl_pre_actions = cJSON_GetObjectItem( cfg_system_query_item_etl, "pre_actions" );
                        if( cfg_system_query_item_etl_pre_actions == NULL ) {
                            LOG_print( &dcpam_etl_log, "NOTICE: No PreETL actions defined.\n" );
                            tmp_cdc->pre_actions = NULL;
                            tmp_cdc->pre_actions_count = 0;
                        } else {
                            tmp_cdc->pre_actions_count = cJSON_GetArraySize( cfg_system_query_item_etl_pre_actions );
                            tmp_cdc->pre_actions = SAFEMALLOC( tmp_cdc->pre_actions_count * sizeof * tmp_cdc->pre_actions, __FILE__, __LINE__ );

                            for( int k = 0; k < tmp_cdc->pre_actions_count; k++ ) {
                                cfg_system_query_item_etl_pre_action_item = cJSON_GetArrayItem( cfg_system_query_item_etl_pre_actions, k );
                                if( cfg_system_query_item_etl_pre_action_item == NULL ) {
                                    LOG_print( &dcpam_etl_log, "WARNING: element at position %d in PreCDC actions is invalid!\n", k );
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
                            LOG_print( &dcpam_etl_log, "NOTICE: No PostETL actions defined.\n" );
                            tmp_cdc->post_actions = NULL;
                            tmp_cdc->post_actions_count = 0;
                        } else {
                            tmp_cdc->post_actions_count = cJSON_GetArraySize( cfg_system_query_item_etl_post_actions );
                            tmp_cdc->post_actions = SAFEMALLOC( tmp_cdc->post_actions_count * sizeof * tmp_cdc->post_actions, __FILE__, __LINE__ );

                            for( int k = 0; k < tmp_cdc->post_actions_count; k++ ) {
                                cfg_system_query_item_etl_post_action_item = cJSON_GetArrayItem( cfg_system_query_item_etl_post_actions, k );
                                if( cfg_system_query_item_etl_post_action_item == NULL ) {
                                    LOG_print( &dcpam_etl_log, "WARNING: element at position %d in PreCDC actions is invalid!\n", k );
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
                            LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.extract\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        /*
                            etl.extract.inserted
                        */
                        cfg_system_query_item_etl_extract_inserted = cJSON_GetObjectItem( cfg_system_query_item_etl_extract, "inserted" );
                        if( cfg_system_query_item_etl_extract_inserted == NULL ) {
                            LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.extract.inserted\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        /*
                            etl.extract.inserted.primary_db
                        */
                        cfg_system_query_item_etl_extract_inserted_primary_db = cJSON_GetObjectItem( cfg_system_query_item_etl_extract_inserted, "primary_db" );
                        if( cfg_system_query_item_etl_extract_inserted_primary_db == NULL ) {
                            LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.extract.inserted.primary_db\" key not found.\n", i, j );
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
                            LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.extract.inserted.primary_db_sql\" key not found.\n", i, j );
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
                            LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.extract.inserted.secondary_db\" key not found.\n", i, j );
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
                            LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.extract.inserted.secondary_db_sql\" key not found.\n", i, j );
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
                            LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.extract.modified\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        /*
                            etl.extract.modified.primary_db
                        */
                        cfg_system_query_item_etl_extract_modified_primary_db = cJSON_GetObjectItem( cfg_system_query_item_etl_extract_modified, "primary_db" );
                        if( cfg_system_query_item_etl_extract_modified_primary_db == NULL ) {
                            LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.extract.modified.primary_db\" key not found.\n", i, j );
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
                            LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.extract.modified.primary_db_sql\" key not found.\n", i, j );
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
                            LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.extract.modified.secondary_db\" key not found.\n", i, j );
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
                            LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.extract.modified.secondary_db_sql\" key not found.\n", i, j );
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
                            LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.extract.deleted\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        /*
                            etl.extract.deleted.primary_db
                        */
                        cfg_system_query_item_etl_extract_deleted_primary_db = cJSON_GetObjectItem( cfg_system_query_item_etl_extract_deleted, "primary_db" );
                        if( cfg_system_query_item_etl_extract_deleted_primary_db == NULL ) {
                            LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.extract.deleted.primary_db\" key not found.\n", i, j );
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
                            LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.extract.deleted.primary_db_sql\" key not found.\n", i, j );
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
                            LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.extract.deleted.secondary_db\" key not found.\n", i, j );
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
                            LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.extract.deleted.secondary_db_sql\" key not found.\n", i, j );
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
                            LOG_print( &dcpam_etl_log, "NOTICE: \"system[%d].queries[%d].etl.stage\" key not found.\n", i, j );
                            tmp_cdc->stage = NULL;
                        } else {
                            tmp_cdc->stage = SAFEMALLOC( sizeof( DB_SYSTEM_ETL_STAGE ), __FILE__, __LINE__ );
                            /*
                                etl.stage.inserted
                            */
                            cfg_system_query_item_etl_stage_inserted = cJSON_GetObjectItem( cfg_system_query_item_etl_stage, "inserted" );
                            if( cfg_system_query_item_etl_stage_inserted == NULL ) {
                                LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.stage.inserted\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                return FALSE;
                            }
                            /*
                                etl.stage.inserted.sql
                            */
                            cfg_system_query_item_etl_stage_inserted_sql = cJSON_GetObjectItem( cfg_system_query_item_etl_stage_inserted, "sql" );
                            if( cfg_system_query_item_etl_stage_inserted_sql == NULL ) {
                                LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.stage.inserted.sql\" key not found.\n", i, j );
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
                                LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.stage.inserted.extracted_values\" key not found.\n", i, j );
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
                                LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.stage.deleted\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                return FALSE;
                            }
                            /*
                                etl.stage.deleted.sql
                            */
                            cfg_system_query_item_etl_stage_deleted_sql = cJSON_GetObjectItem( cfg_system_query_item_etl_stage_deleted, "sql" );
                            if( cfg_system_query_item_etl_stage_deleted_sql == NULL ) {
                                LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.stage.deleted.sql\" key not found.\n", i, j );
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
                                LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.stage.deleted.extracted_values\" key not found.\n", i, j );
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
                                LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.stage.modified\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                return FALSE;
                            }
                            /*
                                etl.stage.modified.sql
                            */
                            cfg_system_query_item_etl_stage_modified_sql = cJSON_GetObjectItem( cfg_system_query_item_etl_stage_modified, "sql" );
                            if( cfg_system_query_item_etl_stage_modified_sql == NULL ) {
                                LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.stage.modified.sql\" key not found.\n", i, j );
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
                                LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.stage.modified.extracted_values\" key not found.\n", i, j );
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
                            LOG_print( &dcpam_etl_log, "NOTICE: \"system[%d].queries[%d].etl.transform\" key not found.\n", i, j );
                            tmp_cdc->transform = NULL;
                        } else {
                            tmp_cdc->transform = SAFEMALLOC( sizeof( DB_SYSTEM_ETL_TRANSFORM ), __FILE__, __LINE__ );

                            /*
                                etl.transform.inserted
                            */
                            cfg_system_query_item_etl_transform_inserted_array = cJSON_GetObjectItem( cfg_system_query_item_etl_transform, "inserted" );
                            if( cfg_system_query_item_etl_transform_inserted_array == NULL ) {
                                LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.transform.inserted\" key not found.\n", i, j );
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
                                    str_len3
                                );

                                cfg_system_query_item_etl_transform_inserted_api_key = cJSON_GetObjectItem( cfg_system_query_item_etl_transform_inserted_item, "api_key" );
                                size_t str_len4 = strlen( cfg_system_query_item_etl_transform_inserted_api_key->valuestring );
                                tmp_cdc->transform->inserted[ k ]->api_key = SAFECALLOC( str_len4 + 1, sizeof( char ), __FILE__, __LINE__ );
                                strncpy(
                                    tmp_cdc->transform->inserted[ k ]->api_key,
                                    cfg_system_query_item_etl_transform_inserted_api_key->valuestring,
                                    str_len4
                                );
                            }

                            /*
                                etl.transform.deleted
                            */
                            cfg_system_query_item_etl_transform_deleted_array = cJSON_GetObjectItem( cfg_system_query_item_etl_transform, "deleted" );
                            if( cfg_system_query_item_etl_transform_deleted_array == NULL ) {
                                LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.transform.deleted\" key not found.\n", i, j );
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

                                cfg_system_query_item_etl_transform_deleted_api_key = cJSON_GetObjectItem( cfg_system_query_item_etl_transform_deleted_item, "api_key" );
                                size_t str_len4 = strlen( cfg_system_query_item_etl_transform_deleted_api_key->valuestring );
                                tmp_cdc->transform->deleted[ k ]->api_key = SAFECALLOC( str_len4 + 1, sizeof( char ), __FILE__, __LINE__ );
                                strncpy(
                                    tmp_cdc->transform->deleted[ k ]->api_key,
                                    cfg_system_query_item_etl_transform_deleted_api_key->valuestring,
                                    str_len4
                                );
                            }

                            /*
                                etl.transform.modified
                            */
                            cfg_system_query_item_etl_transform_modified_array = cJSON_GetObjectItem( cfg_system_query_item_etl_transform, "modified" );
                            if( cfg_system_query_item_etl_transform_modified_array == NULL ) {
                                LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.transform.modified\" key not found.\n", i, j );
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

                                cfg_system_query_item_etl_transform_modified_api_key = cJSON_GetObjectItem( cfg_system_query_item_etl_transform_modified_item, "api_key" );
                                size_t str_len4 = strlen( cfg_system_query_item_etl_transform_modified_api_key->valuestring );
                                tmp_cdc->transform->modified[ k ]->api_key = SAFECALLOC( str_len4 + 1, sizeof( char ), __FILE__, __LINE__ );
                                strncpy(
                                    tmp_cdc->transform->modified[ k ]->api_key,
                                    cfg_system_query_item_etl_transform_modified_api_key->valuestring,
                                    str_len4
                                );
                            }
                        }

                        /*
                            etl.load
                        */
                        cfg_system_query_item_etl_load = cJSON_GetObjectItem( cfg_system_query_item_etl, "load" );
                        if( cfg_system_query_item_etl_load == NULL ) {
                            LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.load\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        /*
                            etl.load.inserted
                        */
                        cfg_system_query_item_etl_load_inserted = cJSON_GetObjectItem( cfg_system_query_item_etl_load, "inserted" );
                        if( cfg_system_query_item_etl_load_inserted == NULL ) {
                            LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.load.inserted\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        /*
                            etl.load.inserted.input_data_sql
                        */
                        cfg_system_query_item_etl_load_inserted_input_data_sql = cJSON_GetObjectItem( cfg_system_query_item_etl_load_inserted, "input_data_sql" );
                        if( cfg_system_query_item_etl_load_inserted_input_data_sql == NULL ) {
                            LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.load.inserted.input_data_sql\" key not found.\n", i, j );
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
                            LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.load.inserted.output_data_sql\" key not found.\n", i, j );
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
                            LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.load.inserted.extracted_values\" key not found.\n", i, j );
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
                            LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.load.deleted\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        /*
                            etl.load.deleted.input_data_sql
                        */
                        cfg_system_query_item_etl_load_deleted_input_data_sql = cJSON_GetObjectItem( cfg_system_query_item_etl_load_deleted, "input_data_sql" );
                        if( cfg_system_query_item_etl_load_deleted_input_data_sql == NULL ) {
                            LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.load.deleted.input_data_sql\" key not found.\n", i, j );
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
                            LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.load.deleted.output_data_sql\" key not found.\n", i, j );
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
                            LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.load.deleted.extracted_values\" key not found.\n", i, j );
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
                            LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.load.modified\" key not found.\n", i, j );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        /*
                            etl.load.modified.input_data_sql
                        */
                        cfg_system_query_item_etl_load_modified_input_data_sql = cJSON_GetObjectItem( cfg_system_query_item_etl_load_modified, "input_data_sql" );
                        if( cfg_system_query_item_etl_load_modified_input_data_sql == NULL ) {
                            LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.load.modified.input_data_sql\" key not found.\n", i, j );
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
                            LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.load.modified.output_data_sql\" key not found.\n", i, j );
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
                            LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].queries[%d].etl.load.modified.extracted_values\" key not found.\n", i, j );
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
                            FALSE,
                            &dcpam_etl_log
                        );

                        tmp_queries_count++;

                        //if( tmp_cdc->stage ) free( tmp_cdc->stage ); tmp_cdc->stage = NULL;
                        free( tmp_cdc ); tmp_cdc = NULL;
                    }

                    cfg_system_flat_file = cJSON_GetObjectItem( array_value, "FILE" );
                    DATABASE_SYSTEM_FLAT_FILE* tmp_flat_file = NULL;
                    if( cfg_system_flat_file ) {
                        LOG_print( &dcpam_etl_log, "[%s] Source is file.\n", TIME_get_gmt() );
                        cfg_system_flat_file_name = cJSON_GetObjectItem( cfg_system_flat_file, "name" );

                        if( cfg_system_flat_file_name == NULL ) {
                            LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].FILE.name\" key not found.\n", i );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }

                        tmp_flat_file = SAFEMALLOC( sizeof( DATABASE_SYSTEM_FLAT_FILE ), __FILE__, __LINE__ );
                        tmp_flat_file->csv_file = NULL;
                        tmp_flat_file->json_file = NULL;

                        LOG_print( &dcpam_etl_log, "[%s] Source file name is \"%s\".\n", TIME_get_gmt(), cfg_system_flat_file_name->valuestring );

                        size_t str_len = strlen( cfg_system_flat_file_name->valuestring );
                        tmp_flat_file->name = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                        snprintf( tmp_flat_file->name, str_len + 1, cfg_system_flat_file_name->valuestring );

                        cfg_system_flat_file_columns_array = cJSON_GetObjectItem( cfg_system_flat_file, "columns" );
                        if( cfg_system_flat_file_columns_array == NULL ) {
                            LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].FILE.columns\" key not found.\n", i );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }

                        tmp_flat_file->columns_len = cJSON_GetArraySize( cfg_system_flat_file_columns_array );
                        tmp_flat_file->columns = SAFEMALLOC( tmp_flat_file->columns_len * sizeof * tmp_flat_file->columns, __FILE__, __LINE__ );
                        for( int j = 0; j < tmp_flat_file->columns_len; j++ ) {
                            cfg_system_flat_file_columns_item = cJSON_GetArrayItem( cfg_system_flat_file_columns_array, j );
                            size_t str_len_col = strlen( cfg_system_flat_file_columns_item->valuestring );
                            tmp_flat_file->columns[ j ] = SAFECALLOC( str_len_col + 1, sizeof( char ), __FILE__, __LINE__ );
                            snprintf( tmp_flat_file->columns[ j ], str_len_col + 1, cfg_system_flat_file_columns_item->valuestring );
                        }

                        cfg_system_flat_file_load_sql = cJSON_GetObjectItem( cfg_system_flat_file, "load_sql" );
                        if( cfg_system_flat_file_load_sql == NULL ) {
                            LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].FILE.load_sql\" key not found.\n", i );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }

                        size_t str_len_load_sql = strlen( cfg_system_flat_file_load_sql->valuestring );
                        tmp_flat_file->load_sql = SAFECALLOC( str_len_load_sql + 1, sizeof( char ), __FILE__, __LINE__ );
                        snprintf( tmp_flat_file->load_sql, str_len_load_sql + 1, cfg_system_flat_file_load_sql->valuestring );

                        if( strstr( tmp_flat_file->name, ".csv" ) ) {
                            tmp_flat_file->type = FFT_CSV;
                        } else if( strstr( tmp_flat_file->name, ".tsv" ) ) {
                            tmp_flat_file->type = FFT_TSV;
                        } else if( strstr( tmp_flat_file->name, ".psv" ) ) {
                            tmp_flat_file->type = FFT_PSV;
                        }

                        if( tmp_flat_file->type == FFT_CSV || tmp_flat_file->type == FFT_TSV || tmp_flat_file->type == FFT_PSV ) {
                            tmp_flat_file->csv_file = SAFEMALLOC( sizeof( CSV_FILE ), __FILE__, __LINE__ );

                            if( tmp_flat_file->type == FFT_CSV ) {
                                cfg_system_flat_file_delimiter = cJSON_GetObjectItem( cfg_system_flat_file, "delimiter" );
                                if( cfg_system_flat_file_delimiter == NULL ) {
                                    LOG_print( &dcpam_etl_log, "ERROR: \"system[%d].FILE.delimiter\" key not found.\n", i );
                                    cJSON_Delete( config_json );
                                    free( config_string ); config_string = NULL;
                                    return FALSE;
                                }
                                snprintf( tmp_flat_file->delimiter, 2, cfg_system_flat_file_delimiter->valuestring );
                            } else if( tmp_flat_file->type == FFT_TSV ) {
                                snprintf( tmp_flat_file->delimiter, 2, "%s", "\t" );
                            } else if( tmp_flat_file->type == FFT_PSV ) {
                                snprintf( tmp_flat_file->delimiter, 2, "%s", "|" );
                            }
                        } else if( strstr( tmp_flat_file->name, ".json" ) ) {
                            tmp_flat_file->type = FFT_JSON;
                            tmp_flat_file->json_file = SAFEMALLOC( sizeof( JSON_FILE ), __FILE__, __LINE__ );
                        }
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
                        TRUE,
                        &dcpam_etl_log
                    );

                    if( tmp_queries ) {
                        DATABASE_SYSTEM_add(
                            cfg_system_name->valuestring,
                            tmp_db,
                            &APP.DB,
                            APP.STAGING ? APP.STAGING : NULL,
                            tmp_flat_file,
                            *tmp_queries,
                            tmp_queries_count,
                            TRUE,
                            &dcpam_etl_log
                        );

                        for( int i = 0; i < tmp_queries_count; i++ ) {
                            free( tmp_queries[ i ] ); tmp_queries[ i ] = NULL;
                        }

                        if( tmp_flat_file ) {
                            free( tmp_flat_file->name ); tmp_flat_file->name = NULL;
                            free( tmp_flat_file->load_sql ); tmp_flat_file->load_sql = NULL;
                            for( int i = 0; i < tmp_flat_file->columns_len; i++ ) {
                                free( tmp_flat_file->columns[ i ] ); tmp_flat_file->columns[ i ] = NULL;
                            }
                            free( tmp_flat_file->columns ); tmp_flat_file->columns = NULL;
                            memset( tmp_flat_file->delimiter, '\0', 1 );
                            free( tmp_flat_file ); tmp_flat_file = NULL;
                        }
                    }

                    free( tmp_db );
                    LOG_print( &dcpam_etl_log, "[%s] Finished loading system %s.\n\n", TIME_get_gmt(), cfg_system_name->valuestring );
                }
                free( tmp_queries ); tmp_queries = NULL;

            } else {
                LOG_print( &dcpam_etl_log, "ERROR: \"system\" key not found.\n" );
                cJSON_Delete( config_json );
                return FALSE;
            }

            cJSON_Delete( config_json );
        }
        free( config_string );
        config_string = NULL;

        result = 1;
    } else {
        LOG_print( &dcpam_etl_log, "[%s] Fatal error: unable to open config file \"%s\"!\n", TIME_get_gmt(), filename );
    }

    return result;
}

void DCPAM_ETL_LCS_listener( COMMUNICATION_SESSION* communication_session, CONNECTED_CLIENT* client, LOG_OBJECT* log ) {
    if( communication_session ) {
        if( communication_session->data_length > 0 ) {

            cJSON *req = cJSON_Parse( communication_session->content );
            if( req ) {
                cJSON *msg = cJSON_GetObjectItem( req, "msg" );

                if( msg ) {

                    if( strcmp( msg->valuestring, "ping" ) == 0 ) {
                        const char* pong_msg = "{\"msg\": \"pong\"}";
                        SOCKET_send( communication_session, client, pong_msg, strlen( pong_msg ) );
                        SOCKET_disconnect_client( communication_session );
                        cJSON_Delete( req );
                        return;
                    }
                }
                cJSON_Delete( req );
            }

            SOCKET_send( communication_session, client, "-1", 2 );
            SOCKET_disconnect_client( communication_session );

        }
    }
}

void* DCPAM_LCS_worker( void* LCS_worker_data ) {
    LOG_init( &dcpam_etl_lcs_log, "dcpam-etl-lcs", 65535 );

    char **allowed_hosts = SAFEMALLOC( sizeof * allowed_hosts, __FILE__, __LINE__);
    size_t lcs_host_len = strlen( APP.lcs_report.lcs_host );
    allowed_hosts[ 0 ] = SAFECALLOC( lcs_host_len + 1, sizeof( char ), __FILE__, __LINE__ );
    snprintf( allowed_hosts[ 0 ], lcs_host_len + 1, APP.lcs_report.lcs_host );

    spc exec_script = ( spc )&DCPAM_ETL_LCS_listener;
    SOCKET_main( &exec_script, APP.lcs_report.port, ( const char** )&( *allowed_hosts ), 1, &dcpam_etl_lcs_log );

    free( allowed_hosts[ 0 ] ); allowed_hosts[ 0 ] = NULL;
    free( allowed_hosts ); allowed_hosts = NULL;
    pthread_exit( NULL );
}


int main( int argc, char** argv ) {
    char        config_file[ MAX_PATH_LENGTH + 1 ];
    pthread_t   lcs_worker_pid;

    signal( SIGINT, (__sighandler_t)&app_terminate );
#ifndef _WIN32
    signal( SIGPIPE, SIG_IGN );
#endif
    signal( SIGABRT, (__sighandler_t)&app_terminate );
    signal( SIGTERM, (__sighandler_t)&app_terminate );

    srand( time( NULL ) );
    LOG_init( &dcpam_etl_log, "dcpam-etl", 65535 );

    memset( config_file, '\0', MAX_PATH_LENGTH );
    if( argc <= 1 ) {
        snprintf( config_file, MAX_PATH_LENGTH, "./conf/etl_config.json" );
    } else {
        if( strlen( argv[ 1 ] ) > MAX_PATH_LENGTH ) {
            LOG_print( &dcpam_etl_log, "[%s] Notice: \"%s\" is not valid config file name.\n", TIME_get_gmt(), argv[ 1 ] );
            snprintf( config_file, MAX_PATH_LENGTH, "etl_config.json" );
        } else {
            snprintf( config_file, MAX_PATH_LENGTH, "%s", argv[ 1 ] );
        }
    }

    if( DCPAM_load_configuration( config_file ) == 1 ) {

        if( pthread_create( &lcs_worker_pid, NULL, DCPAM_LCS_worker, NULL ) == 0 ) {
            if( DB_WORKER_init( &dcpam_etl_log ) == 1 ) {
                //while( 1 );
            }
            pthread_join( lcs_worker_pid, NULL );
        } else {
            LOG_print( &dcpam_etl_log, "[%s] Fatal error: unable to start thread for DCPAM LCS reporting.\n", TIME_get_gmt() );
        }
    }

    DCPAM_free_configuration();

    LOG_free( &dcpam_etl_log );

    return 0;
}
