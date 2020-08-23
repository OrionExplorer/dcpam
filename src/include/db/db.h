#ifndef DB_H
#define DB_H

#include "../shared.h"
#include "../utils/log.h"

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
    int             field_count;
} DB_RECORD;

typedef struct {
    int             row_count;
    int             field_count;
    DB_RECORD       *records;
    char            *sql;
} DB_QUERY;

typedef void ( *qec )( DB_RECORD*, void *data_ptr1, void *data_ptr2, LOG_OBJECT *log ); /* Query Exec Callback */


void            DB_QUERY_init( DB_QUERY *db_query );
void            DB_QUERY_free( DB_QUERY *db_query );
void            DB_QUERY_record_free( DB_RECORD *record );
int             DB_QUERY_format( const char *src, char **dst, size_t *dst_length, const char* const* param_values, const int params_count, const int *param_lengths, LOG_OBJECT *log );
DB_QUERY_TYPE   DB_QUERY_get_type( const char *sql );

#endif
