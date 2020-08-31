#ifndef HTTP_H
#define HTTP_H

#include "../../utils/log.h"

#define FD_SETSIZE  1024

#ifdef _WIN32
    #ifndef _WIN32_WINNT
        #define _WIN32_WINNT 0x0501
    #endif
#include <WinSock2.h>
#include <WS2tcpip.h>
#endif

#ifdef _MSC_VER
#pragma comment( lib, "WS2_32.lib" )
#endif

#ifndef _MSC_VER
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#ifndef _WIN32
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/select.h>
#endif


typedef struct {
    int     sock;
    int     bytes_received; 
    char    send_data[ 1024 ];
    char    *recv_data;
    struct  sockaddr_in server_addr;
    struct  hostent *he;
} HTTP_CLIENT;

char* HTTP_CLIENT_get_file( HTTP_CLIENT *client, const char *host, const char *path, const int port, size_t *content_len, LOG_OBJECT *log );

#endif
