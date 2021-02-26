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
#include "../include/utils/regex.h"
#include "../include/utils/memory.h"
#include "../include/utils/time.h"
#include "../include/db/db.h"


void REGEX_replace(char **str, const char *pattern, const char *replace, size_t *dst_len, LOG_OBJECT *log ) {
   size_t maxMatches = 20;
   size_t maxGroups = 30;

   LOG_print( log, "[%s] REGEX_replace( %s, %s, %s ):\n", TIME_get_gmt(), *str, pattern, replace );
   if( *str == NULL || pattern == NULL || replace == NULL ) {
      LOG_print( log, "\t- error: not all parameter are valid.\n" );
      return;
   }

   size_t rep_len = strlen( replace );
   char *result = strdup( replace );
  
   regex_t regexCompiled;
   regmatch_t groupArray[maxGroups];

   if( regcomp( &regexCompiled, pattern, REG_EXTENDED ) ) {
      LOG_print( log, "\t- error: could not compile regular expression.\n" );
      return;
   }

   char *cursor = *str;
   char *new_value = NULL;
   size_t new_value_len = 0;
   for( unsigned int m = 0; m < maxMatches; m++ ) {

      if( regexec( &regexCompiled, cursor, maxGroups, groupArray, 0 ) ) {
         break;  // No more matches
      }

      unsigned int offset = 0;
      for( unsigned int g = 0; g < maxGroups; g++ ) {
         if( groupArray[ g ].rm_so == ( size_t ) - 1 ) {
            break;  // No more groups
         }

         if( g == 0 ) {
            offset = groupArray[ g ].rm_eo;
            continue;
         }

         char cursorCopy[ strlen( cursor ) + 1 ];
         strcpy( cursorCopy, cursor );
         cursorCopy[ groupArray[ g ].rm_eo ] = 0;
         /*printf( "Match %u, Group %u: [%2u-%2u, %2u characters]: %s\n",
                 m,
                 g,
                 groupArray[ g ].rm_so, groupArray[ g ].rm_eo,
                 groupArray[ g ].rm_eo - groupArray[ g ].rm_so,
                 cursorCopy + groupArray[ g ].rm_so
         );*/

         size_t value_len = strlen( cursorCopy + groupArray[ g ].rm_so );
         char *value = SAFECALLOC( value_len + 1, sizeof( char ), __FILE__, __LINE__ );
         memcpy( value, cursorCopy + groupArray[ g ].rm_so, value_len );

         /* Prepare group identifier for the replace string */
         char group_id[ 3 ] = { 0 };
         snprintf( group_id, 3, "\\%d", g );

         /* Get current replace length. */
         size_t str_len = strlen( result );

         /* Decrease size by group identifier length */
         str_len -= strlen( group_id );

         /* Increase size by value length */
         str_len += value_len;

         /* Memory allocation for new buffer */
         int nv_init = new_value == NULL ? 0 : 1;
         new_value = realloc( new_value, str_len + 1 );
         if( nv_init == 0 ) {
            strlcpy( new_value, result, str_len );
         }
         new_value_len = str_len;
         LOG_print( log, "\t- looking for \"%s\" in \"%s\" to replace with \"%s\"...\n", group_id, new_value, value );

         DB_QUERY_internal_replace_str_( new_value, group_id, value, &str_len );
         free( value ); value = NULL;

      }
      cursor += offset;
   }

   free( result );

   if( new_value_len > 0 ) {
      *dst_len = new_value_len;
      if( *str ) {
         free( *str ); *str = NULL;
         *str = SAFECALLOC( new_value_len + 1, sizeof( char ), __FILE__, __LINE__ );
         strlcpy( *str, new_value, new_value_len );
      }

      free( new_value ); new_value = NULL;
   } else {
      LOG_print( log, "\t- finished: pattern not found.\n" );
   }

   regfree(&regexCompiled);

   return;

}
