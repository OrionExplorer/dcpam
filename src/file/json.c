#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/file/json.h"
#include "../include/utils/time.h"
#include "../include/utils/memory.h"

int JSON_FILE_load( JSON_FILE* dst, const char* filename, jlc* json_load_callback, void *data_ptr1, void *data_ptr2, LOG_OBJECT* log ) {

    LOG_print( log, "[%s] JSON_FILE_load( %s )...\n", TIME_get_gmt(), filename );
    /*FILE* json_f = fopen( filename , "r" );

    if( json_f == NULL ) {
        LOG_print( log, "[%s] JSON_FILE_load failed.\n", TIME_get_gmt() );
        return 0;
    }

    char        buf[ 1024 ];
    int         row_count = 0;
    char        **columns = NULL;
    char        **row_values = NULL;
    JSON_RECORD *json_columns = SAFEMALLOC( sizeof( JSON_RECORD ), __FILE__, __LINE__ );

    fclose( json_f );*/
    return 1;
}
