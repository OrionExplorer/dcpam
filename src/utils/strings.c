#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void strpl( char *src ) {
    int         i = 0;
    int         src_len = ( int )strlen( src );
    char        *dst = ( char* )calloc( src_len +1, sizeof( char ) );
    if( src_len > 0 ) {
        for( i = 0; i < src_len; i++ ) {
            
            if( src[i] == -71/*-91*/ ) {
                dst[i] = 'a';//484;//'a';
                continue;
            } else {
                dst[i] = src[i];
            }

            if( src[i] == -100/*-104*/ ) {
                dst[i] = 's';//s
                continue;
            } else {
                dst[i] = src[i];
            }

            if( src[i] == -26/*-122*/ ) {
                dst[i] = 'c';
                continue;
            } else {
                dst[i] = src[i];
            }

            if( src[i] == -22/*-87*/ ) {
                dst[i] = 'e';
                continue;
            } else {
                dst[i] = src[i];
            }

            if( src[i] == -77/*-120*/ ) {
                dst[i] = 'l';
                continue;
            } else {
                dst[i] = src[i];
            }

            if( src[i] == -15/*-28*/ ) {
                dst[i] = 'n';
                continue;
            } else {
                dst[i] = src[i];
            }

            if( src[i] == -13/*-94*/ ) {
                dst[i] = 'o';
                continue;
            } else {
                dst[i] = src[i];
            }

            if( src[i] == -97/*-85*/ ) {
                dst[i] = 'z';
                continue;
            } else {
                dst[i] = src[i];
            }

            if( src[i] == -65/*-66*/ ) {
                dst[i] = 'z';
                continue;
            } else {
                dst[i] = src[i];
            }

            if( src[i] == -91/*-92*/ ) {
                dst[i] = 'A';
                continue;
            } else {
                dst[i] = src[i];
            }

            if( src[i] == -58/*-113*/ ) {
                dst[i] = 'C';
                continue;
            } else {
                dst[i] = src[i];
            }

            if( src[i] == -54/*-88*/ ) {
                dst[i] = 'E';
                continue;
            } else {
                dst[i] = src[i];
            }

            if( src[i] == -93/*-99*/ ) {
                dst[i] = 'L';
                continue;
            } else {
                dst[i] = src[i];
            }

            if( src[i] == -47/*-29*/ ) {
                dst[i] = 'N';
                continue;
            } else {
                dst[i] = src[i];
            }

            if( src[i] == -45/*-32*/ ) {
                dst[i] = 'O';
                continue;
            } else {
                dst[i] = src[i];
            }

            if( src[i] == -116/*-105*/ ) {
                dst[i] = 'S';
                continue;
            } else {
                dst[i] = src[i];
            }

            if( src[i] == -113/*-115*/ ) {
                dst[i] = 'Z';
                continue;
            } else {
                dst[i] = src[i];
            }

            if( src[i] == -81 ) {
                dst[i] = 'Z';
                continue;
            } else {
                dst[i] = src[i];
            }

        }
    }

    strncpy( src, dst, src_len );
    free( dst );
}


void strrnl( const char *src, char *dst ) {
    int         i = 0;
    int         src_len = ( int )strlen( src );

    if( src_len > 0 ) {
        for( i = 0; i < src_len; i++ ) {
            dst[i] = (src[i] != '\'' ? src[i] : '`');
        }

        dst[i] = '\0';
    }
}


void strhtmlnl( const char *src, char *dst ) {
    int         i = 0;
    int         src_len = ( int )strlen( src );
    char        *pch;
    char*       tmp_buf;

    if( src_len > 0 ) {
        tmp_buf = ( char* )calloc( 65535, sizeof( char ) );
        if( tmp_buf ) {
            for( i = 0; i < src_len; i++ ) {
                tmp_buf[i] = src[i] != '\n' ? src[i] : '|';
            }
            pch = strtok (tmp_buf,"|");
            while (pch != NULL) {
                strcat( dst, pch );
                strcat( dst, "<br/>" );
                pch = strtok (NULL, "|");
            }
            free( tmp_buf );
            tmp_buf = NULL;
        }
    }
}

int strpos( char *haystack, char *needle ) {
    char *p = strstr( haystack, needle );
    if ( p )
        return ( int )( p - haystack );
    return -1;
}
