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
    size_t              sql_length,
    DB_QUERY            *dst_result,
    const char* const   *param_values,
    const int           params_count,
    const int           *param_lengths,
    const int           *param_formats,
    const char          *param_types,
    qec                 *query_exec_callback,
    void                *data_ptr1,
    void                *data_ptr2,
    LOG_OBJECT          *log
) {
    char            *sql_bound = NULL;
    size_t          sql_bound_len = 0;
    char            *sql = ( char* )sql_template;
    size_t          *sql_len = &sql_length;
    int             q_ret = 0;

    if( DB_QUERY_format( sql_template, &sql_bound, &sql_bound_len, param_values, params_count, param_lengths, log ) == FALSE ) {
        LOG_print( log, "[%s] DB_QUERY_format error.\n", TIME_get_gmt() );
        return FALSE;
    }

    sql = sql_bound ? sql_bound : ( char *)sql_template;
    sql_len = sql_bound ? &sql_bound_len : &sql_length;

    switch( db->driver ) {
        case D_POSTGRESQL : {
            q_ret = PG_exec( &db->db_conn.pgsql_conn, sql, *sql_len, dst_result, NULL, 0, NULL, NULL, NULL, query_exec_callback, data_ptr1, data_ptr2, log );
        } break;

        case D_MYSQL : {
            q_ret = MYSQL_exec( &db->db_conn.mysql_conn, sql, *sql_len, dst_result, NULL, 0, NULL, NULL, NULL, query_exec_callback, data_ptr1, data_ptr2, log );
        } break;

        case D_MARIADB : {
            q_ret = MARIADB_exec( &db->db_conn.mariadb_conn, sql, *sql_len, dst_result, NULL, 0, NULL, NULL, NULL, query_exec_callback, data_ptr1, data_ptr2, log );
        } break;

        case D_ODBC : {
            q_ret = ODBC_exec( &db->db_conn.odbc_conn, sql, *sql_len, dst_result, NULL, 0, NULL, NULL, NULL, query_exec_callback, data_ptr1, data_ptr2, log );
        } break;

        case D_ORACLE: {
            q_ret = ORACLE_exec( &db->db_conn.oracle_conn, sql, *sql_len, dst_result, NULL, 0, NULL, NULL, query_exec_callback, data_ptr1, data_ptr2, log );
        } break;

        case D_SQLITE: {
            q_ret = SQLITE_exec( &db->db_conn.sqlite_conn, sql, *sql_len, dst_result, NULL, 0, NULL, NULL, query_exec_callback, data_ptr1, data_ptr2, log );
        }

        case D_MONGODB: {
            q_ret = MONGODB_exec( &db->db_conn.mongodb_conn, sql, *sql_len, dst_result, NULL, 0, NULL, NULL, NULL, query_exec_callback, data_ptr1, data_ptr2, log );
        }
    }

    free( sql_bound ); sql_bound = NULL;

    return q_ret;
}


void SYSTEM_ETL_CONFIG_free( DB_SYSTEM_ETL *dst ) {
    if( dst->pre_actions != NULL ) {
        for( int i = 0; i < dst->pre_actions_count; i++ ) {
            free( dst->pre_actions[ i ]->sql ); dst->pre_actions[ i ]->sql = NULL;
            free( dst->pre_actions[ i ] ); dst->pre_actions[ i ] = NULL;
        }
        free( dst->pre_actions ); dst->pre_actions = NULL;
    }

    if( dst->post_actions != NULL ) {
        for( int i = 0; i < dst->post_actions_count; i++ ) {
            free( dst->post_actions[ i ]->sql ); dst->post_actions[ i ]->sql = NULL;
            free( dst->post_actions[ i ] ); dst->post_actions[ i ] = NULL;
        }
        free( dst->post_actions ); dst->post_actions = NULL;
    }

    if( dst->extract.inserted.primary_db_sql != NULL ) {
        free( dst->extract.inserted.primary_db_sql ); dst->extract.inserted.primary_db_sql = NULL;
    }
    if( dst->extract.inserted.secondary_db_sql != NULL ) {
        free( dst->extract.inserted.secondary_db_sql ); dst->extract.inserted.secondary_db_sql = NULL;
    }
    if( dst->extract.inserted.primary_db != NULL ) {
        free( dst->extract.inserted.primary_db ); dst->extract.inserted.primary_db = NULL;
    }
    if( dst->extract.inserted.primary_db_result_replace_len > 0 ) {
        if( dst->extract.inserted.primary_db_result_replace != NULL ) {
            free( dst->extract.inserted.primary_db_result_replace ); dst->extract.inserted.primary_db_result_replace = NULL;
        }
        dst->extract.inserted.primary_db_result_replace_len = 0;
    }
    if( dst->extract.inserted.secondary_db != NULL ) {
        free( dst->extract.inserted.secondary_db ); dst->extract.inserted.secondary_db = NULL;
    }
    if( dst->extract.inserted.secondary_db_result_replace_len > 0 ) {
        if( dst->extract.inserted.secondary_db_result_replace != NULL ) {
            free( dst->extract.inserted.secondary_db_result_replace ); dst->extract.inserted.secondary_db_result_replace = NULL;
        }
        dst->extract.inserted.secondary_db_result_replace_len = 0;
    }

    if( dst->extract.deleted.primary_db_sql != NULL ) {
        free( dst->extract.deleted.primary_db_sql ); dst->extract.deleted.primary_db_sql = NULL;
    }
    if( dst->extract.deleted.secondary_db_sql != NULL ) {
        free( dst->extract.deleted.secondary_db_sql ); dst->extract.deleted.secondary_db_sql = NULL;
    }
    if( dst->extract.deleted.primary_db != NULL ) {
        free( dst->extract.deleted.primary_db ); dst->extract.deleted.primary_db = NULL;
    }
    if( dst->extract.deleted.primary_db_result_replace_len > 0 ) {
        if( dst->extract.deleted.primary_db_result_replace != NULL ) {
            free( dst->extract.deleted.primary_db_result_replace ); dst->extract.deleted.primary_db_result_replace = NULL;
        }
        dst->extract.deleted.primary_db_result_replace_len = 0;
    }
    if( dst->extract.deleted.secondary_db != NULL ) {
        free( dst->extract.deleted.secondary_db ); dst->extract.deleted.secondary_db = NULL;
    }
    if( dst->extract.deleted.secondary_db_result_replace_len > 0 ) {
        if( dst->extract.deleted.secondary_db_result_replace != NULL ) {
            free( dst->extract.deleted.secondary_db_result_replace ); dst->extract.deleted.secondary_db_result_replace = NULL;
        }
        dst->extract.deleted.secondary_db_result_replace_len = 0;
    }

    if( dst->extract.modified.primary_db_sql != NULL ) {
        free( dst->extract.modified.primary_db_sql ); dst->extract.modified.primary_db_sql = NULL;
    }
    if( dst->extract.modified.secondary_db_sql != NULL ) {
        free( dst->extract.modified.secondary_db_sql ); dst->extract.modified.secondary_db_sql = NULL;
    }
    if( dst->extract.modified.primary_db != NULL ) {
        free( dst->extract.modified.primary_db ); dst->extract.modified.primary_db = NULL;
    }
    if( dst->extract.modified.primary_db_result_replace_len > 0 ) {
        if( dst->extract.modified.primary_db_result_replace != NULL ) {
            free( dst->extract.modified.primary_db_result_replace ); dst->extract.modified.primary_db_result_replace = NULL;
        }
        dst->extract.modified.primary_db_result_replace_len = 0;
    }
    if( dst->extract.modified.secondary_db != NULL ) {
        free( dst->extract.modified.secondary_db ); dst->extract.modified.secondary_db = NULL;
    }
    if( dst->extract.modified.secondary_db_result_replace_len > 0 ) {
        if( dst->extract.modified.secondary_db_result_replace != NULL ) {
            free( dst->extract.modified.secondary_db_result_replace ); dst->extract.modified.secondary_db_result_replace = NULL;
        }
        dst->extract.modified.secondary_db_result_replace_len = 0;
    }

    if( dst->transform ) {

        for( int i = 0; i < dst->transform->inserted_count; i++ ) {
            free( dst->transform->inserted[ i ]->module ); dst->transform->inserted[ i ]->module = NULL;
            free( dst->transform->inserted[ i ]->staged_data ); dst->transform->inserted[ i ]->staged_data = NULL;
            free( dst->transform->inserted[ i ]->source_system_update ); dst->transform->inserted[ i ]->source_system_update = NULL;
            free( dst->transform->inserted[ i ]->api_key ); dst->transform->inserted[ i ]->api_key = NULL;
            free( dst->transform->inserted[ i ] ); dst->transform->inserted[ i ] = NULL;
        }
        free( dst->transform->inserted ); dst->transform->inserted = NULL;

        for( int i = 0; i < dst->transform->deleted_count; i++ ) {
            free( dst->transform->deleted[ i ]->module ); dst->transform->deleted[ i ]->module = NULL;
            free( dst->transform->deleted[ i ]->staged_data ); dst->transform->deleted[ i ]->staged_data = NULL;
            free( dst->transform->deleted[ i ]->source_system_update ); dst->transform->deleted[ i ]->source_system_update = NULL;
            free( dst->transform->deleted[ i ]->api_key ); dst->transform->deleted[ i ]->api_key = NULL;
            free( dst->transform->deleted[ i ] ); dst->transform->deleted[ i ] = NULL;
        }
        free( dst->transform->deleted ); dst->transform->deleted = NULL;

        for( int i = 0; i < dst->transform->modified_count; i++ ) {
            free( dst->transform->modified[ i ]->module ); dst->transform->modified[ i ]->module = NULL;
            free( dst->transform->modified[ i ]->staged_data ); dst->transform->modified[ i ]->staged_data = NULL;
            free( dst->transform->modified[ i ]->source_system_update ); dst->transform->modified[ i ]->source_system_update = NULL;
            free( dst->transform->modified[ i ]->api_key ); dst->transform->modified[ i ]->api_key = NULL;
            free( dst->transform->modified[ i ] ); dst->transform->modified[ i ] = NULL;
        }
        free( dst->transform->modified ); dst->transform->modified = NULL;

        free( dst->transform ); dst->transform = NULL;
    }

    if( dst->stage ) {
        for( int i = 0; i < dst->stage->inserted_count; i++ ) {
            if( dst->stage->inserted[ i ]->sql != NULL ) {
                free( dst->stage->inserted[ i ]->sql ); dst->stage->inserted[ i ]->sql = NULL;
            }
            free( dst->stage->inserted[ i ] ); dst->stage->inserted[ i ] = NULL;
        }
        free( dst->stage->inserted ); dst->stage->inserted = NULL;

        for( int i = 0; i < dst->stage->deleted_count; i++ ) {
            if( dst->stage->deleted[ i ]->sql != NULL ) {
                free( dst->stage->deleted[ i ]->sql ); dst->stage->deleted[ i ]->sql = NULL;
            }
            free( dst->stage->deleted[ i ] ); dst->stage->deleted[ i ] = NULL;
        }
        free( dst->stage->deleted ); dst->stage->deleted = NULL;
        
        for( int i = 0; i < dst->stage->modified_count; i++ ) {
            if( dst->stage->modified[ i ]->sql != NULL ) {
                free( dst->stage->modified[ i ]->sql ); dst->stage->modified[ i ]->sql = NULL;
            }
            free( dst->stage->modified[ i ] ); dst->stage->modified[ i ] = NULL;
        }
        free( dst->stage->modified ); dst->stage->modified = NULL;

        free( dst->stage ); dst->stage = NULL;
    }

    if( dst->load.inserted.input_data_sql != NULL ) {
        free( dst->load.inserted.input_data_sql ); dst->load.inserted.input_data_sql = NULL;
    }
    for( int i = 0; i < dst->load.inserted.target_count; i++ ) {
        if( dst->load.inserted.target[ i ]->output_data_sql != NULL ) {
            free( dst->load.inserted.target[ i ]->output_data_sql ); dst->load.inserted.target[ i ]->output_data_sql = NULL;
        }
        free( dst->load.inserted.target[ i ] ); dst->load.inserted.target[ i ] = NULL;
    }
    free( dst->load.inserted.target ); dst->load.inserted.target = NULL;
    
    if( dst->load.deleted.input_data_sql!= NULL ) {
        free( dst->load.deleted.input_data_sql ); dst->load.deleted.input_data_sql = NULL;
    }
    for( int i = 0; i < dst->load.deleted.target_count; i++ ) {
        if( dst->load.deleted.target[ i ]->output_data_sql != NULL ) {
            free( dst->load.deleted.target[ i ]->output_data_sql ); dst->load.deleted.target[ i ]->output_data_sql = NULL;
        }
        free( dst->load.deleted.target[ i ] ); dst->load.deleted.target[ i ] = NULL;
    }
    free( dst->load.deleted.target ); dst->load.deleted.target = NULL;

    if( dst->load.modified.input_data_sql!= NULL ) {
        free( dst->load.modified.input_data_sql ); dst->load.modified.input_data_sql= NULL;
    }
    for( int i = 0; i < dst->load.modified.target_count; i++ ) {
        if( dst->load.modified.target[ i ]->output_data_sql != NULL ) {
            free( dst->load.modified.target[ i ]->output_data_sql ); dst->load.modified.target[ i ]->output_data_sql = NULL;
        }
        free( dst->load.modified.target[ i ] ); dst->load.modified.target[ i ] = NULL;
    }
    free( dst->load.modified.target ); dst->load.modified.target = NULL;
}


void SYSTEM_QUERY_free( DATABASE_SYSTEM_QUERY *dst ) {

    if( dst->name != NULL ) {
        free( dst->name ); dst->name = NULL;
    }

    dst->mode = M_ETL;

    SYSTEM_ETL_CONFIG_free( &dst->etl_config );

    /*if( dst->etl_config.pre_actions != NULL ) {
        for( int i = 0; i < dst->etl_config.pre_actions_count; i++ ) {
            free( dst->etl_config.pre_actions[ i ]->sql ); dst->etl_config.pre_actions[ i ]->sql = NULL;
            free( dst->etl_config.pre_actions[ i ] ); dst->etl_config.pre_actions[ i ] = NULL;
        }
        free( dst->etl_config.pre_actions ); dst->etl_config.pre_actions = NULL;
    }

    if( dst->etl_config.post_actions != NULL ) {
        for( int i = 0; i < dst->etl_config.post_actions_count; i++ ) {
            free( dst->etl_config.post_actions[ i ]->sql ); dst->etl_config.post_actions[ i ]->sql = NULL;
            free( dst->etl_config.post_actions[ i ] ); dst->etl_config.post_actions[ i ] = NULL;
        }
        free( dst->etl_config.post_actions ); dst->etl_config.post_actions = NULL;
    }

    if( dst->etl_config.extract.inserted.primary_db_sql != NULL ) {
        free( dst->etl_config.extract.inserted.primary_db_sql ); dst->etl_config.extract.inserted.primary_db_sql = NULL;
    }
    if( dst->etl_config.extract.inserted.secondary_db_sql != NULL ) {
        free( dst->etl_config.extract.inserted.secondary_db_sql ); dst->etl_config.extract.inserted.secondary_db_sql = NULL;
    }
    if( dst->etl_config.extract.inserted.primary_db != NULL ) {
        free( dst->etl_config.extract.inserted.primary_db ); dst->etl_config.extract.inserted.primary_db = NULL;
    }
    if( dst->etl_config.extract.inserted.secondary_db != NULL ) {
        free( dst->etl_config.extract.inserted.secondary_db ); dst->etl_config.extract.inserted.secondary_db = NULL;
    }

    if( dst->etl_config.extract.deleted.primary_db_sql != NULL ) {
        free( dst->etl_config.extract.deleted.primary_db_sql ); dst->etl_config.extract.deleted.primary_db_sql = NULL;
    }
    if( dst->etl_config.extract.deleted.secondary_db_sql != NULL ) {
        free( dst->etl_config.extract.deleted.secondary_db_sql ); dst->etl_config.extract.deleted.secondary_db_sql = NULL;
    }
    if( dst->etl_config.extract.deleted.primary_db != NULL ) {
        free( dst->etl_config.extract.deleted.primary_db ); dst->etl_config.extract.deleted.primary_db = NULL;
    }
    if( dst->etl_config.extract.deleted.secondary_db != NULL ) {
        free( dst->etl_config.extract.deleted.secondary_db ); dst->etl_config.extract.deleted.secondary_db = NULL;
    }

    if( dst->etl_config.extract.modified.primary_db_sql != NULL ) {
        free( dst->etl_config.extract.modified.primary_db_sql ); dst->etl_config.extract.modified.primary_db_sql = NULL;
    }
    if( dst->etl_config.extract.modified.secondary_db_sql != NULL ) {
        free( dst->etl_config.extract.modified.secondary_db_sql ); dst->etl_config.extract.modified.secondary_db_sql = NULL;
    }
    if( dst->etl_config.extract.modified.primary_db != NULL ) {
        free( dst->etl_config.extract.modified.primary_db ); dst->etl_config.extract.modified.primary_db = NULL;
    }
    if( dst->etl_config.extract.modified.secondary_db != NULL ) {
        free( dst->etl_config.extract.modified.secondary_db ); dst->etl_config.extract.modified.secondary_db = NULL;
    }

    if( dst->etl_config.transform ) {

        for( int i = 0; i < dst->etl_config.transform->inserted_count; i++ ) {
            free( dst->etl_config.transform->inserted[ i ]->module ); dst->etl_config.transform->inserted[ i ]->module = NULL;
            free( dst->etl_config.transform->inserted[ i ]->staged_data ); dst->etl_config.transform->inserted[ i ]->staged_data = NULL;
            free( dst->etl_config.transform->inserted[ i ]->source_system_update ); dst->etl_config.transform->inserted[ i ]->source_system_update = NULL;
            free( dst->etl_config.transform->inserted[ i ]->api_key ); dst->etl_config.transform->inserted[ i ]->api_key = NULL;
            free( dst->etl_config.transform->inserted[ i ] ); dst->etl_config.transform->inserted[ i ] = NULL;
        }
        free( dst->etl_config.transform->inserted ); dst->etl_config.transform->inserted = NULL;

        for( int i = 0; i < dst->etl_config.transform->deleted_count; i++ ) {
            free( dst->etl_config.transform->deleted[ i ]->module ); dst->etl_config.transform->deleted[ i ]->module = NULL;
            free( dst->etl_config.transform->deleted[ i ]->staged_data ); dst->etl_config.transform->deleted[ i ]->staged_data = NULL;
            free( dst->etl_config.transform->deleted[ i ]->source_system_update ); dst->etl_config.transform->deleted[ i ]->source_system_update = NULL;
            free( dst->etl_config.transform->deleted[ i ]->api_key ); dst->etl_config.transform->deleted[ i ]->api_key = NULL;
            free( dst->etl_config.transform->deleted[ i ] ); dst->etl_config.transform->deleted[ i ] = NULL;
        }
        free( dst->etl_config.transform->deleted ); dst->etl_config.transform->deleted = NULL;

        for( int i = 0; i < dst->etl_config.transform->modified_count; i++ ) {
            free( dst->etl_config.transform->modified[ i ]->module ); dst->etl_config.transform->modified[ i ]->module = NULL;
            free( dst->etl_config.transform->modified[ i ]->staged_data ); dst->etl_config.transform->modified[ i ]->staged_data = NULL;
            free( dst->etl_config.transform->modified[ i ]->source_system_update ); dst->etl_config.transform->modified[ i ]->source_system_update = NULL;
            free( dst->etl_config.transform->modified[ i ]->api_key ); dst->etl_config.transform->modified[ i ]->api_key = NULL;
            free( dst->etl_config.transform->modified[ i ] ); dst->etl_config.transform->modified[ i ] = NULL;
        }
        free( dst->etl_config.transform->modified ); dst->etl_config.transform->modified = NULL;

        free( dst->etl_config.transform ); dst->etl_config.transform = NULL;
    }

    if( dst->etl_config.stage ) {
        if( dst->etl_config.stage->inserted.sql != NULL ) {
            free( dst->etl_config.stage->inserted.sql ); dst->etl_config.stage->inserted.sql = NULL;
        }
        if( dst->etl_config.stage->deleted.sql != NULL ) {
            free( dst->etl_config.stage->deleted.sql ); dst->etl_config.stage->deleted.sql = NULL;
        }
        if( dst->etl_config.stage->modified.sql != NULL ) {
            free( dst->etl_config.stage->modified.sql ); dst->etl_config.stage->modified.sql = NULL;
        }

        free( dst->etl_config.stage ); dst->etl_config.stage = NULL;
    }

    if( dst->etl_config.load.inserted.input_data_sql != NULL ) {
        free( dst->etl_config.load.inserted.input_data_sql ); dst->etl_config.load.inserted.input_data_sql = NULL;
    }
    if( dst->etl_config.load.inserted.output_data_sql != NULL ) {
        free( dst->etl_config.load.inserted.output_data_sql ); dst->etl_config.load.inserted.output_data_sql = NULL;
    }
    if( dst->etl_config.load.deleted.input_data_sql!= NULL ) {
        free( dst->etl_config.load.deleted.input_data_sql ); dst->etl_config.load.deleted.input_data_sql = NULL;
    }
    if( dst->etl_config.load.deleted.output_data_sql != NULL ) {
        free( dst->etl_config.load.deleted.output_data_sql ); dst->etl_config.load.deleted.output_data_sql = NULL;
    }
    if( dst->etl_config.load.modified.input_data_sql!= NULL ) {
        free( dst->etl_config.load.modified.input_data_sql ); dst->etl_config.load.modified.input_data_sql= NULL;
    }
    if( dst->etl_config.load.modified.output_data_sql != NULL ) {
        free( dst->etl_config.load.modified.output_data_sql ); dst->etl_config.load.modified.output_data_sql = NULL;
    }*/
}


void DATABASE_SYSTEM_QUERY_add(
    const char              *name,
    DB_SYSTEM_MODE          mode,
    DB_SYSTEM_ETL           etl,
    DATABASE_SYSTEM_QUERY   *dst,
    short                   verbose,
    LOG_OBJECT              *log
) {
    int     i = 0;

    if( verbose > 0 ) LOG_print( log, "[%s] DATABASE_SYSTEM_QUERY_add(\"%s\", ... ).\n", TIME_get_gmt(), name );
    size_t str_len = strlen( name );
    dst->name = ( char * )SAFECALLOC( str_len+1, sizeof( char ), __FILE__, __LINE__ );
    if( dst->name ) {
        if( verbose > 0 ) LOG_print( log, "\t· name=\"%s\"\n", name );
        strlcpy(
            dst->name,
            name,
            str_len
        );
    }

    dst->mode = mode;
    dst->etl_config = etl;

    if( verbose > 0 ) LOG_print( log, "\t· mode = %s\n", mode == M_ETL ? "ETL" : "ELT" );

    if( etl.pre_actions_count > 0 ) {
        if( verbose > 0 ) LOG_print( log, "\t· PreETL Actions:\n" );

        for( int i = 0; i < etl.pre_actions_count; i++ ) {
            if( verbose > 0 ) LOG_print( log, "\t\t·\"%.70s(...)\"\n", etl.pre_actions[ i ]->sql );
        }
    }

    if( verbose > 0 ) LOG_print( log, "\t· extract\n\t\t·inserted\n\t\t\t·primary_db_sql: \"%.70s(...)\"\n", etl.extract.inserted.primary_db_sql );
    if( verbose > 0 ) LOG_print( log, "\t\t\t·primary_db: \"%s\"\n", etl.extract.inserted.primary_db );
    if( verbose > 0 ) {
        if( etl.extract.inserted.primary_db_result_replace_len > 0 ) {
            LOG_print( log, "\t\t\t·primary_db_regex_replace (%d):\n", etl.extract.inserted.primary_db_result_replace_len );
            for( int i = 0; i < etl.extract.inserted.primary_db_result_replace_len; i++ ) {
                LOG_print( log, "\t\t\t\t· search: [%s], replace: [%s]\n", etl.extract.inserted.primary_db_result_replace[ i ].search, etl.extract.inserted.primary_db_result_replace[ i ].replace );
            }
        }
    }
    if( verbose > 0 ) LOG_print( log, "\t\t\t·secondary_db_sql: \"%.70s(...)\"\n", etl.extract.inserted.secondary_db_sql );
    if( verbose > 0 ) LOG_print( log, "\t\t\t·secondary_db: \"%s\"\n", etl.extract.inserted.secondary_db );
    if( verbose > 0 ) {
        if( etl.extract.inserted.secondary_db_result_replace_len > 0 ) {
            LOG_print( log, "\t\t\t·secondary_db_regex_replace (%d):\n", etl.extract.inserted.secondary_db_result_replace_len );
            for( int i = 0; i < etl.extract.inserted.secondary_db_result_replace_len; i++ ) {
                LOG_print( log, "\t\t\t\t· search: [%s], replace: [%s]\n", etl.extract.inserted.secondary_db_result_replace[ i ].search, etl.extract.inserted.secondary_db_result_replace[ i ].replace );
            }
        }
    }
    if( verbose > 0 ) LOG_print( log, "\t\t·modified\n\t\t\t·primary_db_sql: \"%.70s(...)\"\n", etl.extract.modified.primary_db_sql );
    if( verbose > 0 ) LOG_print( log, "\t\t\t·primary_db: \"%s\"\n", etl.extract.modified.primary_db );
    if( verbose > 0 ) {
        if( etl.extract.modified.primary_db_result_replace_len > 0 ) {
            LOG_print( log, "\t\t\t·primary_db_regex_replace (%d):\n", etl.extract.modified.primary_db_result_replace_len );
            for( int i = 0; i < etl.extract.modified.primary_db_result_replace_len; i++ ) {
                LOG_print( log, "\t\t\t\t· search: [%s], replace: [%s]\n", etl.extract.modified.primary_db_result_replace[ i ].search, etl.extract.modified.primary_db_result_replace[ i ].replace );
            }
        }
    }
    if( verbose > 0 ) LOG_print( log, "\t\t\t·secondary_db_sql: \"%.70s(...)\"\n", etl.extract.modified.secondary_db_sql );
    if( verbose > 0 ) LOG_print( log, "\t\t\t·secondary_db: \"%s\"\n", etl.extract.modified.secondary_db );
    if( verbose > 0 ) {
        if( etl.extract.modified.secondary_db_result_replace_len > 0 ) {
            LOG_print( log, "\t\t\t·secondary_db_regex_replace (%d):\n", etl.extract.modified.secondary_db_result_replace_len );
            for( int i = 0; i < etl.extract.modified.secondary_db_result_replace_len; i++ ) {
                LOG_print( log, "\t\t\t\t· search: [%s], replace: [%s]\n", etl.extract.modified.secondary_db_result_replace[ i ].search, etl.extract.modified.secondary_db_result_replace[ i ].replace );
            }
        }
    }
    if( verbose > 0 ) LOG_print( log, "\t\t·deleted\n\t\t\t·primary_db_sql: \"%.70s(...)\"\n", etl.extract.deleted.primary_db_sql );
    if( verbose > 0 ) LOG_print( log, "\t\t\t·primary_db: \"%s\"\n", etl.extract.deleted.primary_db );
    if( verbose > 0 ) {
        if( etl.extract.deleted.primary_db_result_replace_len > 0 ) {
            LOG_print( log, "\t\t\t·primary_db_regex_replace (%d):\n", etl.extract.deleted.primary_db_result_replace_len );
            for( int i = 0; i < etl.extract.deleted.primary_db_result_replace_len; i++ ) {
                LOG_print( log, "\t\t\t\t· search: [%s], replace: [%s]\n", etl.extract.deleted.primary_db_result_replace[ i ].search, etl.extract.deleted.primary_db_result_replace[ i ].replace );
            }
        }
    }
    if( verbose > 0 ) LOG_print( log, "\t\t\t·secondary_db_sql: \"%.70s(...)\"\n", etl.extract.deleted.secondary_db_sql );
    if( verbose > 0 ) LOG_print( log, "\t\t\t·secondary_db: \"%s\"\n", etl.extract.deleted.secondary_db );
    if( verbose > 0 ) {
        if( etl.extract.deleted.secondary_db_result_replace_len > 0 ) {
            LOG_print( log, "\t\t\t·secondary_db_regex_replace (%d):\n", etl.extract.deleted.secondary_db_result_replace_len );
            for( int i = 0; i < etl.extract.deleted.secondary_db_result_replace_len; i++ ) {
                LOG_print( log, "\t\t\t\t· search: [%s], replace: [%s]\n", etl.extract.deleted.secondary_db_result_replace[ i ].search, etl.extract.deleted.secondary_db_result_replace[ i ].replace );
            }
        }
    }
    
    if( etl.stage ) {
        for( int i = 0; i < etl.stage->inserted_count; i++ ) {
            if( verbose > 0 ) LOG_print( log, "\t· stage\n\t\t·inserted[%d]\n\t\t\t·sql: \"%.70s(...)\"\n", i, etl.stage->inserted[ i ]->sql );
            if( verbose > 0 ) LOG_print( log, "\t\t\t·extracted_values: " );
            for( int j = 0; j < etl.stage->inserted[ i ]->extracted_values_len; j++ ) {
                if( verbose > 0 ) LOG_print( log, "'%s', ", etl.stage->inserted[ i ]->extracted_values[ j ] );
            }
            if( verbose > 0 ) LOG_print( log, "\n" );    
        }
        
        for( int i = 0; i < etl.stage->deleted_count; i++ ) {
            if( verbose > 0 ) LOG_print( log, "\t· stage\n\t\t·deleted[%d]\n\t\t\t·sql: \"%.70s(...)\"\n", i, etl.stage->deleted[ i ]->sql );
            if( verbose > 0 ) LOG_print( log, "\t\t\t·extracted_values: " );
            for( int j = 0; j < etl.stage->deleted[ i ]->extracted_values_len; j++ ) {
                if( verbose > 0 ) LOG_print( log, "'%s', ", etl.stage->deleted[ i ]->extracted_values[ j ] );
            }
            if( verbose > 0 ) LOG_print( log, "\n" );    
        }

        for( int i = 0; i < etl.stage->modified_count; i++ ) {
            if( verbose > 0 ) LOG_print( log, "\t· stage\n\t\t·modified[%d]\n\t\t\t·sql: \"%.70s(...)\"\n", i, etl.stage->modified[ i ]->sql );
            if( verbose > 0 ) LOG_print( log, "\t\t\t·extracted_values: " );
            for( int j = 0; j < etl.stage->modified[ i ]->extracted_values_len; j++ ) {
                if( verbose > 0 ) LOG_print( log, "'%s', ", etl.stage->modified[ i ]->extracted_values[ j ] );
            }
            if( verbose > 0 ) LOG_print( log, "\n" );    
        }


        if( verbose > 0 ) LOG_print( log, "\n" );
    }

    for( int i = 0; i < etl.transform->inserted_count; i++ ) {
        if( verbose > 0 ) LOG_print( log, "\t· transform\n\t\t·inserted\n\t\t\t·module: %.70s(...)\n", etl.transform->inserted[ i ]->module );
        if( verbose > 0 ) LOG_print( log, "\t\t\t·staged_data: \"%.70s(...)\"\n", etl.transform->inserted[ i ]->staged_data );
        if( verbose > 0 ) LOG_print( log, "\t\t\t·source_system_update: \"%.70s(...)\"\n", etl.transform->inserted[ i ]->source_system_update);
        if( verbose > 0 ) LOG_print( log, "\t\t\t·api_key: \"%.70s(...)\"\n", etl.transform->inserted[ i ]->api_key );
    }

    for( int i = 0; i < etl.transform->deleted_count; i++ ) {
        if( verbose > 0 ) LOG_print( log, "\t· transform\n\t\t·deleted\n\t\t\t·module: %.70s(...)\n", etl.transform->deleted[ i ]->module );
        if( verbose > 0 ) LOG_print( log, "\t\t\t·staged_data: \"%.70s(...)\"\n", etl.transform->deleted[ i ]->staged_data );
        if( verbose > 0 ) LOG_print( log, "\t\t\t·source_system_update: \"%.70s(...)\"\n", etl.transform->deleted[ i ]->source_system_update );
        if( verbose > 0 ) LOG_print( log, "\t\t\t·api_key: \"%.70s(...)\"\n", etl.transform->deleted[ i ]->api_key );
    }

    for( int i = 0; i < etl.transform->modified_count; i++ ) {
        if( verbose > 0 ) LOG_print( log, "\t· transform\n\t\t·modified\n\t\t\t·module: %.70s(...)\n", etl.transform->modified[ i ]->module );
        if( verbose > 0 ) LOG_print( log, "\t\t\t·staged_data: \"%.70s(...)\"\n", etl.transform->modified[ i ]->staged_data );
        if( verbose > 0 ) LOG_print( log, "\t\t\t·source_system_update: \"%.70s(...)\"\n", etl.transform->modified[ i ]->source_system_update );
        if( verbose > 0 ) LOG_print( log, "\t\t\t·api_key: \"%.70s(...)\"\n", etl.transform->modified[ i ]->api_key );
    }
    

    if( verbose > 0 ) LOG_print( log, "\t· load\n\t\t·inserted\n\t\t\t·input_data_sql: \"%.70s(...)\"\n", etl.load.inserted.input_data_sql );

    for( int i = 0; i < etl.load.inserted.target_count; i++ ) {
        if( verbose > 0 ) LOG_print( log, "\t\t\t·target[%d]: ", i );
        if( verbose > 0 ) LOG_print( log, "\t\t\t\t·extracted_values: " );
        for( int j = 0; j < etl.load.inserted.target[ i ]->extracted_values_len; j++ ) {
            if( verbose > 0 ) LOG_print( log, "'%s', ", etl.load.inserted.target[ i ]->extracted_values[ j ] );
        }
        if( verbose > 0 ) LOG_print( log, "\n" );
        if( verbose > 0 ) LOG_print( log, "\t\t\t\t·output_data_sql: \"%.70s(...)\"\n", etl.load.inserted.target[ i ]->output_data_sql );
    }

    if( verbose > 0 ) LOG_print( log, "\t\t·deleted\n\t\t\t·input_data_sql: \"%.70s(...)\"\n", etl.load.deleted.input_data_sql );
    for( int i = 0; i < etl.load.deleted.target_count; i++ ) {
        if( verbose > 0 ) LOG_print( log, "\t\t\t·target[%d]: ", i );
        if( verbose > 0 ) LOG_print( log, "\t\t\t\t·extracted_values: " );
        for( int j = 0; j < etl.load.deleted.target[ i ]->extracted_values_len; j++ ) {
            if( verbose > 0 ) LOG_print( log, "'%s', ", etl.load.deleted.target[ i ]->extracted_values[i]);
        }
        if( verbose > 0 ) LOG_print( log, "\n" );
        if( verbose > 0 ) LOG_print( log, "\t\t\t\t·output_data_sql: \"%.70s(...)\"\n", etl.load.inserted.target[ i ]->output_data_sql );
    }

    if( verbose > 0 ) LOG_print( log, "\t\t·modified\n\t\t\t·input_data_sql: \"%.70s(...)\"\n", etl.load.modified.input_data_sql );
    for( int i = 0; i < etl.load.modified.target_count; i++ ) {
        if( verbose > 0 ) LOG_print( log, "\t\t\t·target[%d]: ", i );
        if( verbose > 0 ) LOG_print( log, "\t\t\t\t·extracted_values: " );
        for( int j = 0; j < etl.load.modified.target[ i ]->extracted_values_len; j++ ) {
            if( verbose > 0 ) LOG_print( log, "'%s', ", etl.load.modified.target[ i ]->extracted_values[i]);
        }
        if( verbose > 0 ) LOG_print( log, "\n" );
        if( verbose > 0 ) LOG_print( log, "\t\t\t\t·output_data_sql: \"%.70s(...)\"\n", etl.load.modified.target[ i ]->output_data_sql );
    }

    if( etl.post_actions_count > 0 ) {
        if( verbose > 0 ) LOG_print( log, "\t· PostETL Actions:\n" );

        for( int i = 0; i < etl.post_actions_count; i++ ) {
            if( verbose > 0 ) LOG_print( log, "\t\t·\"%.70s(...)\"\n", etl.post_actions[ i ]->sql );
        }
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
    const char              *name,
    short                   verbose,
    LOG_OBJECT              *log
) {

    size_t str_len = 0;

    if( verbose > 0 ) {
        LOG_print( log, "[%s] DATABASE_SYSTEM_DB_add(\"%s\", %d, \"%s\", \"%s\", ... ).\n", TIME_get_gmt(), ip, port, user, name );
        LOG_print( log, "\t· ip=\"%s\".\n", ip );
        LOG_print( log, "\t· name=\"%s\".\n", name );
    }

    str_len = strlen( name );
    dst->name = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
    if( dst->name ) {
        strlcpy(
            dst->name,
            name,
            str_len
        );
    }

    dst->ip = SAFECALLOC( TINY_BUFF_SIZE, sizeof( char ), __FILE__, __LINE__ );
    if( dst->ip ) {
        strlcpy(
            dst->ip,
            ip,
            TINY_BUFF_SIZE
        );
    }

    if( verbose > 0 ) LOG_print( log, "\t· port=\"%d\"\n", port );
    dst->port = port;

    if( verbose > 0 ) LOG_print( log, "\t· driver=\"%s\"\n",
        driver == D_POSTGRESQL ? "PostgreSQL" 
        : driver == D_MYSQL ? "MySQL" 
        : driver == D_MARIADB ? "MariaDB" 
        : driver == D_ODBC ? "ODBC" 
        : driver == D_ORACLE ? "ORACLE"
        : driver == D_SQLITE ? "SQLite3"
        : "MongoDB"
    );
    dst->driver = ( DB_DRIVER )driver;

    switch( dst->driver ) {
        case D_POSTGRESQL: {
            dst->db_conn.pgsql_conn.active = 0;
        } break;

        case D_MYSQL: {
            dst->db_conn.mysql_conn.active = 0;
        } break;

        case D_MARIADB: {
            dst->db_conn.mariadb_conn.active = 0;
        } break;

        case D_ODBC: {
            dst->db_conn.odbc_conn.active = 0;
        } break;

        case D_ORACLE: {
            dst->db_conn.oracle_conn.active = 0;
        } break;

        case D_SQLITE: {
            dst->db_conn.sqlite_conn.active = 0;
        } break;

        case D_MONGODB: {
            dst->db_conn.mongodb_conn.active = 0;
        } break;
    }

    if( verbose > 0 ) LOG_print( log, "\t· user=\"%s\"\n", user );
    str_len = strlen(user);
    dst->user = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
    if( dst->user ) {
        strlcpy(
            dst->user,
            user,
            str_len
        );
    }

    if( verbose > 0 ) LOG_print( log, "\t· password=\"%.1s***\"\n", password );
    str_len = strlen(password);
    dst->password = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
    if( dst->password ) {
        strlcpy(
            dst->password,
            password,
            str_len
        );
    }

    if( verbose > 0 ) LOG_print( log, "\t· db=\"%s\"\n", db );
    str_len = strlen(db);
    dst->db = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
    if( dst->db ) {
        strlcpy(
            dst->db,
            db,
            str_len
        );
    }

    if( connection_string != NULL ) {
        if( verbose > 0 ) LOG_print( log, "\t· connection_string=\"%s\"\n", connection_string );
        str_len = strlen( connection_string );
        dst->connection_string = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
        if( dst->connection_string ) {
            strlcpy(
                dst->connection_string,
                connection_string,
                str_len
            );
        }
    }
}

void DATABASE_SYSTEM_DB_close( DATABASE_SYSTEM_DB* db, LOG_OBJECT *log ) {

    if( db->ip != NULL ) {
        LOG_print( log, "[%s] DATABASE_SYSTEM_DB_close( %s@%s:%d )...\n", TIME_get_gmt(), db->user, db->ip, db->port );
    }

    switch( db->driver ) {
        case D_POSTGRESQL:
        {
            if( db->db_conn.pgsql_conn.active == 1 ) {
                LOG_print( log, "\t· Driver: \"PostgreSQL\". Disconnecting...\n" );
                PG_disconnect( &db->db_conn.pgsql_conn, log );
            }
        } break;
        case D_MYSQL:
        {
            if( db->db_conn.mysql_conn.active == 1 ) {
                LOG_print( log, "\t· Driver: \"MySQL\". Disconnecting...\n" );
                MYSQL_disconnect( &db->db_conn.mysql_conn, log );
            }
        } break;
        case D_MARIADB:
        {
            if( db->db_conn.mariadb_conn.active == 1 ) {
                LOG_print( log, "\t· Driver: \"MariaDB\". Disconnecting...\n" );
                MARIADB_disconnect( &db->db_conn.mariadb_conn, log );
            }
        } break;
        case D_ODBC:
        {
            if( db->db_conn.odbc_conn.active == 1 ) {
                LOG_print( log, "\t· Driver: \"ODBC\". Disconnecting...\n" );
                ODBC_disconnect( &db->db_conn.odbc_conn, log );
            }
        } break;
        case D_ORACLE:
        {
            if( db->db_conn.oracle_conn.active == 1 ) {
                LOG_print( log, "\t· Driver: \"Oracle\". Disconnecting...\n" );
                ORACLE_disconnect( &db->db_conn.oracle_conn, log );
            }
        } break;
        case D_SQLITE:
        {
            if( db->db_conn.sqlite_conn.active == 1 ) {
                LOG_print( log, "\t· Driver: \"SQLite3\". Disconnecting...\n" );
                SQLITE_disconnect( &db->db_conn.sqlite_conn, log );
            }
        } break;
        case D_MONGODB :
        {
            if( db->db_conn.mongodb_conn.active == 1 ) {
                LOG_print( log, "\t· Driver: \"MongoDB\". Disconnecting...\n" );
                MONGODB_disconnect( &db->db_conn.mongodb_conn, log );   
            }
        } break;
        default:
        {
        }
    }
}


void DATABASE_SYSTEM_DB_free( DATABASE_SYSTEM_DB *db, LOG_OBJECT *log ) {

    if( db->ip != NULL ) {
        LOG_print( log, "[%s] DATABASE_SYSTEM_DB_free( %s@%s:%d )...\n", TIME_get_gmt(), db->user, db->ip, db->port );
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
    if( db->name != NULL ) {
        free( db->name ); db->name = NULL;
    }

    switch( db->driver ) {
        case D_POSTGRESQL : {
            if( db->db_conn.pgsql_conn.active == 1 ) {
                LOG_print( log, "\t· Driver: \"PostgreSQL\". Disconnecting...\n" );
                PG_disconnect( &db->db_conn.pgsql_conn, log );
            }
        } break;
        case D_MYSQL : {
            if( db->db_conn.mysql_conn.active == 1 ) {
                LOG_print( log, "\t· Driver: \"MySQL\". Disconnecting...\n" );
                MYSQL_disconnect( &db->db_conn.mysql_conn, log );
            }
        } break;
         case D_MARIADB : {
             if( db->db_conn.mariadb_conn.active == 1 ) {
                 LOG_print( log, "\t· Driver: \"MariaDB\". Disconnecting...\n" );
                 MARIADB_disconnect( &db->db_conn.mariadb_conn, log );
            }
        } break;
        case D_ODBC : {
            if( db->db_conn.odbc_conn.active == 1 ) {
                LOG_print( log, "\t· Driver: \"ODBC\". Disconnecting...\n" );
                ODBC_disconnect( &db->db_conn.odbc_conn, log );
            }
        } break;
        case D_ORACLE : {
            if( db->db_conn.oracle_conn.active == 1 ) {
                LOG_print( log, "\t· Driver: \"Oracle\". Disconnecting...\n" );
                ORACLE_disconnect( &db->db_conn.oracle_conn, log );
            }
        } break;
        case D_SQLITE : {
            if( db->db_conn.sqlite_conn.active == 1 ) {
                LOG_print( log, "\t· Driver: \"SQLite3\". Disconnecting...\n" );
                SQLITE_disconnect( &db->db_conn.sqlite_conn, log );
            }
        } break;
        case D_MONGODB : {
            if( db->db_conn.mongodb_conn.active == 1 ) {
                LOG_print( log, "\t· Driver: \"MongoDB\". Disconnecting...\n" );
                MONGODB_disconnect( &db->db_conn.mongodb_conn, log );
            }
        }
        default : {
        }
    }
    db->driver = 0;
}


int DATABASE_SYSTEM_DB_init( DATABASE_SYSTEM_DB *db, LOG_OBJECT *log ) {
    int         ret = FALSE;

    /* Connect with defined database */
    switch( db->driver ) {
        case D_POSTGRESQL : {
            LOG_print( log, "\t· Driver: \"PostgreSQL\". Connecting..." );
            ret = PG_connect( &db->db_conn.pgsql_conn, db->ip, db->port, db->db, db->user, db->password, db->connection_string, db->name, log );
        } break;
        case D_MYSQL : {
            LOG_print( log, "\t· Driver: \"MySQL\". Connecting..." );
            ret = MYSQL_connect( &db->db_conn.mysql_conn, db->ip, db->port, db->db, db->user, db->password, db->connection_string, db->name, log );
        } break;
        case D_MARIADB : {
            LOG_print( log, "\t· Driver: \"MariaDB\". Connecting..." );
            ret = MARIADB_connect( &db->db_conn.mariadb_conn, db->ip, db->port, db->db, db->user, db->password, db->connection_string, db->name, log );
        } break;
        case D_ODBC : {
            LOG_print( log, "\t· Driver: \"ODBC\". Connecting (%s)...", db->connection_string, db->name );
            ret = ODBC_connect( &db->db_conn.odbc_conn, db->ip, db->port, db->db, db->user, db->password, db->connection_string, db->name, log );
        } break;
        case D_ORACLE : {
            LOG_print( log, "\t· Driver: \"Oracle\". Connecting..." );
            ret = ORACLE_connect( &db->db_conn.oracle_conn, db->ip, db->port, db->db, db->user, db->password, db->connection_string, db->name, log );
        } break;
        case D_SQLITE : {
            LOG_print( log, "\t· Driver: \"SQLite3\". Connecting..." );
            ret = SQLITE_connect( &db->db_conn.sqlite_conn, db->connection_string /* As filename for now. DCPAM would download DB file in future. */, db->name, log );
        } break;
        case D_MONGODB : {
            LOG_print( log, "\t· Driver: \"MongoDB\". Connecting..." );
            ret = MONGODB_connect( &db->db_conn.mongodb_conn, db->ip, db->port, db->db, db->user, db->password, db->connection_string, db->name, log );
        } break;
        default : {
            LOG_print( log, "Error: unknown driver: \"%d (%s)\".\n", db->driver, db->name );
        }
    }

    return ret;
}


void DATABASE_SYSTEM_close( DATABASE_SYSTEM *system, LOG_OBJECT *log ) {

    if( system != NULL ) {
        if( system->name != NULL ) {
            LOG_print( log, "[%s] DATABASE_SYSTEM_close(\"%s\").\n", TIME_get_gmt(), system->name );
            free( system->name ); system->name = NULL;
        }
        for( int i = 0; i < system->queries_len; i++ ) {
            SYSTEM_QUERY_free( &system->queries[ i ] );
        }
        DATABASE_SYSTEM_DB_free( &system->system_db, log );
        DATABASE_SYSTEM_DB_free( &system->dcpam_db, log );
        if( system->staging_db ) {
            DATABASE_SYSTEM_DB_free( system->staging_db, log );
            free( system->staging_db ); system->staging_db;
        }

        if( system->flat_file ) {
            free( system->flat_file->name ); system->flat_file->name = NULL;
            free( system->flat_file->preprocessor ); system->flat_file->preprocessor = NULL;
            free( system->flat_file->load_sql ); system->flat_file->load_sql = NULL;
            for( int i = 0; i < system->flat_file->columns_len; i++ ) {
                free( system->flat_file->columns[ i ] ); system->flat_file->columns[ i ] = NULL;
            }
            free( system->flat_file->columns ); system->flat_file->columns = NULL;
            memset( system->flat_file->delimiter, '\0', 1 );

            free( system->flat_file->http.method ); system->flat_file->http.method = NULL;
            free( system->flat_file->http.payload ); system->flat_file->http.payload = NULL;
            system->flat_file->http.payload_len = 0;

            for( int i = 0; i < system->flat_file->http.headers_len; i++ ) {
                free( system->flat_file->http.headers[ i ].name ); system->flat_file->http.headers[ i ].name = NULL;
                free( system->flat_file->http.headers[ i ].value ); system->flat_file->http.headers[ i ].value = NULL;
            }
            free( system->flat_file->http.headers ); system->flat_file->http.headers = NULL;

            if( system->flat_file->type == FFT_CSV ) {
                free( system->flat_file->csv_file ); system->flat_file->csv_file = NULL;
            } else if( system->flat_file->type == FFT_JSON ) {
                free( system->flat_file->json_file ); system->flat_file->json_file = NULL;
            }
            free( system->flat_file );
        }
    }
}


void DATABASE_SYSTEM_add(
    const char              *name,
    DATABASE_SYSTEM_DB      *source_db,
    DATABASE_SYSTEM_DB      *dcpam_db,
    DATABASE_SYSTEM_DB      *staging_db,
    DATABASE_SYSTEM_FLAT_FILE *flat_file,
    DATABASE_SYSTEM_QUERY   queries[ MAX_SYSTEM_QUERIES ],
    const int               queries_len,
    short                   verbose,
    LOG_OBJECT              *log
) {

    if( verbose > 0 ) LOG_print( log, "[%s] DATABASE_SYSTEM_add( \"%s\", ... ).\n", TIME_get_gmt(), name );
    if( DATABASE_SYSTEMS_COUNT < MAX_DATA_SYSTEMS ) {

        int j = 0;

        DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].failure = 0;

        size_t name_len = strlen(name);
        DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].name = SAFECALLOC( name_len + 1, sizeof( char ), __FILE__, __LINE__ );
        if( DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].name ) {
            strlcpy(
                DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].name,
                name,
                name_len + 1
            );
        }

        DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].queries_len = queries_len;

        for( j = 0; j < queries_len; j++ ) {
            DATABASE_SYSTEM_QUERY_add(
                queries[ j ].name,
                queries[ j ].mode,
                queries[ j ].etl_config,
                &DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].queries[ j ],
                TRUE,
                log
            );
        }

        for( j = 0; j < queries_len; j++ ) {
            free( queries[ j ].name ); queries[ j ].name = NULL;
        }

        /* Create source system DB connection */
        DATABASE_SYSTEM_DB_add(
            source_db->ip,
            source_db->port,
            source_db->driver,
            source_db->user,
            source_db->password,
            source_db->db,
            source_db->connection_string ? source_db->connection_string : NULL,
            &DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].system_db,
            source_db->name,
            TRUE,
            log
        );

        /* Create new instance of DCPAM DB connection */
        DATABASE_SYSTEM_DB_add(
            dcpam_db->ip,
            dcpam_db->port,
            dcpam_db->driver,
            dcpam_db->user,
            dcpam_db->password,
            dcpam_db->db,
            dcpam_db->connection_string ? dcpam_db->connection_string : NULL,
            &DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].dcpam_db,
            dcpam_db->name,
            TRUE,
            log
        );

        /* Create new instance od Staging Area connection */
        if( staging_db ) {
            DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].staging_db = SAFEMALLOC( sizeof( DATABASE_SYSTEM_DB ), __FILE__, __LINE__ );

            DATABASE_SYSTEM_DB_add(
                staging_db->ip,
                staging_db->port,
                staging_db->driver,
                staging_db->user,
                staging_db->password,
                staging_db->db,
                staging_db->connection_string ? staging_db->connection_string : NULL,
                DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].staging_db,
                staging_db->name,
                TRUE,
                log
            );
        }

        /* Flat file configuration */
        if( flat_file ) {
            DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].flat_file = SAFEMALLOC( sizeof( DATABASE_SYSTEM_FLAT_FILE ), __FILE__, __LINE__ );
            if( flat_file->type == FFT_CSV || flat_file->type == FFT_TSV || flat_file->type == FFT_PSV ) {
                DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].flat_file->csv_file = SAFEMALLOC( sizeof( CSV_FILE ), __FILE__, __LINE__ );
                DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].flat_file->csv_file->loaded = 0;
            } else if( flat_file->type == FFT_JSON ) {
                DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].flat_file->json_file = SAFEMALLOC( sizeof( JSON_FILE ), __FILE__, __LINE__ );
                DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].flat_file->json_file->loaded = 0;
            }

            DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].flat_file->type = flat_file->type;

            size_t name_len = strlen( flat_file->name );
            DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].flat_file->name = SAFECALLOC( name_len + 1, sizeof( char ), __FILE__, __LINE__ );
            snprintf( DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].flat_file->name, name_len + 1, flat_file->name );

            if( flat_file->preprocessor ) {
                size_t preprocessor_len = strlen( flat_file->preprocessor );
                DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].flat_file->preprocessor = SAFECALLOC( preprocessor_len + 1, sizeof( char ), __FILE__, __LINE__ );
                snprintf( DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].flat_file->preprocessor, preprocessor_len + 1, flat_file->preprocessor );
            }

            DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].flat_file->columns_len = flat_file->columns_len;
            DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].flat_file->columns = SAFEMALLOC( flat_file->columns_len * sizeof * DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].flat_file->columns, __FILE__, __LINE__ );
            for( int i = 0; i < flat_file->columns_len; i++ ) {
                size_t col_name_len = strlen( flat_file->columns[ i ] );
                DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].flat_file->columns[ i ] = SAFECALLOC( col_name_len + 1, sizeof( char ), __FILE__, __LINE__ );
                snprintf( DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].flat_file->columns[ i ], col_name_len + 1, flat_file->columns[ i ] );
            }

            size_t sql_len = strlen( flat_file->load_sql );
            DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].flat_file->load_sql= SAFECALLOC( sql_len + 1, sizeof( char ), __FILE__, __LINE__ );
            snprintf( DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].flat_file->load_sql, sql_len + 1, flat_file->load_sql );

            if( flat_file->http.method ) {
                size_t method_len = strlen( flat_file->http.method );
                DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].flat_file->http.method = SAFECALLOC( method_len + 1, sizeof( char ), __FILE__, __LINE__ );
                snprintf( DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].flat_file->http.method, method_len + 1, flat_file->http.method );
            }

            if( flat_file->http.payload ) {
                DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].flat_file->http.payload_len = flat_file->http.payload_len;
                DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].flat_file->http.payload = SAFECALLOC( flat_file->http.payload_len + 1, sizeof( char ), __FILE__, __LINE__ );
                snprintf( DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].flat_file->http.payload,  flat_file->http.payload_len + 1, flat_file->http.payload );
            }

            if( flat_file->http.headers ) {
                DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].flat_file->http.headers_len = flat_file->http.headers_len;
                DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].flat_file->http.headers = SAFEMALLOC( flat_file->http.headers_len * sizeof * flat_file->http.headers, __FILE__, __LINE__ );
                for( int i = 0; i < flat_file->http.headers_len; i++ ) {
                    size_t name_len = strlen( flat_file->http.headers[ i ].name );
                    DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].flat_file->http.headers[ i ].name = SAFECALLOC( name_len + 1, sizeof( char ), __FILE__, __LINE__ );
                    snprintf( DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].flat_file->http.headers[ i ].name, name_len + 1, flat_file->http.headers[ i ].name );

                    size_t value_len = strlen( flat_file->http.headers[ i ].value );
                    DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].flat_file->http.headers[ i ].value = SAFECALLOC( value_len + 1, sizeof( char ), __FILE__, __LINE__ );
                    snprintf( DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].flat_file->http.headers[ i ].value, value_len + 1, flat_file->http.headers[ i ].value );
                }
            }

            DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].flat_file->http.active = 0;

            if( flat_file->type == FFT_CSV || flat_file->type == FFT_TSV || flat_file->type == FFT_PSV ) {
                snprintf( DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].flat_file->delimiter, 1, flat_file->delimiter );
                snprintf( DATABASE_SYSTEMS[ DATABASE_SYSTEMS_COUNT ].flat_file->csv_file->delimiter, 1, flat_file->delimiter );
            }
        }

        /* API configuration */

        free( source_db->ip ); source_db->ip = NULL;
        free( source_db->user ); source_db->user = NULL;
        free( source_db->password ); source_db->password = NULL;
        free( source_db->db ); source_db->db = NULL;
        free( source_db->name ); source_db->name = NULL;

        if( source_db->connection_string != NULL ) {
            free( source_db->connection_string ); source_db->connection_string = NULL;
        }

        DATABASE_SYSTEMS_COUNT++;
    } else {
        LOG_print( log, "error. Maximum data systems limit exceeded.\n" );
    }
}

DATABASE_SYSTEM_DB* DATABASE_SYSTEM_DB_get( const char* name ) {
    for( int i = 0; i < P_APP.DB_len; i++ ) {
        if( strcmp( P_APP.DB[ i ]->name, name ) == 0 ) {
            return P_APP.DB[ i ];
        }
    }
    return NULL;
}
