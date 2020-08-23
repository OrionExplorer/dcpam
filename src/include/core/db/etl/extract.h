#ifndef CDC_EXTRACT
#define CDC_EXTRACT

#include "../etl_schema.h"
#include "../system_schema.h"


void _ExtractGeneric_callback( DB_RECORD* record, DB_SYSTEM_ETL_STAGE* stage, DB_SYSTEM_ETL_STAGE_QUERY* stage_element, DATABASE_SYSTEM_DB* db, LOG_OBJECT* log );
void _ExtractInserted_callback( DB_RECORD* record, void* data_ptr1, void* data_ptr2, LOG_OBJECT* log );
void _ExtractDeleted_callback( DB_RECORD* record, void* data_ptr1, void* data_ptr2, LOG_OBJECT* log );
void _ExtractModified_callback( DB_RECORD* record, void* data_ptr1, void* data_ptr2, LOG_OBJECT* log );
int DB_CDC_ExtractInserted( DB_SYSTEM_ETL_EXTRACT *extract, DATABASE_SYSTEM_DB *system_db, DATABASE_SYSTEM_DB *dcpam_db, qec *query_exec_callback, void *data_ptr1, void *data_ptr2, LOG_OBJECT* log );
int DB_CDC_ExtractDeleted( DB_SYSTEM_ETL_EXTRACT *extract, DATABASE_SYSTEM_DB *system_db, DATABASE_SYSTEM_DB* dcpam_db, qec *query_exec_callback, void* data_ptr1, void* data_ptr2, LOG_OBJECT* log );
int DB_CDC_ExtractModified( DB_SYSTEM_ETL_EXTRACT *extract, DATABASE_SYSTEM_DB *system_db, DATABASE_SYSTEM_DB* dcpam_db, qec *query_exec_callback, void* data_ptr1, void* data_ptr2, LOG_OBJECT* log );

#endif
