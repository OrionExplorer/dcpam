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

    if( strstr( communication_session->content, "m=./" ) ) {
        FILE    *script = NULL;
        char    command[ 4096 ];
        size_t  res_len = 0;
        char    res[ 16 ];

        memset( res, '\0', 16 );
        sscanf( communication_session->content, "m=%99s dhost=", command );
        LOG_print( "\t- executing local script: %s...", command );
        script = popen( command, READ_BINARY );
        if( script == NULL ) {
            LOG_print( "error.\n" );
            SOCKET_send( communication_session, client, "-1", 2 );
            pclose( script );
            return;
        }
        res_len = fread( res, sizeof( char ), 16, script );
        if( res_len == 0 ) {
            LOG_print( "error. No data returned from script.\n" );
            SOCKET_send( communication_session, client, "-1", 2 );
            pclose( script );
            return;
        }
        LOG_print( "ok. Result: %s.\n", res );
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

    LOG_init( "dcpam-exec" );
    LOG_print( "DCPAM EXEC\n");

    spc exec_script = ( spc )&DCPAM_script_exec;
    SOCKET_main( &exec_script );

    return 0;
}
