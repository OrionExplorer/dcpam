#include <stdio.h>
#include <stdlib.h>
#include "../include/utils/memory.h"
#include "../include/utils/time.h"
#include "../include/core/lcs_report.h"
#include "../include/utils/log.h"

int LCS_REPORT_init( LCS_REPORT *connection, const char *address, const char *component_name, const char *component_version, LOG_OBJECT *log ) {
    char    host[ 100 ];
    int     port = 7777;

    connection->log = log;

    LOG_print( log, "[%s] LCS_REPORT_init( %s, %s, %s )...\n", TIME_get_gmt(), address, component_name, component_version );

    connection->conn = SAFEMALLOC( sizeof( NET_CONN ), __FILE__, __LINE__ );
    connection->conn->log = log;

    connection->active = 1;

    if( sscanf( address, "dcpam://%99[^:]:%99d", host, &port ) == 2 ) {
        if( NET_CONN_init( connection->conn, host, port ) == 1 ) {
            LOG_print( log, "[%s] LCS_REPORT_init finished.\n", TIME_get_gmt() );
            return 1;
        } else {
            connection->conn->log = NULL;
            free( connection->conn ); connection->conn = NULL;
            return 0;
        }
    } else {
        LOG_print( log, "[%s] LCS_REPORT_init failed: address %s is invalid.\n", TIME_get_gmt(), address );
        return 0;
    }

    return 1;
}

int LCS_REPORT_free( LCS_REPORT* connection ) {

    if( connection && connection->active == 1 ) {
        LOG_print( connection->log, "[%s] LCS_REPORT_free( %s )...\n", TIME_get_gmt(), connection->address );

        free( connection->address ); connection->address = NULL;

        if( connection->active == 1 ) {
            
            free( connection->address ); connection->address = NULL;

            NET_CONN_disconnect( connection->conn );
            connection->conn->log = NULL;
            free( connection->conn ); connection->conn = NULL;

            connection->active = 0;
        }
    }
}

int LCS_REPORT_send( LCS_REPORT* connection, const char* data, size_t data_len ) {

    if( connection ) {
        LOG_print( connection->log, "[%s] LCS_REPORT_send( %s, %s )...\n", TIME_get_gmt(), connection->address, data );

        if( NET_CONN_send( connection->conn, data, data_len ) == 1 ) {
            return 1;
        }
    }

    return 0;
}
