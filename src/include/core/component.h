#ifndef COMPONENT_H
#define COMPONENT_H

//#include "app_schema.h"
#include "../utils/log.h"
#include "../third-party/cJSON.h"

typedef enum COMPONENT_ACTION_RESULT {
    DCR_SUCCESS = 1,
    DCR_FAILURE
} COMPONENT_ACTION_RESULT;

typedef enum COMPONENT_ACTION_TYPE {
    DCT_START = 1,
    DCT_STOP
} COMPONENT_ACTION_TYPE;


typedef struct COMPONENT_ACTION {
    char* description;               /* Action description */
    char                    timestamp[ 20 ];            /* Action started */
    COMPONENT_ACTION_TYPE   type;                       /* Action type */
} COMPONENT_ACTION;


typedef struct DCPAM_COMPONENT {
    char* ip;                        /* Component IP address*/
    int                     port;                       /* Component socket port */
    char* name;                      /* Component name */
    char* version;                   /* Component version */
    int                     active;                     /* Is component active? */
    char                    timestamp[ 20 ];            /* Last verification (YYYY-MM-DD HH:mm:SS)*/
    COMPONENT_ACTION** actions;                  /* Component actions list */
    int                     actions_len;                /* Component actions count */
} DCPAM_COMPONENT;


int LCS_COMPONENT_register( DCPAM_COMPONENT *dst, const char *name, const char *version, const char *ip, const int port, LOG_OBJECT *log );
int LCS_COMPONENT_free( DCPAM_COMPONENT *src );
int LCS_COMPONENT_check( DCPAM_COMPONENT* dst, LOG_OBJECT *log );
int LCS_COMPONENT_ACTION_register( DCPAM_COMPONENT* dst, const char* description, COMPONENT_ACTION_TYPE action_type, LOG_OBJECT *log );
cJSON *LCS_COMPONENT_get_states( DCPAM_COMPONENT **components, int components_len );
cJSON *LCS_COMPONENT_get_actions( DCPAM_COMPONENT** components, int components_len, const char *component_name );
cJSON *LCS_COMPONENT_process_report( cJSON *json_request, DCPAM_COMPONENT** components, int components_len, LOG_OBJECT *log );


#endif
