#include <stdio.h>
#include <signal.h>
#include "../include/core/network/socket_io.h"
#include "../include/utils/log.h"
#include "../include/utils/time.h"

char                    app_path[ MAX_PATH_LENGTH + 1 ];
char                    LOG_filename[ MAX_PATH_LENGTH ];

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

int main( void ) {

    signal( SIGINT, ( sighandler )&app_terminate );
    #ifndef _WIN32
        signal( SIGPIPE, SIG_IGN );
    #endif
    signal( SIGABRT, ( sighandler )&app_terminate );
    signal( SIGTERM, ( sighandler )&app_terminate );

    LOG_init( "dcpam-rdp" );
    LOG_print( "DCPAM RDP\n");

    spc exec_script = ( spc )&DCPAM_script_exec;
    SOCKET_main( &exec_script );

    return 0;
}
