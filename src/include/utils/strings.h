#ifndef STRINGS_H
#define STRINGS_H

char* mkrndstr( size_t len );
char* rtrim( char* string, char junk );
int strpos( char *haystack, char *needle );
size_t strlcat( char *dst, const char *src, size_t size );
size_t strlcpy( char *dst, const char *src, size_t size );

#endif
