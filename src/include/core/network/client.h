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

#ifndef NETWORK_CLIENT
#define NETWORK_CLIENT

#include <stdio.h>
#include <stdlib.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
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

typedef struct HTTP_HEADER {
    char                        *name;
    char                        *value;
} HTTP_HEADER;

typedef struct {
    int                     active;
    HTTP_HEADER             *headers;
    int                     headers_len;
    char                    *payload;
    int                     payload_len;
    char                    *method;
} HTTP_DATA;


typedef struct NET_CONN {
    char                *host;
    int                 port;
    SOCKET              socket;
    struct sockaddr_in  server;
    char                *response;
    size_t              response_len;
    LOG_OBJECT          *log;
    int                 initialized;
    int                 connected;
    SSL_CTX             *sslctx;
    SSL                 *cSSL;
} NET_CONN;

int NET_CONN_init( NET_CONN *connection, const char *host, const int port, const int secure );
int NET_CONN_connect( NET_CONN *connection, const char *host, const int port, const int secure );
int NET_CONN_disconnect( NET_CONN *connection );
int NET_CONN_send( NET_CONN *connection, const char *data, size_t data_len );

#endif
