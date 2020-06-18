#include <stdio.h>
#include <stdlib.h>
#include <string.h>


size_t dcpam_strnlen(const char *str, size_t max_len) {
    const       char *end = ( const char  *)memchr( str, '\0', max_len );

    if (end == NULL)
        return max_len;
    else
        return end - str;
}


int strpos( char *haystack, char *needle ) {
    char *p = strstr( haystack, needle );
    if ( p )
        return ( int )( p - haystack );
    return -1;
}
