#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/core/component.h"
#include "../include/utils/memory.h"
#include "../include/utils/log.h"
#include "../include/utils/time.h"
#include "../include/third-party/cJSON.h"

int COMPONENT_ACTION_register( DCPAM_COMPONENT* dst, const char* description, COMPONENT_ACTION_TYPE action_type, LOG_OBJECT *log ) {

    if( dst && description ) {
        LOG_print( log, "[%s] COMPONENT_ACTION_register( %s, %s, %s, %s )...\n", TIME_get_gmt(), dst->name, dst->version, dst->ip, description );

        dst->actions_len++;
        COMPONENT_ACTION **tmp = realloc( dst->actions, dst->actions_len * sizeof dst->actions );

        if( tmp ) {
            dst->actions = tmp;
            dst->actions[ dst->actions_len ] = SAFEMALLOC( sizeof( COMPONENT_ACTION ), __FILE__, __LINE__ );

            size_t description_len = strlen( description );
            dst->actions[ dst->actions_len ]->description = SAFECALLOC( description_len + 1, sizeof( char ), __FILE__, __LINE__ );
            strncpy( dst->actions[ dst->actions_len ]->description, description, description_len );

            dst->actions[ dst->actions_len ]->type = action_type;

            if( action_type == DCT_START ) {
                strncpy( dst->actions[ dst->actions_len ]->start_timestamp, TIME_get_gmt(), 20 );
            } else {
                strncpy( dst->actions[ dst->actions_len ]->stop_timestamp, TIME_get_gmt(), 20 );
            }

            LOG_print( log, "[%s] COMPONENT_ACTION_register completed.\n", TIME_get_gmt() );

            return 1;
        } else {
            return 0;
        }

    } else {
        LOG_print( log, "[%s] COMPONENT_add_action fatal error: parameters are invalid.\n", TIME_get_gmt() );
        return 0;
    }

    return 0;
}

int COMPONENT_register( DCPAM_COMPONENT* dst, const char* name, const char* version, const char* ip, const int port, LOG_OBJECT* log ) {

    const char* component_registration_message = "Component registration.\0";

    LOG_print( log, "[%s] COMPONENT_register( %s, %s, %s, %d, \"%s\" )...\n", TIME_get_gmt(), name, version, ip, port, component_registration_message );

    if( dst && name && version && ip ) {
        size_t name_len = strlen( name );
        size_t version_len = strlen( version );
        size_t ip_len = strlen( ip );

        dst->port = port;

        dst->name = SAFECALLOC( name_len + 1, sizeof( char ), __FILE__, __LINE__ );
        strncpy( dst->name, name, name_len );

        dst->version = SAFECALLOC( version_len + 1, sizeof( char ), __FILE__, __LINE__ );
        strncpy( dst->version, version, version_len );

        dst->ip = SAFECALLOC( ip_len + 1, sizeof( char ), __FILE__, __LINE__ );
        strncpy( dst->ip, ip, ip_len );

        dst->active = 1;

        dst->actions = SAFEMALLOC( sizeof * dst->actions, __FILE__, __LINE__ );
        dst->actions_len = 1;

        size_t crm_len = strlen( component_registration_message );
        dst->actions[ 0 ] = SAFEMALLOC( sizeof( COMPONENT_ACTION ), __FILE__, __LINE__ );
        dst->actions[ 0 ]->description = SAFECALLOC( crm_len + 1, sizeof( char ), __FILE__, __LINE__ );
        strncpy( dst->actions[ 0 ]->description, component_registration_message, crm_len );

        strncpy( dst->actions[ 0 ]->start_timestamp, TIME_get_gmt(), 20 );
        strncpy( dst->actions[ 0 ]->stop_timestamp, TIME_get_gmt(), 20 );

        dst->actions[ 0 ]->success = DCR_SUCCESS;

        if( COMPONENT_check( dst, log ) == 1 ) {
            LOG_print( log, "[%s] COMPONENT_register finished successfully.\n", TIME_get_gmt() );
            return 1;
        } else {
            LOG_print( log, "[%s] COMPONENT_register fatal error: DCPAM Component \"%s\" is offline!\n", TIME_get_gmt(), dst->name );
            return 0;
        }

    } else {
        LOG_print( log, "[%s] COMPONENT_register fatal error: parameters are invalid.\n", TIME_get_gmt() );
        return 0;
    }

    return 0;
}

int COMPONENT_check( DCPAM_COMPONENT* dst, LOG_OBJECT *log ) {
    if( dst ) {
        NET_CONN *conn = SAFEMALLOC( sizeof( NET_CONN ), __FILE__, __LINE__ );

        if( conn ) {
            conn->log = log;
            if( NET_CONN_init( conn, dst->ip, dst->port ) == 1 ) {
                if( NET_CONN_connect( conn, dst->ip, dst->port ) == 1 ) {
                    const char* ping_msg = "{\"msg\": \"ping\"}";
                    if( NET_CONN_send( conn, ping_msg, strlen( ping_msg ) ) == 1 ) {
                        if( conn->response && conn->response_len > 0 ) {

                            cJSON* resp = cJSON_Parse( conn->response );
                            if( resp ) {
                                cJSON* msg = cJSON_GetObjectItem( resp, "msg" );
                                if( msg ) {
                                    if( strcmp( msg->valuestring, "pong" ) == 0 ) {
                                        NET_CONN_disconnect( conn );
                                        conn->log = NULL;
                                        free( conn ); conn = NULL;
                                        cJSON_Delete( resp );
                                        return 1;
                                    }
                                }
                                cJSON_Delete( resp );
                            }
                        }
                    }
                }
            }
            conn->log = NULL;
            free( conn ); conn = NULL;
        }
    }

    return 0;
}

int COMPONENT_free( DCPAM_COMPONENT* src ) {

    if( src ) {
        for( int i = 0; i < src->actions_len; i++ ) {
            free( src->actions[ i ]->description ); src->actions[ i ]->description = NULL;
            memset( src->actions[ i ]->start_timestamp, '\0', 20 );
            memset( src->actions[ i ]->stop_timestamp, '\0', 20 );
            src->actions[ i ]->success = DCR_FAILURE;
            free( src->actions[ i ] );
        }
        free( src->actions ); src->actions = NULL;
        src->actions_len = 0;

        free( src->name ); src->name = NULL;
        free( src->version ); src->version = NULL;
        free( src->ip ); src->ip = NULL;

        memset( src->timestamp, '\0', 20 );

        src->active = 0;

        return 1;
    }

    return 0;
}
