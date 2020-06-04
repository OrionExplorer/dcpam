#ifndef MEMORY_H
#define MEMORY_H

void* safe_malloc( size_t n, const char* filename, int line );
void* safe_calloc( size_t n, size_t m, const char* filename, int line );

#define SAFEMALLOC( n, f, l )         safe_malloc( n, f, l )
#define SAFECALLOC( n, m, f, l )      safe_calloc( n, m, f, l )

#endif
