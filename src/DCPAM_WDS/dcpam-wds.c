#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include "../include/utils/log.h"
#include "../include/core/schema.h"
#include "../include/third-party/cJSON.h"
#include "../include/utils/memory.h"
#include "../include/utils/time.h"
#include "../include/utils/strings.h"
#include "../include/utils/filesystem.h"
#include "../include/core/db/system.h"
#include "../include/core/cache.h"
#include "../include/core/network/socket_io.h"
#include "../include/core/lcs_report.h"

#define WDS_RESPONSE_ERROR( communication_session, client ) SOCKET_send( communication_session, client, "{\"success\":false,\"data\":[],\"length\":0}", 38 );

#pragma warning( disable : 6031 )

char                    app_path[ MAX_PATH_LENGTH + 1 ];
LOG_OBJECT              dcpam_wds_log;
LOG_OBJECT              dcpam_wds_lcs_log;
extern int              app_terminated = 0;

extern DATABASE_SYSTEM  DATABASE_SYSTEMS[ MAX_DATA_SYSTEMS ];
extern int              DATABASE_SYSTEMS_COUNT;
extern P_DCPAM_APP      P_APP;

void DCPAM_WDS_free_configuration( void );


void app_terminate( void ) {
    LOG_print( &dcpam_wds_log, "\r\n[%s] Received termination signal.\n", TIME_get_gmt() );
    if( app_terminated == 0 ) {
        app_terminated = 1;
        printf( "\r" );
        DCPAM_WDS_free_configuration();
        LOG_print( &dcpam_wds_log, "[%s] DCPAM WDS graceful shutdown finished.\n", TIME_get_gmt() );
    }

    return;
}

void DCPAM_WDS_free_configuration( void ) {

    LCS_REPORT_free( &P_APP.lcs_report );

    for( int i = 0; i < P_APP.CACHE_len; i++ ) {
        LOG_print( &dcpam_wds_log, "[%s] Removing cache %d of %d...\n", TIME_get_gmt(), i + 1, P_APP.CACHE_len );
        DB_CACHE_free( P_APP.CACHE[ i ], &dcpam_wds_log );
        free( P_APP.CACHE[ i ] ); P_APP.CACHE[ i ] = NULL;
    }
    free( P_APP.CACHE ); P_APP.CACHE = NULL;
    P_APP.CACHE_len = 0;

    for( int i = 0; i < P_APP.ALLOWED_HOSTS_len; i++ ) {
        free( P_APP.ALLOWED_HOSTS_[ i ]->ip ); P_APP.ALLOWED_HOSTS_[ i ]->ip = NULL;
        free( P_APP.ALLOWED_HOSTS_[ i ]->api_key ); P_APP.ALLOWED_HOSTS_[ i ]->api_key = NULL;
        free( P_APP.ALLOWED_HOSTS_[ i ] ); P_APP.ALLOWED_HOSTS_[ i ] = NULL;
    }
    free( P_APP.ALLOWED_HOSTS_ ); P_APP.ALLOWED_HOSTS_ = NULL;
    P_APP.ALLOWED_HOSTS_len = 0;

    for( int i = 0; i < P_APP.DB_len; i++ ) {
        DATABASE_SYSTEM_DB_free( P_APP.DB[ i ], &dcpam_wds_log );
        free( P_APP.DB[ i ] ); P_APP.DB[ i ] = NULL;
    }
    free( P_APP.DB ); P_APP.DB = NULL;
    P_APP.DB_len = 0;

    if( P_APP.version != NULL ) { free( P_APP.version ); P_APP.version = NULL; }
    if( P_APP.name != NULL ) { free( P_APP.name ); P_APP.name = NULL; }

    for( int i = 0; i < P_APP.DATA_len; i++ ) {
        if( P_APP.DATA[ i ].id != NULL ) { free( P_APP.DATA[ i ].id ); P_APP.DATA[ i ].id = NULL; }
        if( P_APP.DATA[ i ].name != NULL ) { free( P_APP.DATA[ i ].name ); P_APP.DATA[ i ].name = NULL; }
        if( P_APP.DATA[ i ].db_name != NULL ) { free( P_APP.DATA[ i ].db_name ); P_APP.DATA[ i ].db_name = NULL; }
        if( P_APP.DATA[ i ].description != NULL ) { free( P_APP.DATA[ i ].description ); P_APP.DATA[ i ].description = NULL; }

        for( int j = 0; j < P_APP.DATA[ i ].actions_len; j++ ) {
            if( P_APP.DATA[ i ].actions[ j ].name != NULL ) { free( P_APP.DATA[ i ].actions[ j ].name ); P_APP.DATA[ i ].actions[ j ].name = NULL; }
            if( P_APP.DATA[ i ].actions[ j ].description != NULL ) { free( P_APP.DATA[ i ].actions[ j ].description ); P_APP.DATA[ i ].actions[ j ].description = NULL; }
            if( P_APP.DATA[ i ].actions[ j ].condition != NULL ) { free( P_APP.DATA[ i ].actions[ j ].condition ); P_APP.DATA[ i ].actions[ j ].condition = NULL; }
            if( P_APP.DATA[ i ].actions[ j ].sql != NULL ) { free( P_APP.DATA[ i ].actions[ j ].sql ); P_APP.DATA[ i ].actions[ j ].sql = NULL; }
        }
    }
}

int DCPAM_WDS_load_configuration( const char* filename ) {
    cJSON* config_json = NULL;

    cJSON* cfg_app = NULL;
    cJSON* cfg_app_version = NULL;
    cJSON* cfg_app_name = NULL;
    cJSON* cfg_lcs = NULL;
    cJSON* cfg_lcs_address = NULL;
    cJSON* cfg_lcs_port = NULL;
    cJSON* cfg_app_max_memory = NULL;
    cJSON* cfg_app_network = NULL;
    cJSON* cfg_app_network_port = NULL;
    cJSON* cfg_app_network_allowed_hosts = NULL;
    cJSON* cfg_app_network_allowed_host_item = NULL;
    cJSON* cfg_app_network_allowed_host_item_ip = NULL;
    cJSON* cfg_app_network_allowed_host_item_key = NULL;
    cJSON* cfg_app_db_array = NULL;
    cJSON* cfg_app_db_item = NULL;
    cJSON* cfg_app_db = NULL;
    cJSON* cfg_app_db_ip = NULL;
    cJSON* cfg_app_db_port = NULL;
    cJSON* cfg_app_db_driver = NULL;
    cJSON* cfg_app_db_user = NULL;
    cJSON* cfg_app_db_password = NULL;
    cJSON* cfg_app_db_connection_string = NULL;
    cJSON* cfg_app_db_db = NULL;
    cJSON* cfg_app_db_name = NULL;

    cJSON* cfg_app_data = NULL;
    cJSON* cfg_app_data_item = NULL;
    cJSON* cfg_app_data_item_id = NULL;
    cJSON* cfg_app_data_item_name = NULL;
    cJSON* cfg_app_data_item_db_name = NULL;
    cJSON* cfg_app_data_item_db_table_name = NULL;
    cJSON* cfg_app_data_item_description = NULL;
    cJSON* cfg_app_data_actions = NULL;
    cJSON* cfg_app_data_actions_item = NULL;
    cJSON* cfg_app_data_actions_item_columns = NULL;
    cJSON* cfg_app_data_actions_item_columns_name = NULL;
    cJSON* cfg_app_data_actions_item_name = NULL;
    cJSON* cfg_app_data_actions_item_description = NULL;
    cJSON* cfg_app_data_actions_item_type = NULL;
    cJSON* cfg_app_data_actions_item_internal = NULL;
    cJSON* cfg_app_data_actions_item_condition = NULL;
    cJSON* cfg_app_data_actions_item_sql = NULL;

    int                         result = 0;
    char*                       config_string = NULL;


    LOG_print( &dcpam_wds_log, "[%s] DCPAM_WDS_load_configuration( %s ).\n", TIME_get_gmt(), filename );

    config_string = file_get_content( filename );

    if( config_string ) {
        config_json = cJSON_Parse( config_string );
        if( config_json ) {

            cfg_app = cJSON_GetObjectItem( config_json, "app" );
            if( cfg_app ) {

                cfg_app_name = cJSON_GetObjectItem( cfg_app, "name" );
                if( cfg_app_name ) {
                    size_t str_len = strlen( cfg_app_name->valuestring );
                    P_APP.name = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                    snprintf( P_APP.name, str_len + 1, cfg_app_name->valuestring );
                } else {
                    LOG_print( &dcpam_wds_log, "ERROR: \"app.name\" key not found.\n" );
                    cJSON_Delete( config_json );
                    free( config_string ); config_string = NULL;
                    return FALSE;
                }

                cfg_app_version = cJSON_GetObjectItem( cfg_app, "version" );
                if( cfg_app_version ) {
                    size_t str_len = strlen( cfg_app_version->valuestring );
                    P_APP.version = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                    snprintf( P_APP.version, str_len + 1, cfg_app_version->valuestring );
                    LOG_print( &dcpam_wds_log, "%s v%s.\n", P_APP.name, P_APP.version );
                } else {
                    LOG_print( &dcpam_wds_log, "ERROR: \"app.version\" key not found.\n" );
                    cJSON_Delete( config_json );
                    free( config_string ); config_string = NULL;
                    return FALSE;
                }

                cfg_app_network = cJSON_GetObjectItem( cfg_app, "network" );
                if( cfg_app_network ) {

                    cfg_app_network_port = cJSON_GetObjectItem( cfg_app_network, "port" );
                    if( cfg_app_network_port ) {
                        P_APP.network_port = cfg_app_network_port->valueint;
                    } else {
                        P_APP.network_port = 9090;
                    }
                    LOG_print( &dcpam_wds_log, "Network port is set to %d.\n", P_APP.network_port );

                    cfg_app_network_allowed_hosts = cJSON_GetObjectItem( cfg_app_network, "allowed_hosts" );
                    if( cfg_app_network_allowed_hosts ) {
                        P_APP.ALLOWED_HOSTS_len = cJSON_GetArraySize( cfg_app_network_allowed_hosts );
                        P_APP.ALLOWED_HOSTS_ = SAFEMALLOC( P_APP.ALLOWED_HOSTS_len * sizeof * P_APP.ALLOWED_HOSTS_, __FILE__, __LINE__ );

                        for( int i = 0; i < P_APP.ALLOWED_HOSTS_len; i++ ) {
                            cfg_app_network_allowed_host_item = cJSON_GetArrayItem( cfg_app_network_allowed_hosts, i );
                            if( cfg_app_network_allowed_host_item ) {
                                P_APP.ALLOWED_HOSTS_[ i ] = SAFEMALLOC( sizeof( DCPAM_ALLOWED_HOST ), __FILE__, __LINE__ );

                                cfg_app_network_allowed_host_item_ip = cJSON_GetObjectItem( cfg_app_network_allowed_host_item, "ip" );
                                if( cfg_app_network_allowed_host_item_ip ) {
                                    size_t str_len = strlen( cfg_app_network_allowed_host_item_ip->valuestring );
                                    P_APP.ALLOWED_HOSTS_[ i ]->ip = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                                    snprintf( P_APP.ALLOWED_HOSTS_[ i ]->ip, str_len + 1, cfg_app_network_allowed_host_item_ip->valuestring );
                                }

                                cfg_app_network_allowed_host_item_key = cJSON_GetObjectItem( cfg_app_network_allowed_host_item, "key" );
                                if( cfg_app_network_allowed_host_item_key ) {
                                    size_t str_len = strlen( cfg_app_network_allowed_host_item_key->valuestring );
                                    P_APP.ALLOWED_HOSTS_[ i ]->api_key = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                                    snprintf( P_APP.ALLOWED_HOSTS_[ i ]->api_key, str_len + 1, cfg_app_network_allowed_host_item_key->valuestring );
                                }
                            }
                        }
                    } else {
                        LOG_print( &dcpam_wds_log, "ERROR: \"app.network.allowed_hosts\" key not found.\n" );
                        cJSON_Delete( config_json );
                        free( config_string ); config_string = NULL;
                        return FALSE;
                    }
                } else {
                    LOG_print( &dcpam_wds_log, "ERROR: \"app.network\" key not found.\n" );
                    cJSON_Delete( config_json );
                    free( config_string ); config_string = NULL;
                    return FALSE;
                }

                cfg_app_max_memory = cJSON_GetObjectItem( cfg_app, "max_memory" );
                if( cfg_app_max_memory ) {
                    P_APP.CACHE_MAX_size = ( size_t )cfg_app_max_memory->valueint;
                    LOG_print( &dcpam_wds_log, "Maximum memory usage: %ld KB.\n", P_APP.CACHE_MAX_size );
                } else {
                    LOG_print( &dcpam_wds_log, "ERROR: \"app.max_memory\" key not found.\n" );
                    cJSON_Delete( config_json );
                    free( config_string ); config_string = NULL;
                    return FALSE;
                }

                cfg_app_db_array = cJSON_GetObjectItem( cfg_app, "DB" );
                if( cfg_app_db_array ) {

                    P_APP.DB_len = cJSON_GetArraySize( cfg_app_db_array );
                    P_APP.CACHE_len = 0;

                    P_APP.DB = SAFEMALLOC( P_APP.DB_len * sizeof * P_APP.DB, __FILE__, __LINE__ );

                    for( int i = 0; i < P_APP.DB_len; i++ ) {
                        cfg_app_db = cJSON_GetArrayItem( cfg_app_db_array, i );

                        cfg_app_db_ip = cJSON_GetObjectItem( cfg_app_db, "ip" );
                        if( cfg_app_db_ip == NULL ) {
                            LOG_print( &dcpam_wds_log, "ERROR: \"app.DB.ip\" key not found.\n" );
                        }

                        cfg_app_db_port = cJSON_GetObjectItem( cfg_app_db, "port" );
                        if( cfg_app_db_port == NULL ) {
                            LOG_print( &dcpam_wds_log, "ERROR: \"app.DB.port\" key not found.\n" );
                        }

                        cfg_app_db_driver = cJSON_GetObjectItem( cfg_app_db, "driver" );
                        if( cfg_app_db_driver == NULL ) {
                            LOG_print( &dcpam_wds_log, "ERROR: \"app.DB.driver\" key not found.\n" );
                        }

                        cfg_app_db_user = cJSON_GetObjectItem( cfg_app_db, "user" );
                        if( cfg_app_db_user == NULL ) {
                            LOG_print( &dcpam_wds_log, "ERROR: \"app.DB.user\" key not found.\n" );
                        }

                        cfg_app_db_password = cJSON_GetObjectItem( cfg_app_db, "password" );
                        if( cfg_app_db_password == NULL ) {
                            LOG_print( &dcpam_wds_log, "ERROR: \"app.DB.password\" key not found.\n" );
                        }

                        cfg_app_db_connection_string = cJSON_GetObjectItem( cfg_app_db, "connection_string" );
                        if( cfg_app_db_connection_string == NULL ) {
                            LOG_print( &dcpam_wds_log, "ERROR: \"app.DB.connection_string\" key not found.\n" );
                        }

                        cfg_app_db_db = cJSON_GetObjectItem( cfg_app_db, "db" );
                        if( cfg_app_db_db == NULL ) {
                            LOG_print( &dcpam_wds_log, "ERROR: \"app.DB.db\" key not found.\n" );
                        }

                        cfg_app_db_name = cJSON_GetObjectItem( cfg_app_db, "name" );
                        if( cfg_app_db_name == NULL ) {
                            LOG_print( &dcpam_wds_log, "Error: \"app.DB.name\" key not found.\n" );
                        }

                        P_APP.DB[ i ] = SAFEMALLOC( sizeof( DATABASE_SYSTEM_DB ), __FILE__, __LINE__ );

                        DATABASE_SYSTEM_DB_add(
                            cfg_app_db_ip->valuestring,
                            cfg_app_db_port->valueint,
                            cfg_app_db_driver->valueint,
                            cfg_app_db_user->valuestring,
                            cfg_app_db_password->valuestring,
                            cfg_app_db_db->valuestring,
                            cfg_app_db_connection_string->valuestring,
                            P_APP.DB[ i ],
                            cfg_app_db_name->valuestring,
                            TRUE,
                            &dcpam_wds_log
                        );
                    }

                } else {
                    LOG_print( &dcpam_wds_log, "ERROR: \"app.DB\" key not found.\n" );
                    cJSON_Delete( config_json );
                    free( config_string ); config_string = NULL;
                    return FALSE;
                }

                LOG_print( &dcpam_wds_log, "[%s] Loading app.DATA item ", TIME_get_gmt() );
                cfg_app_data = cJSON_GetObjectItem( cfg_app, "DATA" );
                if( cfg_app_data ) {
                    P_APP.DATA_len = cJSON_GetArraySize( cfg_app_data );

                    for( int i = 0; i < P_APP.DATA_len; i++ ) {
                        cfg_app_data_item = cJSON_GetArrayItem( cfg_app_data, i );

                        cfg_app_data_item_id = cJSON_GetObjectItem( cfg_app_data_item, "id" );
                        if( cfg_app_data_item_id == NULL ) {
                            LOG_print( &dcpam_wds_log, "ERROR: \"app.DATA[%d].id\" key not found.\n", i );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        size_t str_len = strlen( cfg_app_data_item_id->valuestring );
                        P_APP.DATA[ i ].id = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                        snprintf( P_APP.DATA[ i ].id, str_len + 1, cfg_app_data_item_id->valuestring );
                        LOG_print( &dcpam_wds_log, "#%d: \"%s\"\n", i + 1, P_APP.DATA[ i ].id );

                        cfg_app_data_item_name = cJSON_GetObjectItem( cfg_app_data_item, "name" );
                        if( cfg_app_data_item_name == NULL ) {
                            LOG_print( &dcpam_wds_log, "ERROR: \"app.DATA[%d].name\" key not found.\n", i );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        size_t str_len2 = strlen( cfg_app_data_item_name->valuestring );
                        P_APP.DATA[ i ].name = SAFECALLOC( str_len2 + 1, sizeof( char ), __FILE__, __LINE__ );
                        snprintf( P_APP.DATA[ i ].name, str_len2+1, cfg_app_data_item_name->valuestring );
                        LOG_print( &dcpam_wds_log, "\t· name=\"%s\"\n", P_APP.DATA[ i ].name );

                        cfg_app_data_item_db_table_name = cJSON_GetObjectItem( cfg_app_data_item, "db_table_name" );
                        if( cfg_app_data_item_db_table_name == NULL ) {
                            LOG_print( &dcpam_wds_log, "ERROR: \"app.DATA[%d].db_table_name\" key not found.\n", i );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }

                        cfg_app_data_item_db_name = cJSON_GetObjectItem( cfg_app_data_item, "db_name" );
                        if( cfg_app_data_item_db_name == NULL ) {
                            LOG_print( &dcpam_wds_log, "ERROR: \"app.DATA[%d].db_name\" key not found.\n", i );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        size_t str_len3a = strlen( cfg_app_data_item_db_name->valuestring );
                        P_APP.DATA[ i ].db_name = SAFECALLOC( str_len3a + 1, sizeof( char ), __FILE__, __LINE__ );
                        snprintf( P_APP.DATA[ i ].db_name, str_len3a+1, cfg_app_data_item_db_name->valuestring );
                        LOG_print( &dcpam_wds_log, "\t· db_name=\"%s\"\n", P_APP.DATA[ i ].db_name );

                        cfg_app_data_actions_item_columns = cJSON_GetObjectItem( cfg_app_data_item, "columns" );
                        P_APP.DATA[ i ].columns_len = cJSON_GetArraySize( cfg_app_data_actions_item_columns );
                        if( cfg_app_data_actions_item_columns ) {
                            LOG_print( &dcpam_wds_log, "\t· columns=[" );
                            for( int k = 0; k < P_APP.DATA[ i ].columns_len; k++ ) {
                                cfg_app_data_actions_item_columns_name = cJSON_GetArrayItem( cfg_app_data_actions_item_columns, k );
                                memset( P_APP.DATA[ i ].columns[ k ], '\0', MAX_COLUMN_NAME_LEN );
                                snprintf( P_APP.DATA[ i ].columns[ k ], MAX_COLUMN_NAME_LEN, "%s", cfg_app_data_actions_item_columns_name->valuestring );
                                LOG_print( &dcpam_wds_log, "\"%s\", ", cfg_app_data_actions_item_columns_name->valuestring );
                            }
                            LOG_print( &dcpam_wds_log, "]\n" );
                        } else {
                            LOG_print( &dcpam_wds_log, "ERROR: \"app.DATA[%d].columns\" key not found.\n", i );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }

                        cfg_app_data_item_description = cJSON_GetObjectItem( cfg_app_data_item, "description" );
                        if( cfg_app_data_item_description == NULL ) {
                            LOG_print( &dcpam_wds_log, "ERROR: \"app.DATA[%d].description\" key not found.\n", i );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        size_t str_len4 = strlen( cfg_app_data_item_description->valuestring );
                        P_APP.DATA[ i ].description = SAFECALLOC( str_len4 + 1, sizeof( char ), __FILE__, __LINE__ );
                        snprintf( P_APP.DATA[ i ].description, str_len4+1, cfg_app_data_item_description->valuestring );
                        LOG_print( &dcpam_wds_log, "\t· description=\"%s\"\n", P_APP.DATA[ i ].description );

                        cfg_app_data_actions = cJSON_GetObjectItem( cfg_app_data_item, "actions" );
                        if( cfg_app_data_actions ) {
                            P_APP.DATA[ i ].actions_len = cJSON_GetArraySize( cfg_app_data_actions );

                            P_APP.CACHE_len += P_APP.DATA[ i ].actions_len;

                            for( int j = 0; j < P_APP.DATA[ i ].actions_len; j++ ) {
                                cfg_app_data_actions_item = cJSON_GetArrayItem( cfg_app_data_actions, j );

                                cfg_app_data_actions_item_name = cJSON_GetObjectItem( cfg_app_data_actions_item, "name" );
                                if( cfg_app_data_actions_item_name == NULL ) {
                                    LOG_print( &dcpam_wds_log, "ERROR: \"app.DATA[%d].actions[%d].name\" key not found.\n", i, j );
                                    cJSON_Delete( config_json );
                                    return FALSE;
                                }
                                size_t str_len = strlen( cfg_app_data_actions_item_name->valuestring );
                                P_APP.DATA[ i ].actions[ j ].name = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                                snprintf( P_APP.DATA[ i ].actions[ j ].name, str_len + 1, cfg_app_data_actions_item_name->valuestring );
                                LOG_print( &dcpam_wds_log, "\t· action #%d: \"%s\"\n", j + 1, P_APP.DATA[ i ].actions[ j ].name );

                                cfg_app_data_actions_item_description = cJSON_GetObjectItem( cfg_app_data_actions_item, "description" );
                                if( cfg_app_data_actions_item_description == NULL ) {
                                    LOG_print( &dcpam_wds_log, "ERROR: \"app.DATA[%d].actions[%d].description\" key not found.\n", i, j );
                                    cJSON_Delete( config_json );
                                    return FALSE;
                                }
                                size_t str_len2 = strlen( cfg_app_data_actions_item_description->valuestring );
                                P_APP.DATA[ i ].actions[ j ].description = SAFECALLOC( str_len2 + 1, sizeof( char ), __FILE__, __LINE__ );
                                snprintf( P_APP.DATA[ i ].actions[ j ].description, str_len2+1, cfg_app_data_actions_item_description->valuestring );
                                LOG_print( &dcpam_wds_log, "\t\t· description=\"%s\"\n", P_APP.DATA[ i ].actions[ j ].description );

                                cfg_app_data_actions_item_type = cJSON_GetObjectItem( cfg_app_data_actions_item, "type" );
                                if( cfg_app_data_actions_item_type == NULL ) {
                                    LOG_print( &dcpam_wds_log, "ERROR: \"app.DATA[%d].actions[%d].type\" key not found.\n", i, j );
                                    cJSON_Delete( config_json );
                                    return FALSE;
                                }
                                P_APP.DATA[ i ].actions[ j ].type = cfg_app_data_actions_item_type->valueint;
                                LOG_print( &dcpam_wds_log, "\t\t· type=\"%s\"\n", P_APP.DATA[ i ].actions[ j ].type == AT_READ ? "READ" : "WRITE" );

                                cfg_app_data_actions_item_internal = cJSON_GetObjectItem( cfg_app_data_actions_item, "internal" );
                                if( cfg_app_data_actions_item_internal == NULL ) {
                                    LOG_print( &dcpam_wds_log, "ERROR: \"app.DATA[%d].actions[%d].internal\" key not found.\n", i, j );
                                    cJSON_Delete( config_json );
                                    return FALSE;
                                }
                                P_APP.DATA[ i ].actions[ j ].internal = cfg_app_data_actions_item_internal->valueint;
                                LOG_print( &dcpam_wds_log, "\t\t· internal=%s\n", P_APP.DATA[ i ].actions[ j ].internal == 1 ? "TRUE" : "FALSE" );

                                cfg_app_data_actions_item_condition = cJSON_GetObjectItem( cfg_app_data_actions_item, "condition" );
                                if( cfg_app_data_actions_item_condition == NULL ) {
                                    LOG_print( &dcpam_wds_log, "ERROR: \"app.DATA[%d].actions[%d].condition\" key not found.\n", i, j );
                                    cJSON_Delete( config_json );
                                    return FALSE;
                                }
                                size_t str_len3 = strlen( cfg_app_data_actions_item_condition->valuestring );
                                P_APP.DATA[ i ].actions[ j ].condition = SAFECALLOC( str_len3 + 1, sizeof( char ), __FILE__, __LINE__ );
                                snprintf( P_APP.DATA[ i ].actions[ j ].condition, str_len3+1, cfg_app_data_actions_item_condition->valuestring );
                                LOG_print( &dcpam_wds_log, "\t\t· condition=\"%s\"\n", P_APP.DATA[ i ].actions[ j ].condition );

                                cfg_app_data_actions_item_sql = cJSON_GetObjectItem( cfg_app_data_actions_item, "sql" );
                                if( cfg_app_data_actions_item_sql == NULL ) {
                                    LOG_print( &dcpam_wds_log, "ERROR: \"app.DATA[%d].actions[%d].sql\" key not found.\n", i, j );
                                    cJSON_Delete( config_json );
                                    return FALSE;
                                }
                                size_t str_len4 = strlen( cfg_app_data_actions_item_sql->valuestring );
                                P_APP.DATA[ i ].actions[ j ].sql = SAFECALLOC( str_len4 + 1, sizeof( char ), __FILE__, __LINE__ );
                                strncpy( P_APP.DATA[ i ].actions[ j ].sql, cfg_app_data_actions_item_sql->valuestring, str_len4+1 );
                                LOG_print( &dcpam_wds_log, "\t\t· sql=\"%s\"\n", P_APP.DATA[ i ].actions[ j ].sql );
                            }
                        } else {
                            LOG_print( &dcpam_wds_log, "ERROR: \"app.DATA[%d].actions\" key not found.\n", i );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                    }
                } else {
                    LOG_print( &dcpam_wds_log, "ERROR: \"app.DATA\" key not found.\n " );
                    cJSON_Delete( config_json );
                    free( config_string ); config_string = NULL;
                    return FALSE;
                }
            } else {
                LOG_print( &dcpam_wds_log, "ERROR: \"app\" key not found.\n " );
                cJSON_Delete( config_json );
                free( config_string ); config_string = NULL;
                return FALSE;
            }

            cfg_lcs = cJSON_GetObjectItem( config_json, "LCS" );
            if( cfg_lcs ) {

                //P_APP.lcs_report.port = P_APP.network_port;

                cfg_lcs_address = cJSON_GetObjectItem( cfg_lcs, "address" );

                if( cfg_lcs_address ) {
                    size_t address_len = strlen( cfg_lcs_address->valuestring );
                    P_APP.lcs_report.address = SAFECALLOC( address_len + 1, sizeof( char ), __FILE__, __LINE__ );
                    snprintf( P_APP.lcs_report.address, address_len + 1, cfg_lcs_address->valuestring );
                    if( LCS_REPORT_init( &P_APP.lcs_report, P_APP.lcs_report.address, P_APP.name, P_APP.version, &dcpam_wds_log ) == 0 ) {
                        LOG_print( &dcpam_wds_log, "ERROR: unable to connect to Live Component State host at %s.\n", P_APP.lcs_report.address );
                        free( P_APP.lcs_report.address ); P_APP.lcs_report.address = NULL;
                        free( P_APP.lcs_report.conn ); P_APP.lcs_report.conn = NULL;
                        cJSON_Delete( config_json );
                        free( config_string ); config_string = NULL;
                        return FALSE;
                    } else {
                        LOG_print( &dcpam_wds_log, "[%s] Initialized LCS report module.\n", TIME_get_gmt() );
                    }
                } else {
                    LOG_print( &dcpam_wds_log, "ERROR: \"LCS.address\" key not found.\n " );
                    cJSON_Delete( config_json );
                    free( config_string ); config_string = NULL;
                    return FALSE;
                }

            } else {
                LOG_print( &dcpam_wds_log, "ERROR: \"LCS\" key not found.\n " );
                cJSON_Delete( config_json );
                free( config_string ); config_string = NULL;
                return FALSE;
            }

            cJSON_Delete( config_json );
        }
        free( config_string );
        config_string = NULL;

        result = 1;
    } else {
        LOG_print( &dcpam_wds_log, "[%s] Fatal error: unable to open config file \"%s\"!\n", TIME_get_gmt(), filename );
    }

    return result;
}

int DCPAM_WDS_init_cache( void ) {

    LOG_print( &dcpam_wds_log, "[%s] Init memory cache (slots: %d)...", TIME_get_gmt(), P_APP.CACHE_len );
    P_APP.CACHE_size = 0;
    P_APP.CACHE = SAFEMALLOC( P_APP.CACHE_len * sizeof * P_APP.CACHE, __FILE__, __LINE__ );
    for( int i = 0; i < P_APP.CACHE_len; i++ ) {
        P_APP.CACHE[ i ] = SAFEMALLOC( sizeof( D_CACHE ), __FILE__, __LINE__ );
        P_APP.CACHE[ i ]->query = NULL;
        P_APP.CACHE[ i ]->size = 0;
    }

    LOG_print( &dcpam_wds_log, "ok.\n" );

    /* Initialize database connections */
    for( int i = 0; i < P_APP.DB_len; i++ ) {
        LOG_print( &dcpam_wds_log, "[%s] Connecting to \"%s\"...\n", TIME_get_gmt(), P_APP.DB[ i ]->name );
        if( DATABASE_SYSTEM_DB_init( P_APP.DB[ i ], &dcpam_wds_log ) == 0 ) {
            LOG_print( &dcpam_wds_log, "[%s] Error: Unable to connect with \"%s\".\n", TIME_get_gmt(), P_APP.DB[ i ]->name );
            return 0;
        }
    }

    int initialized = 0;

    for( int i = 0; i < P_APP.DATA_len; i++ ) {
        for( int j = 0; j < P_APP.DATA[ i ].actions_len; j++ ) {

            /* Cache queries without variable conditions only. */
            if( P_APP.DATA[ i ].actions[ j ].condition && strlen( P_APP.DATA[ i ].actions[ j ].condition ) == 0 ) {

                DATABASE_SYSTEM_DB* src = NULL;

                LOG_print( &dcpam_wds_log, "[%s] Caching \"%s\" from \"%s\"...\n", TIME_get_gmt(), P_APP.DATA[ i ].actions[ j ].description, P_APP.DATA[ i ].db_name );
                src = DATABASE_SYSTEM_DB_get( P_APP.DATA[ i ].db_name );
                if( src ) {
                    LOG_print( &dcpam_wds_log, "\t- Database found.\n" );

                    int res = DB_CACHE_init(
                        P_APP.CACHE[ initialized ],
                        src,
                        P_APP.DATA[ i ].actions[ j ].sql,
                        &dcpam_wds_log
                    );

                    if( res == 1 ) {
                        DB_CACHE_print( P_APP.CACHE[ initialized ], &dcpam_wds_log );
                    } else {
                        LOG_print( &dcpam_wds_log, "[%s] Fatal error: DB_CACHE_init failed.\n", TIME_get_gmt() );
                    }

                    initialized++;

                } else {
                    LOG_print( &dcpam_wds_log, "[%s] Fatal error: database \"%s\" is not valid!\n", TIME_get_gmt(), P_APP.DATA[ i ].db_name );
                }
            } else {
                LOG_print( &dcpam_wds_log, "[%s] NOTICE: \"%s\" from \"%s\" is not static cache.\n", TIME_get_gmt(), P_APP.DATA[ i ].actions[ j ].description, P_APP.DATA[ i ].db_name );
            }
        }
    }

    return 1;
}

void DCPAM_WDS_get_data( const char *sql, const char *db, char **dst_json ) {
    DB_QUERY *cached_result = NULL;

    if( sql && db ) {
        LOG_print( &dcpam_wds_log, "[%s] DCPAM_WDS_get_data( %s, %s )...\n", TIME_get_gmt(), sql, db );

        char *action_description = NULL;
        char* gd_descr = "[DCPAM_WDS] Get data from %s database: %s";
        size_t gd_len = strlen( gd_descr );
        size_t gd_db_len = strlen( db );
        size_t gd_sql_len = strlen( sql );
        size_t gd_dst_buf_len = gd_len + gd_sql_len + gd_db_len;
        action_description = SAFECALLOC( gd_dst_buf_len + 1, sizeof( char ), __FILE__, __LINE__ );
        snprintf( action_description, gd_dst_buf_len + 1, gd_descr, db, sql );
        LCS_REPORT_send( &P_APP.lcs_report, action_description, DCT_START );
        free( action_description ); action_description = NULL;

        DB_CACHE_get( sql, &cached_result );

        if( cached_result ) {
            cJSON* record = NULL;
            cJSON* all_data = cJSON_CreateArray();
            cJSON* response = cJSON_CreateObject();

            LOG_print( &dcpam_wds_log, "[%s] DCPAM_WDS_get_data( %s, %s ): Found records: %d.\n", TIME_get_gmt(), sql, db, cached_result->row_count );
            for( int i = 0; i < cached_result->row_count; i++ ) {
                record = cJSON_CreateObject();
                for( int j = 0; j < cached_result->field_count; j++ ) {
                    cJSON_AddStringToObject( record, cached_result->records[ i ].fields[ j ].label, cached_result->records[ i ].fields[ j ].value );
                }

                cJSON_AddItemToArray( all_data, record );
            }

            cJSON_AddItemToObject( response, "data", all_data );
            cJSON_AddBoolToObject( response, "success", 1 );
            cJSON_AddNumberToObject( response, "length", cached_result->row_count );

            char* _res = cJSON_Print( response );
            *dst_json = SAFECALLOC( strlen( _res ) + 1, sizeof( char ), __FILE__, __LINE__ );
            strncpy( *dst_json, _res, strlen( _res ) );
            cJSON_Delete( response );
            free( _res );
        } else {
            LOG_print( &dcpam_wds_log, "requested data is not cached.\n" );

            DATABASE_SYSTEM_DB *src_db = DATABASE_SYSTEM_DB_get( db );
            if( src_db ) {

                P_APP.CACHE = realloc( P_APP.CACHE, (P_APP.CACHE_len + 1 ) * sizeof * P_APP.CACHE );
                if( P_APP.CACHE != NULL ) {

                    P_APP.CACHE[ P_APP.CACHE_len ] = SAFEMALLOC( sizeof( D_CACHE ), __FILE__, __LINE__ );
                    P_APP.CACHE[ P_APP.CACHE_len ]->query = NULL;

                    char* cache_descr = "[DCPAM_WDS] Cache query \"%s\". Database: %s";
                    size_t cache_len = strlen( cache_descr );
                    size_t cache_db_len = strlen( db );
                    size_t cache_sql_len = strlen( sql );
                    size_t cache_dst_buf_len = cache_len + cache_sql_len + cache_db_len;
                    action_description = SAFECALLOC( cache_dst_buf_len + 1, sizeof( char ), __FILE__, __LINE__ );
                    snprintf( action_description, cache_dst_buf_len + 1, cache_descr, sql, db );
                    LCS_REPORT_send( &P_APP.lcs_report, action_description, DCT_START );
                    free( action_description ); action_description = NULL;

                    int cache_res = DB_CACHE_init(
                        P_APP.CACHE[ P_APP.CACHE_len ],
                        src_db,
                        sql,
                        &dcpam_wds_log
                    );

                    action_description = SAFECALLOC( cache_dst_buf_len + 1, sizeof( char ), __FILE__, __LINE__ );
                    snprintf( action_description, cache_dst_buf_len + 1, cache_descr, sql, db );
                    LCS_REPORT_send( &P_APP.lcs_report, action_description, DCT_STOP );
                    free( action_description ); action_description = NULL;

                    if( cache_res == 1 ) { /* OK */
                        P_APP.CACHE_len++;
                        LOG_print( &dcpam_wds_log, "[%s] Data for request cached successfully.\n", TIME_get_gmt() );
                        DCPAM_WDS_get_data( sql, db, &(*dst_json) );
                    } else {
                        LOG_print( &dcpam_wds_log, "[%s] Error: unable to cache data for request.\n", TIME_get_gmt() );
                        if( cache_res == 2 ) { /* Memory limit exceeded */
                            P_APP.CACHE_len++;
                            DCPAM_WDS_get_data( sql, db, &( *dst_json ) );
                            P_APP.CACHE_len--;
                            DB_CACHE_free( P_APP.CACHE[ P_APP.CACHE_len ], &dcpam_wds_log );
                            free( P_APP.CACHE[ P_APP.CACHE_len ] );
                        }
                    }
                }

            } else {
                LOG_print( &dcpam_wds_log, "[%s] Error: database \"%s\" not found!\n", TIME_get_gmt(), db );
            }
        }

        action_description = SAFECALLOC( gd_dst_buf_len + 1, sizeof( char ), __FILE__, __LINE__ );
        snprintf( action_description, gd_dst_buf_len + 1, gd_descr, db, sql );
        LCS_REPORT_send( &P_APP.lcs_report, action_description, DCT_STOP );
        free( action_description ); action_description = NULL;
    } else {
        LOG_print( &dcpam_wds_log, "[%s] DCPAM_WDS_get_data error: not all parameters are valid!\n", TIME_get_gmt() );
    }
}

void DCPAM_WDS_query( COMMUNICATION_SESSION *communication_session, CONNECTED_CLIENT *client  ) {

    if( communication_session->data_length > 0 ) {

        char* request = NULL;
        cJSON *json_request = NULL;

        request = SAFECALLOC( communication_session->data_length + 1, sizeof( char ), __FILE__, __LINE__ );
        strncpy( request, communication_session->content, communication_session->data_length );

        json_request = cJSON_Parse( request );
        if( json_request ) {
            char* ip = inet_ntoa( communication_session->address.sin_addr );
            cJSON *sql = NULL;
            cJSON *db = NULL;
            cJSON *key = NULL;
            cJSON *msg = NULL;

            msg = cJSON_GetObjectItem( json_request, "msg" );
            if( msg ) {

                if( strcmp( msg->valuestring, "ping" ) == 0 ) {
                    const char* pong_msg = "{\"msg\": \"pong\"}";
                    SOCKET_send( communication_session, client, pong_msg, strlen( pong_msg ) );
                    SOCKET_disconnect_client( communication_session );
                    cJSON_Delete( json_request );
                    free( request ); request = NULL;
                    return;
                }
            }

            key = cJSON_GetObjectItem( json_request, "key" );
            if( key == NULL ) {
                LOG_print( &dcpam_wds_log, "[%s] Error: no \"key\" in request is found.\n", TIME_get_gmt() );
                WDS_RESPONSE_ERROR( communication_session, client );
                cJSON_Delete( json_request );
                free( request ); request = NULL;
                return;
            } else {

                for( int i = 0; i < P_APP.ALLOWED_HOSTS_len; i++ ) {
                    if( strcmp( ip, P_APP.ALLOWED_HOSTS_[ i ]->ip ) == 0 ) {
                        if( strcmp( key->valuestring, P_APP.ALLOWED_HOSTS_[ i ]->api_key ) != 0 ) {
                            LOG_print( &dcpam_wds_log, "[%s] Error: \"key\" in request is invalid.\n", TIME_get_gmt() );
                            WDS_RESPONSE_ERROR( communication_session, client );
                            cJSON_Delete( json_request );
                            free( request ); request = NULL;
                            return;
                        }
                    }
                }

            }

            LOG_print( &dcpam_wds_log, "[%s] Access granted for client %s with key %s.\n", TIME_get_gmt(), ip, key->valuestring );

            sql = cJSON_GetObjectItem( json_request, "sql" );
            if( sql == NULL) {
                LOG_print( &dcpam_wds_log, "[%s] Error: no SQL in request is found.\n", TIME_get_gmt() );
                WDS_RESPONSE_ERROR( communication_session, client );
                cJSON_Delete( json_request );
                free( request ); request = NULL;
                return;
            }

            db = cJSON_GetObjectItem( json_request, "db" );
            if( db  == NULL ) {
                LOG_print( &dcpam_wds_log, "[%s] Error: no DB name in request is found.\n", TIME_get_gmt() );
                WDS_RESPONSE_ERROR( communication_session, client );
                cJSON_Delete( json_request );
                free( request ); request = NULL;
                return;
            }

            DB_QUERY_TYPE dqt = DB_QUERY_get_type( sql->valuestring );
            if(
                dqt != DQT_SELECT
                ) {
                WDS_RESPONSE_ERROR( communication_session, client );
            } else {
                char* json_response = NULL;

                DCPAM_WDS_get_data( sql->valuestring, db->valuestring, &json_response );

                if( json_response ) {
                    SOCKET_send( communication_session, client, json_response, strlen( json_response ) );
                    SOCKET_disconnect_client( communication_session );
                    free( json_response ); json_response = NULL;
                } else {
                    WDS_RESPONSE_ERROR( communication_session, client );
                }
            }
            cJSON_Delete( json_request );
            free( request ); request = NULL;
        } else {
            LOG_print( &dcpam_wds_log, "[%s] Error: request is not valid JSON.\n", TIME_get_gmt() );
            WDS_RESPONSE_ERROR( communication_session, client );
            free( request ); request = NULL;
            return;
        }
    }

}

void* DCPAM_WDS_worker( void* LCS_worker_data ) {
    LOG_init( &dcpam_wds_lcs_log, "dcpam-wds-lcs", 65535 );

    int total_hosts = P_APP.ALLOWED_HOSTS_len + 1;

    char** allowed_hosts = SAFEMALLOC( ( total_hosts ) * sizeof * allowed_hosts, __FILE__, __LINE__ );

    for( int i = 0; i < P_APP.ALLOWED_HOSTS_len; i++ ) {
        size_t host_len = strlen( P_APP.ALLOWED_HOSTS_[ i ]->ip );
        allowed_hosts[ i ] = SAFECALLOC( host_len + 1, sizeof( char ), __FILE__, __LINE__ );
        strncpy( allowed_hosts[ i ], P_APP.ALLOWED_HOSTS_[ i ]->ip, host_len );
    }

    size_t lcs_host_len = strlen( P_APP.lcs_report.lcs_host );
    allowed_hosts[ P_APP.ALLOWED_HOSTS_len ] = SAFECALLOC( lcs_host_len + 1, sizeof( char ), __FILE__, __LINE__ );
    strncpy( allowed_hosts[ P_APP.ALLOWED_HOSTS_len ], P_APP.lcs_report.lcs_host, lcs_host_len );
    
    spc exec_script = ( spc )&DCPAM_WDS_query;
    SOCKET_main( &exec_script, P_APP.network_port, ( const char** )&( *allowed_hosts ), total_hosts, &dcpam_wds_lcs_log );

    for( int i = 0; i < total_hosts; i++ ) {
        free( allowed_hosts[ i ] ); allowed_hosts[ i ] = NULL;
    }
    free( allowed_hosts ); allowed_hosts = NULL;

    pthread_exit( NULL );
}

int main( int argc, char** argv ) {
    char        config_file[ MAX_PATH_LENGTH + 1 ];

    signal( SIGINT, (__sighandler_t)&app_terminate );
#ifndef _WIN32
    signal( SIGPIPE, SIG_IGN );
#endif
    signal( SIGABRT, (__sighandler_t)&app_terminate );
    signal( SIGTERM, (__sighandler_t)&app_terminate );

    LOG_init( &dcpam_wds_log, "dcpam-wds", 65535 );

    memset( config_file, '\0', MAX_PATH_LENGTH );
    if( argc <= 1 ) {
        snprintf( config_file, MAX_PATH_LENGTH, "./conf/wds_config.json" );
    } else {
        if( strlen( argv[ 1 ] ) > MAX_PATH_LENGTH ) {
            LOG_print( &dcpam_wds_log, "[%s] Notice: \"%s\" is not valid config file name.\n", TIME_get_gmt(), argv[ 1 ] );
            snprintf( config_file, MAX_PATH_LENGTH, "wds_config.json" );
        } else {
            snprintf( config_file, MAX_PATH_LENGTH, "%s", argv[ 1 ] );
        }
    }

    if( DCPAM_WDS_load_configuration( config_file ) == 1 ) {
        LOG_print( &dcpam_wds_log, "[%s] DCPAM Warehouse Data Server configuration loaded.\n", TIME_get_gmt() );

        if( DCPAM_WDS_init_cache() == 1 ) {
            pthread_t   lcs_worker_pid;
            LOG_print( &dcpam_wds_log, "[%s] Cache initialization finished.\n", TIME_get_gmt() );

            if( pthread_create( &lcs_worker_pid, NULL, DCPAM_WDS_worker, NULL ) == 0 ) {
                pthread_join( lcs_worker_pid, NULL );
            } else {
                LOG_print( &dcpam_wds_log, "[%s] Fatal error: unable to start thread for DCPAM LCS reporting.\n", TIME_get_gmt() );
            }

        } else {
            LOG_print( &dcpam_wds_log, "[%s] Warning: cache initialization failed.\n", TIME_get_gmt() );
        }

        //DCPAM_WDS_free_configuration();
    }

    LOG_print( &dcpam_wds_log, "[%s] DCPAM Warehouse Data Server finished.\n", TIME_get_gmt() );

    LOG_free( &dcpam_wds_log );
    LOG_free( &dcpam_wds_lcs_log );

    return 0;
}
