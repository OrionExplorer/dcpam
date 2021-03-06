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

#include "../include/db/oracle.h"
#include "../include/utils/time.h"
#include "../include/utils/log.h"
#include "../include/utils/memory.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>


static pthread_mutex_t      db_exec_mutex = PTHREAD_MUTEX_INITIALIZER;


void ORACLE_get_error( OCIError *errhp, sword status, int line, LOG_OBJECT *log ) {
    text errbuf[ 512 ];
    sb4 errcode = 0;

    switch( status ) {
        case OCI_SUCCESS:
            break;
        case OCI_SUCCESS_WITH_INFO:
            LOG_print( log, "[Line:%d] Error - OCI_SUCCESS_WITH_INFO\n", line );
            break;
        case OCI_NEED_DATA:
            LOG_print( log, "[Line:%d] Error - OCI_NEED_DATA\n", line );
            break;
        case OCI_NO_DATA:
            LOG_print( log, "[Line:%d] Error - OCI_NODATA\n", line );
            break;
        case OCI_ERROR:
            OCIErrorGet( ( dvoid* )errhp, ( ub4 )1, ( text* )NULL, &errcode,
                errbuf, ( ub4 )sizeof( errbuf ), OCI_HTYPE_ERROR );
            LOG_print( log, "[Line:%d] Error - %.*s\n", line, 512, errbuf );
            break;
        case OCI_INVALID_HANDLE:
            LOG_print( log, "[Line:%d] Error - OCI_INVALID_HANDLE\n", line );
            break;
        case OCI_STILL_EXECUTING:
            LOG_print( log, "[Line:%d] Error - OCI_STILL_EXECUTE\n", line );
            break;
        case OCI_CONTINUE:
            LOG_print( log, "[Line:%d] Error - OCI_CONTINUE\n", line );
            break;
        default:
            break;
    }
}


void ORACLE_disconnect( ORACLE_CONNECTION* db_connection, LOG_OBJECT *log ) {
    OCIError        *errhp;

    OCIHandleAlloc( ( dvoid* )db_connection->envhp, ( dvoid** )&errhp, OCI_HTYPE_ERROR,
        ( size_t )0, ( dvoid** )0 );

    if( db_connection != NULL ) {
        LOG_print( log, "[%s]\tORACLE_disconnect( <'%s'> ).\n", TIME_get_gmt(), db_connection->id != NULL ? db_connection->id : "x" );

        if( db_connection->svchp && db_connection->authp ) {
            OCISessionEnd( db_connection->svchp, errhp, db_connection->authp, ( ub4 )OCI_DEFAULT );
            OCIServerDetach( db_connection->srvhp, errhp, ( ub4 )OCI_DEFAULT );
        }

        if( db_connection->authp ) {
            OCIHandleFree( ( dvoid* )db_connection->authp, OCI_HTYPE_ENV );
            db_connection->authp = NULL;
        }

        if( db_connection->envhp ) {
            OCIHandleFree( ( dvoid* )db_connection->envhp, OCI_HTYPE_ENV );
            db_connection->envhp = NULL;
        }
        if( db_connection->svchp ) {
            OCIHandleFree( ( dvoid* )db_connection->svchp, OCI_HTYPE_SVCCTX );
            db_connection->svchp = NULL;
        }
        if( db_connection->srvhp ) {
            OCIHandleFree( ( dvoid* )db_connection->srvhp, OCI_HTYPE_SVCCTX );
            db_connection->srvhp = NULL;
        }
        if( db_connection->id != NULL ) {
            free( db_connection->id );
            db_connection->id = NULL;
        }
        db_connection->active = 0;

        LOG_print( log, "[%s]\tORACLE_disconnect.\n", TIME_get_gmt() );
    }
}


int ORACLE_connect(
    ORACLE_CONNECTION*  db_connection,
    const char*         host,
    const int           port,
    const char*         dbname,
    const char*         user,
    const char*         password,
    const char*         connection_string,
    const char*         name,
    LOG_OBJECT*         log
) {
    sword       retcode = 0;
    OCIError    *errhp;
    
    db_connection->authp = ( OCISession* )0;

    db_connection->id = ( char * )SAFECALLOC( 1024, sizeof( char ), __FILE__, __LINE__ );
    
    snprintf( db_connection->id, 1024, "%s@%s[db=%s] (%s)", user, host, dbname, name );

    retcode = OCIEnvCreate( ( OCIEnv** )&db_connection->envhp, ( ub4 )OCI_DEFAULT,
        ( dvoid* )0, ( dvoid * ( * )( dvoid*, size_t ) ) 0,
        ( dvoid * ( * )( dvoid*, dvoid*, size_t ) ) 0,
        ( void ( * )( dvoid*, dvoid* ) ) 0, ( size_t )0, ( dvoid** )0 );

    if( retcode != 0 ) {
        LOG_print( log, "[%s] Error: OCIEnvCreate failed with errcode = %d.\n", TIME_get_gmt(), retcode );
        return 0;
    }

    OCIHandleAlloc( ( dvoid* )db_connection->envhp, ( dvoid** )&errhp, OCI_HTYPE_ERROR,
        ( size_t )0, ( dvoid** )0 );

    OCIHandleAlloc( ( dvoid* )db_connection->envhp, ( dvoid** )&db_connection->srvhp, OCI_HTYPE_SERVER,
        ( size_t )0, ( dvoid** )0 );

    OCIHandleAlloc( ( dvoid* )db_connection->envhp, ( dvoid** )&db_connection->svchp, OCI_HTYPE_SVCCTX,
        ( size_t )0, ( dvoid** )0 );

    if( connection_string ) {
        retcode = OCIServerAttach( db_connection->srvhp, errhp, ( text* )connection_string, ( sb4 )strlen( connection_string), 0 );
    } else {
        retcode = OCIServerAttach( db_connection->srvhp, errhp, ( text* )host, ( sb4 )strlen( host ), 0 );
    }
    if( retcode != OCI_SUCCESS ) {
        ORACLE_get_error( errhp, retcode, __LINE__, log );
        db_connection->active = 0;
        free( db_connection->id ); db_connection->id = NULL;
        return 0;
    }

    OCIAttrSet( ( dvoid* )db_connection->svchp, OCI_HTYPE_SVCCTX, ( dvoid* )db_connection->srvhp,
        ( ub4 )0, OCI_ATTR_SERVER, ( OCIError* )errhp );

    OCIHandleAlloc( ( dvoid* )db_connection->envhp, ( dvoid** )&db_connection->authp,
        ( ub4 )OCI_HTYPE_SESSION, ( size_t )0, ( dvoid** )0 );

    OCIAttrSet( ( dvoid* )db_connection->authp, ( ub4 )OCI_HTYPE_SESSION,
        ( dvoid* )user, ( ub4 )strlen( ( char* )user ),
        ( ub4 )OCI_ATTR_USERNAME, errhp );

    OCIAttrSet( ( dvoid* )db_connection->authp, ( ub4 )OCI_HTYPE_SESSION,
        ( dvoid* )password, ( ub4 )strlen( ( char* )password ),
        ( ub4 )OCI_ATTR_PASSWORD, errhp );

    retcode = OCISessionBegin( db_connection->svchp, errhp, db_connection->authp, OCI_CRED_RDBMS,
        ( ub4 )OCI_DEFAULT );
    if( retcode != OCI_SUCCESS ) {
        ORACLE_get_error( errhp, retcode, __LINE__, log );
        db_connection->active = 0;
        free( db_connection->id ); db_connection->id = NULL;
        return 0;
    }

    OCIAttrSet( ( dvoid* )db_connection->svchp, ( ub4 )OCI_HTYPE_SVCCTX,
        ( dvoid* )db_connection->authp, ( ub4 )0,
        ( ub4 )OCI_ATTR_SESSION, errhp );

    db_connection->active = 1;

    LOG_print( log, "ok.\n" );

    return 1;
}


int ORACLE_exec(
    ORACLE_CONNECTION   *db_connection,
    const char          *sql,
    size_t              sql_length, 
    DB_QUERY            *dst_result,
    const char* const   *param_values,
    const int           params_count,
    const int           *param_lengths,
    const int           *param_formats,
    qec* query_exec_callback,
    void* data_ptr1,
    void* data_ptr2,
    LOG_OBJECT *log
) {
    OCIStmt         *stmthp = NULL;
    ub2             stmt_type;
    OCIError        *errhp = NULL;
    sword           retcode = 0;
    OCIParam        *mypard = ( OCIParam* )0;
    ub2             dtype;
    text            *_col_name;
    text            *_col_data[ MAX_COLUMNS ];
    ub4             counter, col_name_len, char_semantics;
    ub2             col_width;
    sb4             _real_col_width[ MAX_COLUMNS ];
    sb4             parm_status;
    ub4             iters;
    OCIDefine       *defnp;
    DB_RECORD       *tmp_records = NULL;

    if( db_connection->active == 0 || db_connection->id == NULL  || db_connection->svchp == NULL ) {
        return 0;
    }

    LOG_print( log, "[%s]\tORACLE_exec( <'%s'>, \"%s\", ... ).\n", TIME_get_gmt(), db_connection->id, sql );
    pthread_mutex_lock( &db_exec_mutex );

    if( dst_result ) {
        dst_result->row_count = 0;
        dst_result->field_count = 0;

        dst_result->sql = SAFECALLOC( sql_length + 1, sizeof( char ), __FILE__, __LINE__ );
        for( unsigned long l = 0; l < sql_length; l++ ) {
            *( dst_result->sql + l ) = sql[ l ];
        }
    }


    OCIHandleAlloc( ( dvoid* )db_connection->envhp, ( dvoid** )&errhp, OCI_HTYPE_ERROR,
        ( size_t )0, ( dvoid** )0 );

    OCIHandleAlloc( ( dvoid* )db_connection->envhp, ( dvoid** )&stmthp,
        OCI_HTYPE_STMT, ( size_t )0, ( dvoid** )0 );
    retcode = OCIStmtPrepare( stmthp, errhp, ( text* )sql,
        ( ub4 )sql_length,
        ( ub4 )OCI_NTV_SYNTAX, ( ub4 )OCI_DEFAULT );
    if( retcode != OCI_SUCCESS ) {
        ORACLE_get_error( errhp, retcode, __LINE__, log );
        OCIHandleFree( ( dvoid* )stmthp, ( ub4 )OCI_HTYPE_STMT );
        pthread_mutex_unlock( &db_exec_mutex );
        return 0;
    }

    retcode = OCIAttrGet( ( dvoid* )stmthp, OCI_HTYPE_STMT, ( void* )&stmt_type, 0, OCI_ATTR_STMT_TYPE, ( OCIError * )errhp );
    if( retcode != OCI_SUCCESS ) {
        ORACLE_get_error( errhp, retcode, __LINE__, log );
        OCIHandleFree( ( dvoid* )stmthp, ( ub4 )OCI_HTYPE_STMT );
        pthread_mutex_unlock( &db_exec_mutex );
        return 0;
    }

    iters = stmt_type == OCI_STMT_SELECT ? 0 : 1;

    retcode = OCIStmtExecute( db_connection->svchp, stmthp, errhp, iters, 0, ( OCISnapshot* )0, ( OCISnapshot* )0, OCI_DEFAULT );
    if( retcode != OCI_SUCCESS ) {
        ORACLE_get_error( errhp, retcode, __LINE__, log );
        OCIHandleFree( ( dvoid* )stmthp, ( ub4 )OCI_HTYPE_STMT );
        pthread_mutex_unlock( &db_exec_mutex );
        return 0;
    }

    if( stmt_type == OCI_STMT_SELECT ) {

        int  field_count = 0;
        char *column_name[ MAX_COLUMNS ];

        for( int i = 0; i < MAX_COLUMNS; i++ ) {
            column_name[ i ] = NULL;
        }

        counter = 1;
        parm_status = OCIParamGet( ( dvoid* )stmthp, OCI_HTYPE_STMT, errhp, ( dvoid** )&mypard, ( ub4 )counter );

        while( parm_status == OCI_SUCCESS ) {

            field_count++;

            /* Retrieve the datatype attribute */
            retcode = OCIAttrGet( ( dvoid* )mypard, ( ub4 )OCI_DTYPE_PARAM,
                ( dvoid* )&dtype, ( ub4* )0, ( ub4 )OCI_ATTR_DATA_TYPE,
                ( OCIError* )errhp );
            if( retcode != OCI_SUCCESS ) {
                ORACLE_get_error( errhp, retcode, __LINE__, log );
                OCIHandleFree( ( dvoid* )stmthp, ( ub4 )OCI_HTYPE_STMT );
                pthread_mutex_unlock( &db_exec_mutex );
                return 0;
            }

            /* Retrieve the column name attribute */
            col_name_len = 0;
            retcode = OCIAttrGet( ( dvoid* )mypard, ( ub4 )OCI_DTYPE_PARAM,
                ( dvoid** )&_col_name, ( ub4* )&col_name_len, ( ub4 )OCI_ATTR_NAME,
                ( OCIError* )errhp );
            if( retcode != OCI_SUCCESS ) {
                ORACLE_get_error( errhp, retcode, __LINE__, log );
                OCIHandleFree( ( dvoid* )stmthp, ( ub4 )OCI_HTYPE_STMT );
                pthread_mutex_unlock( &db_exec_mutex );
                return 0;
            }
            column_name[ counter - 1 ] = SAFECALLOC( ( size_t )col_name_len + 1, sizeof( char ), __FILE__, __LINE__ );
            memcpy( column_name[ counter - 1 ], ( char* )_col_name, ( size_t )col_name_len );

            /* Retrieve the length semantics for the column */
            char_semantics = 0;
            retcode = OCIAttrGet( ( dvoid* )mypard, ( ub4 )OCI_DTYPE_PARAM,
                ( dvoid* )&char_semantics, ( ub4* )0, ( ub4 )OCI_ATTR_CHAR_USED,
                ( OCIError* )errhp );
            if( retcode != OCI_SUCCESS ) {
                ORACLE_get_error( errhp, retcode, __LINE__ , log);
                OCIHandleFree( ( dvoid* )stmthp, ( ub4 )OCI_HTYPE_STMT );
                pthread_mutex_unlock( &db_exec_mutex );
                return 0;
            }
            col_width = 0;
            if( char_semantics ) {
                /* Retrieve the column width in characters */
                retcode = OCIAttrGet( ( dvoid* )mypard, ( ub4 )OCI_DTYPE_PARAM,
                    ( dvoid* )&col_width, ( ub4* )0, ( ub4 )OCI_ATTR_CHAR_SIZE,
                    ( OCIError* )errhp );
                if( retcode != OCI_SUCCESS ) {
                    ORACLE_get_error( errhp, retcode, __LINE__, log );
                    OCIHandleFree( ( dvoid* )stmthp, ( ub4 )OCI_HTYPE_STMT );
                    pthread_mutex_unlock( &db_exec_mutex );
                    return 0;
                }
            } else {
                /* Retrieve the column width in bytes */
                retcode = OCIAttrGet( ( dvoid* )mypard, ( ub4 )OCI_DTYPE_PARAM,
                    ( dvoid* )&col_width, ( ub4* )0, ( ub4 )OCI_ATTR_DATA_SIZE,
                    ( OCIError* )errhp );
                if( retcode != OCI_SUCCESS ) {
                    ORACLE_get_error( errhp, retcode, __LINE__, log );
                    OCIHandleFree( ( dvoid* )stmthp, ( ub4 )OCI_HTYPE_STMT );
                    pthread_mutex_unlock( &db_exec_mutex );
                    return 0;
                }
            }

            _real_col_width[ counter - 1 ] = col_width * 6 + 1;
            _col_data[ counter - 1 ] = SAFECALLOC( ( ( size_t )_real_col_width[ counter - 1 ] ), sizeof( text ), __FILE__, __LINE__ );

            OCIDefineByPos( stmthp, &defnp, errhp, counter, ( ub1* )_col_data[ counter - 1 ], ( sb4 )_real_col_width[ counter - 1 ], SQLT_STR, ( dvoid* )0, ( ub2* )0, ( ub2* )0, OCI_DEFAULT );
            /* increment counter and get next descriptor, if there is one */
            counter++;
            parm_status = OCIParamGet( ( dvoid* )stmthp, OCI_HTYPE_STMT, errhp,
                ( dvoid** )&mypard, ( ub4 )counter );
        }

        int row_count = 0;

        if( counter >= 1 ) {

            while( TRUE ) {
                retcode = OCIStmtFetch2( stmthp, errhp, 1, OCI_FETCH_NEXT, 0, OCI_DEFAULT );
                if( retcode != OCI_SUCCESS && retcode != OCI_NO_DATA ) {
                    ORACLE_get_error( errhp, retcode, __LINE__, log );
                    break;
                } else if( retcode == OCI_NO_DATA ) {
                    break;
                }

                if( query_exec_callback ) {

                    DB_RECORD   *record = SAFEMALLOC( 1 * sizeof( DB_RECORD ), __FILE__, __LINE__ );

                    record->field_count = field_count;
                    record->fields = ( DB_FIELD* )SAFEMALLOC( field_count * sizeof( DB_FIELD ), __FILE__, __LINE__ );

                    for( int i = 0; i < field_count; i++ ) {
                        _real_col_width[ i ] = ( sb4 )strlen( ( const char* )_col_data[ i ] );
                        strlcpy( record->fields[ i ].label, column_name[ i ], MAX_COLUMN_NAME_LEN );

                        record->fields[ i ].size = _real_col_width[ i ];
                        if( _real_col_width[ i ] > 0 ) {
                            record->fields[ i ].value = SAFECALLOC( _real_col_width[ i ] + 1, sizeof( char ), __FILE__, __LINE__ );
                            for( int j = 0; j < _real_col_width[ i ]; j++ ) {
                                record->fields[ i ].value[ j ] = _col_data[ i ][ j ];
                            }
                        } else {
                            record->fields[ i ].value = NULL;
                        }
                    }

                    pthread_mutex_unlock( &db_exec_mutex );
                    ( *query_exec_callback )( record, data_ptr1, data_ptr2, log );
                }

                if( dst_result ) {
                    dst_result->records = ( DB_RECORD* )realloc( tmp_records, ( row_count + 1 ) * sizeof( DB_RECORD ) );
                    if( dst_result->records ) {
                        tmp_records = dst_result->records;
                        dst_result->records[ row_count ].fields = ( DB_FIELD* )SAFEMALLOC( field_count * sizeof( DB_FIELD ), __FILE__, __LINE__ );
                        dst_result->records[ row_count ].field_count = field_count;

                        for( int i = 0; i < field_count; i++ ) {
                            _real_col_width[ i ] = ( sb4 )strlen( ( const char* )_col_data[ i ] );
                            strlcpy( dst_result->records[ row_count ].fields[ i ].label, column_name[ i ], MAX_COLUMN_NAME_LEN );

                            dst_result->records[ row_count ].fields[ i ].size = _real_col_width[ i ];
                            if( _real_col_width[ i ] > 0 ) {
                                dst_result->records[ row_count ].fields[ i ].value = SAFECALLOC( _real_col_width[ i ] + 1, sizeof( char ), __FILE__, __LINE__ );
                                for( int j = 0; j < _real_col_width[ i ]; j++ ) {
                                    dst_result->records[ row_count ].fields[ i ].value[ j ] = _col_data[ i ][ j ];
                                }
                            } else {
                                dst_result->records[ row_count ].fields[ i ].value = NULL;
                            }
                        }
                    }
                    row_count++;
                }
            }
        }

        if( dst_result ) {
            dst_result->field_count = field_count;
            dst_result->row_count = row_count;
        }

        for( int i = 0; i < field_count; i++ ) {
            free( column_name[ i ] ); column_name[ i ] = NULL;
            free( _col_data[ i ] ); _col_data[ i ] = NULL;
        }
    }
    LOG_print( log, "[%s]\tORACLE_exec.\n", TIME_get_gmt() );
    OCIHandleFree( ( dvoid* )stmthp, ( ub4 )OCI_HTYPE_STMT );
    pthread_mutex_unlock( &db_exec_mutex );

    return 1;
}
