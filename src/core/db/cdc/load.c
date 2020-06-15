#include <stdio.h>
#include <stdlib.h>
#include "../../../include/utils/time.h"
#include "../../../include/utils/memory.h"
#include "../../../include/core/schema.h"
#include "../../../include/core/db/system.h"
#include "../../../include/utils/log.h"
#include "../../../include/core/db/cdc/load.h"

extern DCPAM_APP           APP;

void CDC_LoadGeneric( DB_SYSTEM_CDC_LOAD *load, DB_SYSTEM_CDC_LOAD_QUERY *load_element, DATABASE_SYSTEM_DB *db, DB_QUERY *data );


void CDC_LoadGeneric( DB_SYSTEM_CDC_LOAD *load, DB_SYSTEM_CDC_LOAD_QUERY *load_element, DATABASE_SYSTEM_DB *db, DB_QUERY *data ) {
    DB_QUERY            sql_res;
    int                 j = 0, k = 0;
    char                **q_values = NULL;
    int                 *q_lengths;
    int                 *q_formats;
    Oid                 *q_types;
    
    if( load ) {

        /* Each extracted record is loaded separatedly */
        for( int i = 0; i < data->row_count; i++ ) {

            /* Prepare query data. PostgreSQL compatibile for now only. */
            q_values = SAFEMALLOC( (load_element->extracted_values_len+1) * sizeof *q_values, __FILE__, __LINE__ );
            int q_values_len = 0;

            q_lengths = SAFEMALLOC( (load_element->extracted_values_len+1) * sizeof( int ), __FILE__, __LINE__ );
            q_formats = SAFEMALLOC( (load_element->extracted_values_len+1) * sizeof( int ), __FILE__, __LINE__ );
            q_types = SAFEMALLOC( (load_element->extracted_values_len+1) * sizeof( int ), __FILE__, __LINE__ );

            /* Get defined values only based on "extracted_values" in config.json */
            for( j = 0; j < data->field_count; j++ ) {
                for( k = 0; k < load_element->extracted_values_len; k++ ) {
                    if( strncmp( load_element->extracted_values[ k ], data->records[ i ].fields[ j ].label, MAX_COLUMN_NAME_LEN ) == 0 ) {
                        if( data->records[ i ].fields[ j ].size > 0 ) {
                            q_values[ q_values_len ] = SAFEMALLOC( (data->records[ i ].fields[ j ].size + 1) * sizeof **q_values, __FILE__, __LINE__ );
                            memcpy( q_values[ q_values_len ], data->records[ i ].fields[ j ].value, data->records[ i ].fields[ j ].size + 1 );
                            q_values[ q_values_len+1 ] = '\0';

                            q_lengths[ q_values_len ] = data->records[ i ].fields[ j ].size;
                            q_formats[ q_values_len ] = 0;
                            q_types[ q_values_len ] = 0;
                        } else {
                            q_values[ q_values_len ] = SAFEMALLOC( ( 10 + 1 ) * sizeof **q_values, __FILE__, __LINE__ );
                            strncpy( q_values[ q_values_len ], "dcpamNULL\0", 10 );
                            q_lengths[ q_values_len ] = 9;
                            q_formats[ q_values_len ] = 0;
                            q_types[ q_values_len ] = 0;
                        }

                        q_values_len++;
                        break;
                    }
                }
            }

            if( q_values_len > 0 ) {
                DB_QUERY_init( &sql_res );
                /* Perform DB query and store result in *data */
                int query_ret = DB_exec( &APP.DB, load_element->sql, strlen( load_element->sql ), &sql_res, q_values, q_values_len, q_lengths, q_formats, ( const char* )q_types );

                /* Free memory before next iteration */
                for( j = 0; j < q_values_len; j++ ) {
                    free( q_values[ j ] ); q_values[ j ] = NULL;
                }

                DB_QUERY_free( &sql_res );
            } else {
                LOG_print( "[%s] Error: Extract process returned data, but Load process conditions are not satisfied.\n", TIME_get_gmt() );
            }
            free( q_values ); q_values = NULL;
            free( q_lengths ); q_lengths = NULL;
            free( q_formats ); q_formats = NULL;
            free( q_types ); q_types = NULL;
        }
    }

}

void DB_CDC_LoadInserted( DB_SYSTEM_CDC_LOAD* load, DATABASE_SYSTEM_DB* db, DB_QUERY* data ) {
    if( load && db && data ) {
        LOG_print( "\t· [CDC - LOAD::INSERTED (%d rows)]:\n", data->row_count );
        if( data->row_count == 0 ) {
            LOG_print( "\t\t- Action canceled.\n" );
        } else {
            CDC_LoadGeneric( load, &load->inserted, db, data );
        }
    } else {
        LOG_print( "\t· [CDC - LOAD::INSERTED] Fatal error: not all DB_CDC_LoadInserted parameters are valid!\n" );
    }
    
}

void DB_CDC_LoadDeleted( DB_SYSTEM_CDC_LOAD *load, DATABASE_SYSTEM_DB *db, DB_QUERY *data ) {
    if( load && db && data ) {
        LOG_print( "\t· [CDC - LOAD::DELETED (%d rows)]:\n", data->row_count );
        if( data->row_count == 0 ) {
            LOG_print( "\t\t- Action canceled.\n" );
        } else {
            CDC_LoadGeneric( load, &load->deleted, db, data );
        }
    } else {
        LOG_print( "\t· [CDC - LOAD::DELETED] Fatal error: not all DB_CDC_LoadDeleted parameters are valid!\n" );   
    }
}

void DB_CDC_LoadModified( DB_SYSTEM_CDC_LOAD *load, DATABASE_SYSTEM_DB *db, DB_QUERY *data ) {
    if( load && db && data ) {
        LOG_print( "\t· [CDC - LOAD::MODIFIED (%d rows)]:\n", data->row_count );
        if( data->row_count == 0 ) {
            LOG_print( "\t\t- Action canceled.\n" );
        } else {
            CDC_LoadGeneric( load, &load->modified, db, data );
        }
    } else {
        LOG_print( "\t· [CDC - LOAD::MODIFIED] Fatal error: not all DB_CDC_LoadModified parameters are valid!\n" );
    }
}
