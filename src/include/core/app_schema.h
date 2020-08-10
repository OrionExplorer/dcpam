#ifndef APP_SCHEMA_H
#define APP_SCHEMA_H

#include "db/system_schema.h"

/*
    config.json => app.DATA[ i ]
*/
typedef struct DCPAM_DATA_ACTION {
    char                    *name;
    char                    *description;
    int                     type;
    short                   internal;
    char                    *condition;
    char                    *sql;
} DCPAM_DATA_ACTION;

/*
    config.json => app.DATA
*/
typedef struct DCPAM_DATA {
    char                    *id;
    char                    *name;
    char                    *db_table_name;
    char                    *db_name;
    char                    columns[ MAX_ETL_COLUMNS ][ 32 ];
    int                     columns_len;
    char                    *description;
    DCPAM_DATA_ACTION       actions[ MAX_DCPAM_DATA_ACTIONS ];
    int                     actions_len;
} DCPAM_DATA;

/*
    config.json => app
*/
typedef struct DCPAM_APP {
    char                    *version;
    char                    *name;
    int                     run_once;
    DATABASE_SYSTEM_DB      DB;
    DATABASE_SYSTEM_DB      *STAGING;
    DCPAM_DATA              DATA[ MAX_DCPAM_DATA_ITEMS ];
    int                     DATA_len;
} DCPAM_APP;


typedef struct P_DCPAM_APP {
    char                    *version;
    char                    *name;
    DATABASE_SYSTEM_DB      **DB;
    int                     DB_len;
    DCPAM_DATA              DATA[ MAX_DCPAM_DATA_ITEMS ];
    int                     DATA_len;
    DB_CACHE                **CACHE;
    int                     CACHE_len;
} P_DCPAM_APP;

#endif
