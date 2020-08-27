#ifndef CSV_H
#define CSV_H

#include "../shared.h"


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
} CSV_FILE;


int CSV_FILE_load( CSV_FILE *dst, const char *filename );


#endif
