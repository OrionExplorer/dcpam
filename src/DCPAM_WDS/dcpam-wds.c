/* Copyright (C) 2020-2021 Marcin Kelar

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335 USA */

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
#include "../include/DCPAM_WDS/wds_node.h"

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
void DCPAM_WDS_get_db_data( const char *sql, const char *db, char **dst_json, size_t *dst_json_len );


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

    for( int i = 0; i < P_APP.SUB_CACHE_len; i++ ) {
        LOG_print( &dcpam_wds_log, "[%s] Removing sub-cache %d of %d...\n", TIME_get_gmt(), i + 1, P_APP.SUB_CACHE_len );
        DB_SUB_CACHE_free( P_APP.SUB_CACHE[ i ], &dcpam_wds_log );
        free( P_APP.SUB_CACHE[ i ] ); P_APP.SUB_CACHE[ i ] = NULL;
    }
    free( P_APP.SUB_CACHE ); P_APP.SUB_CACHE = NULL;
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

    for( int i = 0; i < P_APP.WDS_node_count; i++ ) {
        WDS_NODE_free( P_APP.WDS_node[ i ], &dcpam_wds_log );
        free( P_APP.WDS_node[ i ] ); P_APP.WDS_node[ i ] = NULL;
    }
    free( P_APP.WDS_node ); P_APP.WDS_node = NULL;
    P_APP.WDS_node_count = 0;

    if( P_APP.version != NULL ) { free( P_APP.version ); P_APP.version = NULL; }
    if( P_APP.name != NULL ) { free( P_APP.name ); P_APP.name = NULL; }

    for( int i = 0; i < P_APP.DATA_len; i++ ) {
        if( P_APP.DATA[ i ].id != NULL ) { free( P_APP.DATA[ i ].id ); P_APP.DATA[ i ].id = NULL; }
        if( P_APP.DATA[ i ].name != NULL ) { free( P_APP.DATA[ i ].name ); P_APP.DATA[ i ].name = NULL; }
        if( P_APP.DATA[ i ].db_name != NULL ) { free( P_APP.DATA[ i ].db_name ); P_APP.DATA[ i ].db_name = NULL; }
        if( P_APP.DATA[ i ].description != NULL ) { free( P_APP.DATA[ i ].description ); P_APP.DATA[ i ].description = NULL; }
        memset( P_APP.DATA[ i ].db_table_name, '\0', MAX_TABLE_NAME_LEN );

        for( int j = 0; j < P_APP.DATA[ i ].actions_len; j++ ) {
            if( P_APP.DATA[ i ].actions[ j ].name != NULL ) { free( P_APP.DATA[ i ].actions[ j ].name ); P_APP.DATA[ i ].actions[ j ].name = NULL; }
            if( P_APP.DATA[ i ].actions[ j ].description != NULL ) { free( P_APP.DATA[ i ].actions[ j ].description ); P_APP.DATA[ i ].actions[ j ].description = NULL; }
            if( P_APP.DATA[ i ].actions[ j ].condition != NULL ) { free( P_APP.DATA[ i ].actions[ j ].condition ); P_APP.DATA[ i ].actions[ j ].condition = NULL; }
            if( P_APP.DATA[ i ].actions[ j ].sql != NULL ) { free( P_APP.DATA[ i ].actions[ j ].sql ); P_APP.DATA[ i ].actions[ j ].sql = NULL; }
            P_APP.DATA[ i ].actions[ j ].cache_ttl = 0L;
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

    cJSON* cfg_app_wds_array = NULL;
    cJSON* cfg_app_wds_item = NULL;
    cJSON* cfg_app_wds_ip = NULL;
    cJSON* cfg_app_wds_port = NULL;
    cJSON* cfg_app_wds_key = NULL;
    cJSON* cfg_app_wds_tables_array = NULL;
    cJSON* cfg_app_wds_table_name = NULL;

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
    cJSON* cfg_app_data_actions_item_cache_ttl = NULL;

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
                    int mem_value = 0;
                    
                    P_APP.CACHE_size_multiplier = 1;
                    if( cfg_app_max_memory->valuestring && strlen( cfg_app_max_memory->valuestring ) < 32 ) {
                        char mem_unit[3];
                        if( 
                            ( sscanf( cfg_app_max_memory->valuestring, "%6d%2s", &mem_value, mem_unit ) == 2 ) 
                            ||
                            ( sscanf( cfg_app_max_memory->valuestring, "%6d %2s", &mem_value, mem_unit ) == 2 )
                        ) {
                            if( strncmp( mem_unit, "KB", 2 ) == 0 ) {
                                P_APP.CACHE_size_multiplier = 1024;
                                P_APP.CACHE_size_unit = MU_KB;
                            } else if( strncmp( mem_unit, "MB", 2 ) == 0 ) {
                                P_APP.CACHE_size_multiplier = 1024 * 1024;
                                P_APP.CACHE_size_unit = MU_MB;
                            } else if( strncmp( mem_unit, "GB", 2 ) == 0 ) {
                                P_APP.CACHE_size_multiplier = 1024 * 1024 * 1024;
                                P_APP.CACHE_size_unit = MU_GB;
                            } else if( strncmp( mem_unit, "TB", 2 ) == 0 ) {
                                P_APP.CACHE_size_multiplier = 1024 * 1024 * 1024 * 1024;
                                P_APP.CACHE_size_unit = MU_TB;
                            }

                            P_APP.CACHE_MAX_size = ( size_t )mem_value * P_APP.CACHE_size_multiplier;
                            LOG_print( &dcpam_wds_log, "Maximum memory usage: %zu %s (bytes: %zu).\n", P_APP.CACHE_MAX_size / P_APP.CACHE_size_multiplier, mem_unit, P_APP.CACHE_MAX_size );
                        } else {
                            LOG_print( &dcpam_wds_log, "ERROR: \"app.max_memory\" value is invalid.\n" );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                    } else {
                        LOG_print( &dcpam_wds_log, "ERROR: \"app.max_memory\" value is invalid.\n" );
                        cJSON_Delete( config_json );
                        free( config_string ); config_string = NULL;
                        return FALSE;
                    }
                }

                cfg_app_wds_array = cJSON_GetObjectItem( cfg_app, "WDS" );
                if( cfg_app_wds_array ) {
                    P_APP.WDS_node_count = cJSON_GetArraySize( cfg_app_wds_array );
                    P_APP.WDS_node = SAFEMALLOC( P_APP.WDS_node_count * sizeof * P_APP.WDS_node, __FILE__, __LINE__ );

                    for( int i = 0; i < P_APP.WDS_node_count; i++ ) {
                        LOG_print( &dcpam_wds_log, "Initialize DCPAM Warehouse Data Server node #%d:\n", i + 1 );
                        P_APP.WDS_node[ i ] = SAFEMALLOC( sizeof( WDS_NODE ), __FILE__, __LINE__ );

                        cfg_app_wds_item = cJSON_GetArrayItem( cfg_app_wds_array, i );

                        cfg_app_wds_ip = cJSON_GetObjectItem( cfg_app_wds_item, "ip" );
                        if( cfg_app_wds_ip == NULL ) {
                            LOG_print( &dcpam_wds_log, "ERROR: \"app.WDS.ip\" key not found.\n" );
                        } else {
                            size_t str_len = strlen( cfg_app_wds_ip->valuestring );
                            P_APP.WDS_node[ i ]->ip = SAFECALLOC( str_len  + 1, sizeof( char ), __FILE__, __LINE__ );
                            snprintf( P_APP.WDS_node[ i ]->ip, str_len + 1, cfg_app_wds_ip->valuestring );
                            LOG_print( &dcpam_wds_log, "\t· ip: %s\n", P_APP.WDS_node[ i ]->ip );
                        }

                        cfg_app_wds_port = cJSON_GetObjectItem( cfg_app_wds_item, "port" );
                        if( cfg_app_wds_port == NULL ) {
                            LOG_print( &dcpam_wds_log, "ERROR: \"app.WDS.port\" key not found.\n" );
                        } else {
                            P_APP.WDS_node[ i ]->port = cfg_app_wds_port->valueint;
                            LOG_print( &dcpam_wds_log, "\t· port: %d\n", P_APP.WDS_node[ i ]->port );
                        }

                        cfg_app_wds_key = cJSON_GetObjectItem( cfg_app_wds_item, "key" );
                        if( cfg_app_wds_key == NULL ) {
                            LOG_print( &dcpam_wds_log, "ERROR: \"app.WDS.key\" key not found.\n" );
                        } else {
                            size_t str_len = strlen( cfg_app_wds_key->valuestring );
                            P_APP.WDS_node[ i ]->key = SAFECALLOC( str_len  + 1, sizeof( char ), __FILE__, __LINE__ );
                            snprintf( P_APP.WDS_node[ i ]->key, str_len + 1, cfg_app_wds_key->valuestring );
                            LOG_print( &dcpam_wds_log, "\t· key: %s\n", P_APP.WDS_node[ i ]->key );
                        }

                        cfg_app_wds_tables_array = cJSON_GetObjectItem( cfg_app_wds_item, "tables" );
                        if( cfg_app_wds_tables_array == NULL ) {
                            LOG_print( &dcpam_wds_log, "ERROR: \"app.WDS.tables\" key not found.\n" );
                        } else {
                            P_APP.WDS_node[ i ]->tables_len = cJSON_GetArraySize( cfg_app_wds_tables_array );
                            P_APP.WDS_node[ i ]->tables = SAFEMALLOC( P_APP.WDS_node[ i ]->tables_len * sizeof( char* ) , __FILE__, __LINE__ );
                            LOG_print( &dcpam_wds_log, "\t· tables (%d):\n", P_APP.WDS_node[ i ]->tables_len );
                            for( int j = 0; j < P_APP.WDS_node[ i ]->tables_len; j++ ) {
                                cfg_app_wds_table_name = cJSON_GetArrayItem( cfg_app_wds_tables_array, j );
                                P_APP.WDS_node[ i ]->tables[ j ] = SAFECALLOC( MAX_TABLE_NAME_LEN, sizeof( char ), __FILE__, __LINE__ );
                                snprintf( P_APP.WDS_node[ i ]->tables[ j ], MAX_TABLE_NAME_LEN, cfg_app_wds_table_name->valuestring );
                                LOG_print( &dcpam_wds_log, "\t\t- %s\n", P_APP.WDS_node[ i ]->tables[ j ] );
                            }
                        }
                    }
                } else {
                    //TODO
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
                        snprintf( P_APP.DATA[ i ].db_table_name, MAX_TABLE_NAME_LEN, cfg_app_data_item_db_table_name->valuestring );
                        LOG_print( &dcpam_wds_log, "\t· db_table_name=\"%s\"\n", P_APP.DATA[ i ].db_table_name );

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


                                cfg_app_data_actions_item_cache_ttl = cJSON_GetObjectItem( cfg_app_data_actions_item, "cache_ttl" );
                                if( cfg_app_data_actions_item_cache_ttl == NULL ) {
                                    LOG_print( &dcpam_wds_log, "NOTICE: \"app.DATA[%d].actions[%d].cache_ttl\" key not found. Set default value of 60 seconds.\n", i, j );
                                    P_APP.DATA[ i ].actions[ j ].cache_ttl = 60;
                                }
                                P_APP.DATA[ i ].actions[ j ].cache_ttl = cfg_app_data_actions_item_cache_ttl->valueint;
                                LOG_print( &dcpam_wds_log, "\t\t· cache_ttl=%0.f\n", P_APP.DATA[ i ].actions[ j ].cache_ttl );


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
                                strlcpy( P_APP.DATA[ i ].actions[ j ].sql, cfg_app_data_actions_item_sql->valuestring, str_len4+1 );
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

    P_APP.SUB_CACHE_len = 0;

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
                        P_APP.DATA[ i ].actions[ j ].cache_ttl,
                        &dcpam_wds_log
                    );

                    if( res == -1 ) {
                        LOG_print( &dcpam_wds_log, "[%s] Data is sub-cached.\n", TIME_get_gmt() );
                    } else if( res == 1 ) {
                        DB_CACHE_print( P_APP.CACHE[ initialized ], &dcpam_wds_log );
                        initialized++;
                    } else {
                        LOG_print( &dcpam_wds_log, "[%s] Fatal error: DB_CACHE_init failed.\n", TIME_get_gmt() );
                    }

                } else {
                    LOG_print( &dcpam_wds_log, "[%s] Fatal error: database \"%s\" is not valid!\n", TIME_get_gmt(), P_APP.DATA[ i ].db_name );
                }
            } else {
                LOG_print( &dcpam_wds_log, "[%s] NOTICE: \"%s\" from \"%s\" is not static data.\n", TIME_get_gmt(), P_APP.DATA[ i ].actions[ j ].description, P_APP.DATA[ i ].db_name );
            }
        }
    }

    return 1;
}


void DCPAM_WDS_get_node_data( WDS_NODE* node, const char *sql, char **dst_json, size_t *dst_json_len ) {
    if( node && sql && dst_json ) {
        LOG_print( &dcpam_wds_log, "[%s] DCPAM_WDS_get_node_data( %s:%d, %30s(...) ).\n", TIME_get_gmt(), node->ip, node->port, sql );

        NET_CONN *conn = SAFEMALLOC( sizeof( NET_CONN ), __FILE__, __LINE__ );

        if( conn ) {
            conn->log = &dcpam_wds_log;
            if( NET_CONN_init( conn, node->ip, node->port, 0 ) == 1 ) {
                if( NET_CONN_connect( conn, node->ip, node->port, 0 ) == 1 ) {
                    const char* query_msg_tmp = "{\"key\": \"%s\", \"sql\": \"%s\"}";
                    size_t tmp_msg_len = strlen( query_msg_tmp );
                    size_t msg_len = tmp_msg_len + strlen( node->key ) + strlen( sql );
                    char* query_msg = SAFECALLOC( msg_len + 1, sizeof( char ), __FILE__, __LINE__ );
                    snprintf( query_msg, msg_len + 1, query_msg_tmp, node->key, sql );
                    if( NET_CONN_send( conn, query_msg, strlen( query_msg ) ) == 1 ) {
                        if( conn->response && conn->response_len > 0 ) {
                            if( dst_json_len != NULL) {
                                *dst_json_len = conn->response_len + 1;
                            }
                            *dst_json = SAFECALLOC( conn->response_len + 1, sizeof( char ), __FILE__, __LINE__ );
                            memcpy( *dst_json, conn->response, conn->response_len + 1 );
                        }
                        //NET_CONN_disconnect( conn );
                    }
                    free( query_msg ); query_msg = NULL;
                }
            }
            NET_CONN_disconnect( conn );
            free( conn->host ); conn->host = NULL;
            conn->log = NULL;
            free( conn ); conn = NULL;
        }

    } else {
        LOG_print( &dcpam_wds_log, "[%s] DCPAM_WDS_get_node_data: invalid input data.\n", TIME_get_gmt() );
    }
}

void DCPAM_WDS_get_data( const char *sql, char **dst_json, size_t *dst_json_len ) {

    if( sql ) {

        LOG_print( &dcpam_wds_log, "[%s] DCPAM_WDS_get_data SQL: %s\n", TIME_get_gmt(), sql );
        char *pos = strstr( sql, " FROM " );

        if( pos ) {
            char db_table_name[ 128 ];

            /* Check which main table is selected for query:
                - SELECT * FROM table_name
                - SELECT * FROM table_name WHERE id = 1 ...
                - SELECT * FROM table_name JOIN another_table ...
             */

            if( sscanf( pos, " FROM %128s", db_table_name ) == 1 ) {
                LOG_print( &dcpam_wds_log, "[%s] Main table for query: %s\n", TIME_get_gmt(), db_table_name );

                /* Find which DB node for current DCPAM WDS node hosts requested data */
                for( int i = 0; i < P_APP.DATA_len; i++ ) {
                    if( strncmp( P_APP.DATA[ i ].db_table_name, db_table_name, MAX_TABLE_NAME_LEN ) == 0 ) {
                        LOG_print( &dcpam_wds_log, "\t- found %s in db %s\n", P_APP.DATA[ i ].db_table_name, P_APP.DATA[ i ].db_name );
                        DCPAM_WDS_get_db_data( sql, P_APP.DATA[ i ].db_name, dst_json, dst_json_len );
                        return;
                    }
                }

                LOG_print( &dcpam_wds_log, "\t- table %s does not exist on any node for current WDS instance.\n", db_table_name );
                LOG_print( &dcpam_wds_log, "[%s] Searching table %s on external WDS nodes:\n", TIME_get_gmt(), db_table_name );
                /* Find which DB node for other DCPAM WDS nodes hosts requested data */
                for( int i = 0; i < P_APP.WDS_node_count; i++ ) {
                    if( P_APP.WDS_node ) {
                        LOG_print( &dcpam_wds_log, "\t- node #%d...", i + 1 );
                        for( int j = 0; j < P_APP.WDS_node[ i ]->tables_len; j++ ) {
                            if( strncmp( P_APP.WDS_node[ i ]->tables[ j ], db_table_name, MAX_TABLE_NAME_LEN ) == 0 ) {
                                LOG_print( &dcpam_wds_log, "found!\n" );
                                DCPAM_WDS_get_node_data( P_APP.WDS_node[ i ], sql, dst_json, dst_json_len );
                                return;
                            }
                        }
                        LOG_print( &dcpam_wds_log, "not found.\n" );
                    } else {
                        LOG_print( &dcpam_wds_log, "\t- node #%d is invalid!\n", i + 1 );
                    }
                }
                LOG_print( &dcpam_wds_log, "\t- table %s does not exist on any node.\n", db_table_name );
            } else {
                LOG_print( &dcpam_wds_log, "\t- unable to recognize table for query.\n" );
            }
        } else {
            LOG_print( &dcpam_wds_log, "\t- unable to recognize table for query. Is this SQL valid?\n" );
        }
    } else {
        LOG_print( &dcpam_wds_log, "[%s] DCPAM_WDS_get_data data error: sql parameter is missing!\n", TIME_get_gmt() );
    }
    return;
}


void DCPAM_WDS_get_db_data( const char *sql, const char *db, char **dst_json, size_t *dst_json_len ) {

    if( sql && db ) {

        DB_QUERY *cached_result = NULL;
        D_SUB_CACHE *subset = NULL;

        LOG_print( &dcpam_wds_log, "[%s] DCPAM_WDS_get_db_data( %s, %s )...\n", TIME_get_gmt(), sql, db );

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

        DB_CACHE_get( sql, &cached_result, &subset );

        if( cached_result || subset ) {
            cJSON* record = NULL;
            cJSON* all_data = cJSON_CreateArray();
            cJSON* response = cJSON_CreateObject();

            if( cached_result == NULL ) {
                cached_result = subset->src->query;
            }

            if( subset ) {
                LOG_print( &dcpam_wds_log, "[%s] DCPAM_WDS_get_db_data( %s, %s ): Found sub-cached records: %d.\n", TIME_get_gmt(), sql, db, subset->indices_len );
            } else {
                LOG_print( &dcpam_wds_log, "[%s] DCPAM_WDS_get_db_data( %s, %s ): Found cached records: %d.\n", TIME_get_gmt(), sql, db, cached_result->row_count );
            }
            
            if( cached_result && subset == NULL ) {
                /* No subset found, so We take all the cached data. */
                for( int i = 0; i < cached_result->row_count; i++ ) {
                    record = cJSON_CreateObject();
                    for( int j = 0; j < cached_result->field_count; j++ ) {
                        cJSON_AddStringToObject( record, cached_result->records[ i ].fields[ j ].label, cached_result->records[ i ].fields[ j ].value );
                    }

                    cJSON_AddItemToArray( all_data, record );
                }
                cJSON_AddNumberToObject( response, "length", cached_result->row_count );
            } else {
                /* We take only a subset of cached data */
                for( int i = 0; i < subset->indices_len; i++ ) {
                    record = cJSON_CreateObject();
                    for( int j = 0; j < cached_result->field_count; j++ ) {
                        cJSON_AddStringToObject( record, cached_result->records[ subset->indices[ i ] ].fields[ j ].label, cached_result->records[ subset->indices[ i ] ].fields[ j ].value );
                    }

                    cJSON_AddItemToArray( all_data, record );
                }
                cJSON_AddNumberToObject( response, "length", subset->indices_len );
            }

            cJSON_AddItemToObject( response, "data", all_data );
            cJSON_AddBoolToObject( response, "success", 1 );

            char* _res = cJSON_Print( response );
            *dst_json = SAFECALLOC( strlen( _res ) + 1, sizeof( char ), __FILE__, __LINE__ );
            if( dst_json_len != NULL) {
                *dst_json_len = strlen( _res );
            }
            strlcpy( *dst_json, _res, strlen( _res ) );
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
                        60,
                        &dcpam_wds_log
                    );

                    action_description = SAFECALLOC( cache_dst_buf_len + 1, sizeof( char ), __FILE__, __LINE__ );
                    snprintf( action_description, cache_dst_buf_len + 1, cache_descr, sql, db );
                    LCS_REPORT_send( &P_APP.lcs_report, action_description, DCT_STOP );
                    free( action_description ); action_description = NULL;

                    if( cache_res == -1 ) {
                        LOG_print( &dcpam_wds_log, "[%s] Data for request sub-cached successfully.\n", TIME_get_gmt() );

                        free( P_APP.CACHE[ P_APP.CACHE_len ] );
                        P_APP.CACHE = realloc( P_APP.CACHE, (P_APP.CACHE_len ) * sizeof * P_APP.CACHE );
                        DCPAM_WDS_get_db_data( sql, db, &( *dst_json ), &( *dst_json_len ) );
                        //DB_CACHE_free( P_APP.CACHE[ P_APP.CACHE_len + 1 ], &dcpam_wds_log );

                    }
                    else if( cache_res == 1 ) { /* OK */
                        P_APP.CACHE_len++;
                        LOG_print( &dcpam_wds_log, "[%s] Data for request cached successfully.\n", TIME_get_gmt() );
                        DCPAM_WDS_get_db_data( sql, db, &( *dst_json ), &( *dst_json_len ) );
                    } else {
                        LOG_print( &dcpam_wds_log, "[%s] Error: unable to cache data for request.\n", TIME_get_gmt() );
                        if( cache_res == 2 ) { /* Memory limit exceeded */
                            P_APP.CACHE_len++;
                            DCPAM_WDS_get_db_data( sql, db, &( *dst_json ), &( *dst_json_len ) );
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
        LOG_print( &dcpam_wds_log, "[%s] DCPAM_WDS_get_db_data error: not all parameters are valid!\n", TIME_get_gmt() );
    }
}

void DCPAM_WDS_query( COMMUNICATION_SESSION *communication_session, CONNECTED_CLIENT *client  ) {

    if( communication_session->data_length > 0 ) {

        char* request = NULL;
        cJSON *json_request = NULL;

        request = SAFECALLOC( communication_session->data_length + 1, sizeof( char ), __FILE__, __LINE__ );
        strlcpy( request, communication_session->content, communication_session->data_length );

        json_request = cJSON_Parse( request );
        if( json_request ) {
            char* ip = inet_ntoa( communication_session->address.sin_addr );
            cJSON *sql = NULL;
            cJSON *db = NULL;
            cJSON *key = NULL;
            cJSON *msg = NULL;
            cJSON *report = NULL;

            msg = cJSON_GetObjectItem( json_request, "msg" );
            if( msg ) {
                if( strcmp( msg->valuestring, "ping" ) == 0 ) {
                    const char* pong_msg = "{\"msg\": \"pong\"}";
                    SOCKET_send( communication_session, client, pong_msg, strlen( pong_msg ) );
                    SOCKET_disconnect_client( communication_session );
                    SOCKET_unregister_client( client->socket_descriptor, &dcpam_wds_log );
                    cJSON_Delete( json_request );
                    free( request ); request = NULL;
                    return;
                }
            }

            key = cJSON_GetObjectItem( json_request, "key" );
            if( key == NULL ) {
                LOG_print( &dcpam_wds_log, "[%s] Error: no \"key\" in request is found.\n", TIME_get_gmt() );
                WDS_RESPONSE_ERROR( communication_session, client );
                SOCKET_disconnect_client( communication_session );
                SOCKET_unregister_client( client->socket_descriptor, &dcpam_wds_log );
                cJSON_Delete( json_request );
                free( request ); request = NULL;
                return;
            } else {

                for( int i = 0; i < P_APP.ALLOWED_HOSTS_len; i++ ) {
                    if( strcmp( ip, P_APP.ALLOWED_HOSTS_[ i ]->ip ) == 0 ) {
                        if( strcmp( key->valuestring, P_APP.ALLOWED_HOSTS_[ i ]->api_key ) != 0 ) {
                            LOG_print( &dcpam_wds_log, "[%s] Error: \"key\" in request is invalid.\n", TIME_get_gmt() );
                            WDS_RESPONSE_ERROR( communication_session, client );
                            SOCKET_disconnect_client( communication_session );
                            SOCKET_unregister_client( client->socket_descriptor, &dcpam_wds_log );
                            cJSON_Delete( json_request );
                            free( request ); request = NULL;
                            return;
                        }
                    }
                }

            }

            LOG_print( &dcpam_wds_log, "[%s] Access granted for client %s with key %s.\n", TIME_get_gmt(), ip, key->valuestring );

            /* Report data */
            report = cJSON_GetObjectItem( json_request, "report" );
            if( report ) {
                if( strcmp( report->valuestring, "memory" ) == 0 ) {
                    char* mem_size = DB_CACHE_get_usage_str();
                    size_t mem_size_len = strlen( mem_size );
                    char* response_tmp = "{\"memory\": \"%s\"}";
                    size_t response_tmp_len = strlen( response_tmp );
                    size_t dst_buf_len = response_tmp_len + mem_size_len + 1;
                    char* dst_buf = SAFECALLOC( dst_buf_len, sizeof( char ), __FILE__, __LINE__ );
                    snprintf( dst_buf, dst_buf_len, response_tmp, mem_size );
                    free( mem_size ); mem_size = NULL;
                    SOCKET_send( communication_session, client, dst_buf, dst_buf_len );
                    SOCKET_disconnect_client( communication_session );
                    SOCKET_unregister_client( client->socket_descriptor, &dcpam_wds_log );
                    free( dst_buf ); dst_buf = NULL;
                    cJSON_Delete( json_request );
                    free( request ); request = NULL;
                    return;
                }
            }

            /* Query data */
            /* Get SQL */
            sql = cJSON_GetObjectItem( json_request, "sql" );
            if( sql == NULL) {
                LOG_print( &dcpam_wds_log, "[%s] Error: no SQL in request is found.\n", TIME_get_gmt() );
                WDS_RESPONSE_ERROR( communication_session, client );
                SOCKET_disconnect_client( communication_session );
                SOCKET_unregister_client( client->socket_descriptor, &dcpam_wds_log );
                cJSON_Delete( json_request );
                free( request ); request = NULL;
                return;
            }

            /* Get DB name */
            db = cJSON_GetObjectItem( json_request, "db" );

            DB_QUERY_TYPE dqt = DB_QUERY_get_type( sql->valuestring );
            if(
                dqt != DQT_SELECT
            ) {
                WDS_RESPONSE_ERROR( communication_session, client );
                SOCKET_disconnect_client( communication_session );
                SOCKET_unregister_client( client->socket_descriptor, &dcpam_wds_log );
            } else {
                char* json_response = NULL;
                size_t json_response_len = 0;

                if( db ) {
                    /* Query data from specific DB */
                    DCPAM_WDS_get_db_data( sql->valuestring, db->valuestring, &json_response, &json_response_len );
                } else {
                    /* Query data from node that delivers requested data */
                    DCPAM_WDS_get_data( sql->valuestring, &json_response, &json_response_len );
                }

                if( json_response ) {
                    SOCKET_send( communication_session, client, json_response, json_response_len );
                    SOCKET_disconnect_client( communication_session );
                    SOCKET_unregister_client( client->socket_descriptor, &dcpam_wds_log );
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
            SOCKET_disconnect_client( communication_session );
            SOCKET_unregister_client( client->socket_descriptor, &dcpam_wds_log );
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
        strlcpy( allowed_hosts[ i ], P_APP.ALLOWED_HOSTS_[ i ]->ip, host_len );
    }

    size_t lcs_host_len = strlen( P_APP.lcs_report.lcs_host );
    allowed_hosts[ P_APP.ALLOWED_HOSTS_len ] = SAFECALLOC( lcs_host_len + 1, sizeof( char ), __FILE__, __LINE__ );
    strlcpy( allowed_hosts[ P_APP.ALLOWED_HOSTS_len ], P_APP.lcs_report.lcs_host, lcs_host_len );
    
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
    }

    LOG_print( &dcpam_wds_log, "[%s] DCPAM Warehouse Data Server finished.\n", TIME_get_gmt() );

    LOG_free( &dcpam_wds_log );
    LOG_free( &dcpam_wds_lcs_log );

    return 0;
}
