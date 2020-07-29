#ifndef CDC_LOAD
#define CDC_LOAD

#include "../etl_schema.h"
#include "../system_schema.h"

void _LoadGeneric_callback( DB_RECORD* record, void* data_ptr1, void* data_ptr2 );
void _LoadInserted_callback( DB_RECORD* record, void* data_ptr1, void* data_ptr2 );
void _LoadDeleted_callback( DB_RECORD* record, void* data_ptr1, void* data_ptr2 );
void _LoadModified_callback( DB_RECORD* record, void* data_ptr1, void* data_ptr2 );
void DB_CDC_LoadInserted( DB_SYSTEM_ETL_LOAD *load, DATABASE_SYSTEM_DB *source_db, DATABASE_SYSTEM_DB *dcpam_db );
void DB_CDC_LoadDeleted( DB_SYSTEM_ETL_LOAD *load, DATABASE_SYSTEM_DB *source_db, DATABASE_SYSTEM_DB *dcpam_db );
void DB_CDC_LoadModified( DB_SYSTEM_ETL_LOAD *load, DATABASE_SYSTEM_DB *source_db, DATABASE_SYSTEM_DB *dcpam_db );

#endif
