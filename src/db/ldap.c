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

#include "../include/db/ldap.h"
#include "../include/utils/time.h"
#include "../include/utils/log.h"
#include "../include/utils/memory.h"
#include "../include/utils/strings.h"
#include <errno.h>
#define LDAP_DEPRECATED 1
#include <ldap.h>
#include <lber.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

static pthread_mutex_t      db_exec_mutex = PTHREAD_MUTEX_INITIALIZER;


void LDAP_disconnect( LDAP_CONNECTION* db_connection, LOG_OBJECT *log ) {
    LOG_print( log, "[%s]\tLDAP_disconnect( <'%s'> ).\n", TIME_get_gmt(), db_connection->id ? db_connection->id : "" );

    if( db_connection->connection ) {
        ldap_unbind_ext_s( db_connection->connection, NULL, NULL );
        db_connection->connection = NULL;
    }

    if( db_connection->dbname ) {
        free( db_connection->dbname );
        db_connection->dbname = NULL;
    }

    if( db_connection->id ) {
        free( db_connection->id );
        db_connection->id = NULL;
    }

    LOG_print( log, "[%s]\tLDAP_disconnect.\n", TIME_get_gmt() );
}


int LDAP_connect(
    LDAP_CONNECTION* db_connection,
    const char* host,
    const int   port,
    const char* dbname,
    const char* user,
    const char* password,
    const char* connection_string,
    const char* name,
    LOG_OBJECT *log
) {

    pthread_mutex_lock( &db_exec_mutex );
    db_connection->id = ( char* )SAFECALLOC( 1024, sizeof( char ), __FILE__, __LINE__ );
    snprintf( db_connection->id, 1024, "%s@%s[db=%s] (%s)", user, host, dbname, name );

    db_connection->connection = ldap_init( host, port );

    if( db_connection->connection == NULL ) {
        free( db_connection->id ); db_connection->id = NULL;
        LOG_print( log, "error. Function ldap_init failed: %s (%d).\n", strerror( errno ), errno );
        pthread_mutex_unlock( &db_exec_mutex );
        return 0;
    }

    int version = LDAP_VERSION3;
    ldap_set_option( db_connection->connection, LDAP_OPT_PROTOCOL_VERSION, &version );

    /* Bind to the server. */
    int rc = ldap_simple_bind_s( db_connection->connection, user, password );
    if( rc != LDAP_SUCCESS ) {
        free( db_connection->id ); db_connection->id = NULL;
        LOG_print( log, "error. Function ldap_simple_bind_s failed: %s (%d)\n", ldap_err2string( rc ), rc );
        pthread_mutex_unlock( &db_exec_mutex );
        return 0;
    }

    size_t str_len = strlen( dbname );
    db_connection->dbname = SAFECALLOC( str_len + 1, sizeof( char ), __FILE__, __LINE__ );
    strlcpy( db_connection->dbname, dbname, str_len );

    db_connection->active = 1;

    LOG_print( log, "ok.\n" );

    pthread_mutex_unlock( &db_exec_mutex );
    return 1;
}


int LDAP_exec(
    LDAP_CONNECTION*   db_connection,
    const char*        sql,
    size_t             sql_length,
    DB_QUERY*          dst_result,
    const char* const* param_values,
    const int          params_count,
    const int*         param_lengths,
    const int*         param_formats,
    const char* const* param_types,
    qec*               query_exec_callback,
    void*              data_ptr1,
    void*              data_ptr2,
    LOG_OBJECT*        log
) {
    LOG_print( log, "[%s]\tLDAP_exec( <'%s'>, \"%s\", ... ).\n", TIME_get_gmt(), db_connection->id, sql );
    pthread_mutex_lock( &db_exec_mutex );


    if( dst_result ) {

        unsigned long   l = 0;

        dst_result->row_count = 0;
        dst_result->field_count = 0;

        dst_result->sql = ( char* )SAFECALLOC( sql_length + 1, sizeof( char ), __FILE__, __LINE__ );

        for( l = 0; l < sql_length; l++ ) {
            *( dst_result->sql + l ) = sql[ l ];
        }
    }

    if( db_connection->connection ) {
        if( param_values == NULL || params_count == 0 ) {

            LDAPMessage *search_result, *current_entry;

            int query_result = ldap_search_ext_s( db_connection->connection, db_connection->dbname, LDAP_SCOPE_SUBTREE, sql, NULL, 0, NULL, NULL, NULL, 0, &search_result );
            if( query_result != LDAP_SUCCESS ) {
                LOG_print( log, "[%s]\tLDAP_exec: #%d, %s.\n", TIME_get_gmt(), query_result, ldap_err2string( query_result ) );
                pthread_mutex_unlock( &db_exec_mutex );
                return 0;
            }

            BerValue**  vals;
            BerElement* ber;
            char*       attr = NULL;
            int row_count = ldap_count_entries( db_connection->connection, search_result );
            int field_count = 0;

            LOG_print( log, "[%s]\tLDAP_exec: %d entries found.\n", TIME_get_gmt(), row_count );

            current_entry = ldap_first_entry( db_connection->connection, search_result );

            while( current_entry ) {

                attr = ldap_first_attribute( db_connection->connection, current_entry, &ber );

                while( attr ) {

                    vals = ldap_get_values_len( db_connection->connection, current_entry, attr );

                    for( int pos = 0; pos < ldap_count_values_len( vals ); pos++ ) {
                        /* Key */
                        printf("%s: ", attr );
                        /* Value */
                        for( size_t l = 0; l < vals[ pos ]->bv_len; l++ ) {
                            printf("%c", vals[ pos ]->bv_val[ l ] );
                        }
                        printf( "\n" );
                    }
                    ldap_value_free_len( vals );
                    ldap_memfree( attr );

                    attr = ldap_next_attribute( db_connection->connection, current_entry, ber );
                }

                if( ber ) ber_free( ber, 0 );
                current_entry = ldap_next_entry( db_connection->connection, current_entry );
                ldap_memfree( attr );
            }
            ldap_msgfree( current_entry );
            ldap_msgfree( search_result );

        } else {
            /* TODO */
        }
    } else {
        /* TODO */
    }

    /*if( db_connection->connection ) {
        if( param_values == NULL || params_count == 0 ) {

            int row_count = 0;
            int field_count = 0;
            query_result = mysql_real_query( db_connection->connection, sql, ( unsigned long )sql_length );

            if( query_result == 0 ) {
                mysql_result = mysql_store_result( db_connection->connection );
            } else {
                LOG_print( log, "[%s][ERROR]\tLDAP_exec: #%d, %s (\"%s\", len=%ld)\n", TIME_get_gmt(), query_result, mysql_error( db_connection->connection ), sql, sql_length );
                pthread_mutex_unlock( &db_exec_mutex );
                return 0;
            }

            if( mysql_result ) {
                field_count = ( int )mysql_num_fields( mysql_result );
                if( dst_result ) {
                    dst_result->field_count = field_count;
                }
                
                row_count = ( int )mysql_num_rows( mysql_result );
            }

            if( row_count > 0 ) {
                unsigned long val_length = 0;

                if( query_exec_callback ) {

                    char *table_fields[ MAX_COLUMNS ];

                    for( int i = 0; i < field_count; i++ ) {
                        table_fields[ i ] = SAFECALLOC( MAX_COLUMN_NAME_LEN + 1, sizeof( char ), __FILE__, __LINE__ );
                        mysql_field = mysql_fetch_field( mysql_result );
                        strlcpy( table_fields[ i ], mysql_field->name, MAX_COLUMN_NAME_LEN );
                    }

                    for( int i = 0; i < row_count; i++ ) {
                        DB_RECORD* record = SAFEMALLOC( 1 * sizeof( DB_RECORD ), __FILE__, __LINE__ );

                        record->field_count = field_count;
                        record->fields = ( DB_FIELD* )SAFEMALLOC( field_count * sizeof( DB_FIELD ), __FILE__, __LINE__ );

                        mysql_row = mysql_fetch_row( mysql_result );
                        lengths = mysql_fetch_lengths( mysql_result );

                        for( int j = 0; j < field_count; j++ ) {
                            val_length = lengths[ j ];
                            if( val_length > 0 ) {
                                record->fields[ j ].size = val_length;
                                record->fields[ j ].value = SAFECALLOC( val_length + 1, sizeof( char ), __FILE__, __LINE__ );
                                for( unsigned long k = 0; k < val_length; k++ ) {
                                    *( k + record->fields[ j ].value ) = mysql_row[ j ][ k ];
                                }
                            } else {
                                record->fields[ j ].value = NULL;
                                record->fields[ j ].size = 0;
                            }

                            strlcpy( record->fields[ j ].label, table_fields[ j ], MAX_COLUMN_NAME_LEN );
                        }

                        pthread_mutex_unlock( &db_exec_mutex );
                        ( *query_exec_callback )( record, data_ptr1, data_ptr2, log );
                    }

                    for( int i = 0; i < field_count; i++ ) {
                        free( table_fields[ i ] ); table_fields[ i ] = NULL;
                    }
                }

                if( dst_result ) {
                    dst_result->row_count = row_count;
                    dst_result->records = ( DB_RECORD* )SAFEMALLOC( row_count * sizeof( DB_RECORD ), __FILE__, __LINE__ );

                    for( int i = 0; i < row_count; i++ ) {
                        mysql_row = mysql_fetch_row( mysql_result );
                        lengths = mysql_fetch_lengths( mysql_result );
                        dst_result->records[ i ].fields = ( DB_FIELD* )SAFEMALLOC( field_count * sizeof( DB_FIELD ), __FILE__, __LINE__ );
                        dst_result->records[ i ].field_count = field_count;

                        for( int j = 0; j < field_count; j++ ) {
                            val_length = lengths[ j ];
                            if( val_length > 0 ) {
                                dst_result->records[ i ].fields[ j ].size = val_length;
                                dst_result->records[ i ].fields[ j ].value = SAFECALLOC( val_length + 1, sizeof( char ), __FILE__, __LINE__ );
                                for( unsigned long k = 0; k < val_length; k++ ) {
                                    *( k + dst_result->records[ i ].fields[ j ].value ) = mysql_row[ j ][ k ];
                                }
                            } else {
                                dst_result->records[ i ].fields[ j ].value = NULL;
                                dst_result->records[ i ].fields[ j ].size = 0;
                            }
                        }
                    }

                    for( int i = 0; i < field_count; i++ ) {
                        mysql_field = mysql_fetch_field( mysql_result );
                        for( int j = 0; j < row_count; j++ ) {
                            strlcpy( dst_result->records[ j ].fields[ i ].label, mysql_field->name, MAX_COLUMN_NAME_LEN );
                        }
                    }
                }
            }

            mysql_free_result( mysql_result );

        } else if( params_count > 0 ) {
            LOG_print( log, "[%s] Warning: not yet implemented!\n", TIME_get_gmt() );
        }

    } else {
        LOG_print( log, "[%s][ERROR]\tLDAP_exec: #%d, %s.\n", TIME_get_gmt(), query_result, mysql_error( db_connection->connection ) );
    }*/

    LOG_print( log, "[%s]\tLDAP_exec.\n", TIME_get_gmt() );
    pthread_mutex_unlock( &db_exec_mutex );

    return 1;
}
