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

void CDC_TransformGeneric( DB_SYSTEM_ETL_TRANSFORM_QUERY *transform_element, DATABASE_SYSTEM_DB* dcpam_db, DATABASE_SYSTEM_DB* system_db );


void CDC_TransformGeneric( DB_SYSTEM_ETL_TRANSFORM_QUERY *transform_element, DATABASE_SYSTEM_DB* dcpam_db, DATABASE_SYSTEM_DB* system_db ) {

    LOG_print( "[%s] Running transform data process ", TIME_get_gmt() );

    if( strstr( transform_element->module, "dcpam://" ) == NULL ) {
        FILE    *script = NULL;
        char    command[ 4096 ];
        size_t  res_len = 0;
        char    res[ 16 ];

        /* Prepare argument list */
        snprintf( command, 4096, "%s --dhost=\"%s\" --dport=%d --duser=\"%s\" --dpass=\"%s\" --ddriver=%d --dconn=\"%s\" --shost=\"%s\" --sport=%d --suser=\"%s\" --spass=\"%s\" --sdriver=%d --sconn=\"%s\" &>21",
            transform_element->module,
            dcpam_db->ip,
            dcpam_db->port,
            dcpam_db->user,
            dcpam_db->password,
            dcpam_db->driver,
            dcpam_db->connection_string,
            system_db->ip,
            system_db->port,
            system_db->user,
            system_db->password,
            system_db->driver,
            system_db->connection_string
        );

        LOG_print( "%s\n", command );
        LOG_print( "\t- executing local script...", command );
        script = popen( transform_element->module, READ_BINARY );
        if( script == NULL ) {
            LOG_print( "error.\n" );
            pclose( script );
            return;
        }
        res_len = fread( res, sizeof( char ), 16, script );
        if( res_len == 0 ) {
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
        char    command[ 4096 ];

        sscanf( transform_element->module, "dcpam://%99[^:]:%99d/%255[^\n]", host, &port, script );

        /* Prepare query*/
        snprintf( command, 4096, "m=\"%s\" dhost=\"%s\" dport=%d duser=\"%s\" dpass=\"%s\" driver=%d dconn=\"%s\" shost=\"%s\" sport=%d suser=\"%s\" spass=\"%s\" sdriver=%d sconn=\"%s\"",
            script,
            dcpam_db->ip,
            dcpam_db->port,
            dcpam_db->user,
            dcpam_db->password,
            dcpam_db->driver,
            dcpam_db->connection_string,
            system_db->ip,
            system_db->port,
            system_db->user,
            system_db->password,
            system_db->driver,
            system_db->connection_string
        );

        LOG_print( "dcpam://%s:%d > %s\n", host, port, command );

        transform_element->connection = SAFEMALLOC( sizeof( NET_CONN ), __FILE__, __LINE__ );
        if( NET_CONN_connect( transform_element->connection, host, port ) == 1 ) {

            LOG_print( "[%s] NET_CONN_send...", TIME_get_gmt() );
            if( NET_CONN_send( transform_element->connection, command, strlen( command ) ) == 1 ) {
                LOG_print( "ok.\n" );
            } else {
                LOG_print( "error.\n" );
            }
            NET_CONN_disconnect( transform_element->connection );
        }
        free( transform_element->connection ); transform_element->connection = NULL;
    }
}


void DB_CDC_TransformInserted( DB_SYSTEM_ETL_TRANSFORM_QUERY **transform_elements, const int count, DATABASE_SYSTEM_DB* dcpam_db, DATABASE_SYSTEM_DB* system_db ) {
    if( transform_elements && dcpam_db && system_db ) {
        LOG_print( "\t· [CDC - TRANSFORM::INSERTED]:\n" );
        for( int i = 0; i < count; i++ ) {
            CDC_TransformGeneric( transform_elements[ i ], dcpam_db, system_db );
        }
    } else {
        LOG_print( "\t· [CDC - TRANSFORM::INSERTED] Fatal error: not all DB_CDC_TransformInserted parameters are valid!\n" );
    }

}

void DB_CDC_TransformDeleted( DB_SYSTEM_ETL_TRANSFORM_QUERY **transform_elements, const int count, DATABASE_SYSTEM_DB* dcpam_db, DATABASE_SYSTEM_DB* system_db ) {
    if( transform_elements && dcpam_db && system_db ) {
        LOG_print( "\t· [CDC - TRANSFORM::DELETED]:\n" );
        for( int i = 0; i < count; i++ ) {
            CDC_TransformGeneric( transform_elements[ i ], dcpam_db, system_db );
        }
    } else {
        LOG_print( "\t· [CDC - TRANSFORM::DELETED] Fatal error: not all DB_CDC_TransformDeleted parameters are valid!\n" ); 
    }
}

void DB_CDC_TransformModified( DB_SYSTEM_ETL_TRANSFORM_QUERY **transform_elements, const int count, DATABASE_SYSTEM_DB* dcpam_db, DATABASE_SYSTEM_DB* system_db ) {
    if( transform_elements && dcpam_db && system_db ) {
        LOG_print( "\t· [CDC - TRANSFORM::MODIFIED]:\n" );
        for( int i = 0; i < count; i++ ) {
            CDC_TransformGeneric( transform_elements[ i ], dcpam_db, system_db );
        }
    } else {
        LOG_print( "\t· [CDC - TRANSFORM::MODIFIED] Fatal error: not all DB_CDC_TransformModified parameters are valid!\n" );
    }
}
