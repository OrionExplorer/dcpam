/* Copyright (C) 2020-2021 Marcin Kelar

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335 USA */

#include <stdio.h>
#include <stdlib.h>
#include "../include/utils/memory.h"
#include "../include/utils/log.h"
#include "../include/utils/time.h"
#include "../include/core/app_schema.h"
#include "../include/core/cache.h"
#include "../include/core/db/system.h"
#include "../include/DCPAM_WDS/sql_parser.h"

extern P_DCPAM_APP      P_APP;

char* DB_CACHE_get_usage_str( void ) {
    char *dst = SAFECALLOC( 64, sizeof( char ), __FILE__, __LINE__ );
    snprintf( dst, 64, "%1.f/%0.f %s (%zu bytes)",
        ( double )P_APP.CACHE_size / ( double )P_APP.CACHE_size_multiplier,
        ( double )P_APP.CACHE_MAX_size / ( double )P_APP.CACHE_size_multiplier,
        P_APP.CACHE_size_unit == MU_KB ? "KB" : P_APP.CACHE_size_unit == MU_MB ? "MB" : P_APP.CACHE_size_unit == MU_GB ? "GB" : P_APP.CACHE_size_unit == MU_TB ? "TB" : "unknown",
        P_APP.CACHE_size
    );
    return dst;
}

static inline void _DB_CACHE_show_usage( LOG_OBJECT *log ) {
    char *usage = DB_CACHE_get_usage_str();
    LOG_print( log, "[%s] Total cached data: %s.\n", TIME_get_gmt(), usage );
    free( usage ); usage = NULL;
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

D_CACHE* _DB_CACHE_subset_search( const char *sql ) {
    for( int i = 0; i < P_APP.CACHE_len; i++ ) {
        /* Check if requested query is subset of already cached data. */
        if( P_APP.CACHE[ i ] && P_APP.CACHE[ i ]->query && P_APP.CACHE[ i ]->query->sql ) {
            if( strstr( sql, P_APP.CACHE[ i ]->query->sql ) != NULL ) {
                return P_APP.CACHE[ i ];
            }
        }
    }

    return NULL;
}

D_SUB_CACHE* _DB_SUB_CACHE_exists( const char *sql ) {
    for( int i = 0; i < P_APP.SUB_CACHE_len; i++ ) {
        if( P_APP.SUB_CACHE[ i ] ) {
            if( strcmp( sql, P_APP.SUB_CACHE[ i ]->sql ) == 0 ) {
                return P_APP.SUB_CACHE[ i ];
            }
        }
    }

    return NULL;
}

void _DB_CACHE_on_db_record( DB_RECORD* record, void* data_ptr1, void* data_ptr2, LOG_OBJECT *log ) {
    D_CACHE *cache = ( D_CACHE* )data_ptr1;
    D_SUB_CACHE *sub_cache = ( D_SUB_CACHE* )data_ptr2;

    /* 1. Find record index in cached data. */
    for( int i = 0; i < cache->query->row_count; i++ ) {

        int record_match = 0;

        for( int j = 0 ; j < cache->query->field_count; j++ ) {
            int fields_match = 0;
            /* Check if fields match. */
            if( strncmp( cache->query->records[ i ].fields[ j ].label, record->fields[ j ].label, MAX_COLUMN_NAME_LEN ) == 0 ) {
                /* Check if value sizes are equal. */
                if( cache->query->records[ i ].fields[ j ].size == record->fields[ j ].size ) {
                    /* Check if values are equal. */
                    if( memcmp( cache->query->records[ i ].fields[ j ].value, record->fields[ j ].value, record->fields[ j ].size ) == 0 ) {
                        /* This is the same record. We can store it's index in *sub_cache. */
                        record_match = 1;
                    } else {
                        record_match = 0;
                        break;
                    }
                } else {
                    record_match = 0;
                    break;
                }
            } else {
                record_match = 0;
                break;
            }
        }

        if( record_match == 1 ) {
            sub_cache->indices = realloc( sub_cache->indices, ( sub_cache->indices_len + 1 ) * sizeof ( D_SUB_CACHE ) );
            if( sub_cache->indices ) {
                sub_cache->indices[ sub_cache->indices_len ] = i;
            }
            sub_cache->indices_len++;
            DB_QUERY_record_free( record );
            return;
        }
    }

    DB_QUERY_record_free( record );

}

int DB_SUB_CACHE_init( D_CACHE *cache, D_SUB_CACHE *sub_cache, const char *sql, LOG_OBJECT *log ) {
    if( cache && sub_cache ) {
        LOG_print( log, "[%s] DB_SUB_CACHE_init( %s, \"%.30s(...)\" ) started...\n", TIME_get_gmt(), cache->db->name, sql );

        sub_cache->src = cache;
        sub_cache->indices = NULL;
        sub_cache->indices_len = 0;
        sub_cache->generate_time = cache->generate_time;
        size_t sql_len = strlen( sql );
        sub_cache->sql = SAFECALLOC( sql_len + 1, sizeof( char ), __FILE__, __LINE__ );
        strlcpy( sub_cache->sql, sql, sql_len );

        int local_data_collect = 0;
        int action_result = 0;
        /* Check, if we can handle data without need to send a request to the database. */
        SQL_CONDITION sql_condition = SQL_PARSER_supported_conditions( sql );
        if( sql_condition != SQL_COND_UNDEF ) {
            LOG_print( log, "[%s] Query contains internally-supported conditions.\n", TIME_get_gmt() );
            LOG_print( log, "[%s] Attempt to filter data...\n", TIME_get_gmt() );
            
            local_data_collect = SQL_PARSER_collect_data( cache, sub_cache, sql, sql_condition, log );
            LOG_print( log, "[%s] Local data collect finished: %s.\n", TIME_get_gmt(), local_data_collect == 1 ? "success" : "failure" );
        }

        if( local_data_collect == 0 ) {
            /* Query conditions are to complex to handle for us. */
            qec on_db_record = ( qec )&_DB_CACHE_on_db_record;
            action_result =  DB_exec(
                cache->db,
                sql,
                strlen( sql ),
                NULL,
                NULL,
                0,
                NULL, NULL, NULL, 
                &on_db_record,
                ( void* )cache, ( void* )sub_cache,
                log
            );    
        } else {
            action_result = local_data_collect;
        }

        LOG_print( log, "[%s] DB_SUB_CACHE_init( %s, \"%.30s(...)\", %0.f ) finished (timestamp: %ld): %s\n", TIME_get_gmt(), cache->db->name, sql, cache->ttl, cache->generate_time, action_result == 1 ? "success" : "failure" );
        return action_result;

    } else {
        LOG_print( log, "[%s] DB_SUB_CACHE_init( \"%.30s(...)\" ) error: invalid arguments.\n", TIME_get_gmt(), sql );
        return 0;
    }
}

void DB_SUB_CACHE_free( D_SUB_CACHE *dst, LOG_OBJECT *log ) {
    if( dst ) {
        dst->src = NULL;

        free( dst->sql ); dst->sql = NULL;

        free( dst->indices ); dst->indices = NULL;
        dst->indices_len = 0;

        dst->generate_time = 0;
        free( dst->sql ); dst->sql = NULL;

    }
}

int DB_CACHE_init( D_CACHE *dst, DATABASE_SYSTEM_DB *db, const char *sql, double cache_ttl, LOG_OBJECT *log ) {

    if( dst && db ) {
        LOG_print( log, "[%s] DB_CACHE_init( %s, \"%.30s(...)\" ) started...\n", TIME_get_gmt(), db->name, sql );

        /* Check, if requested query is subset of already cached data. */
        D_CACHE *cached_data = _DB_CACHE_subset_search( sql );
        if( cached_data ) {
            LOG_print( log, "[%s] DB_CACHE_init: requested query is part of already cached data. \n", TIME_get_gmt() );
            /* Data already exists. Check if it's not int the SUB_CACHE already. */
            D_SUB_CACHE *sub_cache = _DB_SUB_CACHE_exists( sql );
            if( sub_cache == NULL ) {
                LOG_print( log, "[%s] DB_CACHE_init: creating specific sub-cache object. \n", TIME_get_gmt() );
                /* Data is not in the SUB_CACHE. Create D_SUB_CACHE object to keep D_CACHE record indices. */
                P_APP.SUB_CACHE = realloc( P_APP.SUB_CACHE, (P_APP.SUB_CACHE_len + 1 ) * sizeof * P_APP.SUB_CACHE );
                if( P_APP.SUB_CACHE != NULL ) {
                    P_APP.SUB_CACHE[ P_APP.SUB_CACHE_len ] = SAFEMALLOC( sizeof( D_SUB_CACHE ), __FILE__, __LINE__ );
                    P_APP.SUB_CACHE[ P_APP.SUB_CACHE_len ]->indices_len = 0;
                    if( DB_SUB_CACHE_init( cached_data, P_APP.SUB_CACHE[ P_APP.SUB_CACHE_len ], sql, log ) == 1 ) {
                        LOG_print( log, "[%s] DB_CACHE_init: successfully created specific sub-cache object.\n", TIME_get_gmt() );
                        P_APP.SUB_CACHE_len++;
                        return -1;
                    } else {
                        LOG_print( log, "[%s] DB_CACHE_init: failed to create specific sub-cache object.\n", TIME_get_gmt() );
                        free( P_APP.CACHE[ P_APP.SUB_CACHE_len ] );
                        P_APP.CACHE = realloc( P_APP.SUB_CACHE, (P_APP.SUB_CACHE_len ) * sizeof * P_APP.SUB_CACHE );
                        return 0;
                    }
                }
            } else {
                LOG_print( log, "[%s] DB_CACHE_init: specific sub-cache object already exists.\n", TIME_get_gmt() );
                return -1;
            }
        } else {
            LOG_print( log, "[%s] DB_CACHE_init: requested query is not a part of already cached data. \n", TIME_get_gmt() );
        }

        dst->query = SAFEMALLOC( sizeof( DB_QUERY ), __FILE__, __LINE__ );
        DB_QUERY_init( dst->query );

        dst->size = 0;
        dst->db = db;

        dst->ttl = cache_ttl;
        dst->generate_time = TIME_get_epoch();

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

        LOG_print( log, "[%s] DB_CACHE_init( %s, \"%.30s(...)\", %0.f ) finished (timestamp: %ld).\n", TIME_get_gmt(), db->name, sql, cache_ttl, dst->generate_time );
        if( res == 1 ) {
            _DB_CACHE_calc_size( dst );
            _DB_CACHE_add_size( dst );
            if( P_APP.CACHE_size > P_APP.CACHE_MAX_size ) {
                char *usage = DB_CACHE_get_usage_str();
                LOG_print( log, "\t- Fatal error: memory limit exceeded: %s!\n", usage );
                free( usage ); usage = NULL;
                LOG_print( log, "\t- Data would be removed from heap after completing this request.\n" );
                _DB_CACHE_show_usage( log );
                return 2;
            }
            LOG_print(
                log,
                "\t- Cached records: %d (%0.f %s).\n",
                dst->query->row_count,
                ( double )dst->size / ( double )P_APP.CACHE_size_multiplier,
                P_APP.CACHE_size_unit == MU_KB ? "KB" : P_APP.CACHE_size_unit == MU_MB ? "MB" : P_APP.CACHE_size_unit == MU_GB ? "GB" : P_APP.CACHE_size_unit == MU_TB ? "TB" : "unknown"
            );
            _DB_CACHE_show_usage( log );
        } else {
            LOG_print( log, "\t- DB_exec failed!\n" );
            DB_QUERY_free( dst->query );
            free( dst->query ); dst->query = NULL;
        }
        return res;
    } else {
        LOG_print( log, "[%s] DB_CACHE_init error: not all parameters are valid!\n", TIME_get_gmt() );
        dst->db = NULL;
        dst->query = NULL;
    }

    return 0;
}

void DB_CACHE_get( const char* sql, DB_QUERY** dst, D_SUB_CACHE** s_dst ) {

    *dst = NULL;
    *s_dst = NULL;

    /* Find requested query result in generated data (main cache). */
    for( int i = 0; i < P_APP.CACHE_len; i++ ) {
        if( P_APP.CACHE[ i ]  && P_APP.CACHE[ i ]->query && P_APP.CACHE[ i ]->query->sql ) {
            if( strcmp( sql, P_APP.CACHE[ i ]->query->sql ) == 0 ) {
                *dst = P_APP.CACHE[ i ]->query;
                break;
            }
        }
    }

    /* Find requested query result in sub-cache. */
    for( int i = 0; i < P_APP.SUB_CACHE_len; i++ ) {
        if( P_APP.SUB_CACHE[ i ] && P_APP.SUB_CACHE[ i ]->sql ) {
            if( strcmp( sql, P_APP.SUB_CACHE[ i ]->sql ) == 0 ) {
                *s_dst = P_APP.SUB_CACHE[ i ];
                break;
            }
        }
    }
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
        dst->ttl = 0;
        dst->generate_time = 0;
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
