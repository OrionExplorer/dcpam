#ifndef CDC_LOAD
#define CDC_LOAD

#include "../cdc_schema.h"
#include "../system_schema.h"

void DB_CDC_LoadInserted( DB_SYSTEM_CDC_LOAD *load, DATABASE_SYSTEM_DB *db );
void DB_CDC_LoadDeleted( DB_SYSTEM_CDC_LOAD *load, DATABASE_SYSTEM_DB *db );
void DB_CDC_LoadModified( DB_SYSTEM_CDC_LOAD *load, DATABASE_SYSTEM_DB *db );

#endif
