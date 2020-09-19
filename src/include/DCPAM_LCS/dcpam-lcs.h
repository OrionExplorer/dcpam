#ifndef DCPAM_LCS_H
#define DCPAM_LCS_H

#include "../../include/core/component.h"
/*
    lcs_config.json => app
*/
typedef struct L_DCPAM_APP {
    char* version;
    char* name;
    int                     network_port;
    DCPAM_COMPONENT** COMPONENTS;
    int                     COMPONENTS_len;
    DCPAM_ALLOWED_HOST** ALLOWED_HOSTS_;
    int                     ALLOWED_HOSTS_len;
} L_DCPAM_APP;


L_DCPAM_APP         L_APP;

#endif
