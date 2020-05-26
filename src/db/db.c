#include "../include/utils/time.h"
#include "../include/utils/log.h"
#include "../include/db/db.h"
#include <stdlib.h>


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


void DB_QUERY_field_type( DB_FIELD *field, char *dst ) {
    
}