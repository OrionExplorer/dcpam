#ifndef APP_SCHEMA_H
#define APP_SCHEMA_H

#include "db/system_schema.h"
#include "../core/cache.h"

#define MAX_DCPAM_DATA_ITEMS                5
#define MAX_DCPAM_DATA_ACTIONS              8

typedef struct DCPAM_ALLOWED_HOST {
    char                    *ip;
    char                    *api_key;
} DCPAM_ALLOWED_HOST;

typedef struct LCS_REPORT {
    NET_CONN                *conn;
    char                    *address;
    int                     port;
    char                    *lcs_host;
    int                     lcs_port;
    LOG_OBJECT              *log;
    int                     active;
    char                    *component;
    char                    *version;
} LCS_REPORT;

typedef enum COMPONENT_ACTION_RESULT {
    DCR_SUCCESS = 1,
    DCR_FAILURE
} COMPONENT_ACTION_RESULT;

typedef enum COMPONENT_ACTION_TYPE {
    DCT_START = 1,
    DCT_STOP
} COMPONENT_ACTION_TYPE;


typedef struct COMPONENT_ACTION {
    char                    *description;               /* Action description */
    char                    start_timestamp[ 20 ];      /* Action started */
    char                    stop_timestamp[ 20 ];       /* Action finished */
    COMPONENT_ACTION_TYPE   type;                       /* Action type */
    COMPONENT_ACTION_RESULT success;                    /* Action result */
} COMPONENT_ACTION;


typedef struct DCPAM_COMPONENT {
    char                    *ip;                        /* Component IP address*/
    int                     port;                       /* Component socket port */
    char                    *name;                      /* Component name */
    char                    *version;                   /* Component version */
    int                     active;                     /* Is component active? */
    char                    timestamp[ 20 ];            /* Last verification (YYYY-MM-DD HH:mm:SS)*/
    COMPONENT_ACTION        **actions;                  /* Component actions list */
    int                     actions_len;                /* Component actions count */

} DCPAM_COMPONENT;

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

/*
    rdp_config.json => app
*/
typedef struct R_DCPAM_APP {
    char                    *version;
    char                    *name;
    int                     network_port;
    char                    **ALLOWED_HOSTS;
    DCPAM_ALLOWED_HOST      **ALLOWED_HOSTS_;
    int                     ALLOWED_HOSTS_len;
    LCS_REPORT              lcs_report; 
} R_DCPAM_APP;


/*
    lcs_config.json => app
*/
typedef struct L_DCPAM_APP {
    char                    *version;
    char                    *name;
    int                     network_port;
    DCPAM_COMPONENT         **COMPONENTS;
    int                     COMPONENTS_len;
    DCPAM_ALLOWED_HOST      **ALLOWED_HOSTS_;
    int                     ALLOWED_HOSTS_len;
} L_DCPAM_APP;

#endif
