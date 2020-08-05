#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "../../../include/utils/time.h"
#include "../../../include/utils/memory.h"
#include "../../../include/core/schema.h"
#include "../../../include/utils/log.h"
#include "../../../include/core/network/client.h"
#include "../../../include/core/db/etl/transform.h"

#ifdef _WIN32
#define popen           _popen
#define pclose          _pclose
#define READ_BINARY     "rb"
#else
#define READ_BINARY     "re"
#endif

extern DCPAM_APP           APP;

void CDC_TransformGeneric( DB_SYSTEM_ETL_TRANSFORM_QUERY *transform_element );


void CDC_TransformGeneric( DB_SYSTEM_ETL_TRANSFORM_QUERY *transform_element ) {

    LOG_print( "[%s] Running transform data process \"%s\":\n", TIME_get_gmt(), transform_element->module );

    if( strstr( transform_element->module, "dcpam://" ) == NULL ) {
        FILE    *script = NULL;
        char    command[ 4096 ];
        size_t  res_len = 0;
        char    res[ 16 ];

        LOG_print( "\t- executing local script..." );
        script = popen( transform_element->module, READ_BINARY );
        if( script == NULL ) {
            LOG_print( "error.\n" );
            pclose( script );
            return;
        }
        res_len = fread( res, sizeof( char ), 16, script );
        if( res_len <= 0 ) {
            LOG_print( "error. No data returned from script.\n" );
            pclose( script );
            return;
        }
        LOG_print( "ok. Result: %s.\n", res );
        pclose( script );

    } else {
        char    host[ 100 ];
        int     port = 9091;
        char    script[ 256 ];
        sscanf( transform_element->module, "dcpam://%99[^:]:%99d/%255[^\n]", host, &port, script );
        LOG_print( "\t- executing remote script:\n" );
        LOG_print( "\t\t- %s\n", host );
        LOG_print( "\t\t- %d\n", port );
        LOG_print( "\t\t- %s\n", script );

        transform_element->connection = SAFEMALLOC( sizeof( NET_CONN ), __FILE__, __LINE__ );
        NET_CONN_connect( transform_element->connection, host, port );
        NET_CONN_disconnect( transform_element->connection );
        free( transform_element->connection ); transform_element->connection = NULL;
    }
}


void DB_CDC_TransformInserted( DB_SYSTEM_ETL_TRANSFORM_QUERY **transform_elements, const int count ) {
    if( transform_elements ) {
        LOG_print( "\t· [CDC - TRANSFORM::INSERTED]:\n" );
        for( int i = 0; i < count; i++ ) {
            CDC_TransformGeneric( transform_elements[ i ] );
        }
    } else {
        LOG_print( "\t· [CDC - TRANSFORM::INSERTED] Fatal error: not all DB_CDC_TransformInserted parameters are valid!\n" );
    }

}

void DB_CDC_TransformDeleted( DB_SYSTEM_ETL_TRANSFORM_QUERY **transform_elements, const int count ) {
    if( transform_elements ) {
        LOG_print( "\t· [CDC - TRANSFORM::DELETED]:\n" );
        for( int i = 0; i < count; i++ ) {
            CDC_TransformGeneric( transform_elements[ i ] );
        }
    } else {
        LOG_print( "\t· [CDC - TRANSFORM::DELETED] Fatal error: not all DB_CDC_TransformDeleted parameters are valid!\n" ); 
    }
}

void DB_CDC_TransformModified( DB_SYSTEM_ETL_TRANSFORM_QUERY **transform_elements, const int count ) {
    if( transform_elements ) {
        LOG_print( "\t· [CDC - TRANSFORM::MODIFIED]:\n" );
        for( int i = 0; i < count; i++ ) {
            CDC_TransformGeneric( transform_elements[ i ] );
        }
    } else {
        LOG_print( "\t· [CDC - TRANSFORM::MODIFIED] Fatal error: not all DB_CDC_TransformModified parameters are valid!\n" );
    }
}
