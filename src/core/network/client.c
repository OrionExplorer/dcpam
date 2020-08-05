#include "../../include/core/network/client.h"
#include "../../include/utils/time.h"
#include "../../include/utils/memory.h"
#include "../../include/utils/log.h"

int NET_CONN_connect( NET_CONN *connection, const char *host, const int port ) {

    LOG_print( "[%s] NET_CONN_connect( %s, %d )...", TIME_get_gmt(), host, port );

    connection->socket = socket( AF_INET, SOCK_STREAM, 0 );
    if ( connection->socket == -1) {
        LOG_print("error. Could not create socket.\n");
    }
    LOG_print( "ok.\n" );
    
    connection->server.sin_addr.s_addr = inet_addr( host );
    connection->server.sin_family = AF_INET;
    connection->server.sin_port = htons( port );

    connection->response = NULL;

    connection->host = SAFECALLOC( 256, sizeof( char ), __FILE__, __LINE__ );
    snprintf( connection->host, 255, host );
    connection->port = port;

    LOG_print( "[%s] Connecting...", TIME_get_gmt() );
    if (connect( connection->socket , (struct sockaddr *)&connection->server , sizeof( connection->server ) ) < 0 ) {
        LOG_print( "error.\n" );
        return 0;
    }

    LOG_print("ok.\n");
    return 0;
}

int NET_CONN_disconnect( NET_CONN *connection ) {
    LOG_print( "[%s] NET_CONN_disconnect( %s, %d )...", TIME_get_gmt(), connection->host, connection->port );

    free( connection->host ); connection->host = NULL;
    free( connection->response ); connection->response = NULL;

    LOG_print( "ok.\n" );

    return closesocket( connection->socket );
}

int NET_CONN_send( NET_CONN *connection, const char *data, size_t data_len ) {

    char    response[ 256 ];

    if( send( connection->socket, data, data_len, 0 ) < 0 ) {
        LOG_print( "[%s] Error sending data to %s.\n", TIME_get_gmt(), connection->host );
        return 0;
    }

    int response_len = recv( connection->socket, response, 255, 0 );
    if( response_len < 0 ) {
        LOG_print( "[%s] Error receiving data to %s.\n", TIME_get_gmt(), connection->host );
        return 0;
    }

    connection->response = SAFECALLOC( response_len + 1, sizeof( char ), __FILE__, __LINE__ );
    strncpy(
        connection->response,
        response,
        response_len
    );

    return 1;
}
