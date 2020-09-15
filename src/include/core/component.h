#ifndef COMPONENT_H
#define COMPONENT_H

#include "app_schema.h"
#include "../utils/log.h"
#include "../third-party/cJSON.h"

int LCS_COMPONENT_register( DCPAM_COMPONENT *dst, const char *name, const char *version, const char *ip, const int port, LOG_OBJECT *log );
int LCS_COMPONENT_free( DCPAM_COMPONENT *src );
int LCS_COMPONENT_check( DCPAM_COMPONENT* dst, LOG_OBJECT *log );
int LCS_COMPONENT_ACTION_register( DCPAM_COMPONENT* dst, const char* description, COMPONENT_ACTION_TYPE action_type, LOG_OBJECT *log );
cJSON *LCS_COMPONENT_get_states( DCPAM_COMPONENT **components, int components_len );
cJSON *LCS_COMPONENT_get_actions( DCPAM_COMPONENT** components, int components_len, const char *component_name );
cJSON *LCS_COMPONENT_process_report( cJSON *json_request, DCPAM_COMPONENT** components, int components_len, LOG_OBJECT *log );


#endif
