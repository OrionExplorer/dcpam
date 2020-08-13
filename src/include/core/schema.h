#ifndef SCHEMA_H
#define SCHEMA_H

#define MAX_DATA_SYSTEMS                    4

#include "db/etl_schema.h"
#include "db/system_schema.h"
#include "app_schema.h"

int                         DATABASE_SYSTEMS_COUNT;
DATABASE_SYSTEM             DATABASE_SYSTEMS[ MAX_DATA_SYSTEMS ];   /* Database-based systems | config.json => "system" */

DCPAM_APP                   APP;                                    /* Main application object | config.json => "app" */
P_DCPAM_APP                 P_APP;                                  /* Main application object | config.json => "app" */

#endif
