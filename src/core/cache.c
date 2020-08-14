#include <stdio.h>
#include <stdlib.h>
#include "../include/utils/memory.h"
#include "../include/utils/log.h"
#include "../include/utils/time.h"
#include "../include/core/app_schema.h"
#include "../include/core/cache.h"
#include "../include/core/db/system.h"

extern P_DCPAM_APP      P_APP;

int DB_CACHE_init( D_CACHE *dst, DATABASE_SYSTEM_DB *db, const char *sql, const char *table ) {

    LOG_print( "[%s] DB_CACHE_init( %s, \"%.30s(...)\", %s ) started...\n", TIME_get_gmt(), db->name, sql, table );
    if( dst && db && table) {
        dst->query = SAFEMALLOC( sizeof( DB_QUERY ), __FILE__, __LINE__ );
        DB_QUERY_init( dst->query );

        dst->db = db;

        size_t table_len = strlen( table );
        dst->table = SAFECALLOC( table_len + 1, sizeof( char ), __FILE__, __LINE__ );
        snprintf( dst->table, table_len + 1, table );

        int res =  DB_exec(
            db,
            sql,
            strlen( sql ),
            dst->query,
            NULL,
            0,
            NULL, NULL, NULL, NULL, NULL, NULL
        );

        LOG_print( "[%s] DB_CACHE_init( %s, \"%.30s(...)\", %s ) finished %s:\n", TIME_get_gmt(), db->name, sql, table, res == 1 ? "successfully" : "with error" );
        if( res == 1 ) {
            LOG_print( "\t- Cached records: %d.\n", dst->query->row_count );
        } else {
            LOG_print( "\t- DB_exec failed!" );
        }
        return res;
    } else {
        dst->db = NULL;
        dst->query = NULL;
        dst->table = NULL;
    }

    return 0;
}

void DB_CACHE_get( const char* sql, DB_QUERY** dst ) {
    for( int i = 0; i < P_APP.CACHE_len; i++ ) {
        if( P_APP.CACHE[ i ]  && P_APP.CACHE[ i ]->query && P_APP.CACHE[ i ]->query->sql ) {
            if( strcmp( sql, P_APP.CACHE[ i ]->query->sql ) == 0 ) {
                *dst = P_APP.CACHE[ i ]->query;
                return;
            }
        }
    }
    *dst = NULL;
}

void DB_CACHE_free( D_CACHE* dst ) {
    if( dst ) {
        if( dst->query ) {
            DB_QUERY_free( dst->query );
            free( dst->query ); dst->query = NULL;
        }

        dst->db = NULL;
        free( dst->table ); dst->table = NULL;
    }
}

void DB_CACHE_print( D_CACHE *dst ) {
    LOG_print( "[%s] DB_CACHE_print:\n", TIME_get_gmt() );

    if( dst ) {

        LOG_print( "\t- SQL: \"%s\"\n", dst->query->sql );
        LOG_print( "\t- Cached records: %d, columns: %d\n", dst->query->row_count, dst->query->field_count );

    } else {
        LOG_print( "\t- fatal error: not all parameters are valid!\n" );
    }
}
