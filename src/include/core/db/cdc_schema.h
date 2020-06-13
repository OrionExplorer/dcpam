#ifndef DB_CDC_SCHEMA_H
#define DB_CDC_SCHEMA_H

#include "../../shared.h"

/*
    config.json => system[].queries[].change_data_capture.extract
*/
typedef struct DB_SYSTEM_CDC_EXTRACT_QUERY {
    char                    *primary_db;
    char                    *primary_db_sql;
    char                    *secondary_db;
    char                    *secondary_db_sql;
} DB_SYSTEM_CDC_EXTRACT_QUERY;

typedef struct DB_SYSTEM_CDC_EXTRACT {
    DB_SYSTEM_CDC_EXTRACT_QUERY inserted;
    DB_SYSTEM_CDC_EXTRACT_QUERY deleted;
    DB_SYSTEM_CDC_EXTRACT_QUERY modified;
} DB_SYSTEM_CDC_EXTRACT;


/******************************************************************************/

/*
    config.json => system[].queries[].change_data_capture.transform
*/
typedef struct DB_SYSTEM_CDC_TRANSFORM_QUERY {
    char                    *column;
    char                    *expression;
} DB_SYSTEM_CDC_TRANSFORM_QUERY;

typedef struct DB_SYSTEM_CDC_TRANSFORM {
    DB_SYSTEM_CDC_TRANSFORM_QUERY       inserted[ MAX_TRANSFORM_ELEMENTS ];
    DB_SYSTEM_CDC_TRANSFORM_QUERY       deleted[ MAX_TRANSFORM_ELEMENTS ];
    DB_SYSTEM_CDC_TRANSFORM_QUERY       modified[ MAX_TRANSFORM_ELEMENTS ];

} DB_SYSTEM_CDC_TRANSFORM;


/******************************************************************************/

/*
    config.json => system[].queries[].change_data_capture.load
*/
typedef struct DB_SYSTEM_CDC_LOAD_QUERY {
    char                    *sql;
    char                    extracted_values[ MAX_CDC_COLUMNS ][ MAX_COLUMN_NAME_LEN ];
    int                     extracted_values_len;
} DB_SYSTEM_CDC_LOAD_QUERY;

typedef struct DB_SYSTEM_CDC_LOAD {
    DB_SYSTEM_CDC_LOAD_QUERY        inserted;
    DB_SYSTEM_CDC_LOAD_QUERY        deleted;
    DB_SYSTEM_CDC_LOAD_QUERY        modified;
} DB_SYSTEM_CDC_LOAD;

/*
    config.json => system[].queries[].change_data_capture
*/
typedef struct DB_SYSTEM_CDC {
    DB_SYSTEM_CDC_EXTRACT   extract;
    DB_SYSTEM_CDC_TRANSFORM transform;
    DB_SYSTEM_CDC_LOAD      load;
} DB_SYSTEM_CDC;


#endif
