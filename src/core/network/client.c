#include "../../include/core/network/client.h"
#include "../../include/utils/time.h"
#include "../../include/utils/memory.h"
#include "../../include/utils/log.h"


int NET_CONN_init( NET_CONN* connection, const char* host, const int port ) {
    LOG_print( connection->log, "[%s] NET_CONN_init( %s, %d )...", TIME_get_gmt(), host, port );

    connection->initialized = 0;
    connection->connected = 0;

    connection->socket = socket( AF_INET, SOCK_STREAM, 0 );
    if( connection->socket == -1 ) {
        LOG_print( connection->log, "error. Could not create socket.\n" );
        return 0;
    }

    connection->server.sin_addr.s_addr = inet_addr( host );
    connection->server.sin_family = AF_INET;
    connection->server.sin_port = htons( port );

    connection->response = NULL;

    connection->host = SAFECALLOC( 256, sizeof( char ), __FILE__, __LINE__ );
    snprintf( connection->host, 255, host );
    connection->port = port;

    LOG_print( connection->log, "ok.\n", TIME_get_gmt() );

    connection->initialized = 1;

    return 1;
}

int NET_CONN_connect( NET_CONN *connection, const char *host, const int port ) {

    LOG_print( connection->log, "[%s] NET_CONN_connect( %s, %d )...", TIME_get_gmt(), host, port );

    connection->connected = 0;

    if( connection->initialized == 0 ) {
        connection->socket = socket( AF_INET, SOCK_STREAM, 0 );
        if( connection->socket == -1 ) {
            LOG_print( connection->log, "error. Could not create socket.\n" );
        }

        connection->server.sin_addr.s_addr = inet_addr( host );
        connection->server.sin_family = AF_INET;
        connection->server.sin_port = htons( port );

        connection->response = NULL;

        connection->host = SAFECALLOC( 256, sizeof( char ), __FILE__, __LINE__ );
        snprintf( connection->host, 255, host );
        connection->port = port;
    }
    

    LOG_print( connection->log, "ok. Connecting...", TIME_get_gmt() );
    int conn_res = connect( connection->socket, ( struct sockaddr* )&connection->server, sizeof( connection->server ) );
    if ( conn_res < 0 ) {
        LOG_print( connection->log, "error (%d): %s.\n", conn_res, strerror( errno ) );
        free( connection->host ); connection->host = NULL;
        return 0;
    }

    connection->connected = 1;

    LOG_print( connection->log, "ok.\n" );
    return 1;
}

int NET_CONN_disconnect( NET_CONN *connection ) {
    if( connection ) {
        LOG_print( connection->log, "[%s] NET_CONN_disconnect( %s, %d )...", TIME_get_gmt(), connection->host, connection->port );

        free( connection->host ); connection->host = NULL;
        free( connection->response ); connection->response = NULL;

        connection->response_len = 0;

        connection->initialized = 0;
        connection->connected = 0;

        LOG_print( connection->log, "ok.\n" );

        
        return closesocket( connection->socket );
    } else {
        printf( "error. Connection pointer is not valid.\n" );
        return 0;
    }
}

int NET_CONN_send( NET_CONN *connection, const char *data, size_t data_len ) {

    if( connection ) {
        LOG_print( connection->log, "[%s] NET_CONN_send...", TIME_get_gmt() );

        if( connection->initialized == 0 ) {
            LOG_print( connection->log, "error. Connection object is not initialized.\n" );
            return 0;
        }

        if( connection->connected == 0 ) {
            if( NET_CONN_connect( connection, connection->host, connection->port ) == 0 ) {
                LOG_print( connection->log, "[%s] NET_CONN_send error: unable to initialize connection object.\n", TIME_get_gmt() );
                return 0;
            }
        }

        if( send( connection->socket, data, data_len, 0 ) < 0 ) {
            LOG_print( connection->log, "error sending data to %s.\n", TIME_get_gmt(), connection->host );
            return 0;
        } else {
            LOG_print( connection->log, "ok. Awaiting response..." );
        }

        char* buffer = NULL;
        unsigned long LEN = 8196;
        unsigned long cur_size = LEN;
        int status = 0;

        buffer = ( char* )SAFECALLOC( LEN, sizeof( char ), __FILE__, __LINE__ );
        do {
            if( status >= LEN ) {
                cur_size += status;
                char *tmp = realloc( buffer, cur_size );
                if( tmp ) {
                    buffer = tmp;
                } else {
                    break;
                }
            }

            status = recv( connection->socket, buffer + cur_size - LEN, LEN, 0 );
            if( status <= 0 ) {
                break;
            }
        } while( status > 0 );

        if( connection->response ) {
            free( connection->response ); connection->response = NULL;
        }
        connection->response = SAFECALLOC( cur_size, sizeof( char ), __FILE__, __LINE__ );
        connection->response_len = cur_size;

        for( size_t i = 0; i < cur_size; i++ ) {
            connection->response[ i ] = buffer[ i ];
        }

        free( buffer ); buffer = NULL;

        LOG_print( connection->log, "ok.\n" );

        return 1;
    } else {
        printf( "[connection pointer is not valid].\n" );
        return 0;
    }
}
