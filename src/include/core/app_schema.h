/* Copyright (C) 2020-2021 Marcin Kelar

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335 USA */

#ifndef APP_SCHEMA_H
#define APP_SCHEMA_H

#include "db/system_schema.h"
#include "../core/cache.h"
#include "../core/lcs_report.h"
#include "../core/component.h"

typedef enum MEM_UNIT {
    MU_KB = 1,
    MU_MB,
    MU_GB,
    MU_TB
} MEM_UNIT;

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
    char                    db_table_name[ MAX_TABLE_NAME_LEN ];
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
    MEM_UNIT                CACHE_size_unit;
    int                     CACHE_size_multiplier;
    char                    **ALLOWED_HOSTS;
    DCPAM_ALLOWED_HOST      **ALLOWED_HOSTS_;
    int                     ALLOWED_HOSTS_len;
    LCS_REPORT              lcs_report;
} P_DCPAM_APP;

#endif
