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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/core/component.h"
#include "../include/core/network/client.h"
#include "../include/utils/memory.h"
#include "../include/utils/log.h"
#include "../include/utils/time.h"
#include "../include/third-party/cJSON.h"

int LCS_COMPONENT_ACTION_register( DCPAM_COMPONENT* dst, const char* description, COMPONENT_ACTION_TYPE action_type, LOG_OBJECT *log ) {

    if( dst && description ) {
        LOG_print( log, "[%s] LCS_COMPONENT_ACTION_register( %s, %s, %s, %s, %s )...\n", TIME_get_gmt(), dst->name, dst->version, dst->ip, description, action_type == DCT_START ? "START" : "STOP" );

        COMPONENT_ACTION **tmp = realloc( dst->actions, ( dst->actions_len + 1 )* sizeof dst->actions );

        if( tmp ) {
            dst->actions = tmp;
            dst->actions[ dst->actions_len ] = SAFEMALLOC( sizeof( COMPONENT_ACTION ), __FILE__, __LINE__ );

            size_t description_len = strlen( description );
            dst->actions[ dst->actions_len ]->description = SAFECALLOC( description_len + 1, sizeof( char ), __FILE__, __LINE__ );
            strlcpy( dst->actions[ dst->actions_len ]->description, description, description_len );

            dst->actions[ dst->actions_len ]->type = action_type;

            strlcpy( dst->actions[ dst->actions_len ]->timestamp, TIME_get_gmt(), 21 );

            dst->actions_len++;

            LOG_print( log, "[%s] LCS_COMPONENT_ACTION_register completed (total actions: %d).\n", TIME_get_gmt(), dst->actions_len );

            return 1;
        } else {
            return 0;
        }

    } else {
        LOG_print( log, "[%s] LCS_COMPONENT_ACTION_register fatal error: parameters are invalid.\n", TIME_get_gmt() );
        return 0;
    }

    return 0;
}

cJSON *LCS_COMPONENT_get_states( DCPAM_COMPONENT** components, int components_len ) {
    cJSON* all_data = cJSON_CreateArray();
    cJSON* record = NULL;
    cJSON* response = cJSON_CreateObject();

    for( int i = 0; i < components_len; i++ ) {
        record = cJSON_CreateObject();
        cJSON_AddStringToObject( record, "name", components[ i ]->name );
        cJSON_AddStringToObject( record, "version", components[ i ]->version );
        cJSON_AddStringToObject( record, "ip", components[ i ]->ip );
        cJSON_AddNumberToObject( record, "port", components[ i ]->port );
        if( components[ i ]->active == 1 ) {
            cJSON_AddTrueToObject( record, "active" );
        } else {
            cJSON_AddFalseToObject( record, "active" );
        }

        cJSON_AddItemToArray( all_data, record );
    }

    cJSON_AddItemToObject( response, "data", all_data );
    cJSON_AddBoolToObject( response, "success", 1 );
    cJSON_AddNumberToObject( response, "length", components_len );

    return response;
}

cJSON *LCS_COMPONENT_get_actions( DCPAM_COMPONENT** components, int components_len, const char* component_name ) {
    cJSON* all_data = cJSON_CreateArray();
    cJSON* record = NULL;
    cJSON* response = cJSON_CreateObject();

    for( int i = 0; i < components_len; i++ ) {
        if( strcmp( components[ i ]->name, component_name ) == 0 ) {

            for( int j = 0; j < components[ i ]->actions_len; j++ ) {
                record = cJSON_CreateObject();
                cJSON_AddStringToObject( record, "description", components[ i ]->actions[ j ]->description );
                cJSON_AddStringToObject( record, "timestamp", components[ i ]->actions[ j ]->timestamp );

                if( components[ i ]->actions[ j ]->type == DCT_START ) {
                    cJSON_AddStringToObject( record, "type", "start" );
                } else {
                    cJSON_AddStringToObject( record, "type", "stop" );
                }

                cJSON_AddItemToArray( all_data, record );
            }

            cJSON_AddItemToObject( response, "data", all_data );
            cJSON_AddBoolToObject( response, "success", 1 );
            cJSON_AddNumberToObject( response, "length", components[ i ]->actions_len );

            break;
        }
    }

    return response;
}

cJSON *LCS_COMPONENT_process_report( cJSON* json_request, DCPAM_COMPONENT** components, int components_len, LOG_OBJECT *log ) {
    cJSON* report = cJSON_GetObjectItem( json_request, "report" );
    if( report ) {

        if( strcmp( report->valuestring, "component_state" ) == 0 ) {
            return LCS_COMPONENT_get_states( components, components_len );
        } else if( strcmp( report->valuestring, "component_actions" ) == 0 ) {
            cJSON* name = cJSON_GetObjectItem( json_request, "name" );
            if( name ) {
                return LCS_COMPONENT_get_actions( components, components_len, name->valuestring );
            } else {
                LOG_print( log, "[%s] Error: \"name\" is missing.\n", TIME_get_gmt() );
                cJSON_Delete( json_request );
                return NULL;
            }
        }
    }
}


int LCS_COMPONENT_register( DCPAM_COMPONENT* dst, const char* name, const char* version, const char* ip, const int port, LOG_OBJECT* log ) {

    const char* component_registration_message = "Component registration.\0";

    LOG_print( log, "[%s] LCS_COMPONENT_register( %s, %s, %s, %d, \"%s\" )...\n", TIME_get_gmt(), name, version, ip, port, component_registration_message );

    if( dst && name && version && ip ) {
        size_t name_len = strlen( name );
        size_t version_len = strlen( version );
        size_t ip_len = strlen( ip );

        dst->port = port;

        dst->name = SAFECALLOC( name_len + 1, sizeof( char ), __FILE__, __LINE__ );
        strlcpy( dst->name, name, name_len );

        dst->version = SAFECALLOC( version_len + 1, sizeof( char ), __FILE__, __LINE__ );
        strlcpy( dst->version, version, version_len );

        dst->ip = SAFECALLOC( ip_len + 1, sizeof( char ), __FILE__, __LINE__ );
        strlcpy( dst->ip, ip, ip_len );

        dst->active = 1;

        dst->actions = SAFEMALLOC( sizeof * dst->actions, __FILE__, __LINE__ );
        dst->actions_len = 1;

        size_t crm_len = strlen( component_registration_message );
        dst->actions[ 0 ] = SAFEMALLOC( sizeof( COMPONENT_ACTION ), __FILE__, __LINE__ );
        dst->actions[ 0 ]->description = SAFECALLOC( crm_len + 1, sizeof( char ), __FILE__, __LINE__ );
        strlcpy( dst->actions[ 0 ]->description, component_registration_message, crm_len );

        strlcpy( dst->actions[ 0 ]->timestamp, TIME_get_gmt(), 20 );

        dst->actions[ 0 ]->type = DCT_START;

        if( LCS_COMPONENT_check( dst, log ) == 1 ) {
            LOG_print( log, "[%s] LCS_COMPONENT_register finished successfully.\n", TIME_get_gmt() );
            return 1;
        } else {
            LOG_print( log, "[%s] LCS_COMPONENT_register fatal error: DCPAM Component \"%s\" is offline!\n", TIME_get_gmt(), dst->name );
            return 0;
        }

    } else {
        LOG_print( log, "[%s] LCS_COMPONENT_register fatal error: parameters are invalid.\n", TIME_get_gmt() );
        return 0;
    }

    return 0;
}

int LCS_COMPONENT_check( DCPAM_COMPONENT* dst, LOG_OBJECT *log ) {
    if( dst ) {
        NET_CONN *conn = SAFEMALLOC( sizeof( NET_CONN ), __FILE__, __LINE__ );

        if( conn ) {
            conn->log = log;
            if( NET_CONN_init( conn, dst->ip, dst->port, 0 ) == 1 ) {
                if( NET_CONN_connect( conn, dst->ip, dst->port, 0 ) == 1 ) {
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
                        NET_CONN_disconnect( conn );
                    }
                }
                NET_CONN_disconnect( conn );
            }
            NET_CONN_disconnect( conn );
            free( conn->host ); conn->host = NULL;
            conn->log = NULL;
            free( conn ); conn = NULL;
        }
    }

    return 0;
}

int LCS_COMPONENT_free( DCPAM_COMPONENT* src ) {

    if( src ) {
        for( int i = 0; i < src->actions_len; i++ ) {
            free( src->actions[ i ]->description ); src->actions[ i ]->description = NULL;
            memset( src->actions[ i ]->timestamp, '\0', 20 );
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
