#ifndef SCHEMA_H
#define SCHEMA_H

#include "db/etl_schema.h"
#include "db/system_schema.h"
#include "app_schema.h"
#include "../shared.h"
#include "../DCPAM_LCS/dcpam-lcs.h"

int                         DATABASE_SYSTEMS_COUNT;
DATABASE_SYSTEM             DATABASE_SYSTEMS[ MAX_DATA_SYSTEMS ];   /* Database-based systems | config.json => "system" */

DCPAM_APP                   APP;                                    /* Main application object | config.json => "app" */
P_DCPAM_APP                 P_APP;                                  /* Main application object | config.json => "app" */
L_DCPAM_APP                 L_APP;                                  /* Main application object | config.json => "app" */

#endif
