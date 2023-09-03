#ifndef GPS_TIME_H
#define GPS_TIME_H

#include <ctime>

struct GPSTime
{
    time_t time; /* time (s) expressed by standard time_t */
    double sec;  /* fraction of second under 1 s */
};

GPSTime GPSTime2UTCTime(int week, double sec, double leapsec);
GPSTime GPSTime2UTCTime(int week, double sec);


#endif
