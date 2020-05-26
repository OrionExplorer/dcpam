#include <stdio.h>
#include <stdlib.h>
#include "../include/utils/memory.h"


void* safe_malloc( size_t n, unsigned long line ) {
    void* p = malloc( n );

    if( !p ) {
        fprintf( stderr, "[%s:%ul]Out of memory(%ul bytes)\n", __FILE__, line, ( unsigned long )n );
        exit( EXIT_FAILURE );
    }
    return p;
}

void* safe_calloc( size_t n, size_t m, unsigned long line ) {
    void* p = calloc( n, m );

    if( !p ) {
        fprintf( stderr, "[%s:%ul]Out of memory(%ul bytes)\n", __FILE__, line, ( unsigned long )n );
        exit( EXIT_FAILURE );
    }
    return p;
}
