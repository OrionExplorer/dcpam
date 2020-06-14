#include <stdio.h>
#include <stdlib.h>
#include "../../../include/core/db/system.h"
#include "../../../include/utils/time.h"
#include "../../../include/utils/memory.h"
#include "../../../include/core/schema.h"
#include "../../../include/utils/log.h"
#include "../../../include/core/db/cdc/extract.h"


//extern DCPAM_APP           APP;

//void CDC_ExtractGeneric( DB_SYSTEM_CDC_EXTRACT *extract, DB_SYSTEM_CDC_EXTRACT_QUERY *extract_element, DATABASE_SYSTEM_DB *db, DB_QUERY *data );


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

void CDC_ExtractGeneric( DB_SYSTEM_CDC_EXTRACT *extract, DB_SYSTEM_CDC_EXTRACT_QUERY *extract_element, DATABASE_SYSTEM_DB *db, DB_QUERY *data ) {
    DB_QUERY            primary_db_sql_res;
    DATABASE_SYSTEM_DB  *primary_db;
    DATABASE_SYSTEM_DB  *secondary_db;
    int                 primary_ret = 0, secondary_ret = 0;
    int                 i = 0, j = 0;
    int                 ret_values_count = 0;
    size_t              ret_values_len = 0;
    char                **ret_values = NULL;
    char                *ret_values_str = NULL;
    char                *secondary_db_sql_p = NULL;
    int                 str_index = 0;

    if( CDC_ExtractQueryTypeValid( extract_element->primary_db_sql ) == 0 || CDC_ExtractQueryTypeValid( extract_element->secondary_db_sql ) == 0 ) {
        LOG_print( "Fatal error: Only SELECT queries are allowed in *_sql commands.\n" );
        return;
    }

    if( extract ) {

        LOG_print( "\t\t- Primary DB: %s\n", extract_element->primary_db );
        LOG_print( "\t\t- Secondary DB: %s\n", extract_element->secondary_db );

        if( strcmp( extract_element->primary_db, "this" ) == 0 ) {
            primary_db = db;
            secondary_db = &APP.DB;
        } else {
            primary_db = &APP.DB;
            secondary_db = db;
        }

        DB_QUERY_init( &primary_db_sql_res );
        primary_ret = DB_exec( primary_db, extract_element->primary_db_sql, strlen( extract_element->primary_db_sql ), &primary_db_sql_res, NULL, 0, NULL, NULL, NULL );

        /* Check if query finished successfully. */
        if( primary_ret == TRUE ) {

            /* Check if query resulted with any data */
            if( primary_db_sql_res.row_count > 0 ) {

                if( primary_db_sql_res.field_count == 1 ) {

                    /* Allocate memory for query result values  */
                    ret_values = SAFEMALLOC( (primary_db_sql_res.row_count + 1) *  sizeof *ret_values, __FILE__, __LINE__ );

                    /* Allocate memory for each row value and copy data */
                    for( i = 0; i < primary_db_sql_res.row_count; i++ ) {
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
                        /* Allocate enough memory for data and commas */
                        ret_values_len += primary_db_sql_res.row_count;
                        ret_values_str = SAFEMALLOC( ret_values_len * sizeof( char ), __FILE__, __LINE__ );
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
                        ret_values_str[ strlen( ret_values_str ) ] = '\0';

                        /* Not needed anymore */
                        for( i = 0; i < primary_db_sql_res.row_count; i++ ) {
                            free( ret_values[ i ] ); ret_values[ i ] = NULL;
                        }
                        free( ret_values );

                        /* Query result from primary_db_sql is no longer needed */
                        DB_QUERY_free( &primary_db_sql_res );

                        LOG_print( "\t· [CDC - EXTRACT] new data:\n" );

                        /* Allocate enough memory for SQL string and concatenated rev_values in ret_values_str */
                        size_t secondary_db_sql_len = strlen( extract_element->secondary_db_sql ) + ret_values_len;
                        secondary_db_sql_p = SAFECALLOC( secondary_db_sql_len, sizeof( char ), __FILE__, __LINE__ );
                        snprintf( secondary_db_sql_p, secondary_db_sql_len, extract_element->secondary_db_sql, ret_values_str );

                        /* Not needed anymore */
                        free( ret_values_str );
                        ret_values_str = NULL;

                        /* Perform DB query and store result in *data */
                        secondary_ret = DB_exec( secondary_db, secondary_db_sql_p, secondary_db_sql_len - 2 /* "%s" */, data, NULL, 0, NULL, NULL, NULL );

                        if( secondary_ret == 1 ) {
                            LOG_print( "\t· [CDC - EXTRACT] records found: %d\n", data->row_count );
                        }

                        free( secondary_db_sql_p );
                        secondary_db_sql_p = NULL;
                    } else {
                        LOG_print( "[%s] Error: query completed, but no significant data returned.\n", TIME_get_gmt() );
                        for( i = 0; i < primary_db_sql_res.row_count; i++ ) {
                            free( ret_values[ i ] ); ret_values[ i ] = NULL;
                        }
                        free( ret_values );

                        /* Query result from primary_db_sql is no longer needed */
                        DB_QUERY_free( &primary_db_sql_res );
                    }
                } else {
                    DB_QUERY_free( &primary_db_sql_res );
                    DB_QUERY_free( data );
                    LOG_print( "Fatal error: query returned more than one column.\n" );
                }
            } else {
                DB_QUERY_free( &primary_db_sql_res );
            }
        } else {
            /* In case of error, free query result struct */
            DB_QUERY_free( &primary_db_sql_res );
            DB_QUERY_free( data );
        }
    }
}

void DB_CDC_ExtractInserted( DB_SYSTEM_CDC_EXTRACT *extract, DATABASE_SYSTEM_DB *db, DB_QUERY *data ) {
    if( extract && db && data ) {
        LOG_print( "\t· [CDC - EXTRACT::INSERTED] existing data:\n" );
        CDC_ExtractGeneric( extract, &extract->inserted, db, data );    
    } else {
        LOG_print( "\t· [CDC - EXTRACT::INSERTED] Fatal error: not all DB_CDC_ExtractInserted parameters are valid!\n" );
    }
}

void DB_CDC_ExtractDeleted( DB_SYSTEM_CDC_EXTRACT *extract, DATABASE_SYSTEM_DB *db, DB_QUERY *data ) {
    if( extract && db && data ) {
        LOG_print( "\t· [CDC - EXTRACT::DELETED] existing data:\n" );
        CDC_ExtractGeneric( extract, &extract->deleted, db, data );
    } else {
        LOG_print( "\t· [CDC - EXTRACT::DELETED] Fatal error: not all DB_CDC_ExtractDeleted parameters are valid!\n" ); 
    }
}

void DB_CDC_ExtractModified( DB_SYSTEM_CDC_EXTRACT *extract, DATABASE_SYSTEM_DB *db, DB_QUERY *data ) {
    if( extract && db && data ) {
        LOG_print( "\t· [CDC - EXTRACT::MODIFIED] existing data:\n" );
        CDC_ExtractGeneric( extract, &extract->modified, db, data );
    } else {
        LOG_print( "\t· [CDC - EXTRACT::MODIFIED] Fatal error: not all DB_CDC_ExtractModified parameters are valid!\n" );
    }
}
