#include <stdio.h>
#include <stdlib.h>
#include "../include/utils/memory.h"
#include "../include/utils/time.h"
#include "../include/core/lcs_report.h"
#include "../include/utils/log.h"
//#include "../include/core/app_schema.h"

int LCS_REPORT_init( LCS_REPORT *connection, const char *address, const char *component_name, const char *component_version, LOG_OBJECT *log ) {
    char    host[ 100 ];
    int     port = 7777;

    connection->log = log;

    LOG_print( log, "[%s] LCS_REPORT_init( %s, %s, %s )...\n", TIME_get_gmt(), address, component_name, component_version );

    connection->conn = SAFEMALLOC( sizeof( NET_CONN ), __FILE__, __LINE__ );
    connection->conn->log = log;
    connection->conn->connected = 0;

    size_t component_name_len = strlen( component_name );
    connection->component = SAFECALLOC( component_name_len + 1, sizeof( char ), __FILE__, __LINE__ );
    strlcpy( connection->component, component_name, component_name_len );

    size_t component_version_len = strlen( component_version );
    connection->version = SAFECALLOC( component_version_len + 1, sizeof( char ), __FILE__, __LINE__ );
    strlcpy( connection->version, component_version, component_version_len );

    connection->active = 1;

    if( sscanf( address, "dcpam://%99[^:]:%99d", host, &port ) == 2 ) {
        size_t host_len = strlen( host );
        connection->lcs_host = SAFECALLOC( host_len + 1, sizeof( char ), __FILE__, __LINE__ );
        //strlcpy( connection->lcs_host, host, host_len );
        snprintf( connection->lcs_host, host_len + 1, host );
        connection->lcs_port = port;
        return 1;
    } else {
        LOG_print( log, "[%s] LCS_REPORT_init failed: address %s is invalid.\n", TIME_get_gmt(), address );
        return 0;
    }

    return 1;
}

int LCS_REPORT_free( LCS_REPORT* connection ) {

    if( connection ) {
        LOG_print( connection->log, "[%s] LCS_REPORT_free( %s )...\n", TIME_get_gmt(), connection->address );

        free( connection->address ); connection->address = NULL;
        free( connection->component ); connection->component = NULL;
        free( connection->version ); connection->version = NULL;
        free( connection->lcs_host ); connection->lcs_host = NULL;

        connection->lcs_port = 0;

        if( connection->active == 1 && connection->conn && connection->conn->connected == 1 ) {
            NET_CONN_disconnect( connection->conn );
            connection->active = 0;
        }

        if( connection->conn != NULL ) {
            connection->conn->log = NULL;
            free( connection->conn ); connection->conn = NULL;
        }
    }
}

int LCS_REPORT_send( LCS_REPORT* connection, const char* action, COMPONENT_ACTION_TYPE action_type ) {

    if( connection ) {
        char *action_template = "{\"app\": \"%s\", \"ver\": \"%s\", \"action\": \"%s\", \"type\": \"%s\"}";
        size_t action_template_len = strlen( action_template );
        size_t app_len = strlen( connection->component );
        size_t ver_len = strlen( connection->version );
        size_t action_len = strlen( action );
        size_t type_len = action_type == DCT_START ? 5 : 4;
        size_t buf_len = action_template_len + app_len + ver_len + action_len + type_len;

        LOG_print( connection->log, "[%s] LCS_REPORT_send( %s, %s, %s )...\n", TIME_get_gmt(), connection->address, action, action_type == DCT_START ? "START" : "STOP" );

        char* dst_buf = SAFECALLOC( buf_len + 1, sizeof( char ), __FILE__, __LINE__ );
        snprintf( dst_buf, buf_len, action_template,
                    connection->component,
                    connection->version,
                    action,
                    action_type == DCT_START ? "start" : "stop"
        );
        if( NET_CONN_init( connection->conn, connection->lcs_host, connection->lcs_port, 0 ) == 1 ) {
            if( NET_CONN_connect( connection->conn, connection->lcs_host, connection->lcs_port, 0 ) == 1 ) {
                if( NET_CONN_send( connection->conn, dst_buf, buf_len ) == 1 ) {
                    free( dst_buf ); dst_buf = NULL;
                    NET_CONN_disconnect( connection->conn );
                    return 1;
                }
            }
        }

        free( dst_buf ); dst_buf = NULL;
    }

    return 0;
}
