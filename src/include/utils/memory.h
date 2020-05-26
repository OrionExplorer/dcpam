#ifndef MEMORY_H
#define MEMORY_H

void* safe_malloc( size_t n, unsigned long line );
void* safe_calloc( size_t n, size_t m, unsigned long line );

#define SAFEMALLOC( n )         safe_malloc( n, __LINE__ )
#define SAFECALLOC( n, m )      safe_calloc( n, m, __LINE__ )

#endif
