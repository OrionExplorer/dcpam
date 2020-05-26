#ifndef TIME_H
#define TIME_H

char* TIME_get_gmt( void );
long int TIME_get_epoch( void );
void TIME_format( long int seconds, char *dst );
void TIME_epoch2time( const char *epoch, char *dst, const int dst_len );
char* TIME_get_time( const char *format );
void TIME_get_month_name( char *dst, const int dst_len );
void TIME_get_year( char *dst, const int dst_len );
long int TIME_epoch_from_datetime( const char *src );

#endif
