#ifndef CDC_TRANSFORM
#define CDC_TRANSFORM

#include "../cdc_schema.h"
#include "../system_schema.h"

void DB_CDC_TransformInserted( DB_SYSTEM_CDC_TRANSFORM *transform, DATABASE_SYSTEM_DB *db, DB_QUERY *data );
void DB_CDC_TransformDeleted( DB_SYSTEM_CDC_TRANSFORM *transform, DATABASE_SYSTEM_DB *db, DB_QUERY *data );
void DB_CDC_TransformModified( DB_SYSTEM_CDC_TRANSFORM *transform, DATABASE_SYSTEM_DB *db, DB_QUERY *data );

#endif
