#ifndef DB_CDC_SCHEMA_H
#define DB_CDC_SCHEMA_H

#include "../../shared.h"

typedef enum DB_SYSTEM_CDC_ETL_STEP {
    ETL_EXTRACT = 1,
    ETL_TRANSFORM,
    ETL_LOAD
} DB_SYSTEM_CDC_ETL_STEP;

/*
    config.json => system[].queries[].change_data_capture.extract
*/
typedef struct DB_SYSTEM_CDC_EXTRACT_QUERY {
    char                    *primary_db;
    char                    *primary_db_sql;
    size_t                  primary_db_sql_len;
    char                    *secondary_db;
    char                    *secondary_db_sql;
    size_t                  secondary_db_sql_len;
} DB_SYSTEM_CDC_EXTRACT_QUERY;

typedef struct DB_SYSTEM_CDC_EXTRACT {
    DB_SYSTEM_CDC_EXTRACT_QUERY inserted;
    DB_SYSTEM_CDC_EXTRACT_QUERY deleted;
    DB_SYSTEM_CDC_EXTRACT_QUERY modified;
} DB_SYSTEM_CDC_EXTRACT;


/******************************************************************************/

/*
    config.json => system[].queries[].change_data_capture.stage
*/
typedef struct DB_SYSTEM_CDC_STAGE_QUERY {
    char                    *sql;
    size_t                  sql_len;
    char                    extracted_values[ MAX_CDC_COLUMNS ][ MAX_COLUMN_NAME_LEN ];
    int                     extracted_values_len;
} DB_SYSTEM_CDC_STAGE_QUERY;

typedef struct DB_SYSTEM_CDC_STAGE {
    DB_SYSTEM_CDC_STAGE_QUERY        inserted;
    DB_SYSTEM_CDC_STAGE_QUERY        deleted;
    DB_SYSTEM_CDC_STAGE_QUERY        modified;
} DB_SYSTEM_CDC_STAGE;

/******************************************************************************/

/*
    config.json => system[].queries[].change_data_capture.transform
*/
typedef struct DB_SYSTEM_CDC_TRANSFORM_QUERY {
    char                    *module;
    char                    *staged_data;
    char                    *source_system_update;
} DB_SYSTEM_CDC_TRANSFORM_QUERY;

typedef struct DB_SYSTEM_CDC_TRANSFORM {
    DB_SYSTEM_CDC_TRANSFORM_QUERY       **inserted;
    int                                 inserted_count;
    DB_SYSTEM_CDC_TRANSFORM_QUERY       **deleted;
    int                                 deleted_count;
    DB_SYSTEM_CDC_TRANSFORM_QUERY       **modified;
    int                                 modified_count;

} DB_SYSTEM_CDC_TRANSFORM;


/******************************************************************************/

/*
    config.json => system[].queries[].change_data_capture.load
*/
typedef struct DB_SYSTEM_CDC_LOAD_QUERY {
    char                    *input_data_sql;
    size_t                  input_data_sql_len;
    char                    *output_data_sql;
    size_t                  output_data_sql_len;
    char                    extracted_values[ MAX_CDC_COLUMNS ][ MAX_COLUMN_NAME_LEN ];
    int                     extracted_values_len;
} DB_SYSTEM_CDC_LOAD_QUERY;

typedef struct DB_SYSTEM_CDC_LOAD {
    DB_SYSTEM_CDC_LOAD_QUERY        inserted;
    DB_SYSTEM_CDC_LOAD_QUERY        deleted;
    DB_SYSTEM_CDC_LOAD_QUERY        modified;
} DB_SYSTEM_CDC_LOAD;


/*
    config.json => system[].queries[].change_data_capture.pre_actions
*/
typedef struct DB_SYSTEM_CDC_PRE {
    char                    *sql;
} DB_SYSTEM_CDC_PRE;

/*
    config.json => system[].queries[].change_data_capture.post_actions
*/
typedef struct DB_SYSTEM_CDC_POST {
    char                    *sql;
} DB_SYSTEM_CDC_POST;

/*
    config.json => system[].queries[].change_data_capture
*/
typedef struct DB_SYSTEM_CDC {
    DB_SYSTEM_CDC_PRE       **pre_actions;      /* PreCDC actions are optional */
    int                     pre_actions_count;
    DB_SYSTEM_CDC_EXTRACT   extract;
    DB_SYSTEM_CDC_STAGE     *stage;             /* Staging is optional */
    DB_SYSTEM_CDC_TRANSFORM *transform;         /* Transformation is optional */
    DB_SYSTEM_CDC_LOAD      load;
    DB_SYSTEM_CDC_POST      **post_actions;     /* PostCDC actions are optional */
    int                     post_actions_count;
} DB_SYSTEM_CDC;


#endif
