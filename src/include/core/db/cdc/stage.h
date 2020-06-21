#ifndef CDC_STAGE
#define CDC_STAGE

#include "../cdc_schema.h"
#include "../system_schema.h"

/*
	We reuse DB_SYSTEM_CDC_STAGE because technically both processes are the same.
*/
void DB_CDC_StageInserted( DB_SYSTEM_CDC_STAGE *stage, DATABASE_SYSTEM_DB *db, DB_QUERY *data );
void DB_CDC_StageDeleted( DB_SYSTEM_CDC_STAGE *stage, DATABASE_SYSTEM_DB *db, DB_QUERY *data );
void DB_CDC_StageModified( DB_SYSTEM_CDC_STAGE *stage, DATABASE_SYSTEM_DB *db, DB_QUERY *data );

#endif
