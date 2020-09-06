#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <signal.h>
#include <string.h>
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

#define WDS_RESPONSE_ERROR( communication_session, client ) { SOCKET_send( communication_session, client, "{\"success\":false,\"data\":[],\"length\":0}", 38 ); SOCKET_disconnect_client( communication_session ); }

#pragma warning( disable : 6031 )

char                    app_path[ MAX_PATH_LENGTH + 1 ];
LOG_OBJECT              dcpam_lcs_log;
extern int              app_terminated = 0;

extern DATABASE_SYSTEM  DATABASE_SYSTEMS[ MAX_DATA_SYSTEMS ];
extern int              DATABASE_SYSTEMS_COUNT;
extern L_DCPAM_APP      L_APP;

void DCPAM_LCS_free_configuration( void );


void app_terminate( void ) {
    LOG_print( &dcpam_lcs_log, "\r\n[%s] Received termination signal.\n", TIME_get_gmt() );
    if( app_terminated == 0 ) {
        app_terminated = 1;
        printf( "\r" );
        DCPAM_LCS_free_configuration();
        LOG_print( &dcpam_lcs_log, "[%s] DCPAM LCS graceful shutdown finished.\n", TIME_get_gmt() );
    }

    return;
}

void DCPAM_LCS_free_configuration( void ) {

    for( int i = 0; i < L_APP.ALLOWED_HOSTS_len; i++ ) {
        free( L_APP.ALLOWED_HOSTS_[ i ]->ip ); L_APP.ALLOWED_HOSTS_[ i ]->ip = NULL;
        free( L_APP.ALLOWED_HOSTS_[ i ]->api_key ); L_APP.ALLOWED_HOSTS_[ i ]->api_key = NULL;
    }
    free( L_APP.ALLOWED_HOSTS_ ); L_APP.ALLOWED_HOSTS_ = NULL;

    if( L_APP.version != NULL ) { free( L_APP.version ); L_APP.version = NULL; }
    if( L_APP.name != NULL ) { free( L_APP.name ); L_APP.name = NULL; }
}

int DCPAM_LCS_load_configuration( const char* filename ) {
    cJSON* config_json = NULL;

    cJSON* cfg_app = NULL;
    cJSON* cfg_app_version = NULL;
    cJSON* cfg_app_name = NULL;
    cJSON* cfg_app_network = NULL;
    cJSON* cfg_app_network_port = NULL;
    cJSON* cfg_app_network_allowed_hosts = NULL;
    cJSON* cfg_app_network_allowed_host_item = NULL;
    cJSON* cfg_app_network_allowed_host_item_ip = NULL;
    cJSON* cfg_app_network_allowed_host_item_key = NULL;

    int                         result = 0;
    char*                       config_string = NULL;


    LOG_print( &dcpam_lcs_log, "[%s] DCPAM_LCS_load_configuration( %s ).\n", TIME_get_gmt(), filename );

    config_string = file_get_content( filename );

    if( config_string ) {
        config_json = cJSON_Parse( config_string );
        if( config_json ) {

            cfg_app = cJSON_GetObjectItem( config_json, "app" );
            if( cfg_app ) {

                cfg_app_name = cJSON_GetObjectItem( cfg_app, "name" );
                if( cfg_app_name ) {
                    size_t str_len = strlen( cfg_app_name->valuestring );
                    L_APP.name = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                    snprintf( L_APP.name, str_len + 1, cfg_app_name->valuestring );
                } else {
                    LOG_print( &dcpam_lcs_log, "ERROR: \"app.name\" key not found.\n" );
                    cJSON_Delete( config_json );
                    free( config_string ); config_string = NULL;
                    return FALSE;
                }

                cfg_app_version = cJSON_GetObjectItem( cfg_app, "version" );
                if( cfg_app_version ) {
                    size_t str_len = strlen( cfg_app_version->valuestring );
                    L_APP.version = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                    snprintf( L_APP.version, str_len + 1, cfg_app_version->valuestring );
                    LOG_print( &dcpam_lcs_log, "%s v%s.\n", L_APP.name, L_APP.version );
                } else {
                    LOG_print( &dcpam_lcs_log, "ERROR: \"app.version\" key not found.\n" );
                    cJSON_Delete( config_json );
                    free( config_string ); config_string = NULL;
                    return FALSE;
                }

                cfg_app_network = cJSON_GetObjectItem( cfg_app, "network" );
                if( cfg_app_network ) {

                    cfg_app_network_port = cJSON_GetObjectItem( cfg_app_network, "port" );
                    if( cfg_app_network_port ) {
                        L_APP.network_port = cfg_app_network_port->valueint;
                    } else {
                        L_APP.network_port = 9090;
                    }
                    LOG_print( &dcpam_lcs_log, "Network port is set to %d.\n", L_APP.network_port );

                    cfg_app_network_allowed_hosts = cJSON_GetObjectItem( cfg_app_network, "allowed_hosts" );
                    if( cfg_app_network_allowed_hosts ) {
                        L_APP.ALLOWED_HOSTS_len = cJSON_GetArraySize( cfg_app_network_allowed_hosts );
                        L_APP.ALLOWED_HOSTS_ = SAFEMALLOC( L_APP.ALLOWED_HOSTS_len * sizeof * L_APP.ALLOWED_HOSTS_, __FILE__, __LINE__ );

                        for( int i = 0; i < L_APP.ALLOWED_HOSTS_len; i++ ) {
                            cfg_app_network_allowed_host_item = cJSON_GetArrayItem( cfg_app_network_allowed_hosts, i );
                            if( cfg_app_network_allowed_host_item ) {
                                L_APP.ALLOWED_HOSTS_[ i ] = SAFEMALLOC( sizeof( DCPAM_ALLOWED_HOST ), __FILE__, __LINE__ );

                                cfg_app_network_allowed_host_item_ip = cJSON_GetObjectItem( cfg_app_network_allowed_host_item, "ip" );
                                if( cfg_app_network_allowed_host_item_ip ) {
                                    size_t str_len = strlen( cfg_app_network_allowed_host_item_ip->valuestring );
                                    L_APP.ALLOWED_HOSTS_[ i ]->ip = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                                    snprintf( L_APP.ALLOWED_HOSTS_[ i ]->ip, str_len + 1, cfg_app_network_allowed_host_item_ip->valuestring );
                                }

                                cfg_app_network_allowed_host_item_key = cJSON_GetObjectItem( cfg_app_network_allowed_host_item, "key" );
                                if( cfg_app_network_allowed_host_item_key ) {
                                    size_t str_len = strlen( cfg_app_network_allowed_host_item_key->valuestring );
                                    L_APP.ALLOWED_HOSTS_[ i ]->api_key = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                                    snprintf( L_APP.ALLOWED_HOSTS_[ i ]->api_key, str_len + 1, cfg_app_network_allowed_host_item_key->valuestring );
                                }
                            }
                        }
                    } else {
                        LOG_print( &dcpam_lcs_log, "WARNING: \"app.network.allowed_hosts\" key not found.\n" );
                        L_APP.ALLOWED_HOSTS_len = 0;
                        L_APP.ALLOWED_HOSTS_ = NULL;
                    }
                } else {
                    LOG_print( &dcpam_lcs_log, "ERROR: \"app.network\" key not found.\n" );
                    cJSON_Delete( config_json );
                    free( config_string ); config_string = NULL;
                    return FALSE;
                }

            } else {
                LOG_print( &dcpam_lcs_log, "ERROR: \"app\" key not found.\n " );
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
        LOG_print( &dcpam_lcs_log, "[%s] Fatal error: unable to open config file \"%s\"!\n", TIME_get_gmt(), filename );
    }

    return result;
}

void DCPAM_LCS_query( COMMUNICATION_SESSION *communication_session, CONNECTED_CLIENT *client  ) {

    if( communication_session->data_length > 0 ) {

        char* request = NULL;
        cJSON *json_request = NULL;

        request = SAFECALLOC( communication_session->data_length + 1, sizeof( char ), __FILE__, __LINE__ );
        strncpy( request, communication_session->content, communication_session->data_length );

        LOG_print( &dcpam_lcs_log, "[%s] Received request (%ld): %s\n", TIME_get_gmt(), communication_session->data_length, request );

        json_request = cJSON_Parse( request );
        if( json_request ) {
            char* ip = inet_ntoa( communication_session->address.sin_addr );
            cJSON *sql = NULL;
            cJSON *db = NULL;
            cJSON *key = NULL;

            LOG_print( &dcpam_lcs_log, "[%s] Request is valid JSON.\n", TIME_get_gmt() );

            key = cJSON_GetObjectItem( json_request, "key" );
            if( key == NULL ) {
                LOG_print( &dcpam_lcs_log, "[%s] Error: no KEY in request is found.\n", TIME_get_gmt() );
                WDS_RESPONSE_ERROR( communication_session, client );
                cJSON_Delete( json_request );
                return;
            } else {

                for( int i = 0; i < L_APP.ALLOWED_HOSTS_len; i++ ) {
                    if( strcmp( ip, L_APP.ALLOWED_HOSTS_[ i ]->ip ) == 0 ) {
                        if( strcmp( key->valuestring, L_APP.ALLOWED_HOSTS_[ i ]->api_key ) != 0 ) {
                            LOG_print( &dcpam_lcs_log, "[%s] Error: KEY in request is invalid.\n", TIME_get_gmt() );
                            WDS_RESPONSE_ERROR( communication_session, client );
                            cJSON_Delete( json_request );
                            return;
                        }
                    }
                }

            }

            LOG_print( &dcpam_lcs_log, "[%s] Access granted for client %s with key %s.\n", TIME_get_gmt(), ip, key->valuestring );
            SOCKET_send( communication_session, client, "TEST OK", strlen( "TEST OK" ) );

            /*sql = cJSON_GetObjectItem( json_request, "sql" );
            if( sql == NULL) {
                LOG_print( &dcpam_lcs_log, "[%s] Error: no SQL in request is found.\n", TIME_get_gmt() );
                WDS_RESPONSE_ERROR( communication_session, client );
                cJSON_Delete( json_request );
                return;
            }

            db = cJSON_GetObjectItem( json_request, "db" );
            if( db  == NULL ) {
                LOG_print( &dcpam_lcs_log, "[%s] Error: no DB name in request is found.\n", TIME_get_gmt() );
                WDS_RESPONSE_ERROR( communication_session, client );
                cJSON_Delete( json_request );
                return;
            }

            DB_QUERY_TYPE dqt = DB_QUERY_get_type( sql->valuestring );
            if(
                dqt != DQT_SELECT
                ) {
                WDS_RESPONSE_ERROR( communication_session, client );
            } else {
                char* json_response = NULL;

                DCPAM_LCS_get_data( sql->valuestring, db->valuestring, &json_response );

                if( json_response ) {
                    SOCKET_send( communication_session, client, json_response, strlen( json_response ) );
                } else {
                    WDS_RESPONSE_ERROR( communication_session, client );
                }
            }*/
        } else {
            LOG_print( &dcpam_lcs_log, "[%s] Error: request is not valid JSON.\n", TIME_get_gmt() );
            WDS_RESPONSE_ERROR( communication_session, client );
            return;
        }
    }

}

int main( int argc, char** argv ) {
    char        config_file[ MAX_PATH_LENGTH + 1 ];

    signal( SIGINT, (__sighandler_t)&app_terminate );
#ifndef _WIN32
    signal( SIGPIPE, SIG_IGN );
#endif
    signal( SIGABRT, (__sighandler_t)&app_terminate );
    signal( SIGTERM, (__sighandler_t)&app_terminate );

    LOG_init( &dcpam_lcs_log, "dcpam-lcs", 65535 );

    memset( config_file, '\0', MAX_PATH_LENGTH );
    if( argc <= 1 ) {
        snprintf( config_file, MAX_PATH_LENGTH, "./conf/lcs_config.json" );
    } else {
        if( strlen( argv[ 1 ] ) > MAX_PATH_LENGTH ) {
            LOG_print( &dcpam_lcs_log, "[%s] Notice: \"%s\" is not valid config file name.\n", TIME_get_gmt(), argv[ 1 ] );
            snprintf( config_file, MAX_PATH_LENGTH, "lcs_config.json" );
        } else {
            snprintf( config_file, MAX_PATH_LENGTH, "%s", argv[ 1 ] );
        }
    }

    if( DCPAM_LCS_load_configuration( config_file ) == 1 ) {
        LOG_print( &dcpam_lcs_log, "[%s] DCPAM Live Component State configuration loaded.\n", TIME_get_gmt() );

        spc exec_script = ( spc )&DCPAM_LCS_query;
        char** allowed_hosts_ip = SAFEMALLOC( L_APP.ALLOWED_HOSTS_len * sizeof * L_APP.ALLOWED_HOSTS_, __FILE__, __LINE__ );

        for( int i = 0; i < L_APP.ALLOWED_HOSTS_len; i++ ) {
            size_t str_len = strlen( L_APP.ALLOWED_HOSTS_[ i ]->ip );
            allowed_hosts_ip[ i ] = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
            snprintf( allowed_hosts_ip[ i ], str_len + 1, L_APP.ALLOWED_HOSTS_[ i ]->ip );
        }

        SOCKET_main( &exec_script, L_APP.network_port, (const char **)&(*allowed_hosts_ip), L_APP.ALLOWED_HOSTS_len, &dcpam_lcs_log );

        DCPAM_LCS_free_configuration();
    }

    LOG_print( &dcpam_lcs_log, "[%s] DCPAM Live Component State finished.\n", TIME_get_gmt() );

    LOG_free( &dcpam_lcs_log );

    return 0;
}
