#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include "include/utils/log.h"
#include "include/core/db/worker.h"
#include "include/core/schema.h"

#include "include/db/mariadb.h"
#include "include/db/mysql.h"
#include "include/db/odbc.h"
#include "include/db/postgresql.h"
#include "include/third-party/cJSON.h"
#include "include/utils/memory.h"
#include "include/utils/time.h"
#include "include/utils/strings.h"
#include "include/core/db/system.h"

#pragma warning( disable : 6031 )

char                    app_path[MAX_PATH_LENGTH];
char                    LOG_filename[MAX_PATH_LENGTH];
extern int              app_terminated = 0;

extern DATABASE_SYSTEM  DATABASE_SYSTEMS[MAX_DATA_SYSTEMS];
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

int DCPAM_load_configuration( const char *filename ) {
    FILE*           file = NULL;
    cJSON           *config_json = NULL;

    cJSON           *cfg_app = NULL;
    cJSON           *cfg_app_version = NULL;
    cJSON           *cfg_app_name = NULL;
    cJSON           *cfg_app_db = NULL;
    cJSON           *cfg_app_db_ip = NULL;
    cJSON           *cfg_app_db_port = NULL;
    cJSON           *cfg_app_db_driver = NULL;
    cJSON           *cfg_app_db_user = NULL;
    cJSON           *cfg_app_db_password = NULL;
    cJSON           *cfg_app_db_connection_string = NULL;
    cJSON           *cfg_app_db_db = NULL;
    cJSON           *cfg_app_data = NULL;
    cJSON           *cfg_app_data_item = NULL;
    cJSON           *cfg_app_data_item_id = NULL;
    cJSON           *cfg_app_data_item_name = NULL;
    cJSON           *cfg_app_data_item_db_table_name = NULL;
    cJSON           *cfg_app_data_item_description = NULL;
    cJSON           *cfg_app_data_actions = NULL;
    cJSON           *cfg_app_data_actions_item = NULL;
    cJSON           *cfg_app_data_actions_item_columns = NULL;
    cJSON           *cfg_app_data_actions_item_columns_name = NULL;
    cJSON           *cfg_app_data_actions_item_name = NULL;
    cJSON           *cfg_app_data_actions_item_description = NULL;
    cJSON           *cfg_app_data_actions_item_type = NULL;
    cJSON           *cfg_app_data_actions_item_internal = NULL;
    cJSON           *cfg_app_data_actions_item_condition = NULL;
    cJSON           *cfg_app_data_actions_item_sql = NULL;

    cJSON           *cfg_system_array = NULL;
    cJSON           *cfg_system_enabled = NULL;
    cJSON           *cfg_system_name = NULL;
    cJSON           *cfg_system_minute_ratio = NULL;
    cJSON           *cfg_system_ip = NULL;
    cJSON           *cfg_system_port = NULL;
    cJSON           *cfg_system_driver = NULL;
    cJSON           *cfg_system_user = NULL;
    cJSON           *cfg_system_db = NULL;
    cJSON           *cfg_system_password = NULL;
    cJSON           *cfg_system_connection_string = NULL;
    cJSON           *cfg_system_queries_array = NULL;
    cJSON           *cfg_system_query_item = NULL;
    cJSON           *cfg_system_query_item_name = NULL;

    cJSON           *cfg_system_query_item_change_data_capture = NULL;
    cJSON           *cfg_system_query_item_change_data_capture_extract = NULL;
    cJSON           *cfg_system_query_item_change_data_capture_extract_inserted = NULL;
    cJSON           *cfg_system_query_item_change_data_capture_extract_inserted_primary_db = NULL;
    cJSON           *cfg_system_query_item_change_data_capture_extract_inserted_primary_db_sql = NULL;
    cJSON           *cfg_system_query_item_change_data_capture_extract_inserted_secondary_db = NULL;
    cJSON           *cfg_system_query_item_change_data_capture_extract_inserted_secondary_db_sql = NULL;
    cJSON           *cfg_system_query_item_change_data_capture_extract_deleted = NULL;
    cJSON           *cfg_system_query_item_change_data_capture_extract_deleted_primary_db_sql = NULL;
    cJSON           *cfg_system_query_item_change_data_capture_extract_deleted_primary_db = NULL;
    cJSON           *cfg_system_query_item_change_data_capture_extract_deleted_secondary_db_sql = NULL;
    cJSON           *cfg_system_query_item_change_data_capture_extract_deleted_secondary_db = NULL;
    cJSON           *cfg_system_query_item_change_data_capture_extract_modified = NULL;
    cJSON           *cfg_system_query_item_change_data_capture_extract_modified_primary_db_sql = NULL;
    cJSON           *cfg_system_query_item_change_data_capture_extract_modified_primary_db = NULL;
    cJSON           *cfg_system_query_item_change_data_capture_extract_modified_secondary_db_sql = NULL;
    cJSON           *cfg_system_query_item_change_data_capture_extract_modified_secondary_db = NULL;
    cJSON           *cfg_system_query_item_change_data_capture_transform = NULL;
    cJSON           *cfg_system_query_item_change_data_capture_transform_inserted = NULL;
    cJSON           *cfg_system_query_item_change_data_capture_transform_deleted = NULL;
    cJSON           *cfg_system_query_item_change_data_capture_transform_modified = NULL;
    cJSON           *cfg_system_query_item_change_data_capture_load = NULL;
    cJSON           *cfg_system_query_item_change_data_capture_load_inserted = NULL;
    cJSON           *cfg_system_query_item_change_data_capture_load_inserted_sql = NULL;
    cJSON           *cfg_system_query_item_change_data_capture_load_inserted_extracted_values_array = NULL;
    cJSON           *cfg_system_query_item_change_data_capture_load_inserted_extracted_values_item = NULL;
    cJSON           *cfg_system_query_item_change_data_capture_load_deleted = NULL;
    cJSON           *cfg_system_query_item_change_data_capture_load_deleted_sql = NULL;
    cJSON           *cfg_system_query_item_change_data_capture_load_deleted_extracted_values_array = NULL;
    cJSON           *cfg_system_query_item_change_data_capture_load_deleted_extracted_values_item = NULL;
    cJSON           *cfg_system_query_item_change_data_capture_load_modified = NULL;
    cJSON           *cfg_system_query_item_change_data_capture_load_modified_sql = NULL;
    cJSON           *cfg_system_query_item_change_data_capture_load_modified_extracted_values_array = NULL;
    cJSON           *cfg_system_query_item_change_data_capture_load_modified_extracted_values_item = NULL;
    
    cJSON           *cfg_system_query_item_columns = NULL;
    cJSON           *cfg_system_query_item_columns_name = NULL;
    cJSON           *cfg_system_query_item_data_types = NULL;
    cJSON           *cfg_system_query_item_data_types_name = NULL;

    cJSON           *cfg_system_info = NULL;

    cJSON           *array_value = NULL;
    int             k = 0;
    int             result = 0;
    
    DATABASE_SYSTEM_DB          tmp_db;
    DATABASE_SYSTEM_QUERY       tmp_queries[ MAX_SYSTEM_QUERIES ];
    DB_SYSTEM_CDC   tmp_cdc;

    int                 tmp_columns_len = 0;
    

    LOG_print( "[%s] DCPAM_load_configuration( %s ).\n", TIME_get_gmt(), filename );
    file = fopen( filename, "r" );
    if( file != NULL ) {

        char   *config_string = NULL;
        size_t file_len = 0;

        config_string = ( char* )SAFECALLOC( MAX_BUFFER, sizeof( char ), __FILE__, __LINE__ );
        file_len = fread( config_string, MAX_BUFFER, 1, file );
        if( strlen( config_string ) > 0 ) {
            config_json = cJSON_Parse( config_string );
            if( config_json ) {

                size_t str_len = 0;

                cfg_app = cJSON_GetObjectItem( config_json, "app" );
                if( cfg_app ) {

                    cfg_app_name = cJSON_GetObjectItem( cfg_app, "name" );
                    if( cfg_app_name ) {
                        str_len = strlen( cfg_app_name->valuestring );
                        
                        /*APP.name = ( char * )SAFECALLOC( str_len+1, sizeof( char ), __FILE__, __LINE__ );
                        strncpy( APP.name, cfg_app_name->valuestring, str_len );*/
                        APP.name = strdup( cfg_app_name->valuestring );
                        LOG_print( "\t· %s", APP.name );
                    } else {
                        LOG_print( "ERROR: \"app.name\" key not found.\n" );
                        cJSON_Delete( config_json );
                        free( config_string ); config_string = NULL;
                        fclose( file );
                        return FALSE;
                    }

                    cfg_app_version = cJSON_GetObjectItem( cfg_app, "version" );
                    if( cfg_app_version ) {
                        /*str_len = strlen( cfg_app_version->valuestring );
                        APP.version = ( char * )SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                        strncpy( APP.version, cfg_app_version->valuestring, str_len );*/
                        APP.version = strdup( cfg_app_version->valuestring );
                        LOG_print( " v%s.\n", APP.version );
                    } else {
                        LOG_print( "ERROR: \"app.version\" key not found.\n" );
                        cJSON_Delete( config_json );
                        free( config_string ); config_string = NULL;
                        fclose( file );
                        return FALSE;
                    }

                    cfg_app_db = cJSON_GetObjectItem( cfg_app, "DB");
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

                        DATABASE_SYSTEM_DB_add(
                            cfg_app_db_ip->valuestring,
                            cfg_app_db_port->valueint,
                            cfg_app_db_driver->valueint,
                            cfg_app_db_user->valuestring,
                            cfg_app_db_password->valuestring,
                            cfg_app_db_db->valuestring,
                            cfg_app_db_connection_string->valuestring,
                            &APP.DB,
                            TRUE
                        );
                    }
                     else {
                        LOG_print( "ERROR: \"app.DB\" key not found.\n" );
                        cJSON_Delete( config_json );
                        free( config_string ); config_string = NULL;
                        fclose( file );
                        return FALSE;
                    }

                    LOG_print( "[%s] Loading app.DATA item ", TIME_get_gmt() );
                    cfg_app_data = cJSON_GetObjectItem( cfg_app, "DATA" );
                    if( cfg_app_data ) {
                        APP.DATA_len = cJSON_GetArraySize( cfg_app_data );
                        for( int i = 0; i < APP.DATA_len; i++ ) {
                            cfg_app_data_item = cJSON_GetArrayItem( cfg_app_data, i );

                            cfg_app_data_item_id = cJSON_GetObjectItem( cfg_app_data_item, "id" );
                            if( cfg_app_data_item_id == NULL ) {
                                LOG_print( "ERROR: \"app.DATA[%d].id\" key not found.\n", i );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                fclose( file );
                                return FALSE;
                            }
                            /*str_len = strlen( cfg_app_data_item_id->valuestring );
                            APP.DATA[ i ].id = ( char * )SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                            strncpy( APP.DATA[ i ].id, cfg_app_data_item_id->valuestring, str_len );*/
                            APP.DATA[ i ].id = strdup( cfg_app_data_item_id->valuestring );
                            LOG_print("#%d: \"%s\"\n", i+1, APP.DATA[ i ].id );

                            cfg_app_data_item_name = cJSON_GetObjectItem( cfg_app_data_item, "name" );
                            if( cfg_app_data_item_name == NULL ) {
                                LOG_print( "ERROR: \"app.DATA[%d].name\" key not found.\n", i );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                fclose( file );
                                return FALSE;
                            }
                            /*str_len = strlen( cfg_app_data_item_name->valuestring );
                            APP.DATA[ i ].name = ( char * )SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                            strncpy( APP.DATA[ i ].name, cfg_app_data_item_name->valuestring, str_len );*/
                            APP.DATA[ i ].name = strdup( cfg_app_data_item_name->valuestring );
                            LOG_print("\t· name=\"%s\"\n", APP.DATA[ i ].name );

                            cfg_app_data_item_db_table_name = cJSON_GetObjectItem( cfg_app_data_item, "db_table_name" );
                            if( cfg_app_data_item_db_table_name == NULL ) {
                                LOG_print( "ERROR: \"app.DATA[%d].db_table_name\" key not found.\n", i );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                fclose( file );
                                return FALSE;
                            }
                            /*str_len = strlen( cfg_app_data_item_db_table_name->valuestring );
                            APP.DATA[ i ].db_table_name = ( char * )SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                            strncpy( APP.DATA[ i ].db_table_name, cfg_app_data_item_db_table_name->valuestring, str_len );*/
                            APP.DATA[ i ].db_table_name = strdup( cfg_app_data_item_db_table_name->valuestring );
                            LOG_print("\t· db_table_name=\"%s\"\n", APP.DATA[ i ].db_table_name );

                            APP.DATA[ i ].columns_len = 0;
                            cfg_app_data_actions_item_columns = cJSON_GetObjectItem( cfg_app_data_item, "columns");
                            if( cfg_app_data_actions_item_columns ) {
                                LOG_print("\t· columns=[");
                                for( int k = 0; k < cJSON_GetArraySize( cfg_app_data_actions_item_columns ); k++ ) {
                                    cfg_app_data_actions_item_columns_name = cJSON_GetArrayItem( cfg_app_data_actions_item_columns, k );
                                    memset( APP.DATA[ i ].columns[ k ], 0, MAX_COLUMN_NAME_LEN );
                                    snprintf( APP.DATA[ i ].columns[ k ], MAX_COLUMN_NAME_LEN, "%s", cfg_app_data_actions_item_columns_name->valuestring );
                                    /*strncpy(
                                        APP.DATA[ i ].columns[ k ],
                                        cfg_app_data_actions_item_columns_name->valuestring,
                                        32
                                    );*/
                                    LOG_print( "\"%s\", ", cfg_app_data_actions_item_columns_name->valuestring );
                                    APP.DATA[ i ].columns_len++;
                                }
                                LOG_print( "]\n" );
                            } else {
                                LOG_print( "ERROR: \"app.DATA[%d].columns\" key not found.\n", i );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                fclose( file );
                                return FALSE;
                            }

                            cfg_app_data_item_description = cJSON_GetObjectItem( cfg_app_data_item, "description" );
                            if( cfg_app_data_item_description == NULL ) {
                                LOG_print( "ERROR: \"app.DATA[%d].description\" key not found.\n", i );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                fclose( file );
                                return FALSE;
                            }
                            /*str_len = strlen( cfg_app_data_item_description->valuestring );
                            APP.DATA[ i ].description = ( char * )SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                            strncpy( APP.DATA[ i ].description, cfg_app_data_item_description->valuestring, str_len );*/
                            APP.DATA[ i ].description = strdup( cfg_app_data_item_description->valuestring );
                            LOG_print("\t· description=\"%s\"\n", APP.DATA[ i ].description );

                            cfg_app_data_actions = cJSON_GetObjectItem( cfg_app_data_item, "actions" );
                            if( cfg_app_data_actions ) {
                                APP.DATA[ i ].actions_len = cJSON_GetArraySize( cfg_app_data_actions );
                                for( int j = 0; j < APP.DATA[ i ].actions_len; j++ ) {
                                    cfg_app_data_actions_item = cJSON_GetArrayItem( cfg_app_data_actions, j );

                                    cfg_app_data_actions_item_name = cJSON_GetObjectItem( cfg_app_data_actions_item, "name" );
                                    if( cfg_app_data_actions_item_name == NULL ) {
                                        LOG_print( "ERROR: \"app.DATA[%d].actions[%d].name\" key not found.\n", i, j );
                                        cJSON_Delete( config_json );
                                        return FALSE;
                                    }
                                    /*str_len = strlen( cfg_app_data_actions_item_name->valuestring );
                                    APP.DATA[ i ].actions[ j ].name = ( char * )SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                                    strncpy( APP.DATA[ i ].actions[ j ].name, cfg_app_data_actions_item_name->valuestring, str_len );*/
                                    APP.DATA[ i ].actions[ j ].name = strdup( cfg_app_data_actions_item_name->valuestring );
                                    LOG_print("\t· action #%d: \"%s\"\n", j + 1, APP.DATA[ i ].actions[ j ].name );

                                    cfg_app_data_actions_item_description = cJSON_GetObjectItem( cfg_app_data_actions_item, "description" );
                                    if( cfg_app_data_item_description == NULL ) {
                                        LOG_print( "ERROR: \"app.DATA[%d].actions[%d].description\" key not found.\n", i, j );
                                        cJSON_Delete( config_json );
                                        return FALSE;
                                    }
                                    /*str_len = strlen( cfg_app_data_actions_item_description->valuestring );
                                    APP.DATA[ i ].actions[ j ].description = ( char * )SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                                    strncpy( APP.DATA[ i ].actions[ j ].description, cfg_app_data_actions_item_description->valuestring, str_len );*/
                                    APP.DATA[ i ].actions[ j ].description = strdup( cfg_app_data_actions_item_description->valuestring );
                                    LOG_print("\t\t· description=\"%s\"\n", APP.DATA[ i ].actions[ j ].description );

                                    cfg_app_data_actions_item_type = cJSON_GetObjectItem( cfg_app_data_actions_item, "type" );
                                    if( cfg_app_data_actions_item_type == NULL ) {
                                        LOG_print( "ERROR: \"app.DATA[%d].actions[%d].type\" key not found.\n", i, j );
                                        cJSON_Delete( config_json );
                                        return FALSE;
                                    }
                                    APP.DATA[ i ].actions[ j ].type = cfg_app_data_actions_item_type->valueint;
                                    LOG_print("\t\t· type=\"%s\"\n", APP.DATA[ i ].actions[ j ].type == AT_READ ? "READ" : "WRITE" );

                                    cfg_app_data_actions_item_internal = cJSON_GetObjectItem( cfg_app_data_actions_item, "internal" );
                                    if( cfg_app_data_actions_item_internal == NULL ) {
                                        LOG_print( "ERROR: \"app.DATA[%d].actions[%d].internal\" key not found.\n", i, j );
                                        cJSON_Delete( config_json );
                                        return FALSE;
                                    }
                                    APP.DATA[ i ].actions[ j ].internal = cfg_app_data_actions_item_internal->valueint;
                                    LOG_print("\t\t· internal=%s\n", APP.DATA[ i ].actions[ j ].internal == 1 ? "TRUE" : "FALSE" );

                                    cfg_app_data_actions_item_condition = cJSON_GetObjectItem( cfg_app_data_actions_item, "condition" );
                                    if( cfg_app_data_actions_item_condition == NULL ) {
                                        LOG_print( "ERROR: \"app.DATA[%d].actions[%d].condition\" key not found.\n", i, j );
                                        cJSON_Delete( config_json );
                                        return FALSE;
                                    }
                                    /*str_len = strlen( cfg_app_data_actions_item_condition->valuestring );
                                    APP.DATA[ i ].actions[ j ].condition = ( char * )SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                                    strncpy( APP.DATA[ i ].actions[ j ].condition, cfg_app_data_actions_item_condition->valuestring, str_len );*/
                                    APP.DATA[ i ].actions[ j ].condition = strdup( cfg_app_data_actions_item_condition->valuestring );
                                    LOG_print("\t\t· condition=\"%s\"\n", APP.DATA[ i ].actions[ j ].condition );

                                    cfg_app_data_actions_item_sql = cJSON_GetObjectItem( cfg_app_data_actions_item, "sql" );
                                    if( cfg_app_data_actions_item_sql == NULL ) {
                                        LOG_print( "ERROR: \"app.DATA[%d].actions[%d].sql\" key not found.\n", i, j );
                                        cJSON_Delete( config_json );
                                        return FALSE;
                                    }
                                    /*str_len = strlen( cfg_app_data_actions_item_sql->valuestring );
                                    APP.DATA[ i ].actions[ j ].sql = ( char * )SAFECALLOC( str_len+1, sizeof( char ), __FILE__, __LINE__ );
                                    strncpy( APP.DATA[ i ].actions[ j ].sql, cfg_app_data_actions_item_sql->valuestring, str_len );*/
                                    APP.DATA[ i ].actions[ j ].sql = strdup( cfg_app_data_actions_item_sql->valuestring );
                                    LOG_print("\t\t· sql=\"%s\"\n", APP.DATA[ i ].actions[ j ].sql );
                                }
                            } else {
                                LOG_print( "ERROR: \"app.DATA[%d].actions\" key not found.\n", i );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                fclose( file );
                                return FALSE;
                            }
                        }
                    } else {
                        LOG_print( "ERROR: \"app.DATA\" key not found.\n " );
                        cJSON_Delete( config_json );
                        free( config_string ); config_string = NULL;
                        fclose( file );
                        return FALSE;   
                    }
                } else {
                    LOG_print( "ERROR: \"app\" key not found.\n " );
                    cJSON_Delete( config_json );
                    free( config_string ); config_string = NULL;
                    fclose( file );
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
                            fclose( file );
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
                            fclose( file );
                            return FALSE;
                        }
                        LOG_print("\n[%s] Loading system data: %s.\n", TIME_get_gmt(), cfg_system_name->valuestring);
                        
                        cfg_system_info = cJSON_GetObjectItem( array_value, "DB");
                        if( cfg_system_info == NULL ) {
                            LOG_print( "ERROR: \"system[%d].DB\" key not found.\n", i );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            fclose( file );
                            return FALSE;
                        }
                        
                        cfg_system_ip = cJSON_GetObjectItem( cfg_system_info, "ip");
                        if( cfg_system_ip == NULL ) {
                            LOG_print( "ERROR: \"system[%d].DB.ip\" key not found.\n", i );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            fclose( file );
                            return FALSE;
                        }
                        cfg_system_port = cJSON_GetObjectItem( cfg_system_info, "port");
                        if( cfg_system_port == NULL ) {
                            LOG_print( "ERROR: \"system[%d].DB.port\" key not found.\n", i );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            fclose( file );
                            return FALSE;
                        }
                        cfg_system_driver = cJSON_GetObjectItem( cfg_system_info, "driver");
                        if( cfg_system_driver == NULL ) {
                            LOG_print( "ERROR: \"system[%d].DB.driver\" key not found.\n", i );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            fclose( file );
                            return FALSE;
                        }
                        cfg_system_user = cJSON_GetObjectItem( cfg_system_info, "user");
                        if( cfg_system_user == NULL ) {
                            LOG_print( "ERROR: \"system[%d].DB.user\" key not found.\n", i );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            fclose( file );
                            return FALSE;
                        }
                        cfg_system_db = cJSON_GetObjectItem( cfg_system_info, "db");
                        if( cfg_system_db == NULL ) {
                            LOG_print( "ERROR: \"system[%d].DB.db\" key not found.\n", i );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            fclose( file );
                            return FALSE;
                        }
                        cfg_system_password = cJSON_GetObjectItem( cfg_system_info, "password");
                        if( cfg_system_password == NULL ) {
                            LOG_print( "ERROR: \"system[%d].DB.password\" key not found.\n", i );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            fclose( file );
                            return FALSE;
                        }
                        cfg_system_connection_string = cJSON_GetObjectItem( cfg_system_info, "connection_string");
                        if( cfg_system_connection_string == NULL ) {
                            LOG_print( "ERROR: \"system[%d].DB.connection_string\" key not found.\n", i );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            fclose( file );
                            return FALSE;
                        }

                        int tmp_queries_count = 0;
                        int tmp_data_types_len = 0;

                        cfg_system_queries_array = cJSON_GetObjectItem( array_value, "queries" );
                        if( cfg_system_queries_array == NULL ) {
                            LOG_print( "ERROR: \"system[%d].queries\" key not found.\n", i );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            fclose( file );
                            return FALSE;
                        }
                        for( int j = 0; j < cJSON_GetArraySize( cfg_system_queries_array ); j++ ) {
                            cfg_system_query_item = cJSON_GetArrayItem( cfg_system_queries_array, j );
                            
                            cfg_system_query_item_name = cJSON_GetObjectItem( cfg_system_query_item, "name");
                            if( cfg_system_query_item_name == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].name\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                fclose( file );
                                return FALSE;
                            }

                            cfg_system_query_item_change_data_capture = cJSON_GetObjectItem( cfg_system_query_item, "change_data_capture" );
                            if( cfg_system_query_item_change_data_capture == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].change_data_capture\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                fclose( file );
                                return FALSE;
                            }
                            /*
                                change_data_capture.extract
                            */
                            cfg_system_query_item_change_data_capture_extract = cJSON_GetObjectItem( cfg_system_query_item_change_data_capture, "extract" );
                            if( cfg_system_query_item_change_data_capture_extract == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].change_data_capture.extract\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                fclose( file );
                                return FALSE;
                            }
                            /*
                                change_data_capture.extract.inserted
                            */
                            cfg_system_query_item_change_data_capture_extract_inserted = cJSON_GetObjectItem( cfg_system_query_item_change_data_capture_extract, "inserted" );
                            if( cfg_system_query_item_change_data_capture_extract_inserted == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].change_data_capture.extract.inserted\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                fclose( file );
                                return FALSE;
                            }
                            /*
                                change_data_capture.extract.inserted.primary_db
                            */
                            cfg_system_query_item_change_data_capture_extract_inserted_primary_db = cJSON_GetObjectItem( cfg_system_query_item_change_data_capture_extract_inserted, "primary_db" );
                            if( cfg_system_query_item_change_data_capture_extract_inserted_primary_db == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].change_data_capture.extract.inserted.primary_db\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                fclose( file );
                                return FALSE;
                            }
                            /*str_len = strlen( cfg_system_query_item_change_data_capture_extract_inserted_primary_db->valuestring );
                            tmp_cdc.extract.inserted.primary_db = ( char * )SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                            strncpy(
                                tmp_cdc.extract.inserted.primary_db,
                                cfg_system_query_item_change_data_capture_extract_inserted_primary_db->valuestring,
                                str_len
                            );*/
                            tmp_cdc.extract.inserted.primary_db = strdup( cfg_system_query_item_change_data_capture_extract_inserted_primary_db->valuestring );
                            /*
                                change_data_capture.extract.inserted.primary_db_sql
                            */
                            cfg_system_query_item_change_data_capture_extract_inserted_primary_db_sql = cJSON_GetObjectItem( cfg_system_query_item_change_data_capture_extract_inserted, "primary_db_sql" );
                            if( cfg_system_query_item_change_data_capture_extract_inserted_primary_db_sql == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].change_data_capture.extract.inserted.primary_db_sql\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                fclose( file );
                                return FALSE;
                            }
                            str_len = strlen( cfg_system_query_item_change_data_capture_extract_inserted_primary_db_sql->valuestring );
                            tmp_cdc.extract.inserted.primary_db_sql_len = str_len;
                            tmp_cdc.extract.inserted.primary_db_sql = strdup( cfg_system_query_item_change_data_capture_extract_inserted_primary_db_sql->valuestring );
                            /*tmp_cdc.extract.inserted.primary_db_sql = ( char * )SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                            tmp_cdc.extract.inserted.primary_db_sql_len = str_len;
                            strncpy(
                                tmp_cdc.extract.inserted.primary_db_sql,
                                cfg_system_query_item_change_data_capture_extract_inserted_primary_db_sql->valuestring,
                                str_len
                            );*/
                            /*
                                change_data_capture.extract.secondary_db
                            */
                            cfg_system_query_item_change_data_capture_extract_inserted_secondary_db = cJSON_GetObjectItem( cfg_system_query_item_change_data_capture_extract_inserted, "secondary_db" );
                            if( cfg_system_query_item_change_data_capture_extract_inserted_secondary_db == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].change_data_capture.extract.inserted.secondary_db\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                fclose( file );
                                return FALSE;
                            }
                            /*str_len = strlen( cfg_system_query_item_change_data_capture_extract_inserted_secondary_db->valuestring );
                            tmp_cdc.extract.inserted.secondary_db = ( char * )SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                            strncpy(
                                tmp_cdc.extract.inserted.secondary_db,
                                cfg_system_query_item_change_data_capture_extract_inserted_secondary_db->valuestring,
                                str_len
                            );*/
                            tmp_cdc.extract.inserted.secondary_db = strdup( cfg_system_query_item_change_data_capture_extract_inserted_secondary_db->valuestring );
                            /*
                                change_data_capture.extract.secondary_db_sql
                            */
                            cfg_system_query_item_change_data_capture_extract_inserted_secondary_db_sql = cJSON_GetObjectItem( cfg_system_query_item_change_data_capture_extract_inserted, "secondary_db_sql" );
                            if( cfg_system_query_item_change_data_capture_extract_inserted_secondary_db_sql == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].change_data_capture.extract.inserted.secondary_db_sql\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                fclose( file );
                                return FALSE;
                            }
                            str_len = strlen( cfg_system_query_item_change_data_capture_extract_inserted_secondary_db_sql->valuestring );
                            tmp_cdc.extract.inserted.secondary_db_sql_len = str_len;
                            /*tmp_cdc.extract.inserted.secondary_db_sql = ( char * )SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                            tmp_cdc.extract.inserted.secondary_db_sql_len = str_len;
                            strncpy(
                                tmp_cdc.extract.inserted.secondary_db_sql,
                                cfg_system_query_item_change_data_capture_extract_inserted_secondary_db_sql->valuestring,
                                str_len
                            );*/
                            tmp_cdc.extract.inserted.secondary_db_sql = strdup( cfg_system_query_item_change_data_capture_extract_inserted_secondary_db_sql->valuestring );
                            /*
                                change_data_capture.extract.modified
                            */
                            cfg_system_query_item_change_data_capture_extract_modified = cJSON_GetObjectItem( cfg_system_query_item_change_data_capture_extract, "modified" );
                            if( cfg_system_query_item_change_data_capture_extract_modified == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].change_data_capture.extract.modified\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                fclose( file );
                                return FALSE;
                            }
                            /*
                                change_data_capture.extract.modified.primary_db
                            */
                            cfg_system_query_item_change_data_capture_extract_modified_primary_db = cJSON_GetObjectItem( cfg_system_query_item_change_data_capture_extract_modified, "primary_db" );
                            if( cfg_system_query_item_change_data_capture_extract_modified_primary_db == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].change_data_capture.extract.modified.primary_db\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                fclose( file );
                                return FALSE;
                            }
                            /*str_len = strlen( cfg_system_query_item_change_data_capture_extract_modified_primary_db->valuestring );
                            tmp_cdc.extract.modified.primary_db = ( char * )SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                            strncpy(
                                tmp_cdc.extract.modified.primary_db,
                                cfg_system_query_item_change_data_capture_extract_modified_primary_db->valuestring,
                                str_len
                            );*/
                            tmp_cdc.extract.modified.primary_db = strdup( cfg_system_query_item_change_data_capture_extract_modified_primary_db->valuestring );
                            /*
                                change_data_capture.extract.modified.primary_db_sql
                            */
                            cfg_system_query_item_change_data_capture_extract_modified_primary_db_sql = cJSON_GetObjectItem( cfg_system_query_item_change_data_capture_extract_modified, "primary_db_sql" );
                            if( cfg_system_query_item_change_data_capture_extract_modified_primary_db_sql == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].change_data_capture.extract.modified.primary_db_sql\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                fclose( file );
                                return FALSE;
                            }
                            str_len = strlen( cfg_system_query_item_change_data_capture_extract_modified_primary_db_sql->valuestring );
                            tmp_cdc.extract.modified.primary_db_sql_len = str_len;
                            /*tmp_cdc.extract.modified.primary_db_sql = ( char * )SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                            tmp_cdc.extract.modified.primary_db_sql_len = str_len;
                            strncpy(
                                tmp_cdc.extract.modified.primary_db_sql,
                                cfg_system_query_item_change_data_capture_extract_modified_primary_db_sql->valuestring,
                                str_len
                            );*/
                            tmp_cdc.extract.modified.primary_db_sql = strdup( cfg_system_query_item_change_data_capture_extract_modified_primary_db_sql->valuestring );
                            /*
                                change_data_capture.extract.modified.secondary_db
                            */
                            cfg_system_query_item_change_data_capture_extract_modified_secondary_db = cJSON_GetObjectItem( cfg_system_query_item_change_data_capture_extract_modified, "secondary_db" );
                            if( cfg_system_query_item_change_data_capture_extract_modified_secondary_db == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].change_data_capture.extract.modified.secondary_db\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                fclose( file );
                                return FALSE;
                            }
                            /*str_len = strlen( cfg_system_query_item_change_data_capture_extract_modified_secondary_db->valuestring );
                            tmp_cdc.extract.modified.secondary_db = ( char * )SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                            strncpy(
                                tmp_cdc.extract.modified.secondary_db,
                                cfg_system_query_item_change_data_capture_extract_modified_secondary_db->valuestring,
                                str_len
                            );*/
                            tmp_cdc.extract.modified.secondary_db = strdup( cfg_system_query_item_change_data_capture_extract_modified_secondary_db->valuestring );
                            /*
                                change_data_capture.extract.modified.secondary_db_sql
                            */
                            cfg_system_query_item_change_data_capture_extract_modified_secondary_db_sql = cJSON_GetObjectItem( cfg_system_query_item_change_data_capture_extract_modified, "secondary_db_sql" );
                            if( cfg_system_query_item_change_data_capture_extract_modified_secondary_db_sql == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].change_data_capture.extract.modified.secondary_db_sql\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                fclose( file );
                                return FALSE;
                            }
                            str_len = strlen( cfg_system_query_item_change_data_capture_extract_modified_secondary_db_sql->valuestring );
                            tmp_cdc.extract.modified.secondary_db_sql_len = str_len;
                            /*tmp_cdc.extract.modified.secondary_db_sql = ( char * )SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                            tmp_cdc.extract.modified.secondary_db_sql_len = str_len;
                            strncpy(
                                tmp_cdc.extract.modified.secondary_db_sql,
                                cfg_system_query_item_change_data_capture_extract_modified_secondary_db_sql->valuestring,
                                str_len
                            );*/
                            tmp_cdc.extract.modified.secondary_db_sql = strdup( cfg_system_query_item_change_data_capture_extract_modified_secondary_db_sql->valuestring );
                            /*
                                change_data_capture.extract.deleted
                            */
                            cfg_system_query_item_change_data_capture_extract_deleted = cJSON_GetObjectItem( cfg_system_query_item_change_data_capture_extract, "deleted" );
                            if( cfg_system_query_item_change_data_capture_extract_deleted == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].change_data_capture.extract.deleted\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                fclose( file );
                                return FALSE;
                            }
                            /*
                                change_data_capture.extract.deleted.primary_db
                            */
                            cfg_system_query_item_change_data_capture_extract_deleted_primary_db = cJSON_GetObjectItem( cfg_system_query_item_change_data_capture_extract_deleted, "primary_db" );
                            if( cfg_system_query_item_change_data_capture_extract_deleted_primary_db == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].change_data_capture.extract.deleted.primary_db\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                fclose( file );
                                return FALSE;
                            }
                            /*str_len = strlen( cfg_system_query_item_change_data_capture_extract_deleted_primary_db->valuestring );
                            tmp_cdc.extract.deleted.primary_db = ( char * )SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                            strncpy(
                                tmp_cdc.extract.deleted.primary_db,
                                cfg_system_query_item_change_data_capture_extract_deleted_primary_db->valuestring,
                                str_len
                            );*/
                            tmp_cdc.extract.deleted.primary_db = strdup( cfg_system_query_item_change_data_capture_extract_deleted_primary_db->valuestring );
                            /*
                                change_data_capture.extract.deleted.primary_db_sql
                            */
                            cfg_system_query_item_change_data_capture_extract_deleted_primary_db_sql = cJSON_GetObjectItem( cfg_system_query_item_change_data_capture_extract_deleted, "primary_db_sql" );
                            if( cfg_system_query_item_change_data_capture_extract_deleted_primary_db_sql == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].change_data_capture.extract.deleted.primary_db_sql\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                fclose( file );
                                return FALSE;
                            }
                            str_len = strlen( cfg_system_query_item_change_data_capture_extract_deleted_primary_db_sql->valuestring );
                            tmp_cdc.extract.deleted.primary_db_sql_len = str_len;
                            /*tmp_cdc.extract.deleted.primary_db_sql = ( char * )SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                            tmp_cdc.extract.deleted.primary_db_sql_len = str_len;
                            strncpy(
                                tmp_cdc.extract.deleted.primary_db_sql,
                                cfg_system_query_item_change_data_capture_extract_deleted_primary_db_sql->valuestring,
                                str_len
                            );*/
                            tmp_cdc.extract.deleted.primary_db_sql = strdup( cfg_system_query_item_change_data_capture_extract_deleted_primary_db_sql->valuestring );
                            /*
                                change_data_capture.extract.deleted.secondary_db
                            */
                            cfg_system_query_item_change_data_capture_extract_deleted_secondary_db = cJSON_GetObjectItem( cfg_system_query_item_change_data_capture_extract_deleted, "secondary_db" );
                            if( cfg_system_query_item_change_data_capture_extract_deleted_secondary_db == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].change_data_capture.extract.deleted.secondary_db\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                fclose( file );
                                return FALSE;
                            }
                            /*str_len = strlen( cfg_system_query_item_change_data_capture_extract_deleted_secondary_db->valuestring );
                            tmp_cdc.extract.deleted.secondary_db = ( char * )SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                            strncpy(
                                tmp_cdc.extract.deleted.secondary_db,
                                cfg_system_query_item_change_data_capture_extract_deleted_secondary_db->valuestring,
                                str_len
                            );*/
                            tmp_cdc.extract.deleted.secondary_db = strdup( cfg_system_query_item_change_data_capture_extract_deleted_secondary_db->valuestring );
                            /*
                                change_data_capture.extract.deleted.secondary_db_sql
                            */
                            cfg_system_query_item_change_data_capture_extract_deleted_secondary_db_sql = cJSON_GetObjectItem( cfg_system_query_item_change_data_capture_extract_deleted, "secondary_db_sql" );
                            if( cfg_system_query_item_change_data_capture_extract_deleted_secondary_db_sql == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].change_data_capture.extract.deleted.secondary_db_sql\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                fclose( file );
                                return FALSE;
                            }
                            str_len = strlen( cfg_system_query_item_change_data_capture_extract_deleted_secondary_db_sql->valuestring );
                            tmp_cdc.extract.deleted.secondary_db_sql_len = str_len;
                            /*tmp_cdc.extract.deleted.secondary_db_sql = ( char * )SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                            tmp_cdc.extract.deleted.secondary_db_sql_len = str_len;
                            strncpy(
                                tmp_cdc.extract.deleted.secondary_db_sql,
                                cfg_system_query_item_change_data_capture_extract_deleted_secondary_db_sql->valuestring,
                                str_len
                            );*/
                            tmp_cdc.extract.deleted.secondary_db_sql = strdup( cfg_system_query_item_change_data_capture_extract_deleted_secondary_db_sql->valuestring );

                            /*
                                change_data_capture.transform 
                            */
                            /* TODO */cfg_system_query_item_change_data_capture_transform = cJSON_GetObjectItem( cfg_system_query_item_change_data_capture, "transform" );/* TODO */
                                if( cfg_system_query_item_change_data_capture_transform == NULL ) {
                                    LOG_print( "ERROR: \"system[%d].queries[%d].change_data_capture.transform\" key not found.\n", i, j );
                                    cJSON_Delete( config_json );
                                    return FALSE;
                                }
                            /*
                                change_data_capture.load
                            */
                            cfg_system_query_item_change_data_capture_load = cJSON_GetObjectItem( cfg_system_query_item_change_data_capture, "load" );
                            if( cfg_system_query_item_change_data_capture_load == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].change_data_capture.load\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                fclose( file );
                                return FALSE;
                            }
                            /*
                                change_data_capture.load.inserted
                            */
                            cfg_system_query_item_change_data_capture_load_inserted = cJSON_GetObjectItem( cfg_system_query_item_change_data_capture_load, "inserted" );
                            if( cfg_system_query_item_change_data_capture_load_inserted == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].change_data_capture.load.inserted\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                fclose( file );
                                return FALSE;
                            }
                            /*
                                change_data_capture.load.inserted.sql
                            */
                            cfg_system_query_item_change_data_capture_load_inserted_sql = cJSON_GetObjectItem( cfg_system_query_item_change_data_capture_load_inserted, "sql" );
                            if( cfg_system_query_item_change_data_capture_load_inserted_sql == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].change_data_capture.load.inserted.sql\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                fclose( file );
                                return FALSE;
                            }
                            str_len = strlen( cfg_system_query_item_change_data_capture_load_inserted_sql->valuestring );
                            tmp_cdc.load.inserted.sql_len = str_len;
                            /*tmp_cdc.load.inserted.sql = ( char * )SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                            tmp_cdc.load.inserted.sql_len = str_len;
                            strncpy(
                                tmp_cdc.load.inserted.sql,
                                cfg_system_query_item_change_data_capture_load_inserted_sql->valuestring,
                                str_len
                            );*/
                            tmp_cdc.load.inserted.sql = strdup( cfg_system_query_item_change_data_capture_load_inserted_sql->valuestring );
                            /*
                                change_data_capture.load.inserted.extracted_values
                            */

                            int k = 0;

                            for( k = 0; k < MAX_CDC_COLUMNS; k++ ) {
                                memset( tmp_cdc.load.inserted.extracted_values[ k ], 0, MAX_COLUMN_NAME_LEN );
                            }
                            tmp_cdc.load.inserted.extracted_values_len = 0;
                            cfg_system_query_item_change_data_capture_load_inserted_extracted_values_array = cJSON_GetObjectItem( cfg_system_query_item_change_data_capture_load_inserted, "extracted_values" );
                            if( cfg_system_query_item_change_data_capture_load_inserted_extracted_values_array == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].change_data_capture.load.inserted.extracted_values\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                fclose( file );
                                return FALSE;
                            }
                            for( k = 0; k < cJSON_GetArraySize( cfg_system_query_item_change_data_capture_load_inserted_extracted_values_array ); k++ ) {
                                cfg_system_query_item_change_data_capture_load_inserted_extracted_values_item = cJSON_GetArrayItem( cfg_system_query_item_change_data_capture_load_inserted_extracted_values_array, k );
                                snprintf( tmp_cdc.load.inserted.extracted_values[ k ], MAX_COLUMN_NAME_LEN, "%s", cfg_system_query_item_change_data_capture_load_inserted_extracted_values_item->valuestring );
                                /*strncpy(
                                    tmp_cdc.load.inserted.extracted_values[ k ],
                                    cfg_system_query_item_change_data_capture_load_inserted_extracted_values_item->valuestring,
                                    MAX_COLUMN_NAME_LEN
                                );*/
                                tmp_cdc.load.inserted.extracted_values_len++;
                            }
                            /*
                                change_data_capture.load.deleted
                            */
                            cfg_system_query_item_change_data_capture_load_deleted = cJSON_GetObjectItem( cfg_system_query_item_change_data_capture_load, "deleted" );
                            if( cfg_system_query_item_change_data_capture_load_deleted == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].change_data_capture.load.deleted\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                fclose( file );
                                return FALSE;
                            }
                            /*
                                change_data_capture.load.deleted.sql
                            */
                            cfg_system_query_item_change_data_capture_load_deleted_sql = cJSON_GetObjectItem( cfg_system_query_item_change_data_capture_load_deleted, "sql" );
                            if( cfg_system_query_item_change_data_capture_load_deleted_sql == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].change_data_capture.load.deleted.sql\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                fclose( file );
                                return FALSE;
                            }
                            str_len = strlen( cfg_system_query_item_change_data_capture_load_deleted_sql->valuestring );
                            tmp_cdc.load.deleted.sql_len = str_len;
                            /*tmp_cdc.load.deleted.sql = ( char * )SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                            tmp_cdc.load.deleted.sql_len = str_len;
                            strncpy(
                                tmp_cdc.load.deleted.sql,
                                cfg_system_query_item_change_data_capture_load_deleted_sql->valuestring,
                                str_len
                            );*/
                            tmp_cdc.load.deleted.sql = strdup( cfg_system_query_item_change_data_capture_load_deleted_sql->valuestring );
                            /*
                                change_data_capture.load.deleted.extracted_values
                            */
                            cfg_system_query_item_change_data_capture_load_deleted_extracted_values_array = cJSON_GetObjectItem( cfg_system_query_item_change_data_capture_load_deleted, "extracted_values" );
                            if( cfg_system_query_item_change_data_capture_load_deleted_extracted_values_array == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].change_data_capture.load.deleted.extracted_values\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                fclose( file );
                                return FALSE;
                            }
                            for( k = 0; k < MAX_CDC_COLUMNS; k++ ) {
                                memset( tmp_cdc.load.deleted.extracted_values[ k ], 0, MAX_COLUMN_NAME_LEN );
                            }
                            tmp_cdc.load.deleted.extracted_values_len = 0;
                            for( k = 0; k < cJSON_GetArraySize( cfg_system_query_item_change_data_capture_load_deleted_extracted_values_array ); k++ ) {
                                cfg_system_query_item_change_data_capture_load_deleted_extracted_values_item = cJSON_GetArrayItem( cfg_system_query_item_change_data_capture_load_deleted_extracted_values_array, k );
                                snprintf( tmp_cdc.load.deleted.extracted_values[ k ], MAX_COLUMN_NAME_LEN, "%s", cfg_system_query_item_change_data_capture_load_deleted_extracted_values_item->valuestring );
                                /*strncpy(
                                    tmp_cdc.load.deleted.extracted_values[ k ],
                                    cfg_system_query_item_change_data_capture_load_deleted_extracted_values_item->valuestring,
                                    MAX_COLUMN_NAME_LEN
                                );*/
                                tmp_cdc.load.deleted.extracted_values_len++;
                            }
                            /*
                                change_data_capture.load.modified
                            */
                            cfg_system_query_item_change_data_capture_load_modified = cJSON_GetObjectItem( cfg_system_query_item_change_data_capture_load, "modified" );
                            if( cfg_system_query_item_change_data_capture_load_modified == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].change_data_capture.load.modified\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                fclose( file );
                                return FALSE;
                            }
                            /*
                                change_data_capture.load.modified.sql
                            */
                            cfg_system_query_item_change_data_capture_load_modified_sql = cJSON_GetObjectItem( cfg_system_query_item_change_data_capture_load_modified, "sql" );
                            if( cfg_system_query_item_change_data_capture_load_modified_sql == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].change_data_capture.load.modified.sql\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                fclose( file );
                                return FALSE;
                            }
                            str_len = strlen( cfg_system_query_item_change_data_capture_load_modified_sql->valuestring );
                            tmp_cdc.load.modified.sql_len = str_len;
                            /*tmp_cdc.load.modified.sql = ( char * )SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                            tmp_cdc.load.modified.sql_len = str_len;
                            strncpy(
                                tmp_cdc.load.modified.sql,
                                cfg_system_query_item_change_data_capture_load_modified_sql->valuestring,
                                str_len
                            );*/
                            tmp_cdc.load.modified.sql = strdup( cfg_system_query_item_change_data_capture_load_modified_sql->valuestring );
                            /*
                                change_data_capture.load.modified.extracted_values
                            */
                            cfg_system_query_item_change_data_capture_load_modified_extracted_values_array = cJSON_GetObjectItem( cfg_system_query_item_change_data_capture_load_modified, "extracted_values" );
                            if( cfg_system_query_item_change_data_capture_load_modified_extracted_values_array == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].change_data_capture.load.modified.extracted_values\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                fclose( file );
                                return FALSE;
                            }
                            for( k = 0; k < MAX_CDC_COLUMNS; k++ ) {
                                memset( tmp_cdc.load.modified.extracted_values[ k ], 0, MAX_COLUMN_NAME_LEN );
                            }
                            tmp_cdc.load.modified.extracted_values_len = 0;
                            for( k = 0; k < cJSON_GetArraySize( cfg_system_query_item_change_data_capture_load_modified_extracted_values_array ); k++ ) {
                                cfg_system_query_item_change_data_capture_load_modified_extracted_values_item = cJSON_GetArrayItem( cfg_system_query_item_change_data_capture_load_modified_extracted_values_array, k );
                                snprintf( tmp_cdc.load.modified.extracted_values[ k ], MAX_COLUMN_NAME_LEN, "%s", cfg_system_query_item_change_data_capture_load_modified_extracted_values_item->valuestring );
                                /*strncpy(
                                    tmp_cdc.load.modified.extracted_values[ k ],
                                    //tmp_extracted_modified_values[ k ],
                                    cfg_system_query_item_change_data_capture_load_modified_extracted_values_item->valuestring,
                                    MAX_COLUMN_NAME_LEN
                                );*/
                                tmp_cdc.load.modified.extracted_values_len++;
                            }

                            cfg_system_query_item_data_types = cJSON_GetObjectItem( cfg_system_query_item, "data_types");
                            if( cfg_system_query_item_data_types == NULL ) {
                                LOG_print( "ERROR: \"system[%d].queries[%d].data_types\" key not found.\n", i, j );
                                cJSON_Delete( config_json );
                                free( config_string ); config_string = NULL;
                                fclose( file );
                                return FALSE;
                            }
                            
                            char tmp_data_types[SMALL_BUFF_SIZE][SMALL_BUFF_SIZE];

                            for( k = 0; k < cJSON_GetArraySize( cfg_system_query_item_data_types ); k++ ) {
                                cfg_system_query_item_data_types_name = cJSON_GetArrayItem( cfg_system_query_item_data_types, k );
                                snprintf( tmp_data_types[ k ], SMALL_BUFF_SIZE, "%s", cfg_system_query_item_data_types_name->valuestring );
                                /*strncpy(
                                    tmp_data_types[ k ],
                                    cfg_system_query_item_data_types_name->valuestring,
                                    SMALL_BUFF_SIZE
                                );*/
                                tmp_data_types_len++;
                            }
                            DATABASE_SYSTEM_QUERY_add(
                                cfg_system_query_item_name->valuestring,
                                tmp_cdc,
                                tmp_data_types,
                                tmp_data_types_len,
                                &tmp_queries[ tmp_queries_count ],
                                FALSE
                            );

                            tmp_queries_count++;
                        }
                        /* Create temporary DATABASE_SYSTEM_DB for future use in DATABASE_SYSTEM_add */
                        DATABASE_SYSTEM_DB_add(
                            cfg_system_ip->valuestring,
                            cfg_system_port->valueint,
                            cfg_system_driver->valueint,
                            cfg_system_user->valuestring,
                            cfg_system_password->valuestring,
                            cfg_system_db->valuestring,
                            cfg_system_connection_string->valuestring,
                            &tmp_db,
                            TRUE
                        );

                        DATABASE_SYSTEM_add(
                            cfg_system_name->valuestring,
                            &tmp_db,
                            tmp_queries,
                            tmp_queries_count,
                            TRUE
                        );
                        LOG_print( "[%s] Finished loading system %s.\n", TIME_get_gmt(), cfg_system_name->valuestring );
                        LOG_print( "===========================\n" );
                    }
                } else {
                    LOG_print( "ERROR: \"system\" key not found.\n" );
                    cJSON_Delete( config_json );
                    return FALSE;
                }

                cJSON_Delete( config_json );
            }
        }
        fclose( file );

        free( config_string );
        config_string = NULL;

        result = 1;
    } else {
        LOG_print( "[%s] Fatal error: unable to open config file \"config.json\"!", TIME_get_gmt() );
    }

    return result;
}


int main( int argc, char** argv ) {

    char        config_file[ MAX_PATH_LENGTH ];
    signal( SIGINT, ( void* )&app_terminate );
#ifndef _WIN32
    signal( SIGPIPE, SIG_IGN );
#endif
    signal( SIGABRT, ( void* )&app_terminate );
    signal( SIGTERM, ( void* )&app_terminate );

    LOG_init();

    if( argc <= 1 ) {
        /*strncpy( config_file, "config.json", MAX_PATH_LENGTH );*/
        snprintf( config_file, MAX_PATH_LENGTH, "config.json" );
    } else if( argc >= 2 ) {
        if( strlen( argv[ 1 ] ) > MAX_PATH_LENGTH ) {
            LOG_print( "[%s] Notice: \"%s\" is not valid config file name.\n", TIME_get_gmt(), argv[ 1 ] );
            snprintf( config_file, MAX_PATH_LENGTH, "config.json" );
        } else {
            /*strncpy( config_file, argv[ 1 ], MAX_PATH_LENGTH );*/
            snprintf( config_file, MAX_PATH_LENGTH, "%s", argv[ 1 ] );
        }
    }

    if( DCPAM_load_configuration( config_file ) == 1 ) {
        /*if( WORKER_init() == 0 ) {
            app_terminate();
            return 1;
        }*/
        if( DB_WORKER_init() == 1 ) {
            //while( 1 );
        }
    }

    DCPAM_free_configuration();

    return 0;
}
