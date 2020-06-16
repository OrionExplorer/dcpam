#include "../include/shared.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

/*
    https://stackoverflow.com/a/26064185
*/
void dcpam_sleep( unsigned int ms ) {

#ifndef _WIN32
    int result = 0;

    {
        struct timespec ts_remaining =
        {
          ms / 1000,
          ( ms % 1000 ) * 1000000L
        };

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
    int year;
    int month;
    int day;
    int hours;
    int minutes;
    struct tm tm;
    time_t epoch;
    long int result = 0;

    memset( &tm, 0, sizeof( struct tm ) );

    sscanf( src, "%d-%d-%d %d:%d", &year, &month, &day, &hours, &minutes );
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hours;
    tm.tm_min = minutes;
    tm.tm_sec = 0;
    epoch = mktime( &tm );

    result = ( long int )epoch;
    result *= 1;

    return result;
}
