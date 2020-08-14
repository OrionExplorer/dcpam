#ifndef CACHE_H
#define CACHE_H

#include "../db/db.h"
#include "../core/db/system_schema.h"

typedef struct {
    DB_QUERY                *query;
    DATABASE_SYSTEM_DB      *db;
    char                    *table;
} D_CACHE;

int DB_CACHE_init( D_CACHE *dst, DATABASE_SYSTEM_DB *db, const char *sql, const char *table );
void DB_CACHE_free( D_CACHE* dst );
void DB_CACHE_get( const char* sql, DB_QUERY** dst );
void DB_CACHE_print( D_CACHE *dst );

#endif
