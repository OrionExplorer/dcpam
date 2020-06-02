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
    char                    columns[ MAX_CDC_COLUMNS ][ 32 ];
    int                     columns_len;
    char                    *description;
    DCPAM_DATA_ACTION      actions[ MAX_DCPAM_DATA_ACTIONS ];
    int                     actions_len;
} DCPAM_DATA;

/*
    config.json => app
*/
typedef struct DCPAM_APP {
    char                    *version;
    char                    *name;
    DATABASE_SYSTEM_DB      DB;
    DCPAM_DATA             DATA[ MAX_DCPAM_DATA_ITEMS ];
    int                     DATA_len;
} DCPAM_APP;


#endif
