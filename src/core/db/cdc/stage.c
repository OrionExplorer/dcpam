#include <stdio.h>
#include <stdlib.h>
#include "../../../include/utils/time.h"
#include "../../../include/utils/memory.h"
#include "../../../include/core/schema.h"
#include "../../../include/core/db/system.h"
#include "../../../include/utils/log.h"
#include "../../../include/core/db/cdc/stage.h"

extern DCPAM_APP           APP;

void DB_CDC_StageGeneric( DB_SYSTEM_CDC_STAGE *stage, DB_SYSTEM_CDC_STAGE_QUERY *stage_element, DATABASE_SYSTEM_DB *db, DB_RECORD *record );


void DB_CDC_StageGeneric( DB_SYSTEM_CDC_STAGE *stage, DB_SYSTEM_CDC_STAGE_QUERY *stage_element, DATABASE_SYSTEM_DB *db, DB_RECORD *record ) {
    //DB_QUERY            sql_res;
    
    if( stage && stage_element && db && record ) {

        /* Each extracted record is loaded separatedly */
        //for( int i = 0; i < data->row_count; i++ ) {

            char **q_values = NULL;
            int *q_lengths;
            int *q_formats;

            /* Prepare query data. PostgreSQL compatibile for now only. */
            q_values = SAFEMALLOC( ( stage_element->extracted_values_len ) * sizeof *q_values, __FILE__, __LINE__ );
            int q_values_len = 0;

            q_lengths = SAFEMALLOC( (stage_element->extracted_values_len) * sizeof( int ), __FILE__, __LINE__ );
            q_formats = SAFEMALLOC( (stage_element->extracted_values_len) * sizeof( int ), __FILE__, __LINE__ );

            /* Get defined values only based on "extracted_values" in config.json */
            for( int k = 0; k < stage_element->extracted_values_len; k++ ) {
                for( int j = 0; j < record->field_count; j++ ) {

                    if( strncmp( stage_element->extracted_values[ k ], record->fields[ j ].label, MAX_COLUMN_NAME_LEN ) == 0 ) {
                        if( record->fields[ j ].size > 0 ) {
                            q_values[ q_values_len ] = SAFECALLOC( ( record->fields[ j ].size + 1 ), sizeof **q_values, __FILE__, __LINE__ );
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

                int query_ret = DB_exec( &APP.DB, stage_element->sql, stage_element->sql_len, NULL, ( const char* const* )q_values, q_values_len, q_lengths, q_formats, NULL, NULL, NULL, NULL );

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
        //}
    }
}

void DB_CDC_StageReset( DB_SYSTEM_CDC_STAGE* stage, DATABASE_SYSTEM_DB* db ) {
    if( stage && db ) {

        DB_QUERY    sql_res;

        LOG_print( "\t· [CDC - STAGE::RESET]:\n" );
        LOG_print( "\t\t- %s\n", stage->reset );

        if( DB_exec( &APP.DB, stage->reset, strlen( stage->reset ), &sql_res, NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL ) == TRUE ) {
            LOG_print( "\t· [CDC - STAGE::RESET] Done. Staging Area cleared.\n" );
        } else {
            LOG_print( "\t· [CDC - STAGE::RESET] Error. DCPAM was unable to clear Staging Area. Check corresponding error in previous messages.\n" );
        }

        DB_QUERY_free( &sql_res );

    } else {
        LOG_print( "\t· [CDC - STAGE::RESET] Fatal error: not all DB_CDC_StageReset parameters are valid!\n" );
    }
}

void DB_CDC_StageInserted( DB_SYSTEM_CDC_STAGE* stage, DATABASE_SYSTEM_DB* db, DB_RECORD *record ) {
    if( stage && db ) {
        LOG_print( "\t· [CDC - STAGE::INSERTED]:\n" );
        DB_CDC_StageGeneric( stage, &stage->inserted, db, record );
    } else {
        LOG_print( "\t· [CDC - STAGE::INSERTED] Fatal error: not all DB_CDC_StageInserted parameters are valid!\n" );
    }
    
}

void DB_CDC_StageDeleted( DB_SYSTEM_CDC_STAGE *stage, DATABASE_SYSTEM_DB *db, DB_RECORD* record ) {
    if( stage && db ) {
        LOG_print( "\t· [CDC - STAGE::DELETED]:\n" );
        DB_CDC_StageGeneric( stage, &stage->deleted, db, record );
    } else {
        LOG_print( "\t· [CDC - STAGE::DELETED] Fatal error: not all DB_CDC_StageDeleted parameters are valid!\n" );   
    }
}

void DB_CDC_StageModified( DB_SYSTEM_CDC_STAGE *stage, DATABASE_SYSTEM_DB *db, DB_RECORD* record ) {
    if( stage && db ) {
        LOG_print( "\t· [CDC - STAGE::MODIFIED]:\n" );
        DB_CDC_StageGeneric( stage, &stage->modified, db, record );
    } else {
        LOG_print( "\t· [CDC - STAGE::MODIFIED] Fatal error: not all DB_CDC_StageModified parameters are valid!\n" );
    }
}
