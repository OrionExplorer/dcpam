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

void DB_QUERY_free( DB_QUERY* db_query ) {
    int         i = 0, j = 0;

    /*LOG_print( "[%s]]\tDB_QUERY_free( <'%s'> ).\n", TIME_get_gmt(), db_query->sql );*/
    if( db_query != NULL ) {

        if( db_query->sql != NULL ) {
            free( db_query->sql ); db_query->sql = NULL;
        }

        if( db_query->row_count > 0 && db_query->field_count > 0 ) {
            for( i = 0; i < db_query->row_count; i++ ) {
                for( j = 0; j < db_query->field_count; j++ ) {
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

void strreplace( char *src, const char *search, const char* const *replaces, const int replace_count, size_t dst_len ) {
    char *buffer = SAFECALLOC( dst_len + 1, sizeof( char ), __FILE__, __LINE__ );
    char* p = src;
    int replace_index = 0;
    while( ( p = strstr( p, search ) ) ) {
        strncpy( buffer, src, p - src );
        buffer[ p - src ] = '\0';
        strcat( buffer, replaces[ replace_index ] );
        strcat( buffer, p + strlen( search ) );
        strcpy( src, buffer );
        p++;
        replace_index++;
    }

    free( buffer ); buffer = NULL;
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

int DB_QUERY_format( const char* src, char **dst, unsigned long *dst_length, const char* const* param_values, const int params_count, const int *param_lengths ) {
    size_t      src_len = 0;
    size_t      dst_len = 0;
    int         i = 0;
    char        *tmp_src = NULL;
    char        *ptr_src = src;
    char        *ptr_dst;
    int         src_params_count = 0;
    int         param_index = 0;

    /* Main checks */
    if( src == NULL ) {
        printf( "Error: src pointer is NULL.\n" );
        return FALSE;
    }

    if( strstr( ptr_src, "?" ) == NULL ) {
        printf( "Notice: SQL statement does not qualify to format.\n" );
        return TRUE;
    }

    if( *dst != NULL ) {
        printf( "Error: dst pointer is already initialized.\n" );
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
    if( src_params_count != params_count ) {
        printf( "Error: params_count and SQL template variables does not match!\n" );
    }

    /* Init dst buffer */
    *dst = SAFECALLOC( dst_len + 1, sizeof( char ), __FILE__, __LINE__ );
    /* Copy SQL template to dst buffer */
    strncpy( *dst, src, dst_len );

    /* Replace all "?" occurrences with values */
    ptr_dst = *dst;
    strreplace( ptr_dst, "?", param_values, params_count, dst_len );
    /* Replace all 'NULL' with NULL */
    ptr_dst = *dst;
    strreplace( ptr_dst, "?", param_values, params_count, dst_len );
    *dst_length = dst_len;

    return TRUE;
}


void DB_QUERY_field_type( DB_FIELD *field, char *dst ) {
    
}