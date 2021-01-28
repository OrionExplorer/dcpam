#ifndef HTTP_H
#define HTTP_H

#include "../../utils/log.h"
#include "../network/client.h"


typedef struct {
    NET_CONN    *connection;
    char        *path;
} HTTP_CLIENT;

char* HTTP_CLIENT_get_content( HTTP_CLIENT *client, const char *host, const char *path, const int port, const int secure, HTTP_DATA *http_data, size_t *content_len, LOG_OBJECT *log );

#endif
