#include <chrono>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <memory>
#include <string>
#include <sys/time.h>
#include <thread>
#include <time.h>
#include "bd_unicore.h"
#include "gps_time.h"
#include "nmea_0183.h"
#include "rtkpps.h"
#include "uart_serial.h"

int main(int argc, char **argv)
{
    int fd = -1;
    if (argc != 2)
    {
        printf("usage: %s rtk_device!\r\n", argv[0]);
        return -1;
    }
    std::string dev = std::string(argv[1]);
    bool ret = OpenUart(fd, dev);
    if (!ret)
    {
        printf("open %s fail\r\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    SerialParams params(115200, 8, 'N', 1);
    ret = SetUart(fd, params);
    if (!ret)
    {
        printf("set uart error(%s)!\n", strerror(errno));
        CloseUart(fd);
        exit(EXIT_FAILURE);
    }

    NMEAData data;
    UnicoreData unicore_data;
    std::unique_ptr<Nmea0183> nmea = std::make_unique<Nmea0183>(false);
    std::unique_ptr<BDUnicore> unicore = std::make_unique<BDUnicore>(false);

    pps_open();

    struct GPSTime gps_time;
    struct timeval utcstamp;
    while (1)
    {
        std::string line = ReadLine(fd);
        if (!line.empty())
        {
            size_t n = line.find_last_not_of("\r\n");
            if (n != std::string::npos)
            {
                line.erase(n + 1, line.size() - n);
            }
            if (line.at(0) == '$')
            {
                gettimeofday(&utcstamp, NULL);

                nmea->Parser(line, data);

                if (data.rmc.gps_status == 'A' && data.type == RMC)
                {
                    //		printf("year=%d, month=%d, day=%d, hour=%d, min=%d, sec=%d msec=%d %lums\n",
                    //		data.rmc.utc_year, data.rmc.utc_month, data.rmc.utc_day, data.rmc.utc_hour, data.rmc.utc_minute,data.rmc.utc_second, data.rmc.utc_msecond, utcstamp.tv_sec*1000+utcstamp.tv_usec/1000);
                    gps_t t;
                    t.stamp = utcstamp;
                    t.utc_time.utc_year = data.rmc.utc_year;
                    t.utc_time.utc_month = data.rmc.utc_month;
                    t.utc_time.utc_day = data.rmc.utc_day;
                    t.utc_time.utc_hour = data.rmc.utc_hour;
                    t.utc_time.utc_minute = data.rmc.utc_minute;
                    t.utc_time.utc_second = data.rmc.utc_second;
                    t.utc_time.utc_msecond = data.rmc.utc_msecond;
                    pps_push_gps_time(t);
                }
            }
            else if (line.at(0) == '#')
            {
                unicore->Parser(line, unicore_data);
                gps_time = GPSTime2UTCTime(unicore_data.pvtsln.week, unicore_data.pvtsln.week_ms);
                //printf("week=%d, %lu, %ld, %f\n", unicore_data.pvtsln.week, unicore_data.pvtsln.week_ms, gps_time.time, gps_time.sec);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    CloseUart(fd);
    pps_close();
    return 0;
}
