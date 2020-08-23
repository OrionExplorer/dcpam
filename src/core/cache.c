#include <stdio.h>
#include <stdlib.h>
#include "../include/utils/memory.h"
#include "../include/utils/log.h"
#include "../include/utils/time.h"
#include "../include/core/app_schema.h"
#include "../include/core/cache.h"
#include "../include/core/db/system.h"

extern P_DCPAM_APP      P_APP;

static inline void _DB_CACHE_show_usage( LOG_OBJECT *log ) {
    LOG_print( log, "[%s] Total cached data: %ld/%ld KB.\n", TIME_get_gmt(), P_APP.CACHE_size / 1024, P_APP.CACHE_MAX_size );
}

void _DB_CACHE_add_size( D_CACHE *src ) {
    if( src && src->size > 0 ) {
        P_APP.CACHE_size += src->size;
    }
}

void _DB_CACHE_sub_size( D_CACHE *src ) {
    if( src && src->size > 0 ) {
        P_APP.CACHE_size -= src->size;
    }
}

void _DB_CACHE_calc_size( D_CACHE* src ) {
    if( src ) {
        size_t result = 0;
        for( int i = 0; i < src->query->row_count; i++ ) {
            for( int j = 0; j < src->query->records[ i ].field_count; j++ ) {
                result += src->query->records[ i ].fields[ j ].size;
                result += strlen( src->query->records[ i ].fields[ j ].label );
            }
        }

        src->size = result;
    }
}

int DB_CACHE_init( D_CACHE *dst, DATABASE_SYSTEM_DB *db, const char *sql, LOG_OBJECT *log ) {

    if( dst && db ) {
        LOG_print( log, "[%s] DB_CACHE_init( %s, \"%.30s(...)\" ) started...\n", TIME_get_gmt(), db->name, sql );
        dst->query = SAFEMALLOC( sizeof( DB_QUERY ), __FILE__, __LINE__ );
        DB_QUERY_init( dst->query );

        dst->size = 0;
        dst->db = db;

        int res =  DB_exec(
            db,
            sql,
            strlen( sql ),
            dst->query,
            NULL,
            0,
            NULL, NULL, NULL, NULL, NULL, NULL,
            log
        );

        LOG_print( log, "[%s] DB_CACHE_init( %s, \"%.30s(...)\" ) finished:\n", TIME_get_gmt(), db->name, sql );
        if( res == 1 ) {
            _DB_CACHE_calc_size( dst );
            _DB_CACHE_add_size( dst );
            if( ( P_APP.CACHE_size / 1024 ) > P_APP.CACHE_MAX_size ) {
                LOG_print( log, "\t- Fatal error: memory limit exceeded!\n" );
                LOG_print( log, "\t- Data would be removed from heap after completing this request.\n" );
                _DB_CACHE_show_usage( log );
                return 2;
            }
            LOG_print( log, "\t- Cached records: %d (%ld KB).\n", dst->query->row_count, dst->size / 1024 );
            _DB_CACHE_show_usage( log );
        } else {
            LOG_print( log, "\t- DB_exec failed!\n" );
        }
        return res;
    } else {
        LOG_print( log, "[%s] DB_CACHE_init error: not all parameters are valid!\n", TIME_get_gmt() );
        dst->db = NULL;
        dst->query = NULL;
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

void DB_CACHE_free( D_CACHE* dst, LOG_OBJECT *log ) {
    if( dst ) {
        if( dst->query ) {
            LOG_print( log, "[%s] DB_CACHE_free( %s, %s )...", TIME_get_gmt(), dst->query->sql, dst->db->name );
            DB_QUERY_free( dst->query );
            free( dst->query ); dst->query = NULL;
            LOG_print( log, "ok.\n" );
        }
        _DB_CACHE_sub_size( dst );
        _DB_CACHE_show_usage( log );
        dst->db = NULL;
        dst->size = 0;
    }
}

void DB_CACHE_print( D_CACHE *dst, LOG_OBJECT *log ) {
    LOG_print( log, "[%s] DB_CACHE_print:\n", TIME_get_gmt() );

    if( dst ) {

        LOG_print( log, "\t- SQL: \"%s\"\n", dst->query->sql );
        LOG_print( log, "\t- Cached records: %d, columns: %d\n", dst->query->row_count, dst->query->field_count );

    } else {
        LOG_print( log, "\t- fatal error: not all parameters are valid!\n" );
    }
}
