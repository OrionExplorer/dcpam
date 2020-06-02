#ifndef PORTABLE_H
#define PORTABLE_H

#ifdef _WIN32
    #include <direct.h>
    #define SLASH           "\\"
    #define C_SLASH         '\\'
    #define READ_BINARY     "rb"
    #define putenv          _putenv
    #define chdir           _chdir
    #define mkdir( a,b )    mkdir( a )
    #define S_IREAD         _S_IREAD
    #define SHUT_RDWR       SD_BOTH
    #define F_SETFL         FIONBIO
    #define S_IXOTH         S_IEXEC
    #define sighandler      __p_sig_fn_t
    #define MSG_NOSIGNAL    0
    #define sleep           Sleep
#else
    #define SLASH           "/"
    #define C_SLASH         '/'
    #define SOCKET_ERROR    ( -1 )
    #define EXIT_FAILURE    ( 1 )
    #define READ_BINARY     "re"
    #define sighandler      __sighandler_t
    #define Sleep(x)        usleep(x*1000)
#endif


#ifdef _MSC_VER
#pragma comment( lib, "pthreadVC2.lib" )
#pragma comment( lib, "libmysql.lib" )
#pragma comment( lib, "libpq.lib" )
#endif // _MSC_VER

#endif
