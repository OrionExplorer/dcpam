#include <stdio.h>
#include <stdlib.h>
#include "../include/core/cache.h"
#include "../include/utils/memory.h"
#include "../include/utils/log.h"
#include "../include/utils/time.h"
#include "../include/core/app_schema.h"

extern P_DCPAM_APP      P_APP;

int DB_CACHE_init( D_CACHE *dst ) {

    if( dst ) {
        dst->query = SAFEMALLOC( sizeof( DB_QUERY ), __FILE__, __LINE__ );
        DB_QUERY_init( dst->query );
        return 1;
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
    }
}

void DB_CACHE_print( const char *sql, D_CACHE *dst ) {
    LOG_print( "[%s] DB_CACHE_print:\n", TIME_get_gmt() );

    if( sql && dst ) {

        LOG_print( "\t- SQL: \"%s\"\n", dst->query->sql );
        LOG_print( "\t- Records: %d, columns: %d\n", dst->query->row_count, dst->query->field_count );
        
        /*LOG_print( "\t- Data:\n========================================================\n" );
        for( int i = 0; i < dst->query->row_count; i++ ) {
            LOG_print( "---[RECORD#%d]--------------------\n", i + 1 );
            for( int j = 0; j < dst->query->field_count; j++ ) {
                LOG_print( "%s: %s\n", dst->query->records[ i ].fields[ j ].label, dst->query->records[ i ].fields[ j ].value );
            }
        }
        LOG_print( "========================================================\n" );*/

    } else {
        LOG_print( "\t- fatal error: not all parameters are valid!\n" );
    }
}
