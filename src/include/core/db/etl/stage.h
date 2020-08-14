#ifndef CDC_STAGE
#define CDC_STAGE

#include "../etl_schema.h"
#include "../system_schema.h"

/*
	We reuse DB_SYSTEM_ETL_STAGE because technically both processes are the same.
*/
void DB_CDC_StageGeneric( DB_SYSTEM_ETL_STAGE* stage, DB_SYSTEM_ETL_STAGE_QUERY* stage_element, DATABASE_SYSTEM_DB* db, DB_RECORD* record );
void DB_CDC_StageInserted( DB_SYSTEM_ETL_STAGE *stage, DATABASE_SYSTEM_DB *db, DB_RECORD *record );
void DB_CDC_StageDeleted( DB_SYSTEM_ETL_STAGE *stage, DATABASE_SYSTEM_DB *db, DB_RECORD* record );
void DB_CDC_StageModified( DB_SYSTEM_ETL_STAGE *stage, DATABASE_SYSTEM_DB *db, DB_RECORD* record );

#endif
