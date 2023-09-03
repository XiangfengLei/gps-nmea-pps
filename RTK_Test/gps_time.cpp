#include <cmath>
#include "gps_time.h"

static const double gpst0[] = {1980, 1, 6, 0, 0, 0}; // 起始时间
static const int LEAPS = 18;

GPSTime Epoch2Time(const double *ep)
{
    const int doy[] = {1, 32, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335};
    GPSTime time = {0};
    int days, sec, year = (int)ep[0], mon = (int)ep[1], day = (int)ep[2];

    if (year < 1970 || 2099 < year || mon < 1 || 12 < mon)
    {
        return time;
    }

    /* leap year if year%4==0 in 1901-2099 */
    days = (year - 1970) * 365 + (year - 1969) / 4 + doy[mon - 1] + day - 2 + (year % 4 == 0 && mon >= 3 ? 1 : 0);
    sec = (int)floor(ep[5]);
    time.time = (time_t)days * 86400 + (int)ep[3] * 3600 + (int)ep[4] * 60 + sec;
    time.sec = ep[5] - sec;
    return time;
}

GPSTime GPSTime2Time(int week, double sec)
{
    GPSTime t = Epoch2Time(gpst0);
    if (sec < -1E9 || 1E9 < sec)
    {
        sec = 0.0;
    }
    t.time += 86400 * 7 * week + (int)sec;
    t.sec = sec - (int)sec;
    return t;
}

void TimeAdd(GPSTime &t, double sec)
{
    double tt;
    t.sec += sec;
    tt = floor(t.sec);
    t.time += (int)tt;
    t.sec -= tt;
}

GPSTime GPSTime2UTCTime(int week, double sec, double leapsec)
{
    GPSTime gpst = GPSTime2Time(week, sec);
    TimeAdd(gpst, -leapsec);
    return gpst;
}

GPSTime GPSTime2UTCTime(int week, double sec)
{
    double leapsec = LEAPS;
    GPSTime gpst = GPSTime2Time(week, sec);
    TimeAdd(gpst, -leapsec);
    return gpst;
}

