#include <stdio.h>
#include <stdlib.h>
#include "../../../include/utils/time.h"
#include "../../../include/utils/memory.h"
#include "../../../include/core/schema.h"

extern DCPAM_APP           APP;

void CDC_TransformGeneric( DB_SYSTEM_CDC_TRANSFORM *transform, DB_SYSTEM_CDC_TRANSFORM_QUERY *transform_element, DATABASE_SYSTEM_DB *db, DB_QUERY *data );


void CDC_TransformGeneric( DB_SYSTEM_CDC_TRANSFORM *transform, DB_SYSTEM_CDC_TRANSFORM_QUERY *transform_element, DATABASE_SYSTEM_DB *db, DB_QUERY *data ) {
}


void DB_CDC_TransformInserted( DB_SYSTEM_CDC_TRANSFORM *transform, DATABASE_SYSTEM_DB *db, DB_QUERY *data ) {
    if( transform && db && data ) {
        LOG_print( "\t· [CDC - TRANSFORM::INSERTED (%d rows)]:\n", data->row_count );
        if( data->row_count == 0 ) {
            LOG_print( "\t\t- Action canceled.\n" );
        } else {
            CDC_TransformGeneric( transform, &transform->inserted, db, data );
        }
    } else {
        LOG_print( "\t· [CDC - TRANSFORM::INSERTED] Fatal error: not all DB_CDC_TransformInserted parameters are valid!\n" );
    }
    
}

void DB_CDC_TransformDeleted( DB_SYSTEM_CDC_TRANSFORM *transform, DATABASE_SYSTEM_DB *db, DB_QUERY *data ) {
    if( transform && db && data ) {
        LOG_print( "\t· [CDC - TRANSFORM::DELETED (%d rows)]:\n", data->row_count );
        if( data->row_count == 0 ) {
            LOG_print( "\t\t- Action canceled.\n" );
        } else {
            CDC_TransformGeneric( transform, &transform->deleted, db, data );
        }
    } else {
        LOG_print( "\t· [CDC - TRANSFORM::DELETED] Fatal error: not all DB_CDC_TransformDeleted parameters are valid!\n" ); 
    }
}

void DB_CDC_TransformModified( DB_SYSTEM_CDC_TRANSFORM *transform, DATABASE_SYSTEM_DB *db, DB_QUERY *data ) {
    if( transform && db && data ) {
        LOG_print( "\t· [CDC - TRANSFORM::MODIFIED (%d rows)]:\n", data->row_count );
        if( data->row_count == 0 ) {
            LOG_print( "\t\t- Action canceled.\n" );
        } else {
            CDC_TransformGeneric( transform, &transform->modified, db, data );
        }
    } else {
        LOG_print( "\t· [CDC - TRANSFORM::MODIFIED] Fatal error: not all DB_CDC_TransformModified parameters are valid!\n" );
    }
}
