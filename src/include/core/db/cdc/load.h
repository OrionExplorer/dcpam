#ifndef CDC_LOAD
#define CDC_LOAD

#include "../schema.h"

void DB_CDC_LoadInserted( DB_SYSTEM_CDC_LOAD *load, DATABASE_SYSTEM_DB *db, DB_QUERY *data );
void DB_CDC_LoadDeleted( DB_SYSTEM_CDC_LOAD *load, DATABASE_SYSTEM_DB *db, DB_QUERY *data );
void DB_CDC_LoadModified( DB_SYSTEM_CDC_LOAD *load, DATABASE_SYSTEM_DB *db, DB_QUERY *data )

#endif
