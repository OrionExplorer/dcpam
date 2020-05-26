#include "../include/db/postgresql.h"
#include "../include/utils/time.h"
#include "../include/utils/log.h"
#include "../include/utils/memory.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>


static pthread_mutex_t      db_exec_mutex = PTHREAD_MUTEX_INITIALIZER;


void PG_disconnect( PG_CONNECTION* db_connection ) {
    if( db_connection != NULL ) {
        LOG_print( "[%s]\tPG_disconnect( <'%s'> ).\n", TIME_get_gmt(), db_connection->id != NULL ? db_connection->id : "x" );

        if( db_connection->connection != NULL ) {
            PQfinish( db_connection->connection );  
            db_connection->connection = NULL;
        }
        if( db_connection->id != NULL ) {
            free( db_connection->id );
            db_connection->id = NULL;
        }
        db_connection->active = 0;

        LOG_print( "[%s]\tPG_disconnect.\n", TIME_get_gmt() );
    }
}


int PG_connect( PG_CONNECTION* db_connection, const char* host, const int port, const char* dbname, const char* user, const char* password ) {
    char        conn_str[512];

    db_connection->id = ( char * )SAFECALLOC( strlen(user)+strlen(host)+strlen(dbname)+8, sizeof( char ) );
    sprintf( conn_str, "dbname=%s host=%s port=%d user=%s password=%s", dbname, host, port, user, password );
    sprintf( db_connection->id, "%s@%s[db=%s]", user, host, dbname );

    db_connection->connection = PQconnectdb( conn_str );

    if ( PQstatus( db_connection->connection ) == CONNECTION_BAD ) {
        db_connection->active = 0;
        free( db_connection->id ); db_connection->id = NULL;
        LOG_print( "Error. Message: \"%s\"", PQerrorMessage( db_connection->connection ) );
        return 0;
    }

    db_connection->active = 1;

    LOG_print( "ok.\n" );

    return 1;
}


int PG_exec(
    PG_CONNECTION       *db_connection,
    const char          *sql,
    unsigned long       sql_length, 
    DB_QUERY            *dst_result,
    const char* const   *param_values,
    const int           params_count,
    const int           *param_lengths,
    const int           *param_formats,
    Oid                 *param_types 
) {
    PGresult        *pg_result;
    int             row_count = 0, field_count = 0;
    int             val_length = 0;
    int             i = 0, j = 0, k = 0;
    unsigned long   l = 0;

    LOG_print( "[%s]\tPG_exec( <'%s'>, \"%s\", ... ).\n", TIME_get_gmt(), db_connection->id, sql );
    pthread_mutex_lock( &db_exec_mutex );

    dst_result->row_count = 0;
    dst_result->field_count = 0;

    dst_result->sql = ( char* )SAFECALLOC( sql_length + 1, sizeof( char ) );
    for( l = 0; l < sql_length; l++ ) {
        *( dst_result->sql + l ) = sql[ l ];
    }

    if( db_connection->connection ) {
        if( param_values == NULL || params_count == 0) {
            pg_result = PQexec( db_connection->connection, dst_result->sql );
        } else if ( param_values != NULL && params_count > 0 ) {
            pg_result = PQexecParams( db_connection->connection,
                dst_result->sql,
                params_count,
                param_types,
                (const char * const *)param_values,
                param_lengths,
                param_formats,
                1
            );
        }
    } else {
        PQclear( pg_result );
        pg_result = CONNECTION_BAD;
    }

    if ( PQresultStatus( pg_result ) != PGRES_TUPLES_OK && PQresultStatus( pg_result ) != PGRES_COMMAND_OK ) {
        PQclear( pg_result );
        LOG_print( "[%s][fail] PG_exec. Message: \"%s\"\n", TIME_get_gmt(), PQerrorMessage( db_connection->connection ) );
        pthread_mutex_unlock( &db_exec_mutex );
        return 0;
    }
    
    row_count = PQntuples( pg_result );
    dst_result->row_count = row_count;
    if( row_count > 0 ) {
        dst_result->records = ( DB_RECORD* )SAFEMALLOC( row_count * sizeof( DB_RECORD ) );

        field_count = PQnfields( pg_result );
        dst_result->field_count = field_count;

        for( i = 0; i < row_count; i++) {
            dst_result->records[i].fields = ( DB_FIELD* )SAFEMALLOC( field_count * sizeof( DB_FIELD ) );
            for( j = 0; j < field_count; j++ ) {
                strncpy( dst_result->records[i].fields[j].label, PQfname( pg_result, j ), 64 );
                val_length = PQgetlength( pg_result, i, j );
                dst_result->records[i].fields[j].value = ( char* )SAFECALLOC( ( val_length+2 ), sizeof( char ) );
                char *tmp_res = PQgetvalue( pg_result, i, j );

                dst_result->records[ i ].fields[ j ].size = val_length;
                for( k = 0; k < val_length; k++ ) {
                    dst_result->records[ i ].fields[ j ].value[ k ] = tmp_res[ k ];
                }
                dst_result->records[ i ].fields[ j ].value[ k + 1 ] = '\0';
            }
        }
    }

    PQclear( pg_result );

    LOG_print( "[%s]\tPG_exec.\n", TIME_get_gmt() );
    pthread_mutex_unlock( &db_exec_mutex );

    return 1;
}
