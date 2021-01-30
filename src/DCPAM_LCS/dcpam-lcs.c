#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include "../include/third-party/cJSON.h"
#include "../include/utils/log.h"
#include "../include/utils/memory.h"
#include "../include/utils/time.h"
#include "../include/utils/strings.h"
#include "../include/utils/filesystem.h"
#include "../include/DCPAM_LCS/dcpam-lcs.h"
#include "../include/shared.h"
#include "../include/core/network/socket_io.h"
#include "../include/core/component.h"
#include "../include/DCPAM_LCS/lcs_worker.h"

#define WDS_RESPONSE_ERROR( communication_session, client ) { \
                SOCKET_send( communication_session, client, "{\"success\":false,\"data\":[],\"length\":0}", 38 ); \
                SOCKET_disconnect_client( communication_session ); \
                SOCKET_close( client->socket_descriptor ); \
}

#pragma warning( disable : 6031 )

char                    app_path[ MAX_PATH_LENGTH + 1 ];
LOG_OBJECT              dcpam_lcs_log;
int                     app_terminated = 0;
extern L_DCPAM_APP      L_APP;

void DCPAM_LCS_free_components( LOG_OBJECT* log );
void DCPAM_LCS_free_configuration( LOG_OBJECT *log );


void app_terminate( void ) {
    LOG_print( &dcpam_lcs_log, "\r\n[%s] Received termination signal.\n", TIME_get_gmt() );
    if( app_terminated == 0 ) {
        app_terminated = 1;
        printf( "\r" );
        LCS_WORKER_shutdown( &dcpam_lcs_log );
        DCPAM_LCS_free_configuration( &dcpam_lcs_log );
        LOG_print( &dcpam_lcs_log, "[%s] DCPAM LCS graceful shutdown finished.\n", TIME_get_gmt() );
    }

    return;
}

void DCPAM_LCS_free_configuration( LOG_OBJECT *log ) {

    LOG_print( log, "[%s] DCPAM_LCS_free_configuration...\n", TIME_get_gmt() );

    DCPAM_LCS_free_components( log );

    for( int i = 0; i < L_APP.ALLOWED_HOSTS_len; i++ ) {
        free( L_APP.ALLOWED_HOSTS_[ i ]->ip ); L_APP.ALLOWED_HOSTS_[ i ]->ip;
        free( L_APP.ALLOWED_HOSTS_[ i ]->api_key ); L_APP.ALLOWED_HOSTS_[ i ]->api_key;
        free( L_APP.ALLOWED_HOSTS_[ i ] ); L_APP.ALLOWED_HOSTS_[ i ] = NULL;
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
    cJSON* cfg_app_required_components = NULL;
    cJSON* cfg_app_required_components_item = NULL;
    cJSON* cfg_app_required_components_item_ip = NULL;
    cJSON* cfg_app_required_components_item_key = NULL;
    cJSON* cfg_app_network_allowed_host = NULL;
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

                    /* Load information about DCPAM Components required DCPAM to work */
                    cfg_app_required_components = cJSON_GetObjectItem( cfg_app_network, "required_components" );
                    if( cfg_app_required_components ) {

                        L_APP.COMPONENTS_len = cJSON_GetArraySize( cfg_app_required_components );
                        L_APP.COMPONENTS = SAFEMALLOC( L_APP.COMPONENTS_len * sizeof * L_APP.COMPONENTS, __FILE__, __LINE__ );

                        for( int i = 0; i < L_APP.COMPONENTS_len; i++ ) {
                            L_APP.COMPONENTS[ i ] = SAFEMALLOC( sizeof( DCPAM_COMPONENT ), __FILE__, __LINE__ );
                        }

                        int registered_components_count = 0; /* Counter in case of failure */
                        int reg_result = 0;

                        for( int i = 0; i < L_APP.COMPONENTS_len; i++ ) {
                            cfg_app_required_components_item = cJSON_GetArrayItem( cfg_app_required_components, i );
                            if( cfg_app_required_components_item ) {

                                registered_components_count++;

                                cJSON *name = cJSON_GetObjectItem( cfg_app_required_components_item, "name" );
                                cJSON *version = cJSON_GetObjectItem( cfg_app_required_components_item, "version" );
                                cJSON *ip = cJSON_GetObjectItem( cfg_app_required_components_item, "ip" );
                                cJSON *port = cJSON_GetObjectItem( cfg_app_required_components_item, "port" );

                                /* Try to register DCPAM Component. Perform network connection with internal ping message. */
                                LOG_print( &dcpam_lcs_log, "[%s] [%d/%d] Registering DCPAM Component: \"%s\"...\n", TIME_get_gmt(), i + 1, L_APP.COMPONENTS_len, name->valuestring );
                                reg_result = LCS_COMPONENT_register(
                                    L_APP.COMPONENTS[ i ],
                                    name->valuestring,
                                    version->valuestring,
                                    ip->valuestring,
                                    port->valueint,
                                    &dcpam_lcs_log
                                );

                                if( reg_result == 1 ) {
                                    LOG_print( &dcpam_lcs_log, "[%s] [%d/%d] DCPAM Component \"%s\" is online.\n", TIME_get_gmt(), i + 1, L_APP.COMPONENTS_len, name->valuestring );
                                } else {
                                    /* DCPAM Component registration failed. DCPAM LCS could not start due to entire DCPAM misconfiguration. */
                                    LOG_print( &dcpam_lcs_log, "[%s] [%d/%d] DCPAM Component \"%s\" registration failed.\n", TIME_get_gmt(), i + 1, L_APP.COMPONENTS_len, name->valuestring );
                                    //break;
                                }
                            }
                        }

                    } else {
                        L_APP.COMPONENTS_len = 0;
                        L_APP.COMPONENTS = NULL;
                        LOG_print( &dcpam_lcs_log, "ERROR: \"app.network.required_components\" key not found.\n" );
                        cJSON_Delete( config_json );
                        free( config_string ); config_string = NULL;
                        return FALSE;
                    }

                    cfg_app_network_allowed_host = cJSON_GetObjectItem( cfg_app_network, "allowed_hosts" );
                    if( cfg_app_network_allowed_host ) {
                        L_APP.ALLOWED_HOSTS_len = cJSON_GetArraySize( cfg_app_network_allowed_host );
                        L_APP.ALLOWED_HOSTS_ = SAFEMALLOC( L_APP.ALLOWED_HOSTS_len * sizeof * L_APP.ALLOWED_HOSTS_, __FILE__, __LINE__ );

                        for( int i = 0; i < L_APP.ALLOWED_HOSTS_len; i++ ) {
                            cfg_app_network_allowed_host_item = cJSON_GetArrayItem( cfg_app_network_allowed_host, i );
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
                        L_APP.ALLOWED_HOSTS_len = 0;
                        LOG_print( &dcpam_lcs_log, "ERROR: \"app.network.allowed_hosts\" key not found.\n" );
                        cJSON_Delete( config_json );
                        free( config_string ); config_string = NULL;
                        return FALSE;
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

DCPAM_COMPONENT* COMPONENT_get( const char* name, const char* version, const char* ip ) {
    for( int i = 0; i < L_APP.COMPONENTS_len; i++ ) {
        if( strcmp( name, L_APP.COMPONENTS[ i ]->name ) == 0 ) {
            if( strcmp( version, L_APP.COMPONENTS[ i ]->version ) == 0 ) {
                if( strcmp( ip, L_APP.COMPONENTS[ i ]->ip ) == 0 ) {
                    return L_APP.COMPONENTS[ i ];
                }
            }
        }
    }

    return NULL;
}

void DCPAM_LCS_listener( COMMUNICATION_SESSION *communication_session, CONNECTED_CLIENT *client, LOG_OBJECT *log  ) {

    if( communication_session->data_length > 0 ) {

        char* request = NULL;
        cJSON *json_request = NULL;

        request = SAFECALLOC( communication_session->data_length + 1, sizeof( char ), __FILE__, __LINE__ );
        strlcpy( request, communication_session->content, communication_session->data_length );

        LOG_print( &dcpam_lcs_log, "[%s] Received request (%ld): %s\n", TIME_get_gmt(), communication_session->data_length, request );

        json_request = cJSON_Parse( request );
        if( json_request ) {
            char* ip = inet_ntoa( communication_session->address.sin_addr );

            /* Check Component action */
            cJSON *app = cJSON_GetObjectItem( json_request, "app" );
            cJSON *version = cJSON_GetObjectItem( json_request, "ver" );
            cJSON *action = cJSON_GetObjectItem( json_request, "action" );
            cJSON *action_type = cJSON_GetObjectItem( json_request, "type" );
            if( app && version && action && action_type ) {

                size_t action_len = strlen( action->valuestring );
                DCPAM_COMPONENT* component = COMPONENT_get( app->valuestring, version->valuestring, ip );

                if( component ) {

                    COMPONENT_ACTION_TYPE tmp_action_type = DCT_START;
                    if( strcmp( action_type->valuestring, "stop" ) == 0 ) {
                        tmp_action_type = DCT_STOP;
                    }

                    int reg_res = LCS_COMPONENT_ACTION_register( component, action->valuestring, tmp_action_type, log );
                    if( reg_res == 1 ) {
                        SOCKET_send( communication_session, client, "1", 1 );
                    } else {
                        SOCKET_send( communication_session, client, "-1", 2 );
                    }
                    /*SOCKET_unregister_client( client->socket_descriptor, log );
                    SOCKET_close( client->socket_descriptor );*/
                    SOCKET_disconnect_client( communication_session );
                    SOCKET_unregister_client( client->socket_descriptor, log );
                    SOCKET_close( communication_session->socket_descriptor );
                } else {
                    LOG_print( &dcpam_lcs_log, "[%s] Error: DCPAM Component( %s v%s, %s ) not found!\n", TIME_get_gmt(), app->valuestring, version->valuestring, ip );
                    SOCKET_send( communication_session, client, "-1", 2 );
                    /*SOCKET_unregister_client( client->socket_descriptor, log );
                    SOCKET_close( client->socket_descriptor );*/
                    SOCKET_disconnect_client( communication_session );
                    SOCKET_unregister_client( client->socket_descriptor, log );
                    SOCKET_close( communication_session->socket_descriptor );
                }
            } else {
                cJSON *key = cJSON_GetObjectItem( json_request, "key" );
                if( key ) {
                    for( int i = 0; i < L_APP.ALLOWED_HOSTS_len; i++ ) {
                        if( strcmp( ip, L_APP.ALLOWED_HOSTS_[ i ]->ip ) == 0 ) {
                            if( strcmp( key->valuestring, L_APP.ALLOWED_HOSTS_[ i ]->api_key ) != 0 ) {
                                LOG_print( &dcpam_lcs_log, "[%s] Error: \"key\" in request is invalid.\n", TIME_get_gmt() );
                                SOCKET_send( communication_session, client, "-1", 2 );
                                /*SOCKET_unregister_client( client->socket_descriptor, log );
                                SOCKET_close( client->socket_descriptor );*/
                                SOCKET_disconnect_client( communication_session );
                                SOCKET_unregister_client( client->socket_descriptor, log );
                                SOCKET_close( communication_session->socket_descriptor );
                                cJSON_Delete( json_request );
                                free( request ); request = NULL;
                                return;
                            }
                        }
                    }

                    LOG_print( &dcpam_lcs_log, "[%s] Access granted for client %s with key %s.\n", TIME_get_gmt(), ip, key->valuestring );

                    cJSON *response = LCS_COMPONENT_process_report( json_request, L_APP.COMPONENTS, L_APP.COMPONENTS_len, &dcpam_lcs_log );//cJSON_GetObjectItem( json_request, "report" );
                    if( response ) {
                        char* _res = cJSON_Print( response );
                        SOCKET_send( communication_session, client, _res, strlen( _res ) );
                        /*SOCKET_unregister_client( client->socket_descriptor, log );
                        SOCKET_close( client->socket_descriptor );*/
                        SOCKET_disconnect_client( communication_session );
                        SOCKET_unregister_client( client->socket_descriptor, log );
                        SOCKET_close( communication_session->socket_descriptor );
                        cJSON_Delete( json_request );
                        cJSON_Delete( response );
                        free( request ); request = NULL;
                        free( _res );
                        return;
                    } else {
                        LOG_print( &dcpam_lcs_log, "[%s] Error: request is invalid.\n", TIME_get_gmt() );
                        WDS_RESPONSE_ERROR( communication_session, client );
                        cJSON_Delete( json_request );
                        free( request ); request = NULL;
                        return;
                    }

                } else {
                    LOG_print( &dcpam_lcs_log, "[%s] Error: request is invalid.\n", TIME_get_gmt() );
                    WDS_RESPONSE_ERROR( communication_session, client );
                    cJSON_Delete( json_request );
                    free( request ); request = NULL;
                    return;
                }
            }
            /*SOCKET_unregister_client( client->socket_descriptor, log );
            SOCKET_close( client->socket_descriptor );*/
            SOCKET_disconnect_client( communication_session );
            SOCKET_unregister_client( client->socket_descriptor, log );
            SOCKET_close( communication_session->socket_descriptor );
            cJSON_Delete( json_request );

        } else {
            LOG_print( &dcpam_lcs_log, "[%s] Error: request is not valid JSON.\n", TIME_get_gmt() );
            WDS_RESPONSE_ERROR( communication_session, client );
            free( request ); request = NULL;
            return;
        }

        /*SOCKET_unregister_client( client->socket_descriptor, log );
        SOCKET_close( client->socket_descriptor );*/
        SOCKET_disconnect_client( communication_session );
        SOCKET_unregister_client( client->socket_descriptor, log );
        SOCKET_close( communication_session->socket_descriptor );
        free( request ); request = NULL;
    }

}

void DCPAM_LCS_free_components( LOG_OBJECT *log ) {
    LOG_print( log, "[%s] DCPAM_LCS_free_components...\n", TIME_get_gmt() );
    for( int i = 0; i < L_APP.COMPONENTS_len; i++ ) {
        if( LCS_COMPONENT_free( L_APP.COMPONENTS[ i ] ) == 0 ) {
            LOG_print( log, "[%s] Error: unable to free component!\n", TIME_get_gmt() );
        } else {
            free( L_APP.COMPONENTS[ i ] ); L_APP.COMPONENTS[ i ] = NULL;
        }
    }
    free( L_APP.COMPONENTS ); L_APP.COMPONENTS = NULL;
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

        pthread_t   lcs_worker_pid;

        LOG_print( &dcpam_lcs_log, "[%s] LCS_WORKER_init...", TIME_get_gmt() );

        if( pthread_create( &lcs_worker_pid, NULL, LCS_WORKER_watcher, NULL ) == 0 ) {
            int     total_hosts = L_APP.ALLOWED_HOSTS_len + L_APP.COMPONENTS_len;
            char    **allowed_hosts = SAFEMALLOC( total_hosts * sizeof * allowed_hosts, __FILE__, __LINE__ );

            for( int i = 0; i < L_APP.COMPONENTS_len; i++ ) {
                size_t host_len = strlen( L_APP.COMPONENTS[ i ]->ip );
                allowed_hosts[ i ] = SAFECALLOC( host_len + 1, sizeof( char ), __FILE__, __LINE__ );
                snprintf( allowed_hosts[ i ], host_len + 1, L_APP.COMPONENTS[ i ]->ip );
            }
            for( int i = L_APP.COMPONENTS_len, j = 0; i < total_hosts; i++, j++ ) {
                
                size_t host_len = strlen( L_APP.ALLOWED_HOSTS_[ j ]->ip );
                allowed_hosts[ i ] = SAFECALLOC( host_len + 1, sizeof( char ), __FILE__, __LINE__ );
                snprintf( allowed_hosts[ i ], host_len + 1, L_APP.ALLOWED_HOSTS_[ j ]->ip );
            }

            spc exec_script = ( spc )&DCPAM_LCS_listener;
            //SOCKET_main( NULL, L_APP.network_port, ( const char** )&( *allowed_hosts ), ( total_hosts ), &dcpam_lcs_log );
            SOCKET_main( &exec_script, L_APP.network_port, ( const char** )&( *allowed_hosts ), ( total_hosts ), &dcpam_lcs_log );

            for( int i = 0; i < total_hosts; i++ ) {
                free( allowed_hosts[ i ] ); allowed_hosts[ i ] = NULL;
            }
            free( allowed_hosts ); allowed_hosts = NULL;

            pthread_join( lcs_worker_pid, NULL );
        } else {
            return 0;
        }
    }

    LOG_print( &dcpam_lcs_log, "[%s] DCPAM Live Component State finished.\n", TIME_get_gmt() );

    LOG_free( &dcpam_lcs_log );

    return 0;
}
