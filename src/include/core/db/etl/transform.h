#ifndef CDC_TRANSFORM
#define CDC_TRANSFORM

#include "../etl_schema.h"
#include "../system_schema.h"

int DB_CDC_TransformInserted( DB_SYSTEM_ETL_TRANSFORM_QUERY **transform, const int count, DATABASE_SYSTEM_DB* dcpam_db, DATABASE_SYSTEM_DB* system_db, LOG_OBJECT* log );
int DB_CDC_TransformDeleted( DB_SYSTEM_ETL_TRANSFORM_QUERY **transform, const int count, DATABASE_SYSTEM_DB* dcpam_db, DATABASE_SYSTEM_DB* system_db, LOG_OBJECT* log );
int DB_CDC_TransformModified( DB_SYSTEM_ETL_TRANSFORM_QUERY **transform, const int count, DATABASE_SYSTEM_DB* dcpam_db, DATABASE_SYSTEM_DB* system_db, LOG_OBJECT* log );

#endif
