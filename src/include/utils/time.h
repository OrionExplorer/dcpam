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

#ifndef TIME_H
#define TIME_H

void dcpam_sleep( unsigned int ms );
char* TIME_get_gmt( void );
long int TIME_get_epoch( void );
void TIME_format( long int seconds, char *dst );
void TIME_epoch2time( const char *epoch, char *dst, const int dst_len );
char* TIME_get_time( const char *format );
void TIME_get_month_name( char *dst, const int dst_len );
void TIME_get_year( char *dst, const int dst_len );
long int TIME_epoch_from_datetime( const char *src );

#endif
