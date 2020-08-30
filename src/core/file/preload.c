#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/core/file/preload.h"
#include "../../include/utils/log.h"
#include "../../include/utils/time.h"
#include "../../include/core/db/system.h"
#include "../../include/utils/memory.h"

void _CSV_load_callback( CSV_RECORD* record, void *data_ptr1, void *data_ptr2, LOG_OBJECT *log ) {
    DATABASE_SYSTEM_FLAT_FILE   *flat_file = ( DATABASE_SYSTEM_FLAT_FILE* )data_ptr1;
    DATABASE_SYSTEM_DB          *system_db = ( DATABASE_SYSTEM_DB* )data_ptr2;

    if( record ) {
        /* Each extracted record is loaded separatedly */
        char** q_values = NULL;
        int* q_lengths;
        int* q_formats;

        /* Prepare query data. */
        q_values = SAFEMALLOC( ( flat_file->columns_len ) * sizeof * q_values, __FILE__, __LINE__ );
        int q_values_len = 0;

        q_lengths = SAFEMALLOC( ( flat_file->columns_len ) * sizeof( int ), __FILE__, __LINE__ );
        q_formats = SAFEMALLOC( ( flat_file->columns_len ) * sizeof( int ), __FILE__, __LINE__ );

        /* Get defined values only based on "extracted_values" in config.json */
        for( int k = 0; k < flat_file->columns_len; k++ ) {
            for( int j = 0; j < record->field_count; j++ ) {
                if( strncmp( flat_file->columns[ k ], record->fields[ j ].label, MAX_COLUMN_NAME_LEN ) == 0 ) {
                    if( record->fields[ j ].size > 0 && record->fields[ j ].value != NULL ) {
                        q_values[ q_values_len ] = SAFECALLOC( ( record->fields[ j ].size + 1 ), sizeof * *q_values, __FILE__, __LINE__ );
                        memcpy( q_values[ q_values_len ], record->fields[ j ].value, record->fields[ j ].size + 1 );

                        q_lengths[ q_values_len ] = record->fields[ j ].size;
                        q_formats[ q_values_len ] = 0;
                    } else {
                        q_values[ q_values_len ] = SAFECALLOC( 10, sizeof( char ), __FILE__, __LINE__ );
                        strcpy( q_values[ q_values_len ], "dcpamNULL" );
                        q_lengths[ q_values_len ] = 9;
                        q_formats[ q_values_len ] = 0;
                    }

                    q_values_len++;
                    break;
                }
            }
        }

        if( q_values_len > 0 ) {
            /* Perform DB query */
            int query_ret = DB_exec( system_db, flat_file->load_sql, strlen( flat_file->load_sql ), NULL, ( const char* const* )q_values, q_values_len, q_lengths, q_formats, NULL, NULL, NULL, NULL, log );

            if( query_ret == FALSE ) {
                LOG_print( log, "[%s] DB_exec error.\n", TIME_get_gmt() );
            }

            /* Free memory */
            for( int i = 0; i < q_values_len; i++ ) {
                free( q_values[ i ] ); q_values[ i ] = NULL;
            }

        } else {
            LOG_print( log, "[%s] Error: conditions are not satisfied.\n", TIME_get_gmt() );
        }

        free( q_values ); q_values = NULL;
        free( q_lengths ); q_lengths = NULL;
        free( q_formats ); q_formats = NULL;
    }

    if( record ) {
        for( int i = 0; i < record->field_count; i++ ) {
            if( record->fields[ i ].value ) {
                free( record->fields[ i ].value );
            }
        }
        free( record->fields );
        free( record );
    }
}

int FILE_ETL_preload( DATABASE_SYSTEM *system, const char* filename, LOG_OBJECT *log ) {

    if( system == NULL ) {
        LOG_print( log, "[%s] Fatal error: system pointer is invalid.\n", TIME_get_gmt() );
        return 0;
    }

    if( system->flat_file == NULL ) {
        LOG_print( log, "[%s] Fatal error: file pointer is invalid.\n", TIME_get_gmt() );
        return 0;
    }

    if( system->flat_file->type == FFT_CSV ) {
        clc csv_load_callback = ( clc )&_CSV_load_callback;

        CSV_FILE_load(
            system->flat_file->file,
            filename,
            &csv_load_callback,
            ( void* )system->flat_file,
            ( void* )&system->system_db,
            log
        );
    }

    return 1;
}
