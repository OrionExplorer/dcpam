
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

#include "../../include/core/network/client.h"
#include "../../include/utils/time.h"
#include "../../include/utils/memory.h"
#include "../../include/utils/log.h"
#include "../../include/portable.h"
#include <string.h>


int NET_CONN_init( NET_CONN* connection, const char* host, const int port, const int secure ) {
    LOG_print( connection->log, "[%s] NET_CONN_init( %s, %d, %s )...", TIME_get_gmt(), host, port, secure == 1 ? "SSL" : "PLAIN" );

    connection->initialized = 0;
    connection->connected = 0;
    connection->cSSL = NULL;
    connection->sslctx = NULL;

    if( secure ) {
        connection->sslctx = SSL_CTX_new( SSLv23_client_method() );
        if( connection->sslctx == NULL ) {
            ERR_print_errors_fp( stderr );
            return 0;
        }
    }

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

int NET_CONN_connect( NET_CONN *connection, const char *host, const int port, const int secure ) {

    LOG_print( connection->log, "[%s] NET_CONN_connect( %s, %d )...", TIME_get_gmt(), host, port );

    connection->connected = 0;

    if( connection->initialized == 0 ) {
        if( secure ) {
            connection->sslctx = SSL_CTX_new( SSLv23_client_method() );
            if( connection->sslctx == NULL ) {
                ERR_print_errors_fp( stderr );
                return 0;
            }
        }
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

    LOG_print( connection->log, "ok. TCP connection...", TIME_get_gmt() );
    int conn_res = connect( connection->socket, ( struct sockaddr* )&connection->server, sizeof( connection->server ) );
    if( conn_res < 0 ) {
        LOG_print( connection->log, "error (%d): %s.\n", conn_res, strerror( errno ) );
        free( connection->host ); connection->host = NULL;
        return 0;
    }

    LOG_print( connection->log, "ok." );

    if( connection->sslctx ) {
        connection->cSSL = SSL_new( connection->sslctx );
        SSL_set_fd( connection->cSSL, connection->socket );
        LOG_print( connection->log, " SSL connection...", TIME_get_gmt() );
        int conn_res = SSL_connect( connection->cSSL );
        if( conn_res < 0 ) {
            LOG_print( connection->log, "error (%d): %s.\n", conn_res, strerror( errno ) );
            free( connection->host ); connection->host = NULL;
            return 0;
        }
    } else {
        LOG_print( connection->log, "\n" );
    }

    connection->connected = 1;

    return 1;
}

int NET_CONN_disconnect( NET_CONN *connection ) {
    if( connection ) {
        int ret_val = 0;
        LOG_print( connection->log, "[%s] NET_CONN_disconnect( %s, %d )...", TIME_get_gmt(), connection->host, connection->port );

        free( connection->host ); connection->host = NULL;
        free( connection->response ); connection->response = NULL;

        connection->response_len = 0;

        connection->initialized = 0;
        connection->connected = 0;

        if( connection->cSSL ) {
            SSL_shutdown( connection->cSSL );
            SSL_free( connection->cSSL );
            SSL_CTX_free( connection->sslctx );
            connection->cSSL = NULL;
            connection->sslctx = NULL;
        }

        SOCKET_unregister_client( connection->socket, connection->log );
        SOCKET_close( connection->socket );
        ret_val = 1;
        //ret_val = shutdown( connection->socket, SHUT_RDWR ) == 0 && closesocket( connection->socket ) == 0;
        //ret_val = closesocket( connection->socket );

        LOG_print( connection->log, "ok.\n" );
        
        return ret_val;
    } else {
        printf( "error. Connection pointer is not valid.\n" );
        return 0;
    }
}

int NET_CONN_send( NET_CONN *connection, const char *data, size_t data_len ) {
    if( connection && connection->log ) {

        LOG_print( connection->log, "[%s] NET_CONN_send...", TIME_get_gmt() );

        if( connection->initialized == 0 ) {
            LOG_print( connection->log, "error. Connection object is not initialized.\n" );
            return 0;
        }

        if( connection->connected == 0 ) {
            LOG_print( connection->log, "[%s] NET_CONN_send error: unable to initialize connection object.\n", TIME_get_gmt() );
            return 0;
        }

        if( connection->cSSL ) {
            if( SSL_write( connection->cSSL, data, data_len ) < 0 ) {
                LOG_print( connection->log, "error sending data to %s.\n", connection->host );
                return 0;
            } else {
                LOG_print( connection->log, "ok. Awaiting response..." );
            }
        } else {
            if( send( connection->socket, data, data_len, 0 ) < 0 ) {
                LOG_print( connection->log, "error sending data to %s.\n", connection->host );
                return 0;
            } else {
                LOG_print( connection->log, "ok. Awaiting response..." );
            }
        }


        char *buffer = NULL;
        unsigned long LEN = 8192;
        unsigned long bytes_received = 0;
        unsigned long cur_size = 0;
        int status = 1;

        while (status > 0)
        {
            if (bytes_received >= cur_size)
            {
                char * tmp;
                cur_size += LEN;
                tmp = realloc(buffer, cur_size);
                if (NULL == tmp)
                {
                  fprintf(stderr, "realloc error=%d\n", WSAGetLastError());
                  break;
                }

                buffer = tmp;
            }

            if( connection->cSSL ) {
                status = SSL_read( connection->cSSL, buffer + cur_size - LEN, LEN );
            } else {
                status = recv( connection->socket, buffer + cur_size - LEN, LEN, 0 );
            }

            if (status > 0) {
              bytes_received += status;
            }
        }

        if( connection->response ) {
            free( connection->response ); connection->response = NULL;
        }

        connection->response = SAFECALLOC( bytes_received + 1, sizeof( char ), __FILE__, __LINE__ );
        connection->response_len = bytes_received;

        for( size_t i = 0; i < bytes_received; i++ ) {
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
