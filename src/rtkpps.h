#ifndef __PPS_TIME__H
#define __PPS_TIME__H

#include <stdint.h>

typedef struct
{
    unsigned int utc_year;
    unsigned int utc_month;
    unsigned int utc_day;
    unsigned int utc_hour;
    unsigned int utc_minute;
    unsigned int utc_second;
    unsigned int utc_msecond;
} utc_t;

/*
 *stamp:
 *utc_time: 解析nmea报文中带的utc时间
 */
typedef struct
{
    struct timeval stamp;
    utc_t utc_time;

} gps_t;

/*
 * open /dev/pps0 device, system need install pps-gpio.ko.
*/
int pps_open(void);

/*
*  close /dev/pps0 device
*/
void pps_close(void);

/*
 *@param t: stamp: 读取到nmea报文时，soc平台记录当前时间戳(gettimeofday)
 *          utc_time: 解析nmea报文中带的utc时间
 *return : 0: success, -1:failed
*/
int8_t pps_push_gps_time(gps_t t);

/*
*return: 1: success, 0:failed
*/
int8_t pps_get_successfully_flag(void);

#endif
