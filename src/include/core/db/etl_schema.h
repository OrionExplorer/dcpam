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

#ifndef DB_ETL_SCHEMA_H
#define DB_ETL_SCHEMA_H

#include "../../shared.h"
#include "../network/client.h"

#define MAX_ETL_COLUMNS                     32

typedef enum DB_SYSTEM_ETL_STEP {
    ETL_EXTRACT = 1,
    ETL_TRANSFORM,
    ETL_LOAD
} DB_SYSTEM_ETL_STEP;

typedef enum DB_SYSTEM_MODE {
    M_ETL = 1,
    M_ELT
} DB_SYSTEM_MODE;

/*
    config.json => system[].queries[].etl.extract
*/
typedef struct DB_SYSTEM_ETL_EXTRACT_QUERY {
    char                    *primary_db;
    char                    *primary_db_sql;
    size_t                  primary_db_sql_len;
    char                    *secondary_db;
    char                    *secondary_db_sql;
    size_t                  secondary_db_sql_len;
} DB_SYSTEM_ETL_EXTRACT_QUERY;

typedef struct DB_SYSTEM_ETL_EXTRACT {
    DB_SYSTEM_ETL_EXTRACT_QUERY inserted;
    DB_SYSTEM_ETL_EXTRACT_QUERY deleted;
    DB_SYSTEM_ETL_EXTRACT_QUERY modified;
} DB_SYSTEM_ETL_EXTRACT;


/******************************************************************************/

/*
    config.json => system[].queries[].etl.stage
*/
typedef struct DB_SYSTEM_ETL_STAGE_QUERY {
    char                    *sql;
    size_t                  sql_len;
    char                    extracted_values[ MAX_ETL_COLUMNS ][ MAX_COLUMN_NAME_LEN ];
    int                     extracted_values_len;
} DB_SYSTEM_ETL_STAGE_QUERY;

typedef struct DB_SYSTEM_ETL_STAGE {
    DB_SYSTEM_ETL_STAGE_QUERY        inserted;
    DB_SYSTEM_ETL_STAGE_QUERY        deleted;
    DB_SYSTEM_ETL_STAGE_QUERY        modified;
} DB_SYSTEM_ETL_STAGE;

/******************************************************************************/

/*
    config.json => system[].queries[].etl.transform
*/
typedef struct DB_SYSTEM_ETL_TRANSFORM_QUERY {
    char                    *module;
    char                    *staged_data;
    char                    *source_system_update;
    NET_CONN                *connection;
    char                    *api_key;
    
} DB_SYSTEM_ETL_TRANSFORM_QUERY;

typedef struct DB_SYSTEM_ETL_TRANSFORM {
    DB_SYSTEM_ETL_TRANSFORM_QUERY       **inserted;
    int                                 inserted_count;
    DB_SYSTEM_ETL_TRANSFORM_QUERY       **deleted;
    int                                 deleted_count;
    DB_SYSTEM_ETL_TRANSFORM_QUERY       **modified;
    int                                 modified_count;

} DB_SYSTEM_ETL_TRANSFORM;


/******************************************************************************/

/*
    config.json => system[].queries[].etl.load
*/
typedef struct DB_SYSTEM_ETL_LOAD_QUERY {
    char                    *input_data_sql;
    size_t                  input_data_sql_len;
    char                    *output_data_sql;
    size_t                  output_data_sql_len;
    char                    extracted_values[ MAX_ETL_COLUMNS ][ MAX_COLUMN_NAME_LEN ];
    int                     extracted_values_len;
} DB_SYSTEM_ETL_LOAD_QUERY;

typedef struct DB_SYSTEM_ETL_LOAD {
    DB_SYSTEM_ETL_LOAD_QUERY        inserted;
    DB_SYSTEM_ETL_LOAD_QUERY        deleted;
    DB_SYSTEM_ETL_LOAD_QUERY        modified;
} DB_SYSTEM_ETL_LOAD;


/*
    config.json => system[].queries[].etl.pre_actions
*/
typedef struct DB_SYSTEM_ETL_PRE {
    char                    *sql;
} DB_SYSTEM_ETL_PRE;

/*
    config.json => system[].queries[].etl.post_actions
*/
typedef struct DB_SYSTEM_ETL_POST {
    char                    *sql;
} DB_SYSTEM_ETL_POST;

/*
    config.json => system[].queries[].etl
*/
typedef struct DB_SYSTEM_ETL {
    DB_SYSTEM_ETL_PRE       **pre_actions;      /* PreCDC actions are optional */
    int                     pre_actions_count;
    DB_SYSTEM_ETL_EXTRACT   extract;
    DB_SYSTEM_ETL_STAGE     *stage;             /* Staging is optional */
    DB_SYSTEM_ETL_TRANSFORM *transform;         /* Transformation is optional */
    DB_SYSTEM_ETL_LOAD      load;
    DB_SYSTEM_ETL_POST      **post_actions;     /* PostCDC actions are optional */
    int                     post_actions_count;
} DB_SYSTEM_ETL;


#endif
