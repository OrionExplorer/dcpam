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

#ifndef LCS_REPORT_H
#define LCS_REPORT_H

#include "network/client.h"
#include "../utils/log.h"
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
