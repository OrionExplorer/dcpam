#ifndef SOCKET_IO_H
#define SOCKET_IO_H

#include "../../shared.h"

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
    struct sockaddr_in          address;
#ifdef _WIN32
    SOCKET                      socket;
#else
    int                         socket;
#endif
    fd_set                      socket_data;
#ifdef _WIN32
    int                         data_length;
#else
    socklen_t                   data_length;
#endif
    int                         socket_descriptor;
    char                        content[ MAX_BUFFER ];
    short                       keep_alive;
} COMMUNICATION_SESSION;

#ifdef _WIN32
    extern WSADATA              wsk;
    extern SOCKET               socket_server;
#else
    extern int                  socket_server;
#endif
extern int                      addr_size;
extern int                      active_port;
extern struct sockaddr_in       server_address;
extern int                      ip_proto_ver;
extern COMMUNICATION_SESSION    communication_session_;
extern fd_set                   master;

typedef struct {
    int                         socket_descriptor;
    short                       connected;
} CONNECTED_CLIENT;

CONNECTED_CLIENT                connected_clients[ MAX_CLIENTS ];


typedef void ( *spc )( COMMUNICATION_SESSION*, CONNECTED_CLIENT* ); /* Socket Process Callback */

void                SOCKET_main( spc *socket_process_callback, const int port );
void                SOCKET_run( spc *socket_process_callback );
void                SOCKET_stop( void );

void                SOCKET_send( COMMUNICATION_SESSION *communication_session, CONNECTED_CLIENT *client, const char *data, unsigned int data_size );

void                SOCKET_disconnect_client( COMMUNICATION_SESSION *communication_session );
void                SOCKET_release( COMMUNICATION_SESSION *communication_session );
char*               SOCKET_get_remote_ip( COMMUNICATION_SESSION *communication_session );
void                SOCKET_close( int socket_descriptor );
void                SOCKET_modify_clients_count( int mod );
void                SOCKET_register_client( int socket_descriptor );
void                SOCKET_unregister_client( int socket_descriptor );
CONNECTED_CLIENT*   SOCKET_find_client( int socket_descriptor );

#endif
