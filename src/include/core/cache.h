#ifndef CACHE_H
#define CACHE_H

#include "../db/db.h"

typedef struct {
    DB_QUERY        *query;
} D_CACHE;

int DB_CACHE_init( D_CACHE *dst );
void DB_CACHE_free( D_CACHE* dst );
void DB_CACHE_get( const char* sql, DB_QUERY** dst );
void DB_CACHE_print( const char *sql, D_CACHE *dst );

#endif
