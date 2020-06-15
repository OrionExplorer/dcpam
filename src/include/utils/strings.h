#ifndef STRINGS_H
#define STRINGS_H

size_t dcpam_strnlen(const char *str, size_t max_len);
void strpl( char *src );
void strrnl( const char *src, char *dst );
void strhtmlnl( const char *src, char *dst );
int  strpos( char *haystack, char *needle );

#endif
