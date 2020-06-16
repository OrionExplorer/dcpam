#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "../../include/shared.h"
#include "../../include/core/db/system.h"
#include "../../include/utils/memory.h"
#include "../../include/utils/log.h"
#include "../../include/utils/time.h"

DATABASE_SYSTEM     DATABASE_SYSTEMS[MAX_DATA_SYSTEMS];
int                 DATABASE_SYSTEMS_COUNT;


int DB_exec(
    DATABASE_SYSTEM_DB  *db,
    const char          *sql_template,
    unsigned long       sql_length,
    DB_QUERY            *dst_result,
    const char* const   *param_values,
    const int           params_count,
    const int           *param_lengths,
    const int           *param_formats,
    const char          *param_types
) {
    char            *sql_bound = NULL;
    unsigned long   sql_bound_len = 0;
    char            *sql = ( char* )sql_template;
    unsigned long   *sql_len = &sql_length;
    int             q_ret = 0;

    if( DB_QUERY_format( sql_template, &sql_bound, &sql_bound_len, param_values, params_count, param_lengths ) == FALSE ) {
        LOG_print( "[%s] DB_QUERY_format error.\n", TIME_get_gmt() );
        return FALSE;
    }

    sql = sql_bound ? sql_bound : sql_template;
    sql_len = sql_bound ? &sql_bound_len : &sql_length;


    switch( db->driver ) {
        case D_POSTGRESQL : {
            q_ret = PG_exec( &db->db_conn.pgsql_conn, sql, *sql_len, dst_result, NULL, 0, NULL, NULL, NULL );
        } break;

        case D_MYSQL : {
            q_ret = MYSQL_exec( &db->db_conn.mysql_conn, sql, *sql_len, dst_result, NULL, 0, NULL, NULL, NULL );
        } break;

        case D_MARIADB : {
            q_ret = MARIADB_exec( &db->db_conn.mariadb_conn, sql, *sql_len, dst_result, NULL, 0, NULL, NULL, NULL );
        } break;

        case D_ODBC : {
            q_ret = ODBC_exec( &db->db_conn.odbc_conn, sql, *sql_len, dst_result, NULL, 0, NULL, NULL, NULL );
        } break;

        case D_ORACLE: {
            q_ret = ORACLE_exec( &db->db_conn.oracle_conn, sql, *sql_len, dst_result, NULL, 0, NULL, NULL );
        }
    }

    free( sql_bound ); sql_bound = NULL;

    return q_ret;
}


void SYSTEM_QUERY_free( DATABASE_SYSTEM_QUERY *dst ) {
    /*int i = 0;*/

    if( dst->name != NULL ) {
        free( dst->name ); dst->name = NULL;
    }

    if( dst->change_data_capture.extract.inserted.primary_db_sql != NULL ) {
        free( dst->change_data_capture.extract.inserted.primary_db_sql ); dst->change_data_capture.extract.inserted.primary_db_sql = NULL;
    }
    if( dst->change_data_capture.extract.inserted.secondary_db_sql != NULL ) {
        free( dst->change_data_capture.extract.inserted.secondary_db_sql ); dst->change_data_capture.extract.inserted.secondary_db_sql = NULL;
    }
    if( dst->change_data_capture.extract.inserted.primary_db != NULL ) {
        free( dst->change_data_capture.extract.inserted.primary_db ); dst->change_data_capture.extract.inserted.primary_db = NULL;
    }
    if( dst->change_data_capture.extract.inserted.secondary_db != NULL ) {
        free( dst->change_data_capture.extract.inserted.secondary_db ); dst->change_data_capture.extract.inserted.secondary_db = NULL;
    }

    if( dst->change_data_capture.extract.deleted.primary_db_sql != NULL ) {
        free( dst->change_data_capture.extract.deleted.primary_db_sql ); dst->change_data_capture.extract.deleted.primary_db_sql = NULL;
    }
    if( dst->change_data_capture.extract.deleted.secondary_db_sql != NULL ) {
        free( dst->change_data_capture.extract.deleted.secondary_db_sql ); dst->change_data_capture.extract.deleted.secondary_db_sql = NULL;
    }
    if( dst->change_data_capture.extract.deleted.primary_db != NULL ) {
        free( dst->change_data_capture.extract.deleted.primary_db ); dst->change_data_capture.extract.deleted.primary_db = NULL;
    }
    if( dst->change_data_capture.extract.deleted.secondary_db != NULL ) {
        free( dst->change_data_capture.extract.deleted.secondary_db ); dst->change_data_capture.extract.deleted.secondary_db = NULL;
    }

    if( dst->change_data_capture.extract.modified.primary_db_sql != NULL ) {
        free( dst->change_data_capture.extract.modified.primary_db_sql ); dst->change_data_capture.extract.modified.primary_db_sql = NULL;
    }
    if( dst->change_data_capture.extract.modified.secondary_db_sql != NULL ) {
        free( dst->change_data_capture.extract.modified.secondary_db_sql ); dst->change_data_capture.extract.modified.secondary_db_sql = NULL;
    }
    if( dst->change_data_capture.extract.modified.primary_db != NULL ) {
        free( dst->change_data_capture.extract.modified.primary_db ); dst->change_data_capture.extract.modified.primary_db = NULL;
    }
    if( dst->change_data_capture.extract.modified.secondary_db != NULL ) {
        free( dst->change_data_capture.extract.modified.secondary_db ); dst->change_data_capture.extract.modified.secondary_db = NULL;
    }

    /*for( i = 0; i < MAX_TRANSFORM_ELEMENTS; i++ != NULL ) {
        if( dst->change_data_capture.transform.inserted[ i ].column != NULL != NULL ) {
            free( dst->change_data_capture.transform.inserted[ i ].column ); dst->change_data_capture.transform.inserted[ i ].column = NULL;
        }
        if( dst->change_data_capture.transform.inserted[ i ].expression != NULL != NULL ) {
            free( dst->change_data_capture.transform.inserted[ i ].expression ); dst->change_data_capture.transform.inserted[ i ].expression = NULL;
        }
        if( dst->change_data_capture.transform.deleted[ i ].column != NULL != NULL ) {
            free( dst->change_data_capture.transform.deleted[ i ].column ); dst->change_data_capture.transform.deleted[ i ].column = NULL;
        }
        if( dst->change_data_capture.transform.deleted[ i ].expression != NULL != NULL ) {
            free( dst->change_data_capture.transform.deleted[ i ].expression ); dst->change_data_capture.transform.deleted[ i ].expression = NULL;
        }
        if( dst->change_data_capture.transform.modified[ i ].column != NULL != NULL ) {
            free( dst->change_data_capture.transform.modified[ i ].column ); dst->change_data_capture.transform.modified[ i ].column = NULL;
        }
        if( dst->change_data_capture.transform.modified[ i ].expression != NULL != NULL ) {
            free( dst->change_data_capture.transform.modified[ i ].expression ); dst->change_data_capture.transform.modified[ i ].expression = NULL;
        }
    }*/

    if( dst->change_data_capture.load.inserted.sql != NULL ) {
        free( dst->change_data_capture.load.inserted.sql ); dst->change_data_capture.load.inserted.sql = NULL;
    }
    if( dst->change_data_capture.load.deleted.sql != NULL ) {
        free( dst->change_data_capture.load.deleted.sql ); dst->change_data_capture.load.deleted.sql = NULL;
    }
    if( dst->change_data_capture.load.modified.sql != NULL ) {
        free( dst->change_data_capture.load.modified.sql ); dst->change_data_capture.load.modified.sql = NULL;
    }

}


void DATABASE_SYSTEM_QUERY_add(
    const char          *name,
    DB_SYSTEM_CDC   cdc,
    const char          data_types[SMALL_BUFF_SIZE][SMALL_BUFF_SIZE],
    const int           data_types_len,
    DATABASE_SYSTEM_QUERY       *dst,
    short               verbose
) {
    int     i = 0;
    size_t  str_len = 0;

    if( verbose > 0 ) LOG_print( "[%s] SYSTEM_query_add(\"%s\", ... ).\n", TIME_get_gmt(), name );
    /*str_len = strlen( name );
    dst->name = ( char * )SAFECALLOC( str_len+1, sizeof( char ), __FILE__, __LINE__ );*/
    dst->name = strdup( name );
    if( verbose > 0 ) LOG_print("\t· name=\"%s\"\n", name );
    /*if( dst->name ) {
        if( verbose > 0 ) LOG_print("\t· name=\"%s\"\n", name );
        strncpy(
            dst->name,
            name,
            str_len
        );
    }*/

    dst->change_data_capture = cdc;

    if( verbose > 0 ) LOG_print("\t· extract\n\t\t·inserted\n\t\t\t·primary_db_sql: \"%.70s(...)\"\n", cdc.extract.inserted.primary_db_sql );
    if( verbose > 0 ) LOG_print("\t\t\t·primary_db: \"%s\"\n", cdc.extract.inserted.primary_db );
    if( verbose > 0 ) LOG_print("\t\t\t·secondary_db_sql: \"%.70s(...)\"\n", cdc.extract.inserted.secondary_db_sql );
    if( verbose > 0 ) LOG_print("\t\t\t·secondary_db: \"%s\"\n", cdc.extract.inserted.secondary_db );
    if( verbose > 0 ) LOG_print("\t\t·modified\n\t\t\t·primary_db_sql: \"%.70s(...)\"\n", cdc.extract.modified.primary_db_sql );
    if( verbose > 0 ) LOG_print("\t\t\t·primary_db: \"%s\"\n", cdc.extract.modified.primary_db );
    if( verbose > 0 ) LOG_print("\t\t\t·secondary_db_sql: \"%.70s(...)\"\n", cdc.extract.modified.secondary_db_sql );
    if( verbose > 0 ) LOG_print("\t\t\t·secondary_db: \"%s\"\n", cdc.extract.modified.secondary_db );
    if( verbose > 0 ) LOG_print("\t\t·deleted\n\t\t\t·primary_db_sql: \"%.70s(...)\"\n", cdc.extract.deleted.primary_db_sql );
    if( verbose > 0 ) LOG_print("\t\t\t·primary_db: \"%s\"\n", cdc.extract.deleted.primary_db );
    if( verbose > 0 ) LOG_print("\t\t\t·secondary_db_sql: \"%.70s(...)\"\n", cdc.extract.deleted.secondary_db_sql );
    if( verbose > 0 ) LOG_print("\t\t\t·secondary_db: \"%s\"\n", cdc.extract.deleted.secondary_db );
    if( verbose > 0 ) LOG_print("\t· load\n\t\t·inserted\n\t\t\t·sql: \"%.70s(...)\"\n", cdc.load.inserted.sql );
    if( verbose > 0 ) LOG_print("\t\t\t·extracted_values: " );
    for( i = 0; i < cdc.load.inserted.extracted_values_len; i++ ) {
        if( verbose > 0 ) LOG_print("'%s', ", cdc.load.inserted.extracted_values[i]);
    }
    if( verbose > 0 ) LOG_print("\n");
    if( verbose > 0 ) LOG_print("\t\t·deleted\n\t\t\t·sql: \"%.70s(...)\"\n", cdc.load.deleted.sql );
    if( verbose > 0 ) LOG_print("\t\t\t·extracted_values: " );
    for( i = 0; i < cdc.load.deleted.extracted_values_len; i++ ) {
        if( verbose > 0 ) printf("'%s', ", cdc.load.deleted.extracted_values[i]);
    }
    if( verbose > 0 ) LOG_print("\n");
    if( verbose > 0 ) LOG_print("\t\t·modified\n\t\t\t·sql: \"%.70s(...)\"\n", cdc.load.modified.sql );
    if( verbose > 0 ) LOG_print("\t\t\t·extracted_values: " );
    for( i = 0; i < cdc.load.modified.extracted_values_len; i++ ) {
        if( verbose > 0 ) LOG_print("'%s', ", cdc.load.modified.extracted_values[i]);
    }
    if( verbose > 0 ) LOG_print("\n");

    dst->data_types_len = data_types_len;
    for( i = 0; i < data_types_len; i++ ) {
        if( verbose > 0 ) LOG_print("\t· data_types[%d]=\"%s\"\n", i, data_types[ i ] );
        strncpy(
            dst->data_types[ i ],
            data_types[ i ],
            SMALL_BUFF_SIZE
        );
    }
}

void DATABASE_SYSTEM_DB_add(
    const char              *ip,
    const int               port,
    const DB_DRIVER         driver,
    const char              *user,
    const char              *password,
    const char              *db,
    const char              *connection_string,
    DATABASE_SYSTEM_DB      *dst,
    short                   verbose
) {
    size_t  str_len = 0;

    if( verbose > 0 ) {
        LOG_print( "[%s] SYSTEM_db_add(\"%s\", %d, \"%s\", ... ).\n", TIME_get_gmt(), ip, port, user );
        LOG_print("\t· ip=\"%s\".\n", ip );
    }
    dst->ip = ( char* )SAFECALLOC( TINY_BUFF_SIZE, sizeof( char ), __FILE__, __LINE__ );
    if( dst->ip ) {
        strncpy(
            dst->ip,
            ip,
            TINY_BUFF_SIZE
        );
    }

    if( verbose > 0 ) LOG_print("\t· port=\"%d\"\n", port );
    dst->port = port;

    if( verbose > 0 ) LOG_print("\t· driver=\"%s\"\n", 
        driver == D_POSTGRESQL ? "PostgreSQL" 
        : driver == D_MYSQL ? "MySQL" 
        : driver == D_MARIADB ? "MariaDB" 
        : driver == D_ODBC ? "ODBC" 
        : "ORACLE"
    );
    dst->driver = ( DB_DRIVER )driver;

    if( verbose > 0 ) LOG_print("\t· user=\"%s\"\n", user );
    dst->user = strdup( user );
    /*str_len = strlen(user);
    dst->user = ( char* )SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
    if( dst->user ) {
        strncpy(
            dst->user,
            user,
            str_len
        );
    }*/

    if( verbose > 0 ) LOG_print("\t· password=\"%.1s***\"\n", password );
    dst->password = strdup( password );
    /*str_len = strlen(password);
    dst->password = ( char* )SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
    if( dst->password ) {
        strncpy(
            dst->password,
            password,
            str_len
        );
    }*/

    if( verbose > 0 ) LOG_print("\t· db=\"%s\"\n", db );
    dst->db = strdup( db );
    /*str_len = strlen(db);
    dst->db = ( char* )SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
    if( dst->db ) {
        strncpy(
            dst->db,
            db,
            str_len
        );
    }*/

    if( connection_string != NULL ) {
        if( verbose > 0 ) LOG_print("\t· connection_string=\"%s\"\n", connection_string );
        dst->connection_string = strdup( connection_string );
        /*str_len = strlen( connection_string );
        dst->connection_string = ( char* )SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
        if( dst->connection_string ) {
            strncpy(
                dst->connection_string,
                connection_string,
                str_len
            );
        }*/
    }
}


void DATABASE_SYSTEM_DB_free( DATABASE_SYSTEM_DB *db ) {

    if( db->ip != NULL ) {
        LOG_print( "[%s] DATABASE_SYSTEM_DB_free( %s@%s:%d )...\n", TIME_get_gmt(), db->user, db->ip, db->port );
        free( db->ip ); db->ip = NULL;
    }
    if( db->user != NULL ) {
        free( db->user ); db->user = NULL;
    }
    if( db->password != NULL ) {
        free( db->password ); db->password = NULL;
    }
    if( db->db != NULL ) {
        free( db->db ); db->db = NULL;
    }
    if( db->connection_string != NULL ) {
        free( db->connection_string ); db->connection_string = NULL;
    }

    switch( db->driver ) {
        case D_POSTGRESQL : {
            
            LOG_print( "\t· Driver: \"PostgreSQL\". Disconnecting...\n" );
            PG_disconnect( &db->db_conn.pgsql_conn );
        } break;
        case D_MYSQL : {
            
            LOG_print( "\t· Driver: \"MySQL\". Disconnecting...\n" );
            MYSQL_disconnect( &db->db_conn.mysql_conn );
        } break;
         case D_MARIADB : {
            
            LOG_print( "\t· Driver: \"MariaDB\". Disconnecting...\n" );
            MARIADB_disconnect( &db->db_conn.mariadb_conn );
        } break;
        case D_ODBC : {
            
            LOG_print( "\t· Driver: \"ODBC\". Disconnecting...\n" );
            ODBC_disconnect( &db->db_conn.odbc_conn );
        } break;
        case D_ORACLE : {

            LOG_print( "\t· Driver: \"Oracle\". Disconnecting...\n" );
            ORACLE_disconnect( &db->db_conn.oracle_conn );
        }
        default : {
        }
    }
    db->driver = 0;
}


int DATABASE_SYSTEM_DB_init( DATABASE_SYSTEM_DB *db ) {
    int         ret = FALSE;

    /* Connect with defined database */
    switch( db->driver ) {
        case D_POSTGRESQL : {
            LOG_print( "\t· Driver: \"PostgreSQL\". Connecting..." );
            ret = PG_connect( &db->db_conn.pgsql_conn, db->ip, db->port, db->db, db->user, db->password, db->connection_string );
        } break;
        case D_MYSQL : {
            LOG_print( "\t· Driver: \"MySQL\". Connecting..." );
            ret = MYSQL_connect( &db->db_conn.mysql_conn, db->ip, db->port, db->db, db->user, db->password, db->connection_string );
        } break;
        case D_MARIADB : {
            LOG_print( "\t· Driver: \"MariaDB\". Connecting..." );
            ret = MARIADB_connect( &db->db_conn.mariadb_conn, db->ip, db->port, db->db, db->user, db->password, db->connection_string );
        } break;
        case D_ODBC : {
            LOG_print( "\t· Driver: \"ODBC\". Connecting (%s)...", db->connection_string );
            ret = ODBC_connect( &db->db_conn.odbc_conn, db->ip, db->port, db->db, db->user, db->password, db->connection_string );
        } break;
        case D_ORACLE : {
            LOG_print( "\t· Driver: \"Oracle\". Connecting..." );
            ret = ORACLE_connect( &db->db_conn.oracle_conn, db->ip, db->port, db->db, db->user, db->password, db->connection_string );
        } break;
        default : {
            LOG_print( "Error: unknown driver: \"%d\".\n", db->driver );
        }
    }

    return ret;
}


void DATABASE_SYSTEM_close( DATABASE_SYSTEM *system ) {

    if( system != NULL ) {
        if( system->name != NULL ) {
            LOG_print( "[%s] DATABASE_SYSTEM_close(\"%s\").\n", TIME_get_gmt(), system->name );
            free( system->name ); system->name = NULL;
        }
        for( int i = 0; i < system->queries_len; i++ ) {
            SYSTEM_QUERY_free( &system->queries[ i ] );
        }
        DATABASE_SYSTEM_DB_free( &system->DB );
    }
}


void DATABASE_SYSTEM_add(
    const char      *name,
    DATABASE_SYSTEM_DB      *db,
    DATABASE_SYSTEM_QUERY   queries[ MAX_SYSTEM_QUERIES ],
    const int       queries_len,
    short           verbose
) {

    if( verbose > 0 ) LOG_print( "[%s] DATABASE_SYSTEM_add( \"%s\", ... ).\n", TIME_get_gmt(), name );
    if( DATABASE_SYSTEMS_COUNT < MAX_DATA_SYSTEMS ) {

        int j = 0;

        DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].name = strdup( name );
        /*name_len = strlen(name);
        DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].name = ( char* )SAFECALLOC( name_len + 1, sizeof( char ), __FILE__, __LINE__ );
        if( DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].name ) {
            strncpy(
                DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].name,
                name,
                name_len
            );
        }*/

        DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].queries_len = queries_len;

        for( j = 0; j < queries_len; j++ ) {
            DATABASE_SYSTEM_QUERY_add(
                queries[ j ].name,
                queries[ j ].change_data_capture,
                queries[ j ].data_types,
                queries[ j ].data_types_len,
                &DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].queries[ j ],
                TRUE
            );
        }

        for( j = 0; j < queries_len; j++ ) {
            free( queries[ j ].name ); queries[ j ].name = NULL;
        }

        DATABASE_SYSTEM_DB_add(
            db->ip,
            db->port,
            db->driver,
            db->user,
            db->password,
            db->db,
            db->connection_string ? db->connection_string : NULL,
            &DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].DB,
            TRUE
        );

        free( db->ip ); db->ip = NULL;
        free( db->user ); db->user = NULL;
        free( db->password ); db->password = NULL;
        free( db->db ); db->db = NULL;

        if( db->connection_string != NULL ) {
            free( db->connection_string ); db->connection_string = NULL;
        }

        DATABASE_SYSTEMS_COUNT++;
    } else {
        LOG_print( "error. Maximum data systems limit exceeded.\n" );
    }
}
