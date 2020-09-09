#ifndef LCS_REPORT_H
#define LCS_REPORT_H

#include "network/client.h"
#include "../utils/log.h"
#include "../core/app_schema.h"

int LCS_REPORT_init( LCS_REPORT *connection, const char *address, const char *component_name, const char *component_version, LOG_OBJECT *log );
int LCS_REPORT_free( LCS_REPORT *connection );
int LCS_REPORT_send( LCS_REPORT *connection, const char *action, COMPONENT_ACTION_TYPE action_type );

#endif
