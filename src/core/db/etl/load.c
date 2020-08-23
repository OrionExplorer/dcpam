#include <stdio.h>
#include <stdlib.h>
#include "../../../include/utils/time.h"
#include "../../../include/utils/memory.h"
#include "../../../include/core/schema.h"
#include "../../../include/core/db/system.h"
#include "../../../include/utils/log.h"
#include "../../../include/core/db/etl/load.h"

extern DCPAM_APP           APP;

int CDC_LoadGeneric( DB_SYSTEM_ETL_LOAD *load, DB_SYSTEM_ETL_LOAD_QUERY *load_element, DATABASE_SYSTEM_DB *source_db, DATABASE_SYSTEM_DB *dcpam_db, LOG_OBJECT *log );


void _LoadGeneric_callback( DB_RECORD *record, void *data_ptr1, void *data_ptr2, LOG_OBJECT *log ) {
    DB_SYSTEM_ETL_LOAD_QUERY* load_element = ( DB_SYSTEM_ETL_LOAD_QUERY* )data_ptr1;
    DATABASE_SYSTEM_DB* dcpam_db = ( DATABASE_SYSTEM_DB* )data_ptr2;

    if( load_element && dcpam_db && record ) {

        /* Each extracted record is loaded separatedly */
        char** q_values = NULL;
        int* q_lengths;
        int* q_formats;

        /* Prepare query data. */
        q_values = SAFEMALLOC( ( load_element->extracted_values_len ) * sizeof * q_values, __FILE__, __LINE__ );
        int q_values_len = 0;

        q_lengths = SAFEMALLOC( ( load_element->extracted_values_len ) * sizeof( int ), __FILE__, __LINE__ );
        q_formats = SAFEMALLOC( ( load_element->extracted_values_len ) * sizeof( int ), __FILE__, __LINE__ );

        /* Get defined values only based on "extracted_values" in config.json */
        for( int k = 0; k < load_element->extracted_values_len; k++ ) {
            for( int j = 0; j < record->field_count; j++ ) {

                if( strncmp( load_element->extracted_values[ k ], record->fields[ j ].label, MAX_COLUMN_NAME_LEN ) == 0 ) {
                    if( record->fields[ j ].size > 0 ) {
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
            int query_ret = DB_exec( dcpam_db, load_element->output_data_sql, load_element->output_data_sql_len, NULL, ( const char* const* )q_values, q_values_len, q_lengths, q_formats, NULL, NULL, NULL, NULL, log );

            if( query_ret == FALSE ) {
                LOG_print( log, "[%s] DB_exec error.\n", TIME_get_gmt() );
            }

            /* Free memory before next iteration */
            for( int i = 0; i < q_values_len; i++ ) {
                free( q_values[ i ] ); q_values[ i ] = NULL;
            }

        } else {
            LOG_print( log, "[%s] Error: Extract process returned data, but Load process conditions are not satisfied.\n", TIME_get_gmt() );
        }

        free( q_values ); q_values = NULL;
        free( q_lengths ); q_lengths = NULL;
        free( q_formats ); q_formats = NULL;

        DB_QUERY_record_free( record );

    }
}

void _LoadInserted_callback( DB_RECORD* record, void* data_ptr1, void* data_ptr2, LOG_OBJECT *log ) {
    DB_SYSTEM_ETL_LOAD* load = ( DB_SYSTEM_ETL_LOAD* )data_ptr1;

    _LoadGeneric_callback( record, load, &load->inserted, log );
}

void _LoadDeleted_callback( DB_RECORD* record, void* data_ptr1, void* data_ptr2, LOG_OBJECT *log ) {
    DB_SYSTEM_ETL_LOAD* load = ( DB_SYSTEM_ETL_LOAD* )data_ptr1;

    _LoadGeneric_callback( record, load, &load->deleted, log );
}

void _LoadModified_callback( DB_RECORD* record, void* data_ptr1, void* data_ptr2, LOG_OBJECT *log ) {
    DB_SYSTEM_ETL_LOAD* load = ( DB_SYSTEM_ETL_LOAD* )data_ptr1;

    _LoadGeneric_callback( record, load, &load->modified, log );
}


int CDC_LoadGeneric( DB_SYSTEM_ETL_LOAD *load, DB_SYSTEM_ETL_LOAD_QUERY *load_element, DATABASE_SYSTEM_DB *source_db, DATABASE_SYSTEM_DB *dcpam_db, LOG_OBJECT *log ) {

    if( load && source_db && dcpam_db ) {

        /* Load data from Staging Area, after finished Transform process. */
        qec load_generic_callback = ( qec )&_LoadGeneric_callback;

        int query_ret = DB_exec( source_db, load_element->input_data_sql, load_element->input_data_sql_len, NULL, NULL, 0, NULL, NULL, NULL, &load_generic_callback, ( void* )load_element, ( void* )dcpam_db, log );

        if( query_ret == FALSE ) {
            LOG_print( log, "[%s] DB_exec error.\n", TIME_get_gmt() );
            return 0;
        }
        return 1;
    } else {
        LOG_print( log, "[%s] Error: not all parameters are valid.\n", TIME_get_gmt() );
        return 0;
    }
}

int DB_CDC_LoadInserted( DB_SYSTEM_ETL_LOAD* load, DATABASE_SYSTEM_DB* source_db, DATABASE_SYSTEM_DB* dcpam_db, LOG_OBJECT *log ) {
    if( load && source_db && dcpam_db ) {
        LOG_print( log, "\t· [CDC - LOAD::INSERTED]:\n" );
        return CDC_LoadGeneric( load, &load->inserted, source_db, dcpam_db, log );
    } else {
        LOG_print( log, "\t· [CDC - LOAD::INSERTED] Fatal error: not all DB_CDC_LoadInserted parameters are valid!\n" );
        return 0;
    }
}

int DB_CDC_LoadDeleted( DB_SYSTEM_ETL_LOAD *load, DATABASE_SYSTEM_DB* source_db, DATABASE_SYSTEM_DB *dcpam_db, LOG_OBJECT *log ) {
    if( load && source_db && dcpam_db ) {
        LOG_print( log, "\t· [CDC - LOAD::DELETED]:\n" );
        return CDC_LoadGeneric( load, &load->deleted, source_db, dcpam_db, log );
    } else {
        LOG_print( log, "\t· [CDC - LOAD::DELETED] Fatal error: not all DB_CDC_LoadDeleted parameters are valid!\n" );
        return 0;
    }
}

int DB_CDC_LoadModified( DB_SYSTEM_ETL_LOAD *load, DATABASE_SYSTEM_DB* source_db, DATABASE_SYSTEM_DB *dcpam_db, LOG_OBJECT *log ) {
    if( load && source_db && dcpam_db ) {
        LOG_print( log, "\t· [CDC - LOAD::MODIFIED]:\n" );
        return CDC_LoadGeneric( load, &load->modified, source_db, dcpam_db, log );
    } else {
        LOG_print( log, "\t· [CDC - LOAD::MODIFIED] Fatal error: not all DB_CDC_LoadModified parameters are valid!\n" );
        return 0;
    }
}
