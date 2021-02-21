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

#include "../include/DCPAM_WDS/wds_node.h"
#include "../include/utils/log.h"
#include "../include/utils/time.h"
#include <stdlib.h>


extern LOG_OBJECT              dcpam_wds_log;

int WDS_NODE_init( WDS_NODE *dst, const char* ip, const int port, const char* key, const char** tables, const int tables_len, LOG_OBJECT* log ) {
   return 1;
}


void WDS_NODE_free( WDS_NODE *src, LOG_OBJECT* log ) {
   if( src == NULL ) {
      LOG_print( log, "[%s] WDS_NODE_free error: node data is invalid.\n", TIME_get_gmt() );
   }
   LOG_print( log, "[%s] WDS_NODE_free( %s:%d )... ", TIME_get_gmt(), src->ip, src->port );
   if( src->ip ) free( src->ip ); src->ip = NULL;
   src->port = 0;
   if( src->key ) free( src->key ); src->key = NULL;

   if( src->tables && src->tables_len > 0 ) {
      for( int i = 0; i < src->tables_len; i++ ) {
         free( src->tables[ i ] ); src->tables[ i ] = NULL;
      }
      free( src->tables ); src->tables = NULL;
      src->tables_len = 0;
   }

   LOG_print( log, "ok.\n" );
}
