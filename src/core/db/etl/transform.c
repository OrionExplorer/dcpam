/* Copyright (C) 2020-2021 Marcin Kelar

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335 USA */

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "../../../include/utils/time.h"
#include "../../../include/utils/memory.h"
#include "../../../include/core/schema.h"
#include "../../../include/utils/log.h"
#include "../../../include/core/network/client.h"
#include "../../../include/core/db/etl/transform.h"
#include <stdio.h>

#ifdef _WIN32
#define popen           _popen
#define pclose          _pclose
#define READ_BINARY     "rb"
#else
#define READ_BINARY     "re"
#endif

extern DCPAM_APP           APP;

int CDC_TransformGeneric( DB_SYSTEM_ETL_TRANSFORM_QUERY *transform_element, DATABASE_SYSTEM_DB* dcpam_db, DATABASE_SYSTEM_DB* system_db, LOG_OBJECT *log );


int CDC_TransformGeneric( DB_SYSTEM_ETL_TRANSFORM_QUERY *transform_element, DATABASE_SYSTEM_DB* dcpam_db, DATABASE_SYSTEM_DB* system_db, LOG_OBJECT *log ) {
    if( strstr( transform_element->module, "dcpam://" ) == NULL && strstr( transform_element->module, "./" ) && strstr( transform_element->module, ".." )  == NULL ) {
        FILE    *script = NULL;
        char    command[ 4096 ];
        size_t  res_len = 0;
        char    res[ 4096 ];

        memset( res, '\0', 4096 );
        memset( command, '\0', 4096 );

        /* Prepare argument list */
        snprintf( command, 4096, "%s --dhost=\"%s\" --dport=%d --duser=\"%s\" --dpass=\"%s\" --ddriver=%d --dconn=\"%s\" --shost=\"%s\" --sport=%d --suser=\"%s\" --spass=\"%s\" --sdriver=%d --sconn=\"%s\"",
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

        LOG_print( log, "[%s] Running local transform data process at %s.\n", TIME_get_gmt(), command );
        LOG_print( log, "[%s] Executing local script %s...\n", TIME_get_gmt(), command );
        script = popen( command, READ_BINARY );
        if( script == NULL ) {
            LOG_print( log, "[%s] Error executing script %s.\n", TIME_get_gmt(), command );
            //pclose( script );
            return 0;
        }
        res_len = fread( res, sizeof( char ), 1, script );
        if( res_len == 0 ) {
            LOG_print( log, "[%s] Error executing script %s. No data returned.\n", TIME_get_gmt(), command );
            pclose( script );
            return 0;
        }
        res[ res_len ] = 0;
        LOG_print( log, "[%s] Script %s finished with result: %s.\n", TIME_get_gmt(), command, res );
        pclose( script );

        return 1;

    } else if( strstr( transform_element->module, "dcpam://" ) ) {
        char    host[ 100 ];
        int     port = 9091;
        char    script[ 256 ];
        char    command[ 4096 ];

        sscanf( transform_element->module, "dcpam://%99[^:]:%99d%255[^\n]", host, &port, script );

        /* Prepare query*/
        snprintf( command, 4096, "m=./%s dhost=%s dport=%d duser=%s dpass=%s ddriver=%d dconn=\"%s\" shost=%s sport=%d suser=%s spass=%s sdriver=%d sconn=\"%s\" key=%s",
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
            system_db->connection_string,
            transform_element->api_key
        );

        LOG_print( log, "[%s] Running remote transform data process at dcpam://%s:%d with payload %s\n", TIME_get_gmt(), host, port, command );

        transform_element->connection = SAFEMALLOC( sizeof( NET_CONN ), __FILE__, __LINE__ );
        transform_element->connection->log = log;
        if( NET_CONN_init( transform_element->connection, host, port, 0 ) == 1 ) {
            if( NET_CONN_connect( transform_element->connection, host, port, 0 ) == 1 ) {

                if( NET_CONN_send( transform_element->connection, command, strlen( command ) ) == 1 ) {
                    LOG_print( log, "[%s] Received response (%zu): %s\n", TIME_get_gmt(), transform_element->connection->response_len, transform_element->connection->response );
                    if( strstr( transform_element->connection->response, "0" ) ) {
                        LOG_print( log, "[%s] Transform process exited with failure.\n", TIME_get_gmt() );
                        NET_CONN_disconnect( transform_element->connection );
                        free( transform_element->connection ); transform_element->connection = NULL;
                        return 0;
                    }
                } else {
                    NET_CONN_disconnect( transform_element->connection );
                    transform_element->connection->log = NULL;
                    free( transform_element->connection ); transform_element->connection = NULL;
                    return 0;
                }
                NET_CONN_disconnect( transform_element->connection );
            } else {
                transform_element->connection->log = NULL;
                free( transform_element->connection ); transform_element->connection = NULL;
                return 0;
            }
            NET_CONN_disconnect( transform_element->connection );
        }
        transform_element->connection->log = NULL;
        free( transform_element->connection ); transform_element->connection = NULL;
    } else {
        LOG_print( log, "[%s] Error: transform script path is invalid!\n", TIME_get_gmt() );
        return 0;
    }

    return 1;
}


int DB_CDC_TransformInserted( DB_SYSTEM_ETL_TRANSFORM_QUERY **transform_elements, const int count, DATABASE_SYSTEM_DB* dcpam_db, DATABASE_SYSTEM_DB* system_db, LOG_OBJECT *log ) {
    if( transform_elements && dcpam_db && system_db ) {
        LOG_print( log, "\t· [CDC - TRANSFORM::INSERTED]:\n" );
        for( int i = 0; i < count; i++ ) {
            return CDC_TransformGeneric( transform_elements[ i ], dcpam_db, system_db, log );
        }
        return 1;
    } else {
        LOG_print( log, "\t· [CDC - TRANSFORM::INSERTED] Fatal error: not all DB_CDC_TransformInserted parameters are valid!\n" );
        return 0;
    }

}

int DB_CDC_TransformDeleted( DB_SYSTEM_ETL_TRANSFORM_QUERY **transform_elements, const int count, DATABASE_SYSTEM_DB* dcpam_db, DATABASE_SYSTEM_DB* system_db, LOG_OBJECT *log ) {
    if( transform_elements && dcpam_db && system_db ) {
        LOG_print( log, "\t· [CDC - TRANSFORM::DELETED]:\n" );
        for( int i = 0; i < count; i++ ) {
            return CDC_TransformGeneric( transform_elements[ i ], dcpam_db, system_db, log );
        }
        return 1;
    } else {
        LOG_print( log, "\t· [CDC - TRANSFORM::DELETED] Fatal error: not all DB_CDC_TransformDeleted parameters are valid!\n" ); 
        return 0;
    }
}

int DB_CDC_TransformModified( DB_SYSTEM_ETL_TRANSFORM_QUERY **transform_elements, const int count, DATABASE_SYSTEM_DB* dcpam_db, DATABASE_SYSTEM_DB* system_db, LOG_OBJECT *log ) {
    if( transform_elements && dcpam_db && system_db ) {
        LOG_print( log, "\t· [CDC - TRANSFORM::MODIFIED]:\n" );
        for( int i = 0; i < count; i++ ) {
            return CDC_TransformGeneric( transform_elements[ i ], dcpam_db, system_db, log );
        }
        return 1;
    } else {
        LOG_print( log, "\t· [CDC - TRANSFORM::MODIFIED] Fatal error: not all DB_CDC_TransformModified parameters are valid!\n" );
        return 0;
    }
}
