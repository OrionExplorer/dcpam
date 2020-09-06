#ifndef LCS_REPORT_H
#define LCS_REPORT_H

#include "network/client.h"
#include "../utils/log.h"

typedef struct {
    NET_CONN    *conn;
    char        *address;
    LOG_OBJECT  *log;
    int         active;
} LCS_REPORT;

int LCS_REPORT_init( LCS_REPORT *connection, const char *address, const char *component_name, const char *component_version, LOG_OBJECT *log );
int LCS_REPORT_free( LCS_REPORT *connection );
int LCS_REPORT_send( LCS_REPORT *connection, const char *data, size_t data_len );

#endif
