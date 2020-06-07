#ifndef DB_H
#define DB_H

#include "../shared.h"

typedef enum {
    DQT_SELECT,
    DQT_INSERT,
    DQT_UPDATE,
    DQT_DELETE,
    DQT_ALTER,
    DQT_DROP,
    DQT_CREATE,
    DQT_USE,
    DQT_SHOW,
    DQT_UNKNOWN
} DB_QUERY_TYPE;

typedef struct {
    char            label[ MAX_COLUMN_NAME_LEN ];
    char            *value;
    unsigned long   size;
    char            type[ 16 ];
} DB_FIELD;

typedef struct {
    DB_FIELD        *fields;
} DB_RECORD;

typedef struct {
    int             row_count;
    int             field_count;
    DB_RECORD       *records;
    char            *sql;
} DB_QUERY;


void DB_QUERY_init( DB_QUERY *db_query );
void DB_QUERY_free( DB_QUERY *db_query );
void DB_QUERY_field_type( DB_FIELD *field, char *dst );
int  DB_QUERY_format( const char *src, char **dst, unsigned long *dst_length, const char* const* param_values, const int params_count, const int *param_lengths );
DB_QUERY_TYPE DB_QUERY_get_type( const char *sql );

#endif
