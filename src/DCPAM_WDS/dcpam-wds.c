#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include "../include/utils/log.h"
#include "../include/core/schema.h"
#include "../include/third-party/cJSON.h"
#include "../include/utils/memory.h"
#include "../include/utils/time.h"
#include "../include/utils/strings.h"
#include "../include/utils/filesystem.h"
#include "../include/core/db/system.h"

#pragma warning( disable : 6031 )

char                    app_path[ MAX_PATH_LENGTH + 1 ];
char                    LOG_filename[ MAX_PATH_LENGTH ];
extern int              app_terminated = 0;

extern DATABASE_SYSTEM  DATABASE_SYSTEMS[ MAX_DATA_SYSTEMS ];
extern int              DATABASE_SYSTEMS_COUNT;
extern P_DCPAM_APP      P_APP;

void DCPAM_free_configuration( void );


void app_terminate( void ) {
    LOG_print( "\r\n[%s] Received termination signal.\n", TIME_get_gmt() );
    if( app_terminated == 0 ) {
        app_terminated = 1;
        printf( "\r" );
        DCPAM_free_configuration();
        LOG_print( "[%s] DCPAM graceful shutdown finished. Waiting for all threads to terminate...\n", TIME_get_gmt() );
    }

    return;
}

void DCPAM_free_configuration( void ) {

    for( int i = 0; i < P_APP.DB_len; i++ ) {
        DATABASE_SYSTEM_DB_free( P_APP.DB[ i ] );
        free( P_APP.DB[ i ] ); P_APP.DB[ i ] = NULL;
    }
    free( P_APP.DB ); P_APP.DB = NULL;
    P_APP.DB_len = 0;

    if( P_APP.version != NULL ) { free( P_APP.version ); P_APP.version = NULL; }
    if( P_APP.name != NULL ) { free( P_APP.name ); P_APP.name = NULL; }

    for( int i = 0; i < P_APP.DATA_len; i++ ) {
        if( P_APP.DATA[ i ].id != NULL ) { free( P_APP.DATA[ i ].id ); P_APP.DATA[ i ].id = NULL; }
        if( P_APP.DATA[ i ].name != NULL ) { free( P_APP.DATA[ i ].name ); P_APP.DATA[ i ].name = NULL; }
        if( P_APP.DATA[ i ].db_table_name != NULL ) { free( P_APP.DATA[ i ].db_table_name ); P_APP.DATA[ i ].db_table_name = NULL; }
        if( P_APP.DATA[ i ].description != NULL ) { free( P_APP.DATA[ i ].description ); P_APP.DATA[ i ].description = NULL; }

        for( int j = 0; j < P_APP.DATA[ i ].actions_len; j++ ) {
            if( P_APP.DATA[ i ].actions[ j ].name != NULL ) { free( P_APP.DATA[ i ].actions[ j ].name ); P_APP.DATA[ i ].actions[ j ].name = NULL; }
            if( P_APP.DATA[ i ].actions[ j ].description != NULL ) { free( P_APP.DATA[ i ].actions[ j ].description ); P_APP.DATA[ i ].actions[ j ].description = NULL; }
            if( P_APP.DATA[ i ].actions[ j ].condition != NULL ) { free( P_APP.DATA[ i ].actions[ j ].condition ); P_APP.DATA[ i ].actions[ j ].condition = NULL; }
            if( P_APP.DATA[ i ].actions[ j ].sql != NULL ) { free( P_APP.DATA[ i ].actions[ j ].sql ); P_APP.DATA[ i ].actions[ j ].sql = NULL; }
        }
    }
}

int DCPAM_load_configuration( const char* filename ) {
    cJSON* config_json = NULL;

    cJSON* cfg_app = NULL;
    cJSON* cfg_app_version = NULL;
    cJSON* cfg_app_name = NULL;
    cJSON* cfg_app_db_array = NULL;
    cJSON* cfg_app_db_item = NULL;
    cJSON* cfg_app_db = NULL;
    cJSON* cfg_app_db_ip = NULL;
    cJSON* cfg_app_db_port = NULL;
    cJSON* cfg_app_db_driver = NULL;
    cJSON* cfg_app_db_user = NULL;
    cJSON* cfg_app_db_password = NULL;
    cJSON* cfg_app_db_connection_string = NULL;
    cJSON* cfg_app_db_db = NULL;
    cJSON* cfg_app_db_name = NULL;

    cJSON* cfg_app_data = NULL;
    cJSON* cfg_app_data_item = NULL;
    cJSON* cfg_app_data_item_id = NULL;
    cJSON* cfg_app_data_item_name = NULL;
    cJSON* cfg_app_data_item_db_table_name = NULL;
    cJSON* cfg_app_data_item_description = NULL;
    cJSON* cfg_app_data_actions = NULL;
    cJSON* cfg_app_data_actions_item = NULL;
    cJSON* cfg_app_data_actions_item_columns = NULL;
    cJSON* cfg_app_data_actions_item_columns_name = NULL;
    cJSON* cfg_app_data_actions_item_name = NULL;
    cJSON* cfg_app_data_actions_item_description = NULL;
    cJSON* cfg_app_data_actions_item_type = NULL;
    cJSON* cfg_app_data_actions_item_internal = NULL;
    cJSON* cfg_app_data_actions_item_condition = NULL;
    cJSON* cfg_app_data_actions_item_sql = NULL;

    cJSON* array_value = NULL;
    int                         result = 0;

    char*                       config_string = NULL;


    LOG_print( "[%s] DCPAM_load_configuration( %s ).\n", TIME_get_gmt(), filename );

    config_string = file_get_content( filename );

    if( config_string ) {
        config_json = cJSON_Parse( config_string );
        if( config_json ) {

            cfg_app = cJSON_GetObjectItem( config_json, "app" );
            if( cfg_app ) {

                cfg_app_name = cJSON_GetObjectItem( cfg_app, "name" );
                if( cfg_app_name ) {
                    size_t str_len = strlen( cfg_app_name->valuestring );
                    P_APP.name = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                    snprintf( P_APP.name, str_len + 1, cfg_app_name->valuestring );
                } else {
                    LOG_print( "ERROR: \"app.name\" key not found.\n" );
                    cJSON_Delete( config_json );
                    free( config_string ); config_string = NULL;
                    return FALSE;
                }

                cfg_app_version = cJSON_GetObjectItem( cfg_app, "version" );
                if( cfg_app_version ) {
                    size_t str_len = strlen( cfg_app_version->valuestring );
                    P_APP.version = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                    snprintf( P_APP.version, str_len + 1, cfg_app_version->valuestring );
                    LOG_print( "%s v%s.\n", P_APP.name, P_APP.version );
                } else {
                    LOG_print( "ERROR: \"app.version\" key not found.\n" );
                    cJSON_Delete( config_json );
                    free( config_string ); config_string = NULL;
                    return FALSE;
                }

                cfg_app_db_array = cJSON_GetObjectItem( cfg_app, "DB" );
                if( cfg_app_db_array ) {

                    P_APP.DB_len = cJSON_GetArraySize( cfg_app_db_array );


                    P_APP.DB = SAFEMALLOC( P_APP.DB_len * sizeof * P_APP.DB, __FILE__, __LINE__ );

                    for( int i = 0; i < P_APP.DB_len; i++ ) {
                        cfg_app_db = cJSON_GetArrayItem( cfg_app_db_array, i );

                        cfg_app_db_ip = cJSON_GetObjectItem( cfg_app_db, "ip" );
                        if( cfg_app_db_ip == NULL ) {
                            LOG_print( "ERROR: \"app.DB.ip\" key not found.\n" );
                        }

                        cfg_app_db_port = cJSON_GetObjectItem( cfg_app_db, "port" );
                        if( cfg_app_db_port == NULL ) {
                            LOG_print( "ERROR: \"app.DB.port\" key not found.\n" );
                        }

                        cfg_app_db_driver = cJSON_GetObjectItem( cfg_app_db, "driver" );
                        if( cfg_app_db_driver == NULL ) {
                            LOG_print( "ERROR: \"app.DB.driver\" key not found.\n" );
                        }

                        cfg_app_db_user = cJSON_GetObjectItem( cfg_app_db, "user" );
                        if( cfg_app_db_user == NULL ) {
                            LOG_print( "ERROR: \"app.DB.user\" key not found.\n" );
                        }

                        cfg_app_db_password = cJSON_GetObjectItem( cfg_app_db, "password" );
                        if( cfg_app_db_password == NULL ) {
                            LOG_print( "ERROR: \"app.DB.password\" key not found.\n" );
                        }

                        cfg_app_db_connection_string = cJSON_GetObjectItem( cfg_app_db, "connection_string" );
                        if( cfg_app_db_connection_string == NULL ) {
                            LOG_print( "ERROR: \"app.DB.connection_string\" key not found.\n" );
                        }

                        cfg_app_db_db = cJSON_GetObjectItem( cfg_app_db, "db" );
                        if( cfg_app_db_db == NULL ) {
                            LOG_print( "ERROR: \"app.DB.db\" key not found.\n" );
                        }

                        cfg_app_db_name = cJSON_GetObjectItem( cfg_app_db, "name" );
                        if( cfg_app_db_name == NULL ) {
                            LOG_print( "Error: \"app.DB.name\" key not found.\n" );
                        }

                        P_APP.DB[ i ] = SAFEMALLOC( sizeof( DATABASE_SYSTEM_DB ), __FILE__, __LINE__ );

                        DATABASE_SYSTEM_DB_add(
                            cfg_app_db_ip->valuestring,
                            cfg_app_db_port->valueint,
                            cfg_app_db_driver->valueint,
                            cfg_app_db_user->valuestring,
                            cfg_app_db_password->valuestring,
                            cfg_app_db_db->valuestring,
                            cfg_app_db_connection_string->valuestring,
                            P_APP.DB[ i ],
                            cfg_app_db_name->valuestring,
                            TRUE
                        );
                    }

                } else {
                    LOG_print( "ERROR: \"app.DB\" key not found.\n" );
                    cJSON_Delete( config_json );
                    free( config_string ); config_string = NULL;
                    return FALSE;
                }

                LOG_print( "[%s] Loading app.DATA item ", TIME_get_gmt() );
                cfg_app_data = cJSON_GetObjectItem( cfg_app, "DATA" );
                if( cfg_app_data ) {
                    P_APP.DATA_len = cJSON_GetArraySize( cfg_app_data );
                    for( int i = 0; i < P_APP.DATA_len; i++ ) {
                        cfg_app_data_item = cJSON_GetArrayItem( cfg_app_data, i );

                        cfg_app_data_item_id = cJSON_GetObjectItem( cfg_app_data_item, "id" );
                        if( cfg_app_data_item_id == NULL ) {
                            LOG_print( "ERROR: \"app.DATA[%d].id\" key not found.\n", i );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        size_t str_len = strlen( cfg_app_data_item_id->valuestring );
                        P_APP.DATA[ i ].id = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                        snprintf( P_APP.DATA[ i ].id, str_len + 1, cfg_app_data_item_id->valuestring );
                        LOG_print( "#%d: \"%s\"\n", i + 1, P_APP.DATA[ i ].id );

                        cfg_app_data_item_name = cJSON_GetObjectItem( cfg_app_data_item, "name" );
                        if( cfg_app_data_item_name == NULL ) {
                            LOG_print( "ERROR: \"app.DATA[%d].name\" key not found.\n", i );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        size_t str_len2 = strlen( cfg_app_data_item_name->valuestring );
                        P_APP.DATA[ i ].name = SAFECALLOC( str_len2 + 1, sizeof( char ), __FILE__, __LINE__ );
                        snprintf( P_APP.DATA[ i ].name, str_len2+1, cfg_app_data_item_name->valuestring );
                        LOG_print( "\t· name=\"%s\"\n", P_APP.DATA[ i ].name );

                        cfg_app_data_item_db_table_name = cJSON_GetObjectItem( cfg_app_data_item, "db_table_name" );
                        if( cfg_app_data_item_db_table_name == NULL ) {
                            LOG_print( "ERROR: \"app.DATA[%d].db_table_name\" key not found.\n", i );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        size_t str_len3 = strlen( cfg_app_data_item_db_table_name->valuestring );
                        P_APP.DATA[ i ].db_table_name = SAFECALLOC( str_len3 + 1, sizeof( char ), __FILE__, __LINE__ );
                        snprintf( P_APP.DATA[ i ].db_table_name, str_len3+1, cfg_app_data_item_db_table_name->valuestring );
                        LOG_print( "\t· db_table_name=\"%s\"\n", P_APP.DATA[ i ].db_table_name );

                        P_APP.DATA[ i ].columns_len = 0;
                        cfg_app_data_actions_item_columns = cJSON_GetObjectItem( cfg_app_data_item, "columns" );
                        if( cfg_app_data_actions_item_columns ) {
                            LOG_print( "\t· columns=[" );
                            for( int k = 0; k < cJSON_GetArraySize( cfg_app_data_actions_item_columns ); k++ ) {
                                cfg_app_data_actions_item_columns_name = cJSON_GetArrayItem( cfg_app_data_actions_item_columns, k );
                                memset( P_APP.DATA[ i ].columns[ k ], 0, MAX_COLUMN_NAME_LEN );
                                snprintf( P_APP.DATA[ i ].columns[ k ], MAX_COLUMN_NAME_LEN, "%s", cfg_app_data_actions_item_columns_name->valuestring );
                                LOG_print( "\"%s\", ", cfg_app_data_actions_item_columns_name->valuestring );
                                P_APP.DATA[ i ].columns_len++;
                            }
                            LOG_print( "]\n" );
                        } else {
                            LOG_print( "ERROR: \"app.DATA[%d].columns\" key not found.\n", i );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }

                        cfg_app_data_item_description = cJSON_GetObjectItem( cfg_app_data_item, "description" );
                        if( cfg_app_data_item_description == NULL ) {
                            LOG_print( "ERROR: \"app.DATA[%d].description\" key not found.\n", i );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                        size_t str_len4 = strlen( cfg_app_data_item_description->valuestring );
                        P_APP.DATA[ i ].description = SAFECALLOC( str_len4 + 1, sizeof( char ), __FILE__, __LINE__ );
                        snprintf( P_APP.DATA[ i ].description, str_len4+1, cfg_app_data_item_description->valuestring );
                        LOG_print( "\t· description=\"%s\"\n", P_APP.DATA[ i ].description );

                        cfg_app_data_actions = cJSON_GetObjectItem( cfg_app_data_item, "actions" );
                        if( cfg_app_data_actions ) {
                            P_APP.DATA[ i ].actions_len = cJSON_GetArraySize( cfg_app_data_actions );
                            for( int j = 0; j < P_APP.DATA[ i ].actions_len; j++ ) {
                                cfg_app_data_actions_item = cJSON_GetArrayItem( cfg_app_data_actions, j );

                                cfg_app_data_actions_item_name = cJSON_GetObjectItem( cfg_app_data_actions_item, "name" );
                                if( cfg_app_data_actions_item_name == NULL ) {
                                    LOG_print( "ERROR: \"app.DATA[%d].actions[%d].name\" key not found.\n", i, j );
                                    cJSON_Delete( config_json );
                                    return FALSE;
                                }
                                size_t str_len = strlen( cfg_app_data_actions_item_name->valuestring );
                                P_APP.DATA[ i ].actions[ j ].name = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
                                snprintf( P_APP.DATA[ i ].actions[ j ].name, str_len + 1, cfg_app_data_actions_item_name->valuestring );
                                LOG_print( "\t· action #%d: \"%s\"\n", j + 1, P_APP.DATA[ i ].actions[ j ].name );

                                cfg_app_data_actions_item_description = cJSON_GetObjectItem( cfg_app_data_actions_item, "description" );
                                if( cfg_app_data_actions_item_description == NULL ) {
                                    LOG_print( "ERROR: \"app.DATA[%d].actions[%d].description\" key not found.\n", i, j );
                                    cJSON_Delete( config_json );
                                    return FALSE;
                                }
                                size_t str_len2 = strlen( cfg_app_data_actions_item_description->valuestring );
                                P_APP.DATA[ i ].actions[ j ].description = SAFECALLOC( str_len2 + 1, sizeof( char ), __FILE__, __LINE__ );
                                snprintf( P_APP.DATA[ i ].actions[ j ].description, str_len2+1, cfg_app_data_actions_item_description->valuestring );
                                LOG_print( "\t\t· description=\"%s\"\n", P_APP.DATA[ i ].actions[ j ].description );

                                cfg_app_data_actions_item_type = cJSON_GetObjectItem( cfg_app_data_actions_item, "type" );
                                if( cfg_app_data_actions_item_type == NULL ) {
                                    LOG_print( "ERROR: \"app.DATA[%d].actions[%d].type\" key not found.\n", i, j );
                                    cJSON_Delete( config_json );
                                    return FALSE;
                                }
                                P_APP.DATA[ i ].actions[ j ].type = cfg_app_data_actions_item_type->valueint;
                                LOG_print( "\t\t· type=\"%s\"\n", P_APP.DATA[ i ].actions[ j ].type == AT_READ ? "READ" : "WRITE" );

                                cfg_app_data_actions_item_internal = cJSON_GetObjectItem( cfg_app_data_actions_item, "internal" );
                                if( cfg_app_data_actions_item_internal == NULL ) {
                                    LOG_print( "ERROR: \"app.DATA[%d].actions[%d].internal\" key not found.\n", i, j );
                                    cJSON_Delete( config_json );
                                    return FALSE;
                                }
                                P_APP.DATA[ i ].actions[ j ].internal = cfg_app_data_actions_item_internal->valueint;
                                LOG_print( "\t\t· internal=%s\n", P_APP.DATA[ i ].actions[ j ].internal == 1 ? "TRUE" : "FALSE" );

                                cfg_app_data_actions_item_condition = cJSON_GetObjectItem( cfg_app_data_actions_item, "condition" );
                                if( cfg_app_data_actions_item_condition == NULL ) {
                                    LOG_print( "ERROR: \"app.DATA[%d].actions[%d].condition\" key not found.\n", i, j );
                                    cJSON_Delete( config_json );
                                    return FALSE;
                                }
                                size_t str_len3 = strlen( cfg_app_data_actions_item_condition->valuestring );
                                P_APP.DATA[ i ].actions[ j ].condition = SAFECALLOC( str_len3 + 1, sizeof( char ), __FILE__, __LINE__ );
                                snprintf( P_APP.DATA[ i ].actions[ j ].condition, str_len3+1, cfg_app_data_actions_item_condition->valuestring );
                                LOG_print( "\t\t· condition=\"%s\"\n", P_APP.DATA[ i ].actions[ j ].condition );

                                cfg_app_data_actions_item_sql = cJSON_GetObjectItem( cfg_app_data_actions_item, "sql" );
                                if( cfg_app_data_actions_item_sql == NULL ) {
                                    LOG_print( "ERROR: \"app.DATA[%d].actions[%d].sql\" key not found.\n", i, j );
                                    cJSON_Delete( config_json );
                                    return FALSE;
                                }
                                size_t str_len4 = strlen( cfg_app_data_actions_item_sql->valuestring );
                                P_APP.DATA[ i ].actions[ j ].sql = SAFECALLOC( str_len4 + 1, sizeof( char ), __FILE__, __LINE__ );
                                strncpy( P_APP.DATA[ i ].actions[ j ].sql, cfg_app_data_actions_item_sql->valuestring, str_len4+1 );
                                LOG_print( "\t\t· sql=\"%s\"\n", P_APP.DATA[ i ].actions[ j ].sql );
                            }
                        } else {
                            LOG_print( "ERROR: \"app.DATA[%d].actions\" key not found.\n", i );
                            cJSON_Delete( config_json );
                            free( config_string ); config_string = NULL;
                            return FALSE;
                        }
                    }
                } else {
                    LOG_print( "ERROR: \"app.DATA\" key not found.\n " );
                    cJSON_Delete( config_json );
                    free( config_string ); config_string = NULL;
                    return FALSE;
                }
            } else {
                LOG_print( "ERROR: \"app\" key not found.\n " );
                cJSON_Delete( config_json );
                free( config_string ); config_string = NULL;
                return FALSE;
            }

            cJSON_Delete( config_json );
        }
        free( config_string );
        config_string = NULL;

        result = 1;
    } else {
        LOG_print( "[%s] Fatal error: unable to open config file \"%s\"!\n", TIME_get_gmt(), filename );
    }

    return result;
}


int main( int argc, char** argv ) {
    char        config_file[ MAX_PATH_LENGTH + 1 ];

    signal( SIGINT, (__sighandler_t)&app_terminate );
#ifndef _WIN32
    signal( SIGPIPE, SIG_IGN );
#endif
    signal( SIGABRT, (__sighandler_t)&app_terminate );
    signal( SIGTERM, (__sighandler_t)&app_terminate );

    LOG_init( "dcpam-wds" );

    memset( config_file, '\0', MAX_PATH_LENGTH );
    if( argc <= 1 ) {
        snprintf( config_file, MAX_PATH_LENGTH, "wds_config.json" );
    } else {
        if( strlen( argv[ 1 ] ) > MAX_PATH_LENGTH ) {
            LOG_print( "[%s] Notice: \"%s\" is not valid config file name.\n", TIME_get_gmt(), argv[ 1 ] );
            snprintf( config_file, MAX_PATH_LENGTH, "wds_config.json" );
        } else {
            snprintf( config_file, MAX_PATH_LENGTH, "%s", argv[ 1 ] );
        }
    }

    if( DCPAM_load_configuration( config_file ) == 1 ) {
        /*if( DB_WORKER_init() == 1 ) {
            //while( 1 );
        }*/
        LOG_print( "[%s] DCPAM Warehouse Data Server configuration loaded.\n", TIME_get_gmt() );
    }

    DCPAM_free_configuration();
    LOG_print( "[%s] DCPAM Warehouse Data Server finished.\n", TIME_get_gmt() );
    return 0;
}
