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
        cJSON*      record_data = NULL;

        cJSON_ArrayForEach( record_data, json_file ) {
            cJSON       *field = NULL;
            row_count++;
            
            if( row_count == 1 ) {
                /* Get all the key names first */
                json_columns->field_count = cJSON_GetArraySize( record_data );
                json_columns->fields = SAFEMALLOC( json_columns->field_count * sizeof( JSON_FIELD ), __FILE__, __LINE__ );

                for( int i = 0; i < json_columns->field_count; i++ ) {
                    cJSON *json_field_name = cJSON_GetArrayItem( record_data, i );
                    strncpy( json_columns->fields[ i ].label, json_field_name->string, MAX_COLUMN_NAME_LEN );
                }
            }

            /* Get values */

            JSON_RECORD* json_record = SAFEMALLOC( sizeof( JSON_RECORD ), __FILE__, __LINE__ );
            json_record->field_count = json_columns->field_count;
            json_record->fields = SAFEMALLOC( json_record->field_count * sizeof( JSON_FIELD ), __FILE__, __LINE__ );

            for( int i = 0; i < json_columns->field_count; i++ ) {
                cJSON* record_field = cJSON_GetArrayItem( record_data, i );
                strncpy( json_record->fields[ i ].label, json_columns->fields[ i ].label, MAX_COLUMN_NAME_LEN );

                if( cJSON_IsString( record_field ) ) {

                    json_record->fields[ i ].size = strlen( record_field->valuestring );
                    json_record->fields[ i ].value = SAFECALLOC( json_record->fields[ i ].size + 1, sizeof( char ), __FILE__, __LINE__ );
                    strncpy( json_record->fields[ i ].value, record_field->valuestring, json_record->fields[ i ].size );

                } else if( cJSON_IsNumber( record_field ) ) {
                    char        tmp_s_val[ 64 ];
                    double      tmp_d_val = cJSON_GetNumberValue( record_field );
                    int         tmp_i_val = ( int )tmp_d_val;

                    snprintf( tmp_s_val, sizeof( tmp_s_val ), "%d", tmp_i_val );

                    json_record->fields[ i ].size = strlen( tmp_s_val );
                    json_record->fields[ i ].value = SAFECALLOC( json_record->fields[ i ].size + 1, sizeof( char ), __FILE__, __LINE__ );
                    strncpy( json_record->fields[ i ].value, tmp_s_val, json_record->fields[ i ].size );

                } else if( cJSON_IsBool( record_field ) ) {

                    if( record_field->valueint == 1 ) {
                        json_record->fields[ i ].size = strlen( "true" );
                        json_record->fields[ i ].value = SAFECALLOC( json_record->fields[ i ].size + 1, sizeof( char ), __FILE__, __LINE__ );
                        strncpy( json_record->fields[ i ].value, "true", json_record->fields[ i ].size );
                    } else {
                        json_record->fields[ i ].size = strlen( "false" );
                        json_record->fields[ i ].value = SAFECALLOC( json_record->fields[ i ].size + 1, sizeof( char ), __FILE__, __LINE__ );
                        strncpy( json_record->fields[ i ].value, "false", json_record->fields[ i ].size );
                    }

                } else {
                    json_record->fields[ i ].size = 0;
                    json_record->fields[ i ].value = NULL;
                }
            }

            if( json_load_callback ) {
                ( *json_load_callback )( json_record, data_ptr1, data_ptr2, log );
            }
        }

        free( json_columns->fields ); json_columns->fields = NULL;
        free( json_columns ); json_columns = NULL;

        free( file_content ); file_content = NULL;
        cJSON_Delete( json_file );

        return 1;
    }

    return 0;
}
