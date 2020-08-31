#include "../../include/core/network/http.h"

char* HTTP_CLIENT_get_file( HTTP_CLIENT *client, const char *host, const char *path, const int port, size_t *content_len, LOG_OBJECT *log ) {
	char *content = NULL;

	if( content_len ) {
		*content_len = 0;
	}

	return content;
}
