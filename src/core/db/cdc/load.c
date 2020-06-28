#include <stdio.h>
#include <stdlib.h>
#include "../../../include/utils/time.h"
#include "../../../include/utils/memory.h"
#include "../../../include/core/schema.h"
#include "../../../include/core/db/system.h"
#include "../../../include/utils/log.h"
#include "../../../include/core/db/cdc/load.h"

extern DCPAM_APP           APP;

void CDC_LoadGeneric( DB_SYSTEM_CDC_LOAD *load, DB_SYSTEM_CDC_LOAD_QUERY *load_element, DATABASE_SYSTEM_DB *db );


void CDC_LoadGeneric( DB_SYSTEM_CDC_LOAD *load, DB_SYSTEM_CDC_LOAD_QUERY *load_element, DATABASE_SYSTEM_DB *db ) {

    if( load ) {

        /* Load data from Staging Area, after finished Transform process. */
        DB_QUERY    stage_data;

        DB_QUERY_init( &stage_data );
        int query_ret = DB_exec( &APP.DB, load_element->input_data_sql, load_element->input_data_sql_len, &stage_data, NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL );

        if( query_ret == FALSE ) {
            LOG_print( "[%s] DB_exec error.\n", TIME_get_gmt() );
            DB_QUERY_free( &stage_data );
            return;
        }

        if( stage_data.row_count == 0 ) {
            LOG_print( "[%s] Load from Staging Area cancelled: given query resulted with no data.\n", TIME_get_gmt() );
            DB_QUERY_free( &stage_data );
            return;
        }

        /* Each record from Staging Area is loaded separatedly */
        for( int i = 0; i < stage_data.row_count; i++ ) {

            char **q_values = NULL;
            int *q_lengths;
            int *q_formats;

            /* Prepare query data. */
            q_values = SAFEMALLOC( (load_element->extracted_values_len) * sizeof *q_values, __FILE__, __LINE__ );
            int q_values_len = 0;

            q_lengths = SAFEMALLOC( (load_element->extracted_values_len) * sizeof( int ), __FILE__, __LINE__ );
            q_formats = SAFEMALLOC( (load_element->extracted_values_len) * sizeof( int ), __FILE__, __LINE__ );

            /* Get defined values only based on "extracted_values" in config.json */
            for( int k = 0; k < load_element->extracted_values_len; k++ ) {
                for( int j = 0; j < stage_data.field_count; j++ ) {
                    if( strncmp( load_element->extracted_values[ k ], stage_data.records[ i ].fields[ j ].label, MAX_COLUMN_NAME_LEN ) == 0 ) {
                        if( stage_data.records[ i ].fields[ j ].size > 0 ) {
                            q_values[ q_values_len ] = SAFECALLOC( ( stage_data.records[ i ].fields[ j ].size + 1), sizeof **q_values, __FILE__, __LINE__ );
                            memcpy( q_values[ q_values_len ], stage_data.records[ i ].fields[ j ].value, stage_data.records[ i ].fields[ j ].size + 1 );

                            q_lengths[ q_values_len ] = stage_data.records[ i ].fields[ j ].size;
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

                int query_ret = DB_exec( &APP.DB, load_element->output_data_sql, load_element->output_data_sql_len, NULL, ( const char* const* )q_values, q_values_len, q_lengths, q_formats, NULL, NULL, NULL, NULL );

                if( query_ret == FALSE ) {
                    LOG_print( "[%s] DB_exec error.\n", TIME_get_gmt() );
                }

                /* Free memory before next iteration */
                for( int i = 0; i < q_values_len; i++ ) {
                    free( q_values[ i ] ); q_values[ i ] = NULL;
                }

            } else {
                LOG_print( "[%s] Error: Extract process returned data, but Load process conditions are not satisfied.\n", TIME_get_gmt() );
            }
            free( q_values ); q_values = NULL;
            free( q_lengths ); q_lengths = NULL;
            free( q_formats ); q_formats = NULL;
        }

        DB_QUERY_free( &stage_data );
    }

}

void DB_CDC_LoadInserted( DB_SYSTEM_CDC_LOAD* load, DATABASE_SYSTEM_DB* db ) {
    if( load && db ) {
        LOG_print( "\t· [CDC - LOAD::INSERTED]:\n" );
        CDC_LoadGeneric( load, &load->inserted, db );
    } else {
        LOG_print( "\t· [CDC - LOAD::INSERTED] Fatal error: not all DB_CDC_LoadInserted parameters are valid!\n" );
    }
}

void DB_CDC_LoadDeleted( DB_SYSTEM_CDC_LOAD *load, DATABASE_SYSTEM_DB *db ) {
    if( load && db ) {
        LOG_print( "\t· [CDC - LOAD::DELETED]:\n" );
        CDC_LoadGeneric( load, &load->deleted, db );
    } else {
        LOG_print( "\t· [CDC - LOAD::DELETED] Fatal error: not all DB_CDC_LoadDeleted parameters are valid!\n" );   
    }
}

void DB_CDC_LoadModified( DB_SYSTEM_CDC_LOAD *load, DATABASE_SYSTEM_DB *db ) {
    if( load && db ) {
        LOG_print( "\t· [CDC - LOAD::MODIFIED]:\n" );
        CDC_LoadGeneric( load, &load->modified, db );
    } else {
        LOG_print( "\t· [CDC - LOAD::MODIFIED] Fatal error: not all DB_CDC_LoadModified parameters are valid!\n" );
    }
}
