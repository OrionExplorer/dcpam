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

#include "../../include/core/network/http.h"
#include "../../include/utils/time.h"
#include "../../include/utils/memory.h"

size_t _HTTP_CLIENT_get_ContentLength( HTTP_CLIENT* client ) {
    size_t result = 0;

    if( client && client->connection && client->connection->response && client->connection->response_len > 0 ) {
        char* ptr = strstr( client->connection->response, "Content-Length:" );
        if( ptr ) {
            if( sscanf( ptr, "%*s %zu", &result ) == 1 ) {
                return result;
            }
        }
    }
    return 0;
}

char* HTTP_CLIENT_get_content( HTTP_CLIENT *client, const char *host, const char *path, const int port, const int secure, HTTP_DATA* http_data, size_t *content_len, LOG_OBJECT *log ) {
    char *raw_content = NULL;
    char *content = NULL;

    if( client ) {
        client->connection = SAFEMALLOC( sizeof( NET_CONN ), __FILE__, __LINE__ );
        client->connection->log = log;
        if( NET_CONN_init( client->connection, host, port, secure ) == 1 ) {
            if( NET_CONN_connect( client->connection, host, port, secure ) == 0 ) {
                free( client->connection ); client->connection = NULL;
                return NULL;
            }
        } else {
            free( client->connection ); client->connection = NULL;
            return NULL;
        }

        /* Initial data length */
        size_t send_data_len = 1024;

        if( http_data ) {
            /* Additional payload length */
            send_data_len += http_data->payload_len;

            /* Additional headers length */
            if( http_data->headers_len ) {
                for( int i = 0; i < http_data->headers_len; i++ ) {
                    char* name = http_data->headers[ i ].name;
                    char* value = http_data->headers[ i ].value;
                    size_t data_len = strlen( name ) + strlen( value ) + /* ": " */ 2 + /* \r\n */ 2;
                    send_data_len += data_len;
                }
            }
        }

        /* Initial data */
        char* send_data = SAFECALLOC( send_data_len + 1, sizeof( char ), __FILE__, __LINE__ );
        snprintf( send_data, send_data_len + 1, "%s /%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n", http_data->method, path, host );
        
        /* Additional headers from HTTP_DATA */
        if( http_data && http_data->headers_len ) {
            for( int i = 0; i < http_data->headers_len; i++ ) {
                char* name = http_data->headers[ i ].name;
                char* value = http_data->headers[ i ].value;
                size_t data_len = strlen( name ) + strlen( value ) + /* ": " */ 2 + /* \r\n */ 2;
                char* header_data = SAFECALLOC( data_len + 1, sizeof( char ), __FILE__, __LINE__ );
                snprintf( header_data, data_len + 1, "%s: %s\r\n", name, value );
                strncat( send_data, header_data, send_data_len );
                free( header_data ); header_data = NULL;
            }
        }

        /* Close HTTP headers data */
        strncat( send_data, "\r\n", send_data_len );

        /* Additional payload */
        if( http_data && http_data->payload_len ) {
            strncat( send_data, http_data->payload, send_data_len );
        }

        if( NET_CONN_send( client->connection, send_data, strlen( send_data ) ) == 0 ) {
            free( client->connection ); client->connection = NULL;
            free( send_data );
            return NULL;
        }

        free( send_data );

        int http_res = -1;
        if( sscanf( client->connection->response, "HTTP/1.1 %999d", &http_res ) == 1 ) {

            LOG_print( log, "[%s] HTTP_CLIENT_get_content( %s ) response code: %d.\n", TIME_get_gmt(), path, http_res );

            if( http_res != 200 ) {
                LOG_print( log, "[%s] Error: response is not valid.\n", TIME_get_gmt() );
            } else {
                raw_content = SAFEMALLOC( client->connection->response_len * sizeof( char ), __FILE__, __LINE__ );
                memcpy(
                    raw_content,
                    client->connection->response,
                    client->connection->response_len
                );

                size_t file_size = _HTTP_CLIENT_get_ContentLength( client );

                LOG_print( log, "[%s] Received file with size %zu bytes.\n", TIME_get_gmt(), file_size );

                content = SAFEMALLOC( file_size * sizeof( char ), __FILE__, __LINE__ );
                char *start_pos = strstr( raw_content, "\r\n\r\n" ) + 4;
                for( size_t i = 0; i < file_size; i++ ) {
                    content[ i ] = start_pos[ i ];
                }

                if( content_len ) {
                    *content_len = file_size;
                }

                free( raw_content ); raw_content = NULL;

                NET_CONN_disconnect( client->connection );

                free( client->connection ); client->connection = NULL;

                return content;
            }
        } else {
            LOG_print( log, "[%s] Error: unable to read HTTP response.\n", TIME_get_gmt() );
        }
    }

    if( content_len ) {
        *content_len = 0;
    }

    return NULL;
}
