#ifndef CDC_TRANSFORM
#define CDC_TRANSFORM

#include "../schema.h"

void DB_CDC_TransformInserted( DB_SYSTEM_CDC_TRANSFORM *extract, DATABASE_SYSTEM_DB *db, DB_QUERY *data );
void DB_CDC_TransformDeleted( DB_SYSTEM_CDC_TRANSFORM *extract, DATABASE_SYSTEM_DB *db, DB_QUERY *data );
void DB_CDC_TransformModified( DB_SYSTEM_CDC_TRANSFORM *extract, DATABASE_SYSTEM_DB *db, DB_QUERY *data )

#endif
