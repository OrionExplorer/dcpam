#ifndef NETWORK_CLIENT
#define NETWORK_CLIENT

#include <stdio.h>
#include <stdlib.h>
#include "../../utils/log.h"

#ifdef _WIN32

#ifndef _WIN32_WINNT
#define _WIN32_WINNT                0x0501
#endif

#include <WinSock2.h>
#include <WS2tcpip.h>
#endif


#ifndef _MSC_VER
    #include <sys/types.h>
    #include <fcntl.h>
    #include <unistd.h>
#endif

#ifdef __linux__
    #include <sys/socket.h>
    #include <netdb.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>
    #include <arpa/inet.h>
    #include <sys/ioctl.h>
    #include <errno.h>

    #define closesocket close
    #define SOCKET int
#endif


typedef struct NET_CONN {
    char                *host;
    int                 port;
    SOCKET              socket;
    struct sockaddr_in  server;
    char                *response;
    LOG_OBJECT          *log;
} NET_CONN;

int NET_CONN_connect( NET_CONN *connection, const char *host, const int port );
int NET_CONN_disconnect( NET_CONN *connection );
int NET_CONN_send( NET_CONN *connection, const char *data, size_t data_len );

#endif
