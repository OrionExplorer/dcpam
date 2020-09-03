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
