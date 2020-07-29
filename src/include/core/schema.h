#ifndef SCHEMA_H
#define SCHEMA_H

#define MAX_DCPAM_DATA_ITEMS                1
#define MAX_DCPAM_DATA_ACTIONS              8
#define MAX_SYSTEM_QUERIES                  32
#define MAX_DATA_SYSTEMS                    4
#define MAX_CDC_COLUMNS                     32

#include "db/etl_schema.h"
#include "db/system_schema.h"
#include "app_schema.h"

int                         DATABASE_SYSTEMS_COUNT;
DATABASE_SYSTEM             DATABASE_SYSTEMS[ MAX_DATA_SYSTEMS ];   /* Database-based systems | config.json => "system" */

DCPAM_APP                   APP;                                    /* Main application object | config.json => "app" */

#endif
