#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/utils/filesystem.h"
#include "../include/utils/strings.h"
#include "../include/file/json.h"
#include "../include/utils/time.h"
#include "../include/utils/memory.h"
#include "../include/third-party/cJSON.h"

int JSON_FILE_load( JSON_FILE* dst, const char* filename, jlc* json_load_callback, void *data_ptr1, void *data_ptr2, LOG_OBJECT* log ) {
    cJSON   *json_file = NULL;
    char    *file_content = NULL;

    LOG_print( log, "[%s] JSON_FILE_load( %s )...\n", TIME_get_gmt(), filename );

    /* HTTP protocol */
    if( strstr( filename, "http://" ) ) {
        char* tmp_file_name = mkrndstr( 16 );
        LOG_print( log, "[%s] JSON_FILE_load temporary file name: %s\n", TIME_get_gmt(), tmp_file_name );

        if( FILE_download( filename, tmp_file_name, "wb", log ) == 1 ) {
            file_content = file_get_content( tmp_file_name );
            free( tmp_file_name ); tmp_file_name = NULL;
            if( file_content ) {
                json_file = cJSON_Parse( file_content );
            } else {
                LOG_print( log, "[%s] JSON_FILE_load error: unable to read from file %s.\n", TIME_get_gmt(), filename );
            }
        } else {
            free( tmp_file_name ); tmp_file_name = NULL;
            return 0;
        }
    } else {
        file_content = file_get_content( filename );
        if( file_content ) {
            json_file = cJSON_Parse( file_content );
        } else {
            LOG_print( log, "[%s] JSON_FILE_load error: unable to read from file %s.\n", TIME_get_gmt(), filename );
        }
    }

    if( json_file && file_content ) {
        int         row_count = 0;
        char        **columns = NULL;
        char        **row_values = NULL;
        JSON_RECORD *json_columns = SAFEMALLOC( sizeof( JSON_RECORD ), __FILE__, __LINE__ );

        free( file_content ); file_content = NULL;
        cJSON_Delete( json_file );
    }

    return 0;
}
