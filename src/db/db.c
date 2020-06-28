#include "../include/utils/time.h"
#include "../include/utils/log.h"
#include "../include/utils/memory.h"
#include "../include/db/db.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


void DB_QUERY_init( DB_QUERY *db_query ) {
    if( db_query != NULL ) {
        db_query->sql = NULL;
        db_query->records = NULL;
        db_query->row_count = 0;
        db_query->field_count = 0;
    }
}

void DB_QUERY_record_free( DB_RECORD* record ) {

    if( record ) {
        for( int j = 0; j < record->field_count; j++ ) {
            if( record->fields[ j ].value != NULL ) {
                free( record->fields[ j ].value ); record->fields[ j ].value = NULL;
            }
        }

        if( record->fields != NULL ) {
            free( record->fields ); record->fields = NULL;
        }

        record->field_count = 0;

        free( record ); record = NULL;
    }
}

void DB_QUERY_free( DB_QUERY* db_query ) {

    /*LOG_print( "[%s]]\tDB_QUERY_free( <'%s'> ).\n", TIME_get_gmt(), db_query->sql );*/
    if( db_query != NULL ) {

        if( db_query->sql != NULL ) {
            free( db_query->sql ); db_query->sql = NULL;
        }

        if( db_query->row_count > 0 && db_query->field_count > 0 ) {
            for( int i = 0; i < db_query->row_count; i++ ) {

                //DB_QUERY_record_free( &db_query->records[ i ] );
                db_query->records[ i ].field_count = 0;

                for( int j = 0; j < db_query->field_count; j++ ) {
                    if( db_query->records[i].fields[ j ].value != NULL ) {
                        free( db_query->records[ i ].fields[ j ].value ); db_query->records[ i ].fields[ j ].value = NULL;
                    }
                }

                if( db_query->records[i].fields != NULL ) {
                    free( db_query->records[i].fields ); db_query->records[i].fields = NULL;
                }

            }

            if( db_query->records != NULL ) {
                free( db_query->records ); db_query->records = NULL;
            }
        }

        db_query->row_count = 0;
        db_query->field_count = 0;
    }
}

int _DB_QUERY_replace_params( char *src,  const char *search, const char* const *replaces, const int* param_lengths, const int replace_count, size_t *dst_len ) {
    /* Allocate buffer for source query copy */
    char *tmp_src = SAFEMALLOC( ( *dst_len + 1 ) * sizeof( char ), __FILE__, __LINE__ );
    /* 2D array for query parts from strtok with "?" */
    char **src_part = NULL;
    char *ptr;
    /* "?" counter */
    int repl_count = 0;

    /* Duplicate source query */
    memcpy( tmp_src, src, *dst_len + 1 );

    /* Split query by "?" */
    ptr = strtok( tmp_src, "?" );
    while( ptr != NULL ) {
        /* Allocate memory for next query part */
        src_part = realloc( src_part, ( repl_count + 1 ) * sizeof( *src_part ) );
        /* Allocate memory for next query part token */
        src_part[ repl_count ] = SAFECALLOC( strlen( ptr ) + 1, sizeof( char ), __FILE__, __LINE__ );
        /* Store token as query part */
        memcpy( src_part[ repl_count ], ptr, strlen( ptr ) );
        ptr = strtok( NULL, "?" );
        repl_count++;
    }

    if( repl_count > 0 ) {
        /* Allocate temporary buffer for writing output query */
        char* dst_buffer = SAFECALLOC( ( *dst_len + 1 ),  sizeof( char ), __FILE__, __LINE__ );
        /* Store already written data length to dst_buffer for concatenate with memcpy */
        size_t current_buff_len = 0;

        for( int i = 0; i < repl_count; i++ ) {
            /* Firstly, append token into temporary buffer */
            memcpy( dst_buffer + current_buff_len, src_part[ i ], strlen( src_part[ i ] ) );
            /* Sum and store already written data length */
            current_buff_len += strlen( src_part[ i ] );
            if( i < replace_count ) { /* repl_count is always greater by 1 than replace_count */
                /* Secondly, append given value into temporary buffer */
                memcpy( dst_buffer + current_buff_len, replaces[ i ], param_lengths[ i ] );
                /* Sum and store already written data lengt */
                current_buff_len += param_lengths[ i ];
            }
        }

        dst_buffer[current_buff_len] = '\0';

        /* Free query parts */
        for( int i = 0; i < repl_count; i++ ) {
            free( src_part[ i] ); src_part[i] = NULL;
        }
        free( src_part ); src_part = NULL;

        /* Write data to destination buffer */
        memcpy( src, dst_buffer, *dst_len );

        /* Free temporary buffers */
        free( dst_buffer); dst_buffer = NULL;
        free( tmp_src ); tmp_src = NULL;
        return repl_count - 1;
    }
    return 0;
}

/* Old function, unusable for blobs */
//int _DB_QUERY_replace_params_b( char *src, const char *search, const char* const *replaces, const int replace_count, size_t *dst_len ) {
//    char *buffer = SAFECALLOC( *dst_len + 1, sizeof( char ), __FILE__, __LINE__ );
//    char* p = src;
//    int replace_index = 0;
//    int occurrences = 0;
//
//    while( ( p = strstr( p, search ) ) ) {
//        strncpy( buffer, src, p - src );
//        buffer[ p - src ] = '\0';
//        strcat( buffer, replaces[ replace_index ] );
//        strcat( buffer, p + strlen( search ) );
//        strcpy( src, buffer );
//        p++;
//        replace_index++;
//        occurrences++;
//    }
//
//    free( buffer ); buffer = NULL;
//
//    return occurrences;
//}

int _DB_QUERY_internal_replace_str_( char* src, const char* search, const char*replace, size_t *dst_len ) {
    char* buffer = SAFECALLOC( *dst_len + 1, sizeof( char ), __FILE__, __LINE__ );
    char* p = src;
    int occurrences = 0;

    while( ( p = strstr( p, search ) ) ) {
        strncpy( buffer, src, p - src );
        buffer[ p - src ] = '\0';
        strcat( buffer, replace );
        strcat( buffer, p + strlen( search ) );
        strcpy( src, buffer );
        p++;
        occurrences++;
    }

    free( buffer ); buffer = NULL;
    return occurrences;
}

DB_QUERY_TYPE DB_QUERY_get_type( const char* sql ) {
    if( strstr( sql, "SELECT" ) ) {
        return DQT_SELECT;
    } else if( strstr( sql, "INSERT" ) ) {
        return DQT_INSERT;
    } else if( strstr( sql, "UPDATE" ) ) {
        return DQT_UPDATE;
    } else if( strstr( sql, "DELETE" ) ) {
        return DQT_DELETE;
    } else if( strstr( sql, "ALTER" ) ) {
        return DQT_ALTER;
    } else if( strstr( sql, "DROP" ) ) {
        return DQT_DROP;
    } else if( strstr( sql, "CREATE" ) ) {
        return DQT_CREATE;
    } else if( strstr( sql, "USE" ) ) {
        return DQT_USE;
    } else if( strstr( sql, "SHOW" ) ) {
        return DQT_SHOW;
    } else {
        return DQT_UNKNOWN;
    }
}

int DB_QUERY_format( const char* src, char **dst, size_t *dst_length, const char* const* param_values, const int params_count, const int *param_lengths ) {
    size_t      src_len = 0;
    size_t      dst_len = 0;
    int         i = 0;
    char        *tmp_src = NULL;
    char        *ptr_src = ( char* )src;
    char        *ptr_dst;
    int         src_params_count = 0;
    int         dcpam_nulls = 0;

    /* Main checks */
    if( src == NULL ) {
        LOG_print( "Error: src pointer is NULL.\n" );
        return FALSE;
    }
    if( strstr( ptr_src, "?" ) == NULL ) {
        LOG_print( "Notice: SQL statement does not qualify to format.\n" );
        return TRUE;
    }
    if( *dst != NULL ) {
        LOG_print( "Error: dst pointer is already initialized.\n" );
        return FALSE;
    }
    /* Get SQL statement template length */
    src_len = strlen( src );
    /* Calculate dst result length. Start with src_len */
    dst_len = src_len;
    /*      1. Sum all param lengths */
    for( i = 0; i < params_count; i++ ) {
        dst_len += param_lengths[ i ];
    }
    ptr_src = ( char *)src;
    /*      2. Subtract all "?" occurrences */
    while( ptr_src = strstr( ptr_src, "?" ) ) {
        dst_len--;
        ptr_src++;
        src_params_count++;
    }
    /* Check if "?" count is equal to params_count */
    if( src_params_count > params_count ) {
        LOG_print( "Error: too few extracted_values! %d are set, but %d are provided!\n", src_params_count, params_count );
        LOG_print( "SQL: %s\n", src );
        return FALSE;
    }
    /* Init dst buffer */
    *dst = SAFECALLOC( dst_len + 1, sizeof( char ), __FILE__, __LINE__ );
    /* Copy SQL template to dst buffer */
    strncpy( *dst, src, dst_len );
    /* Replace all "?" occurrences with values */
    ptr_dst = *dst;
    _DB_QUERY_replace_params( ptr_dst, "?", param_values, param_lengths, params_count, &dst_len );
    /* Replace all 'dcpamNULL' with NULL */
    ptr_dst = *dst;
    dcpam_nulls = _DB_QUERY_internal_replace_str_( ptr_dst, "'dcpamNULL'", "NULL", &dst_len );
    dst_len = dst_len - (dcpam_nulls * 7);
    /* Replace all dcpamNULL with NULL */
    ptr_dst = *dst;
    dcpam_nulls = _DB_QUERY_internal_replace_str_( ptr_dst, "dcpamNULL", "NULL", &dst_len );
    dst_len = dst_len - ( dcpam_nulls * 5 );

    *dst_length = dst_len;

    return TRUE;
}
