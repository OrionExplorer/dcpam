#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char* rtrim( char* string, char junk ) {
    char	*original = string + strlen( string );

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
