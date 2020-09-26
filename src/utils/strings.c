#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* https://codereview.stackexchange.com/a/29276 */
char *mkrndstr( size_t len ) {
    static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    char *randomString = NULL;

    if ( len ) {
        randomString = malloc( len + 1 );

        if ( randomString ) {
            int l = ( int )( sizeof( charset ) - 1 );
            for ( int n = 0; n < len; n++ ) {
                int key = rand() % l;
                randomString[ n ] = charset[ key ];
            }

            randomString[ len ] = '\0';
        }
    }

    return randomString;
}

char* rtrim( char* string, char junk ) {
    char    *original = string + strlen( string );

    while( *--original == junk );
        *( original + 1 ) = '\0';
    return string;
}

int strpos( char *haystack, char *needle ) {
    char *p = strstr( haystack, needle );
    if ( p )
        return ( int )( p - haystack );
    return -1;
}

size_t strlcat( char* dst, const char* src, size_t size ) {
    char        *dst_ptr = dst;
    size_t      dst_len, to_copy = size;
    const char  *src_ptr = src;

    while( to_copy-- && *dst_ptr ) dst_ptr++;
    dst_len = dst_ptr - dst;

    if( !( to_copy = size - dst_len ) ) return dst_len + strlen( src );

    while( *src_ptr ) {
        if( to_copy != 1 ) {
            *dst_ptr++ = *src_ptr;
            to_copy--;
        }
        src_ptr++;
    }
    *dst_ptr = 0;

    return ( dst_len + ( src_ptr - src ) );
}

size_t strlcpy( char* dst, const char* src, size_t size ) {
    char        *dst_ptr = dst;
    size_t      to_copy = size;
    const char  *src_ptr = src;

    if( to_copy && --to_copy ) {
        do {
            if( !( *dst_ptr++ = *src_ptr++ ) ) break;
        } while( --to_copy);
    }

    if( !to_copy ) {
        if( size ) *dst_ptr = 0;
        while( *src_ptr++ );
    }

    return ( src_ptr - src - 1 );
}
