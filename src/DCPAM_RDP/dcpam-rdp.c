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
R_DCPAM_APP             R_APP;

void app_terminate( void ) {
    printf( "\rService is being closed..." );
    SOCKET_stop();
    printf( "ok.\n" );
    return;
}

void DCPAM_script_exec( COMMUNICATION_SESSION *communication_session, CONNECTED_CLIENT *client  ) {
    LOG_print( "[%s] Received data (%ld): %s\n", TIME_get_gmt(), communication_session->data_length, communication_session->content );

    if( strstr( communication_session->content, "m=./" ) && strstr( communication_session->content, ".." ) == NULL ) {
        FILE    *script = NULL;
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

        memset( res, '\0', 4096 );
        sscanf( communication_session->content, "m=%255s dhost=%99s dport=%5d duser=%99s dpass=%99s ddriver=%2d dconn=\"%1024[^\"]\" shost=%99s sport=%5d suser=%99s spass=%99s sdriver=%2d sconn=\"%1024[^\"]\"", module, dhost, &dport, duser, dpass, &ddriver, dconn, shost, &sport, suser, spass, &sdriver, sconn );
        snprintf( command, 4096, "%s --dhost %s --dport %d --duser %s --dpass %s --ddriver %d --dconn \"%s\" --shost %s --sport %d --suser %s --spass %s --sdriver %d --sconn \"%s\"", module, dhost, dport, duser, dpass, ddriver, dconn, shost, sport, suser, spass, sdriver, sconn );
        LOG_print( "[%s] Executing local script %s...\n", TIME_get_gmt(), command );

        script = popen( command, READ_BINARY );
        if( script == NULL ) {
            LOG_print( "[%s] Error executing script %s.\n", TIME_get_gmt(), command );
            SOCKET_send( communication_session, client, "-1", 2 );
            pclose( script );
            return;
        }
        res_len = fread( res, sizeof( char ), 1, script );
        if( res_len == 0 ) {
            LOG_print( "[%s] Error executing script %s. No data returned.\n", TIME_get_gmt(), command );
            SOCKET_send( communication_session, client, "-1", 2 );
            pclose( script );
            return;
        }
        LOG_print( "[%s] Script %s finished with result: %s.\n", TIME_get_gmt(), command, res );
        SOCKET_send( communication_session, client, "1", 1 );
        pclose( script );
    } else {
        LOG_print( "[%s] Error: input data is invalid.\n", TIME_get_gmt() );
        SOCKET_send( communication_session, client, "-1", 2 );
    }
}

void DCPAM_RDP_free_configuration( void ) {
    for( int i = 0; i < R_APP.ALLOWED_HOSTS_len; i++ ) {
        free( R_APP.ALLOWED_HOSTS[ i ] ); R_APP.ALLOWED_HOSTS[ i ] = NULL;
    }
    free( R_APP.ALLOWED_HOSTS ); R_APP.ALLOWED_HOSTS = NULL;

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
    int                         result = 0;
    char* config_string = NULL;


    LOG_print( "[%s] DCPAM_RDP_load_configuration( %s ).\n", TIME_get_gmt(), filename );

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
                    LOG_print( "ERROR: \"app.name\" key not found.\n" );
                    cJSON_Delete( config_json );
                    free( config_string ); config_string = NULL;
                    return FALSE;
                }

                cfg_app_version = cJSON_GetObjectItem( cfg_app, "version" );
                if( cfg_app_version ) {
                    size_t str_len = strlen( cfg_app_version->valuestring );
                    R_APP.version = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                    snprintf( R_APP.version, str_len + 1, cfg_app_version->valuestring );
                    LOG_print( "%s v%s.\n", R_APP.name, R_APP.version );
                } else {
                    LOG_print( "ERROR: \"app.version\" key not found.\n" );
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
                    LOG_print( "Network port is set to %d.\n", R_APP.network_port );

                    cfg_app_network_allowed_hosts = cJSON_GetObjectItem( cfg_app_network, "allowed_hosts" );
                    if( cfg_app_network_allowed_hosts ) {
                        R_APP.ALLOWED_HOSTS_len = cJSON_GetArraySize( cfg_app_network_allowed_hosts );
                        R_APP.ALLOWED_HOSTS = SAFEMALLOC( R_APP.ALLOWED_HOSTS_len * sizeof * R_APP.ALLOWED_HOSTS, __FILE__, __LINE__ );

                        for( int i = 0; i < R_APP.ALLOWED_HOSTS_len; i++ ) {
                            cfg_app_network_allowed_host_item = cJSON_GetArrayItem( cfg_app_network_allowed_hosts, i );
                            if( cfg_app_network_allowed_host_item ) {
                                size_t str_len = strlen( cfg_app_network_allowed_host_item->valuestring );
                                R_APP.ALLOWED_HOSTS[ i ] = SAFECALLOC( ( str_len + 1 ), sizeof( char ), __FILE__, __LINE__ );
                                snprintf( R_APP.ALLOWED_HOSTS[ i ], str_len + 1, cfg_app_network_allowed_host_item->valuestring );
                            }
                        }
                    } else {
                        LOG_print( "ERROR: \"app.network.allowed_hosts\" key not found.\n" );
                        cJSON_Delete( config_json );
                        free( config_string ); config_string = NULL;
                        return FALSE;
                    }
                } else {
                    LOG_print( "ERROR: \"app.network\" key not found.\n" );
                    cJSON_Delete( config_json );
                    free( config_string ); config_string = NULL;
                    return FALSE;
                }
            } else {
                LOG_print( "ERROR: \"app\" key not found.\n " );
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
        LOG_print( "[%s] Fatal error: unable to open config file \"%s\"!\n", TIME_get_gmt(), filename );
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

    LOG_init( "dcpam-rdp" );
    memset( config_file, '\0', MAX_PATH_LENGTH );
    if( argc <= 1 ) {
        snprintf( config_file, MAX_PATH_LENGTH, "./conf/rdp_config.json" );
    } else {
        if( strlen( argv[ 1 ] ) > MAX_PATH_LENGTH ) {
            LOG_print( "[%s] Notice: \"%s\" is not valid config file name.\n", TIME_get_gmt(), argv[ 1 ] );
            snprintf( config_file, MAX_PATH_LENGTH, "rdp_config.json" );
        } else {
            snprintf( config_file, MAX_PATH_LENGTH, "%s", argv[ 1 ] );
        }
    }

    if( DCPAM_RDP_load_configuration( config_file ) == 1 ) {
        LOG_print( "[%s] DCPAM Remote Data Processor configuration loaded.\n", TIME_get_gmt() );

        spc exec_script = ( spc )&DCPAM_script_exec;
        SOCKET_main( &exec_script, R_APP.network_port, ( const char** )&(*R_APP.ALLOWED_HOSTS), R_APP.ALLOWED_HOSTS_len );
    }

    return 0;
}
