#include "../../include/core/network/http.h"
#include "../../include/utils/time.h"
#include "../../include/utils/memory.h"

char* HTTP_CLIENT_get_file( HTTP_CLIENT *client, const char *host, const char *path, const int port, size_t *content_len, LOG_OBJECT *log ) {
    char *content = NULL;

    if( client ) {
        client->connection = SAFEMALLOC( sizeof( NET_CONN ), __FILE__, __LINE__ );
        client->connection->log = log;
        if( NET_CONN_connect( client->connection, host, port ) == 0 ) {
            free( client->connection ); client->connection = NULL;
            return NULL;
        }

        char send_data[ 1024 ];

        snprintf( send_data, sizeof( send_data ), "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n", path, host );

        if( NET_CONN_send( client->connection, send_data, strlen( send_data ) ) == 0 ) {
            free( client->connection ); client->connection = NULL;
            return NULL;
        }

        int http_res = -1;
        if( sscanf( client->connection->response, "HTTP/1.1 %999d", &http_res ) == 1 ) {

            LOG_print( log, "[%s] HTTP_CLIENT_get_file( %s ) response code: %d.\n", TIME_get_gmt(), path, http_res );

            if( http_res != 200 ) {
                LOG_print( log, "[%s] Error: response is not valid.\n", TIME_get_gmt() );
            } else {
                printf( "\n====RECV:\n%s\n========\n", client->connection->response );
                content = SAFEMALLOC( client->connection->response_len * sizeof( char ), __FILE__, __LINE__ );
                memcpy(
                    content,
                    client->connection->response,
                    client->connection->response_len
                );

                if( content_len ) {
                    *content_len = client->connection->response_len;
                }

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
