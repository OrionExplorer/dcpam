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

char* HTTP_CLIENT_get_content( HTTP_CLIENT *client, const char *host, const char *path, const int port, const int secure, size_t *content_len, LOG_OBJECT *log ) {
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
        }

        char send_data[ 1024 ];

        snprintf( send_data, sizeof( send_data ), "GET /%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, host );

        if( NET_CONN_send( client->connection, send_data, strlen( send_data ) ) == 0 ) {
            free( client->connection ); client->connection = NULL;
            return NULL;
        }

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
