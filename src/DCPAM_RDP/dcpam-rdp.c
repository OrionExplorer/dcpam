#include <stdio.h>
#include <signal.h>
#include "../include/core/network/socket_io.h"
#include "../include/utils/log.h"
#include "../include/utils/time.h"
#include "../include/utils/memory.h"
#include "../include/utils/filesystem.h"
#include "../include/third-party/cJSON.h"
#include "../include/core/app_schema.h"

char                    app_path[ MAX_PATH_LENGTH + 1 ];
char                    LOG_filename[ MAX_PATH_LENGTH ];
LOG_OBJECT              dcpam_rdp_log;
R_DCPAM_APP             R_APP;
extern int              app_terminated = 0;

void app_terminate( void ) {
    LOG_print( &dcpam_rdp_log, "\rService is being closed..." );
    SOCKET_stop();
    LOG_print( &dcpam_rdp_log, "ok.\n" );
    return;
}

void DCPAM_script_exec( COMMUNICATION_SESSION *communication_session, CONNECTED_CLIENT *client  ) {
    LOG_print( &dcpam_rdp_log, "[%s] Received data (%ld): %s\n", TIME_get_gmt(), communication_session->data_length, communication_session->content );

    if( strstr( communication_session->content, "m=./" ) && strstr( communication_session->content, ".." ) == NULL ) {
        FILE    *script = NULL;
        char    key[ 64 ];
        char    module[ 256 ];
        char    command[ 4096 ];
        char    dhost[ 100 ], shost[ 100 ];
        int     dport, sport;
        char    duser[ 100 ], suser[ 100 ];
        char    dpass[ 100 ], spass[ 100 ];
        int     ddriver, sdriver;
        char    dconn[ 1024 ], sconn[ 1024 ];
        size_t  res_len = 0;
        char    res[ 4096 ];
        char    *ip = inet_ntoa( communication_session->address.sin_addr );

        memset( res, '\0', 4096 );
        sscanf( communication_session->content, "m=%255s dhost=%99s dport=%5d duser=%99s dpass=%99s ddriver=%2d dconn=\"%1024[^\"]\" shost=%99s sport=%5d suser=%99s spass=%99s sdriver=%2d sconn=\"%1024[^\"]\" key=%63s", module, dhost, &dport, duser, dpass, &ddriver, dconn, shost, &sport, suser, spass, &sdriver, sconn, key );

        for( int i = 0; i < R_APP.ALLOWED_HOSTS_len; i++ ) {
            if( strcmp( ip, R_APP.ALLOWED_HOSTS_[ i ]->ip ) == 0 ) {
                if( strcmp( key, R_APP.ALLOWED_HOSTS_[ i ]->api_key ) != 0 ) {
                    LOG_print( &dcpam_rdp_log, "[%s] Error: KEY in request is invalid.\n", TIME_get_gmt() );
                    SOCKET_send( communication_session, client, "-1", 2 );
                    SOCKET_disconnect_client( communication_session );
                    return;
                }
            }
        }

        LOG_print( &dcpam_rdp_log, "[%s] Access granted for client %s with key %s.\n", TIME_get_gmt(), ip, key );

        snprintf( command, 4096, "%s --dhost %s --dport %d --duser %s --dpass %s --ddriver %d --dconn \"%s\" --shost %s --sport %d --suser %s --spass %s --sdriver %d --sconn \"%s\"", module, dhost, dport, duser, dpass, ddriver, dconn, shost, sport, suser, spass, sdriver, sconn );
        LOG_print( &dcpam_rdp_log, "[%s] Executing local script %s...\n", TIME_get_gmt(), command );

        script = popen( command, READ_BINARY );
        if( script == NULL ) {
            LOG_print( &dcpam_rdp_log, "[%s] Error executing script %s.\n", TIME_get_gmt(), command );
            SOCKET_send( communication_session, client, "-1", 2 );
            SOCKET_disconnect_client( communication_session );
            pclose( script );
            return;
        }
        res_len = fread( res, sizeof( char ), 1, script );
        if( res_len == 0 ) {
            LOG_print( &dcpam_rdp_log, "[%s] Error executing script %s. No data returned.\n", TIME_get_gmt(), command );
            SOCKET_send( communication_session, client, "-1", 2 );
            SOCKET_disconnect_client( communication_session );
            pclose( script );
            return;
        }
        LOG_print( &dcpam_rdp_log, "[%s] Script %s finished with result: %s.\n", TIME_get_gmt(), command, res );
        SOCKET_send( communication_session, client, "1", 1 );
        SOCKET_disconnect_client( communication_session );
        pclose( script );
    } else {
        LOG_print( &dcpam_rdp_log, "[%s] Error: input data is invalid.\n", TIME_get_gmt() );
        SOCKET_send( communication_session, client, "-1", 2 );
        SOCKET_disconnect_client( communication_session );
    }
}

void DCPAM_RDP_free_configuration( void ) {
    for( int i = 0; i < R_APP.ALLOWED_HOSTS_len; i++ ) {
        free( R_APP.ALLOWED_HOSTS_[ i ]->ip ); R_APP.ALLOWED_HOSTS_[ i ]->ip;
        free( R_APP.ALLOWED_HOSTS_[ i ]->api_key ); R_APP.ALLOWED_HOSTS_[ i ]->api_key;
    }
    free( R_APP.ALLOWED_HOSTS_ ); R_APP.ALLOWED_HOSTS_ = NULL;

    if( R_APP.version != NULL ) { free( R_APP.version ); R_APP.version = NULL; }
    if( R_APP.name != NULL ) { free( R_APP.name ); R_APP.name = NULL; }
}

int DCPAM_RDP_load_configuration( const char* filename ) {
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
    char* config_string = NULL;


    LOG_print( &dcpam_rdp_log, "[%s] DCPAM_RDP_load_configuration( %s ).\n", TIME_get_gmt(), filename );

    config_string = file_get_content( filename );

    if( config_string ) {
        config_json = cJSON_Parse( config_string );
        if( config_json ) {

            cfg_app = cJSON_GetObjectItem( config_json, "app" );
            if( cfg_app ) {

                cfg_app_name = cJSON_GetObjectItem( cfg_app, "name" );
                if( cfg_app_name ) {
                    size_t str_len = strlen( cfg_app_name->valuestring );
                    R_APP.name = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                    snprintf( R_APP.name, str_len + 1, cfg_app_name->valuestring );
                } else {
                    LOG_print( &dcpam_rdp_log, "ERROR: \"app.name\" key not found.\n" );
                    cJSON_Delete( config_json );
                    free( config_string ); config_string = NULL;
                    return FALSE;
                }

                cfg_app_version = cJSON_GetObjectItem( cfg_app, "version" );
                if( cfg_app_version ) {
                    size_t str_len = strlen( cfg_app_version->valuestring );
                    R_APP.version = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                    snprintf( R_APP.version, str_len + 1, cfg_app_version->valuestring );
                    LOG_print( &dcpam_rdp_log, "%s v%s.\n", R_APP.name, R_APP.version );
                } else {
                    LOG_print( &dcpam_rdp_log, "ERROR: \"app.version\" key not found.\n" );
                    cJSON_Delete( config_json );
                    free( config_string ); config_string = NULL;
                    return FALSE;
                }

                cfg_app_network = cJSON_GetObjectItem( cfg_app, "network" );
                if( cfg_app_network ) {

                    cfg_app_network_port = cJSON_GetObjectItem( cfg_app_network, "port" );
                    if( cfg_app_network_port ) {
                        R_APP.network_port = cfg_app_network_port->valueint;
                    } else {
                        R_APP.network_port = 9090;
                    }
                    LOG_print( &dcpam_rdp_log, "Network port is set to %d.\n", R_APP.network_port );

                    cfg_app_network_allowed_hosts = cJSON_GetObjectItem( cfg_app_network, "allowed_hosts" );
                    if( cfg_app_network_allowed_hosts ) {
                        R_APP.ALLOWED_HOSTS_len = cJSON_GetArraySize( cfg_app_network_allowed_hosts );
                        R_APP.ALLOWED_HOSTS_ = SAFEMALLOC( R_APP.ALLOWED_HOSTS_len * sizeof * R_APP.ALLOWED_HOSTS_, __FILE__, __LINE__ );

                        for( int i = 0; i < R_APP.ALLOWED_HOSTS_len; i++ ) {
                            cfg_app_network_allowed_host_item = cJSON_GetArrayItem( cfg_app_network_allowed_hosts, i );
                            if( cfg_app_network_allowed_host_item ) {
                                R_APP.ALLOWED_HOSTS_[ i ] = SAFEMALLOC( sizeof( DCPAM_ALLOWED_HOST ), __FILE__, __LINE__ );

                                cfg_app_network_allowed_host_item_ip = cJSON_GetObjectItem( cfg_app_network_allowed_host_item, "ip" );
                                if( cfg_app_network_allowed_host_item_ip ) {
                                    size_t str_len = strlen( cfg_app_network_allowed_host_item_ip->valuestring );
                                    R_APP.ALLOWED_HOSTS_[ i ]->ip = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                                    snprintf( R_APP.ALLOWED_HOSTS_[ i ]->ip, str_len + 1, cfg_app_network_allowed_host_item_ip->valuestring );
                                }

                                cfg_app_network_allowed_host_item_key = cJSON_GetObjectItem( cfg_app_network_allowed_host_item, "key" );
                                if( cfg_app_network_allowed_host_item_key ) {
                                    size_t str_len = strlen( cfg_app_network_allowed_host_item_key->valuestring );
                                    R_APP.ALLOWED_HOSTS_[ i ]->api_key = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                                    snprintf( R_APP.ALLOWED_HOSTS_[ i ]->api_key, str_len + 1, cfg_app_network_allowed_host_item_key->valuestring );
                                }
                            }
                        }

                    } else {
                        LOG_print( &dcpam_rdp_log, "ERROR: \"app.network.allowed_hosts\" key not found.\n" );
                        cJSON_Delete( config_json );
                        free( config_string ); config_string = NULL;
                        return FALSE;
                    }
                } else {
                    LOG_print( &dcpam_rdp_log, "ERROR: \"app.network\" key not found.\n" );
                    cJSON_Delete( config_json );
                    free( config_string ); config_string = NULL;
                    return FALSE;
                }
            } else {
                LOG_print( &dcpam_rdp_log, "ERROR: \"app\" key not found.\n " );
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
        LOG_print( &dcpam_rdp_log, "[%s] Fatal error: unable to open config file \"%s\"!\n", TIME_get_gmt(), filename );
    }

    return result;
}

int main( int argc, char** argv ) {
    char        config_file[ MAX_PATH_LENGTH + 1 ];

    signal( SIGINT, ( __sighandler_t )&app_terminate );
#ifndef _WIN32
    signal( SIGPIPE, SIG_IGN );
#endif
    signal( SIGABRT, ( __sighandler_t )&app_terminate );
    signal( SIGTERM, ( __sighandler_t )&app_terminate );

    LOG_init( &dcpam_rdp_log, "dcpam-rdp", 65535 );
    memset( config_file, '\0', MAX_PATH_LENGTH );
    if( argc <= 1 ) {
        snprintf( config_file, MAX_PATH_LENGTH, "./conf/rdp_config.json" );
    } else {
        if( strlen( argv[ 1 ] ) > MAX_PATH_LENGTH ) {
            LOG_print( &dcpam_rdp_log, "[%s] Notice: \"%s\" is not valid config file name.\n", TIME_get_gmt(), argv[ 1 ] );
            snprintf( config_file, MAX_PATH_LENGTH, "rdp_config.json" );
        } else {
            snprintf( config_file, MAX_PATH_LENGTH, "%s", argv[ 1 ] );
        }
    }

    if( DCPAM_RDP_load_configuration( config_file ) == 1 ) {
        LOG_print( &dcpam_rdp_log, "[%s] DCPAM Remote Data Processor configuration loaded.\n", TIME_get_gmt() );

        spc exec_script = ( spc )&DCPAM_script_exec;
        char **allowed_hosts_ip = SAFEMALLOC( R_APP.ALLOWED_HOSTS_len * sizeof * R_APP.ALLOWED_HOSTS_, __FILE__, __LINE__ );

        for( int i = 0; i < R_APP.ALLOWED_HOSTS_len; i++ ) {
            size_t str_len = strlen( R_APP.ALLOWED_HOSTS_[ i ]->ip );
            allowed_hosts_ip[ i ] = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
            snprintf( allowed_hosts_ip[ i ], str_len + 1, R_APP.ALLOWED_HOSTS_[ i ]->ip );
        }

        SOCKET_main( &exec_script, R_APP.network_port, ( const char** )&(*allowed_hosts_ip), R_APP.ALLOWED_HOSTS_len, &dcpam_rdp_log );
    }

    LOG_free( &dcpam_rdp_log );

    return 0;
}
