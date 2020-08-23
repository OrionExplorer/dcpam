#ifndef CDC_LOAD
#define CDC_LOAD

#include "../etl_schema.h"
#include "../system_schema.h"

void _LoadGeneric_callback( DB_RECORD* record, void* data_ptr1, void* data_ptr2, LOG_OBJECT* log );
void _LoadInserted_callback( DB_RECORD* record, void* data_ptr1, void* data_ptr2, LOG_OBJECT* log );
void _LoadDeleted_callback( DB_RECORD* record, void* data_ptr1, void* data_ptr2, LOG_OBJECT* log );
void _LoadModified_callback( DB_RECORD* record, void* data_ptr1, void* data_ptr2, LOG_OBJECT* log );
int DB_CDC_LoadInserted( DB_SYSTEM_ETL_LOAD *load, DATABASE_SYSTEM_DB *source_db, DATABASE_SYSTEM_DB *dcpam_db, LOG_OBJECT* log );
int DB_CDC_LoadDeleted( DB_SYSTEM_ETL_LOAD *load, DATABASE_SYSTEM_DB *source_db, DATABASE_SYSTEM_DB *dcpam_db, LOG_OBJECT* log );
int DB_CDC_LoadModified( DB_SYSTEM_ETL_LOAD *load, DATABASE_SYSTEM_DB *source_db, DATABASE_SYSTEM_DB *dcpam_db, LOG_OBJECT* log );

#endif
