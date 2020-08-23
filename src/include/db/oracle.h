#ifndef ORACLE_H
#define ORACLE_H

#include <oci.h>
#include "db.h"


typedef struct {
    OCIEnv      *envhp;
    OCIServer   *srvhp;
    OCISvcCtx   *svchp;
    OCISession  *authp;
    char        *id;
    int         active;
} ORACLE_CONNECTION;


int ORACLE_connect(
    ORACLE_CONNECTION* db_connection,
    const char* host,
    const int port,
    const char* dbname,
    const char* user,
    const char* password,
    const char* connection_string,
    const char* name,
    LOG_OBJECT* log
);

int ORACLE_exec(
    ORACLE_CONNECTION* db_connection,
    const char* sql,
    size_t sql_length,
    DB_QUERY* dst_result,
    const char* const *param_values,
    const int params_count,
    const int *param_lengths,
    const int *param_formats,
    qec* query_exec_callback,
    void* data_ptr1,
    void* data_ptr2,
    LOG_OBJECT* log
);
void ORACLE_disconnect( ORACLE_CONNECTION* db_connection, LOG_OBJECT* log );

#endif
