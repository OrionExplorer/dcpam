#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/utils/filesystem.h"
#include "../include/file/csv.h"
#include "../include/utils/time.h"
#include "../include/utils/memory.h"

char** _CSV_parse_line( const char* line, const char* delimiter, int *dst_count, LOG_OBJECT *log ) {
    int     i = 0;
    char    token[ 1024 ];
    int     token_pos = 0;
    int     in_double_quotes = 0;
    int     val_count = 0;
    char    **tmp = SAFEMALLOC( val_count * sizeof * tmp, __FILE__, __LINE__ );
    char    **r_tmp;

    do {
        token[ token_pos++ ] = line[ i ];

        if( !in_double_quotes && ( line[ i ] == delimiter[ 0 ] || line[ i ] == '\n' ) ) {
            val_count++;
            token[ token_pos - 1 ] = 0;
            token_pos = 0;

            r_tmp = realloc( tmp, ( val_count + 1 ) * sizeof * tmp );
            if( r_tmp ) {
                tmp = r_tmp;
                size_t col_name_len = strlen( token );
                if( col_name_len > 0 ) {
                    tmp[ val_count - 1 ] = SAFECALLOC( col_name_len + 1, sizeof( char ), __FILE__, __LINE__ );
                    strlcpy( tmp[ val_count - 1 ], token, col_name_len );
                } else {
                    tmp[ val_count - 1 ] = NULL;
                }
            } else {
                LOG_print( log, "[%s] Fatal error: unable to realloc memory in _CSV_parse_line.\n", TIME_get_gmt() );
            }

        }

        if( line[ i ] == '"' && line[ i + 1 ] != '"' ) {
            token_pos--;
            in_double_quotes = !in_double_quotes;
        }

        if( line[ i ] == '"' && line[ i + 1 ] == '"' )
            i++;

    } while( line[ ++i ] );

    if( dst_count ) {
        *dst_count = val_count;
    }

    return tmp;
}

int CSV_FILE_load( CSV_FILE* dst, const char* filename, clc* csv_load_callback, void *data_ptr1, void *data_ptr2, LOG_OBJECT* log ) {

    LOG_print( log, "[%s] CSV_FILE_load( %s )...\n", TIME_get_gmt(), filename );
    FILE* csv_f = FILE_open( filename, "r", "w", log );

    if( csv_f == NULL ) {
        LOG_print( log, "[%s] CSV_FILE_load failed.\n", TIME_get_gmt() );
        return 0;
    }

    char        buf[ 8192 ];
    int         row_count = 0;
    char        **columns = NULL;
    char        **row_values = NULL;
    CSV_RECORD  *csv_columns = SAFEMALLOC( sizeof( CSV_RECORD ), __FILE__, __LINE__ );

    while( fgets( buf, 8192, csv_f ) ) {
        row_count++;
        if( row_count == 1 ) {
            columns = _CSV_parse_line( buf, dst->delimiter, &csv_columns->field_count, log );
        } else {
            if( csv_columns->field_count > 0 ) {
                CSV_RECORD* csv_record = SAFEMALLOC( sizeof( CSV_RECORD ), __FILE__, __LINE__ );
                csv_record->field_count = csv_columns->field_count;
                csv_record->fields = SAFEMALLOC( csv_columns->field_count * sizeof( CSV_FIELD ), __FILE__, __LINE__ );
                row_values = _CSV_parse_line( buf, dst->delimiter, NULL, log );

                for( int i = 0; i < csv_columns->field_count; i++ ) {
                    if( row_values[ i ] ) {
                        strlcpy( csv_record->fields[ i ].label, columns[ i ], MAX_COLUMN_NAME_LEN );
                        csv_record->fields[ i ].size = strlen( row_values[ i ] );
                        csv_record->fields[ i ].value = SAFECALLOC( ( csv_record->fields[ i ].size + 1 ), sizeof( char ), __FILE__, __LINE__ );
                        strlcpy( csv_record->fields[ i ].value, row_values[ i ], csv_record->fields[ i ].size );
                        free( row_values[ i ] ); row_values[ i ] = NULL;
                    } else {
                        strlcpy( csv_record->fields[ i ].label, columns[ i ], MAX_COLUMN_NAME_LEN );
                        csv_record->fields[ i ].size = 0;
                        csv_record->fields[ i ].value = NULL;
                        free( row_values[ i ] ); row_values[ i ] = NULL;
                    }
                }
                free( row_values ); row_values = NULL;

                if( csv_load_callback ) {
                    ( *csv_load_callback )( csv_record, data_ptr1, data_ptr2, log );
                }
            }
        }
    }

    for( int i = 0; i < csv_columns->field_count; i++ ) {
        free( columns[ i ] ); columns[ i ] = NULL;
    }
    free( columns ); columns = NULL;
    free( csv_columns ); csv_columns = NULL;

    fclose( csv_f );
    return 1;
}
