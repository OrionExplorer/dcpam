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

#ifndef DCPAM_WDS_NODE_H
#define DCPAM_WDS_NODE_H

#include "../shared.h"
#include "../utils/log.h"

/*
    wds_config.json => app
*/
/* DCPAM WDS connected nodes */
typedef struct WDS_NODE {
    char*       ip;
    int         port;
    char*       key;
    char**      tables;
    int         tables_len;
} WDS_NODE;


int WDS_NODE_init( WDS_NODE *dst, const char* ip, const int port, const char* key, const char** tables, const int tables_len, LOG_OBJECT *log );
void WDS_NODE_free( WDS_NODE *src, LOG_OBJECT *log );


#endif
