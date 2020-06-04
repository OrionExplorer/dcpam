#ifndef DB_H
#define DB_H

#include "../shared.h"

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

#endif
