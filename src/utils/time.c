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

#include "../include/shared.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#ifdef _WIN32
#include <Windows.h>
#endif

/*
    https://stackoverflow.com/a/26064185
*/
void dcpam_sleep( unsigned int ms ) {

#ifndef _WIN32
    {
        struct timespec ts_remaining = {
          ms / 1000,
          ( ms % 1000 ) * 1000000L
        };

        int result = 0;

        do
        {
            struct timespec ts_sleep = ts_remaining;
            result = nanosleep( &ts_sleep, &ts_remaining );
        } while( ( EINTR == errno ) && ( -1 == result ) );
    }
#else
    Sleep( ms );
#endif
}


char* TIME_get_gmt( void ) {
    static char s[ TIME_BUFF_SIZE ];
    struct tm tim;
    time_t now;

    now = time( NULL );
    tim = *( localtime( &now ) );
    strftime( s, TIME_BUFF_SIZE, DATETIME, &tim );

    return ( ( char* )&s );
}


char* TIME_get_time( const char* format ) {
    static char s[ TIME_BUFF_SIZE ];
    struct tm tim;
    time_t now;

    now = time( NULL );
    tim = *( localtime( &now ) );
    strftime( s, TIME_BUFF_SIZE, format, &tim );

    return ( ( char* )&s );
}


long int TIME_get_epoch( void ) {
    time_t now;

    now = time( NULL );

    return ( long int )now;
}

void TIME_format( long int seconds, char* dst ) {
    div_t       res;

    res = div( seconds, 60 );
    sprintf( dst, "%d minutes, %d seconds", res.quot, res.rem );
}

void TIME_epoch2time( const char* epoch, char* dst, const int dst_len ) {
    time_t c;

    c = strtoul( epoch, NULL, 0 );
    strftime( dst, dst_len, "%Y-%m-%d %H:%M:%S", localtime( &c ) );
}


void TIME_get_month_name( char* dst, const int dst_len ) {
    time_t now;
    struct tm* tmp;

    now = time( NULL );
    tmp = localtime( &now );
    strftime( dst, dst_len, "%b", tmp );
}


void TIME_get_year( char* dst, const int dst_len ) {
    time_t now;
    struct tm* tmp;

    now = time( NULL );
    tmp = localtime( &now );
    strftime( dst, dst_len, "%Y", tmp );
}


long int TIME_epoch_from_datetime( const char* src ) {
    long int result = 0;
    int year;
    int month;
    int day;
    int hours;
    int minutes;

    if( sscanf( src, "%d-%d-%d %d:%d", &year, &month, &day, &hours, &minutes ) == 5 ) {
        time_t epoch;
        struct tm tm;

        memset( &tm, 0, sizeof( struct tm ) );

        tm.tm_year = year - 1900;
        tm.tm_mon = month - 1;
        tm.tm_mday = day;
        tm.tm_hour = hours;
        tm.tm_min = minutes;
        tm.tm_sec = 0;
        epoch = mktime( &tm );

        result = ( long int )epoch;
        result *= 1;
    }

    return result;
}
