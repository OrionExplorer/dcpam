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

#include "../include/DCPAM_WDS/sql_parser.h"
#include "../include/utils/log.h"
#include "../include/utils/time.h"


/* Really lame and naïve implementation... */


SQL_CONDITION SQL_PARSER_supported_conditions( const char *sql ) {
   /* LIMIT, but no WHERE */
   printf( "[%s] SQL_PARSER_supported_conditions: search for LIMIT in SQL: \"%s\"...", TIME_get_gmt(), sql );
   if( strstr( sql, " LIMIT " ) != NULL && strstr( sql, " WHERE " ) == NULL ) {
      printf( "found.\n" );
      return SQL_COND_LIMIT;
   }
   printf( "not found.\n" );

   return SQL_COND_UNDEF;

   /* WHERE, but no LIMIT */
   /*if( strstr( " WHERE ", sql ) && strstr( " LIMIT ", sql ) == NULL ) {
      return SQL_COND_WHERE;
   }*/

   /* WHERE, but no SELECT TOP - SQL Server/Access */
   /*if( strstr( " WHERE ", sql ) && strstr( "SELECT TOP ", sql ) == NULL ) {
      return SQL_COND_WHERE;
   }*/
}

int SQL_PARSER_collect_data( D_CACHE *cache, D_SUB_CACHE *sub_cache, const char *sql, SQL_CONDITION sql_condition, LOG_OBJECT *log ) {

   if( cache && sub_cache && sql && sql_condition != SQL_COND_UNDEF && log ) {
      if( sql_condition == SQL_COND_LIMIT ) {
         long int limit_value = 0;
         int has_limit_value = 0;

         /*
            LIMIT
         */
         if( sql_condition == SQL_COND_LIMIT ) {
            char *pos = strstr( sql, " LIMIT " );
            has_limit_value = sscanf( pos, " LIMIT %99ld", &limit_value ) == 1;
         }

         if( has_limit_value ) {
            LOG_print( log, "\t- SQL_PARSER: LIMIT value is %ld.\n", limit_value );

            /* Set maximum iterator value */
            long max_count = limit_value;
            if( cache->query->row_count < limit_value ) {
               max_count = cache->query->row_count;
               LOG_print( log, "\t- SQL_PARSER: LIMIT is greater than total cached records. Value set to %ld.\n", max_count );
            }

            /* Init SUB_CACHE memory */
            sub_cache->indices_len = max_count;
            sub_cache->indices = realloc( sub_cache->indices, ( max_count ) * sizeof ( D_SUB_CACHE ) );

            for( int i = 0; i < max_count; i++ ) {
               sub_cache->indices[ i ] = i;
            }

            return 1;

         } else {
            LOG_print( log, "[%s] SQL_PARSER: unable to read LIMIT value.\n", TIME_get_gmt() );
            return 0;
         }
      }

   } else {
      LOG_print( log, "[%s] SQL_PARSER: not all parameters are valid.\n", TIME_get_gmt() );
      return 0;
   }
}
