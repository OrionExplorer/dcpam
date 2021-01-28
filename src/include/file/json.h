#ifndef JSON_H
#define JSON_H

#include "../shared.h"
#include "../utils/log.h"
#include "../core/network/client.h"

typedef struct {
    char            label[ MAX_COLUMN_NAME_LEN ];
    char            *value;
    unsigned long   size;
    char            type[ 16 ];
} JSON_FIELD;

typedef struct {
    JSON_FIELD       *fields;
    int             field_count;
} JSON_RECORD;

typedef struct {
    int             row_count;
    int             field_count;
    JSON_RECORD      *records;
    char            *file_name;
    int             loaded;
} JSON_FILE;

typedef void ( *jlc )( JSON_RECORD*, void* data_ptr1, void* data_ptr2, LOG_OBJECT* log ); /* JSON Load Callback */


int JSON_FILE_load( JSON_FILE* dst, const char* filename, HTTP_DATA* http_data, jlc* json_load_callback, void *data_ptr1, void *data_ptr2, LOG_OBJECT* log );


#endif
