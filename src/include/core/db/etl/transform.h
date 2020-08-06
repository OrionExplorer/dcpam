#ifndef CDC_TRANSFORM
#define CDC_TRANSFORM

#include "../etl_schema.h"
#include "../system_schema.h"

void DB_CDC_TransformInserted( DB_SYSTEM_ETL_TRANSFORM_QUERY **transform, const int count, DATABASE_SYSTEM_DB* dcpam_db, DATABASE_SYSTEM_DB* system_db );
void DB_CDC_TransformDeleted( DB_SYSTEM_ETL_TRANSFORM_QUERY **transform, const int count, DATABASE_SYSTEM_DB* dcpam_db, DATABASE_SYSTEM_DB* system_db );
void DB_CDC_TransformModified( DB_SYSTEM_ETL_TRANSFORM_QUERY **transform, const int count, DATABASE_SYSTEM_DB* dcpam_db, DATABASE_SYSTEM_DB* system_db );

#endif
