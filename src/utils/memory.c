#include <stdio.h>
#include <stdlib.h>
#include "../include/utils/memory.h"


void* safe_malloc( size_t n, const char* filename, int line ) {
    void* p = malloc( n );

    if( !p ) {
        fprintf( stderr, "[%s:%d] Out of memory (%zu bytes)\n", filename, line, ( unsigned long )n );
        exit( EXIT_FAILURE );
    }
    return p;
}

void* safe_calloc( size_t n, size_t m, const char* filename, int line ) {
    void* p = calloc( n, m );

    if( !p ) {
        fprintf( stderr, "[%s:%d] Out of memory (%zu bytes)\n", filename, line, n );
        exit( EXIT_FAILURE );
    }
    return p;
}
