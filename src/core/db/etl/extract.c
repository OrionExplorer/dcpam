#include <stdio.h>
#include <stdlib.h>
#include "../../../include/core/db/system.h"
#include "../../../include/utils/time.h"
#include "../../../include/utils/memory.h"
#include "../../../include/core/schema.h"
#include "../../../include/utils/log.h"
#include "../../../include/core/db/etl/extract.h"
#include "../../../include/core/db/etl/stage.h"


void _ExtractGeneric_callback( DB_RECORD* record, DB_SYSTEM_ETL_STAGE* stage, DB_SYSTEM_ETL_STAGE_QUERY* stage_element, DATABASE_SYSTEM_DB* db ) {

    if( stage ) {
        DB_CDC_StageGeneric( stage, stage_element, db, record );
    }

    DB_QUERY_record_free( record );
}

void _ExtractInserted_callback( DB_RECORD* record, void* data_ptr1, void* data_ptr2 ) {
    DB_SYSTEM_ETL_STAGE* stage = ( DB_SYSTEM_ETL_STAGE* )data_ptr1;
    DATABASE_SYSTEM_DB* db = ( DATABASE_SYSTEM_DB* )data_ptr2;

    _ExtractGeneric_callback( record, stage, &stage->inserted, db );
}

void _ExtractDeleted_callback( DB_RECORD* record, void* data_ptr1, void* data_ptr2 ) {
    DB_SYSTEM_ETL_STAGE* stage = ( DB_SYSTEM_ETL_STAGE* )data_ptr1;
    DATABASE_SYSTEM_DB* db = ( DATABASE_SYSTEM_DB* )data_ptr2;

    _ExtractGeneric_callback( record, stage, &stage->deleted, db );
}

void _ExtractModified_callback( DB_RECORD* record, void* data_ptr1, void* data_ptr2 ) {
    DB_SYSTEM_ETL_STAGE* stage = ( DB_SYSTEM_ETL_STAGE* )data_ptr1;
    DATABASE_SYSTEM_DB* db = ( DATABASE_SYSTEM_DB* )data_ptr2;

    _ExtractGeneric_callback( record, stage, &stage->modified, db );
}

int CDC_ExtractQueryTypeValid( const char *sql ) {

    DB_QUERY_TYPE dqt = DB_QUERY_get_type( sql );
    if(
        dqt == DQT_DELETE ||
        dqt == DQT_INSERT ||
        dqt == DQT_UPDATE
    ) {
        return 0;
    }

    return 1;
}

void CDC_ExtractGeneric( DB_SYSTEM_ETL_EXTRACT *extract, DB_SYSTEM_ETL_EXTRACT_QUERY *extract_element, DATABASE_SYSTEM_DB *system_db, DATABASE_SYSTEM_DB* dcpam_db, qec *query_exec_callback, void* data_ptr1, void* data_ptr2 ) {

    if( CDC_ExtractQueryTypeValid( extract_element->primary_db_sql ) == 0 || CDC_ExtractQueryTypeValid( extract_element->secondary_db_sql ) == 0 ) {
        LOG_print( "Fatal error: Only SELECT queries are allowed in *_sql commands.\n" );
        return;
    }

    if( extract ) {

        DATABASE_SYSTEM_DB  *primary_db;
        DATABASE_SYSTEM_DB  *secondary_db;
        DB_QUERY            primary_db_sql_res;

        LOG_print( "\t\t- Primary DB: %s\n", extract_element->primary_db );
        LOG_print( "\t\t- Secondary DB: %s\n", extract_element->secondary_db );

        if( strcmp( extract_element->primary_db, "this" ) == 0 ) {
            primary_db = system_db;
            secondary_db = dcpam_db;//&APP.DB;
        } else {
            primary_db = dcpam_db;//&APP.DB;
            secondary_db = system_db;
        }

        DB_QUERY_init( &primary_db_sql_res );

        int primary_ret = DB_exec( primary_db, extract_element->primary_db_sql, strnlen( extract_element->primary_db_sql, extract_element->primary_db_sql_len + 1 ), &primary_db_sql_res, NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL );

        /* Check if query finished successfully. */
        if( primary_ret == TRUE ) {

            /* Check if query resulted with any data */
            if( primary_db_sql_res.row_count > 0 ) {

                if( primary_db_sql_res.field_count == 1 ) {

                    size_t ret_values_len = 0;
                    char   **ret_values = NULL;
                    char   *ret_values_str = NULL;

                    /* Allocate memory for query result values  */
                    ret_values = SAFEMALLOC( (primary_db_sql_res.row_count + 1) * sizeof *ret_values, __FILE__, __LINE__ );

                    /* Allocate memory for each row value and copy data */
                    for( int i = 0; i < primary_db_sql_res.row_count; i++ ) {
                        if( primary_db_sql_res.records[ i ].fields[ 0 ].size > 0 ) {
                            ret_values[ i ] = SAFEMALLOC( ( primary_db_sql_res.records[ i ].fields[ 0 ].size + 1 ) * sizeof * *ret_values, __FILE__, __LINE__ );
                            memcpy( ret_values[ i ], primary_db_sql_res.records[ i ].fields[ 0 ].value, primary_db_sql_res.records[ i ].fields[ 0 ].size + 1 );
                            /* Track summary length for future memory allocation */
                            ret_values_len += primary_db_sql_res.records[ i ].fields[ 0 ].size;
                        } else {
                            ret_values[ i ] = NULL;
                        }
                    }

                    if( ret_values_len > 0 ) {

                        int i = 0;
                        int str_index = 0;

                        /* Allocate enough memory for data and commas */
                        ret_values_len += primary_db_sql_res.row_count;
                        ret_values_str = SAFECALLOC( ret_values_len, sizeof( char ), __FILE__, __LINE__ );
                        if( ret_values_str == NULL ) {
                            LOG_print( "Fatal error: unable to SAFEMALLOC( %d * sizeof( char ) ).\n", ret_values_len );
                            for( i = 0; i < primary_db_sql_res.row_count; i++ ) {
                                free( ret_values[ i ] );
                            }
                            free( ret_values );
                            ret_values = NULL;
                            return;
                        }

                        /* Join ret_values with commas */
                        for( i = 0; i < primary_db_sql_res.row_count; i++ ) {
                            str_index += snprintf( &ret_values_str[ str_index ], ret_values_len - str_index, "%s,", ret_values[ i ] );
                        }

                        /* Not needed anymore */
                        for( i = 0; i < primary_db_sql_res.row_count; i++ ) {
                            free( ret_values[ i ] ); ret_values[ i ] = NULL;
                        }
                        free( ret_values );

                        /* Query result from primary_db_sql is no longer needed */
                        DB_QUERY_free( &primary_db_sql_res );

                        LOG_print( "\t· [CDC - EXTRACT] new data:\n" );

                        /* Allocate enough memory for SQL string and concatenated rev_values in ret_values_str */
                        char *secondary_db_sql_p = NULL;
                        size_t secondary_db_sql_len = extract_element->secondary_db_sql_len + ret_values_len;
                        secondary_db_sql_p = SAFECALLOC( secondary_db_sql_len, sizeof( char ), __FILE__, __LINE__ );
                        snprintf( secondary_db_sql_p, secondary_db_sql_len, extract_element->secondary_db_sql, ret_values_str );

                        /* Not needed anymore */
                        free( ret_values_str );
                        ret_values_str = NULL;

                        /* Perform DB query and store result in *data */
                        int secondary_ret = DB_exec( secondary_db, secondary_db_sql_p, secondary_db_sql_len - 2 /* "%s" */, NULL, NULL, 0, NULL, NULL, NULL, query_exec_callback, data_ptr1, data_ptr2 );

                        if( secondary_ret == 1 ) {
                            LOG_print( "\t· [CDC - EXTRACT] Completed.\n" );
                        }

                        free( secondary_db_sql_p );
                        secondary_db_sql_p = NULL;
                    } else {
                        LOG_print( "[%s] Error: query completed, but no significant data returned.\n", TIME_get_gmt() );
                        for( int i = 0; i < primary_db_sql_res.row_count; i++ ) {
                            free( ret_values[ i ] ); ret_values[ i ] = NULL;
                        }
                        free( ret_values );

                        /* Query result from primary_db_sql is no longer needed */
                        DB_QUERY_free( &primary_db_sql_res );
                    }
                } else {
                    DB_QUERY_free( &primary_db_sql_res );
                    /*DB_QUERY_free( data );*/
                    LOG_print( "Fatal error: query returned more than one column.\n" );
                }
            } else {
                DB_QUERY_free( &primary_db_sql_res );
            }
        } else {
            /* In case of error, free query result struct */
            DB_QUERY_free( &primary_db_sql_res );
            /*DB_QUERY_free( data );*/
        }
    }
}

void DB_CDC_ExtractInserted( DB_SYSTEM_ETL_EXTRACT *extract, DATABASE_SYSTEM_DB *system_db, DATABASE_SYSTEM_DB* dcpam_db, qec *query_exec_callback, void* data_ptr1, void* data_ptr2 ) {
    if( extract && system_db && dcpam_db ) {
        LOG_print( "\t· [CDC - EXTRACT::INSERTED] existing data:\n" );
        CDC_ExtractGeneric( extract, &extract->inserted, system_db, dcpam_db, query_exec_callback, data_ptr1, data_ptr2 );
    } else {
        LOG_print( "\t· [CDC - EXTRACT::INSERTED] Fatal error: not all DB_CDC_ExtractInserted parameters are valid!\n" );
    }
}

void DB_CDC_ExtractDeleted( DB_SYSTEM_ETL_EXTRACT *extract, DATABASE_SYSTEM_DB *system_db, DATABASE_SYSTEM_DB* dcpam_db, qec *query_exec_callback, void* data_ptr1, void* data_ptr2 ) {
    if( extract && system_db && dcpam_db ) {
        LOG_print( "\t· [CDC - EXTRACT::DELETED] existing data:\n" );
        CDC_ExtractGeneric( extract, &extract->deleted, system_db, dcpam_db, query_exec_callback, data_ptr1, data_ptr2 );
    } else {
        LOG_print( "\t· [CDC - EXTRACT::DELETED] Fatal error: not all DB_CDC_ExtractDeleted parameters are valid!\n" ); 
    }
}

void DB_CDC_ExtractModified( DB_SYSTEM_ETL_EXTRACT *extract, DATABASE_SYSTEM_DB *system_db, DATABASE_SYSTEM_DB* dcpam_db, qec *query_exec_callback, void* data_ptr1, void* data_ptr2 ) {
    if( extract && system_db && dcpam_db ) {
        LOG_print( "\t· [CDC - EXTRACT::MODIFIED] existing data:\n" );
        CDC_ExtractGeneric( extract, &extract->modified, system_db, dcpam_db, query_exec_callback, data_ptr1, data_ptr2 );
    } else {
        LOG_print( "\t· [CDC - EXTRACT::MODIFIED] Fatal error: not all DB_CDC_ExtractModified parameters are valid!\n" );
    }
}