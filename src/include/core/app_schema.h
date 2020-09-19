#ifndef APP_SCHEMA_H
#define APP_SCHEMA_H

#include "db/system_schema.h"
#include "../core/cache.h"
#include "../core/lcs_report.h"
#include "../core/component.h"

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
    char                    *db_name;
    char                    columns[ MAX_ETL_COLUMNS ][ MAX_COLUMN_NAME_LEN ];
    int                     columns_len;
    char                    *description;
    DCPAM_DATA_ACTION       actions[ MAX_DCPAM_DATA_ACTIONS ];
    int                     actions_len;
} DCPAM_DATA;

/*
    etl_config.json => app
*/
typedef struct DCPAM_APP {
    char                    *version;
    char                    *name;
    int                     run_once;
    DATABASE_SYSTEM_DB      DB;
    DATABASE_SYSTEM_DB      *STAGING;
    DCPAM_DATA              DATA[ MAX_DCPAM_DATA_ITEMS ];
    int                     DATA_len;
    LCS_REPORT              lcs_report;
} DCPAM_APP;

/*
    wds_config.json => app
*/
typedef struct P_DCPAM_APP {
    char                    *version;
    char                    *name;
    int                     network_port;
    DATABASE_SYSTEM_DB      **DB;
    int                     DB_len;
    DCPAM_DATA              DATA[ MAX_DCPAM_DATA_ITEMS ];
    int                     DATA_len;
    D_CACHE                 **CACHE;
    int                     CACHE_len;
    size_t                  CACHE_size;
    size_t                  CACHE_MAX_size;
    char                    **ALLOWED_HOSTS;
    DCPAM_ALLOWED_HOST      **ALLOWED_HOSTS_;
    int                     ALLOWED_HOSTS_len;
    LCS_REPORT              lcs_report;
} P_DCPAM_APP;

#endif
