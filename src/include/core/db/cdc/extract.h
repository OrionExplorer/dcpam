#ifndef CDC_EXTRACT
#define CDC_EXTRACT

#include "../schema.h"

void DB_CDC_ExtractInserted( DB_SYSTEM_CDC_EXTRACT *extract, DATABASE_SYSTEM_DB *db, DB_QUERY *data );
void DB_CDC_ExtractDeleted( DB_SYSTEM_CDC_EXTRACT *extract, DATABASE_SYSTEM_DB *db, DB_QUERY *data );
void DB_CDC_ExtractModified( DB_SYSTEM_CDC_EXTRACT *extract, DATABASE_SYSTEM_DB *db, DB_QUERY *data )

#endif
