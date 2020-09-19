#ifndef LCS_REPORT_H
#define LCS_REPORT_H

#include "network/client.h"
#include "../utils/log.h"
//#include "../core/app_schema.h"
#include "../core/component.h"

typedef struct LCS_REPORT {
    NET_CONN* conn;
    char* address;
    int                     port;
    char* lcs_host;
    int                     lcs_port;
    LOG_OBJECT* log;
    int                     active;
    char* component;
    char* version;
} LCS_REPORT;

int LCS_REPORT_init( LCS_REPORT *connection, const char *address, const char *component_name, const char *component_version, LOG_OBJECT *log );
int LCS_REPORT_free( LCS_REPORT *connection );
int LCS_REPORT_send( LCS_REPORT *connection, const char *action, COMPONENT_ACTION_TYPE action_type );

#endif
