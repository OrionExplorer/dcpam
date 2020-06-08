#include "../include/db/oracle.h"
#include "../include/utils/time.h"
#include "../include/utils/log.h"
#include "../include/utils/memory.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>


static pthread_mutex_t      db_exec_mutex = PTHREAD_MUTEX_INITIALIZER;


void ORACLE_get_error( OCIError *errhp, sword status ) {
    text errbuf[ 512 ];
    sb4 errcode = 0;

    switch( status ) {
        case OCI_SUCCESS:
            break;
        case OCI_SUCCESS_WITH_INFO:
            LOG_print( "Error - OCI_SUCCESS_WITH_INFO\n" );
            break;
        case OCI_NEED_DATA:
            LOG_print( "Error - OCI_NEED_DATA\n" );
            break;
        case OCI_NO_DATA:
            LOG_print( "Error - OCI_NODATA\n" );
            break;
        case OCI_ERROR:
            OCIErrorGet( ( dvoid* )errhp, ( ub4 )1, ( text* )NULL, &errcode,
                errbuf, ( ub4 )sizeof( errbuf ), OCI_HTYPE_ERROR );
            LOG_print( "Error - %.*s\n", 512, errbuf );
            break;
        case OCI_INVALID_HANDLE:
            LOG_print( "Error - OCI_INVALID_HANDLE\n" );
            break;
        case OCI_STILL_EXECUTING:
            LOG_print( "Error - OCI_STILL_EXECUTE\n" );
            break;
        case OCI_CONTINUE:
            LOG_print( "Error - OCI_CONTINUE\n" );
            break;
        default:
            break;
    }
}


void ORACLE_disconnect( ORACLE_CONNECTION* db_connection ) {
    if( db_connection != NULL ) {
        LOG_print( "[%s]\tORACLE_disconnect( <'%s'> ).\n", TIME_get_gmt(), db_connection->id != NULL ? db_connection->id : "x" );

        if( db_connection->envhp ) {
            OCIHandleFree( ( dvoid* )db_connection->envhp, OCI_HTYPE_ENV );
            db_connection->envhp = NULL;
        }
        if( db_connection->id != NULL ) {
            free( db_connection->id );
            db_connection->id = NULL;
        }
        db_connection->active = 0;

        LOG_print( "[%s]\tORACLE_disconnect.\n", TIME_get_gmt() );
    }
}


int ORACLE_connect(
    ORACLE_CONNECTION*  db_connection,
    const char*         host,
    const int           port,
    const char*         dbname,
    const char*         user,
    const char*         password,
    const char*         connection_string
) {
    char        conn_str[ 1024 ];
    sword       retcode = 0;
    OCIError    *errhp;
    OCIServer   *srvhp;
    OCISvcCtx   *svchp;
    OCISession  *authp = ( OCISession* )0;

    db_connection->id = ( char * )SAFECALLOC( strlen(user)+strlen(host)+strlen(dbname)+8, sizeof( char ), __FILE__, __LINE__ );
    if( connection_string ) {
        snprintf( conn_str, 1024, connection_string, dbname, host, port, user, password );
    } else {
        snprintf( conn_str, 1024, "dbname=%s host=%s port=%d user=%s password=%s", dbname, host, port, user, password );
    }
    
    sprintf( db_connection->id, "%s@%s[db=%s]", user, host, dbname );

    retcode = OCIEnvCreate( ( OCIEnv** )&db_connection->envhp, ( ub4 )OCI_DEFAULT,
        ( dvoid* )0, ( dvoid * ( * )( dvoid*, size_t ) ) 0,
        ( dvoid * ( * )( dvoid*, dvoid*, size_t ) ) 0,
        ( void ( * )( dvoid*, dvoid* ) ) 0, ( size_t )0, ( dvoid** )0 );

    if( retcode != 0 ) {
        LOG_print( "[%s] Error: OCIEnvCreate failed with errcode = %d.\n", TIME_get_gmt(), retcode );
        return 0;
    }

    OCIHandleAlloc( ( dvoid* )db_connection->envhp, ( dvoid** )&errhp, OCI_HTYPE_ERROR,
        ( size_t )0, ( dvoid** )0 );

    OCIHandleAlloc( ( dvoid* )db_connection->envhp, ( dvoid** )&srvhp, OCI_HTYPE_SERVER,
        ( size_t )0, ( dvoid** )0 );

    OCIHandleAlloc( ( dvoid* )db_connection->envhp, ( dvoid** )&svchp, OCI_HTYPE_SVCCTX,
        ( size_t )0, ( dvoid** )0 );

    OCIServerAttach( srvhp, errhp, ( text* )host, strlen( host ), 0 );

    OCIAttrSet( ( dvoid* )svchp, OCI_HTYPE_SVCCTX, ( dvoid* )srvhp,
        ( ub4 )0, OCI_ATTR_SERVER, ( OCIError* )errhp );

    OCIHandleAlloc( ( dvoid* )db_connection->envhp, ( dvoid** )&authp,
        ( ub4 )OCI_HTYPE_SESSION, ( size_t )0, ( dvoid** )0 );

    OCIAttrSet( ( dvoid* )authp, ( ub4 )OCI_HTYPE_SESSION,
        ( dvoid* )user, ( ub4 )strlen( ( char* )user ),
        ( ub4 )OCI_ATTR_USERNAME, errhp );

    OCIAttrSet( ( dvoid* )authp, ( ub4 )OCI_HTYPE_SESSION,
        ( dvoid* )password, ( ub4 )strlen( ( char* )password ),
        ( ub4 )OCI_ATTR_PASSWORD, errhp );

    retcode = OCISessionBegin( svchp, errhp, authp, OCI_CRED_RDBMS,
        ( ub4 )OCI_DEFAULT );
    if( retcode != OCI_SUCCESS ) {
        ORACLE_get_error( errhp, retcode );
        db_connection->active = 0;
        free( db_connection->id ); db_connection->id = NULL;
        return 0;
    }

    OCIAttrSet( ( dvoid* )svchp, ( ub4 )OCI_HTYPE_SVCCTX,
        ( dvoid* )authp, ( ub4 )0,
        ( ub4 )OCI_ATTR_SESSION, errhp );

    db_connection->active = 1;

    LOG_print( "ok.\n" );

    return 1;
}


int ORACLE_exec(
    ORACLE_CONNECTION   *db_connection,
    const char          *sql,
    unsigned long       sql_length, 
    DB_QUERY            *dst_result,
    const char* const   *param_values,
    const int           params_count,
    const int           *param_lengths,
    const int           *param_formats
) {
    /*PGresult        *pg_result = NULL;*/
    int             row_count = 0, field_count = 0;
    int             val_length = 0;
    int             i = 0, j = 0, k = 0;
    unsigned long   l = 0;

    LOG_print( "[%s]\tORACLE_exec( <'%s'>, \"%s\", ... ).\n", TIME_get_gmt(), db_connection->id, sql );
    pthread_mutex_lock( &db_exec_mutex );

    dst_result->row_count = 0;
    dst_result->field_count = 0;

    dst_result->sql = ( char* )SAFECALLOC( sql_length + 1, sizeof( char ), __FILE__, __LINE__ );
    for( l = 0; l < sql_length; l++ ) {
        *( dst_result->sql + l ) = sql[ l ];
    }

    LOG_print( "[%s]\tORACLE_exec.\n", TIME_get_gmt() );
    pthread_mutex_unlock( &db_exec_mutex );

    return 1;
}
