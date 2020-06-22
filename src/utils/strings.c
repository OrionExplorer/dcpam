#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int strpos( char *haystack, char *needle ) {
    char *p = strstr( haystack, needle );
    if ( p )
        return ( int )( p - haystack );
    return -1;
}
