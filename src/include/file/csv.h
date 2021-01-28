#ifndef CSV_H
#define CSV_H

#include "../shared.h"
#include "../utils/log.h"
#include "../core/network/client.h"

typedef struct {
    char            label[ MAX_COLUMN_NAME_LEN ];
    char            *value;
    unsigned long   size;
    char            type[ 16 ];
} CSV_FIELD;

typedef struct {
    CSV_FIELD       *fields;
    int             field_count;
} CSV_RECORD;

typedef struct {
    int             row_count;
    int             field_count;
    CSV_RECORD      *records;
    char            *file_name;
    int             loaded;
    char            delimiter[1];
} CSV_FILE;

typedef void ( *clc )( CSV_RECORD*, void* data_ptr1, void* data_ptr2, LOG_OBJECT* log ); /* CSV Load Callback */


int CSV_FILE_load( CSV_FILE* dst, const char* filename, HTTP_DATA* http_data, clc* csv_load_callback, void *data_ptr1, void *data_ptr2, LOG_OBJECT* log );


#endif
