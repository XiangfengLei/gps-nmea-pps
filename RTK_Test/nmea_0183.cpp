#ifdef USE_BOOST
#include <boost/algorithm/string.hpp>
#include <boost/assign/list_of.hpp>
#else
#include "str_tools.h"
#endif
#include <vector>
#include "nmea_0183.h"

Nmea0183::Nmea0183(const bool &debug) : debug_(debug)
{
}

Nmea0183::~Nmea0183()
{
}

bool Nmea0183::Checksum(const std::string &nmea)
{
    size_t len = nmea.length();
    if (len < 6)
    {
        return false;
    }

    // $GPGGA, ... \r\n
    size_t pos_asterisk = nmea.find("*");
    if (pos_asterisk == std::string::npos)
    {
        return false;
    }

    char nmea_sum[3] = {0};
    nmea_sum[0] = nmea[pos_asterisk + 1];
    nmea_sum[1] = nmea[pos_asterisk + 2];
    nmea_sum[2] = '\0';
    unsigned int sum = strtoul(nmea_sum, nullptr, 16); // string to long

    unsigned int result = nmea[1];
    for (size_t i = 2; nmea[i] != '*'; ++i)
    {
        result ^= nmea[i];
    }

    if (sum != result)
    {
        printf("Gps check sum fail, sum %02x result %02x\r\n", sum, result);
        return false;
    }

    return true;
}

void Nmea0183::ParserGGA(GGAData &data)
{
#if 0
    printf("GGA: %s\r\n", nmea_.c_str());
    printf("latitude: %0.15f\r\n", strtod(nmea_split_[2].c_str(), nullptr));
    printf("longitude: %0.15f\r\n", strtod(nmea_split_[4].c_str(), nullptr));
    printf("altitude[9]: %0.15f\r\n", strtod(nmea_split_[9].c_str(), nullptr));
    printf("altitude[11]: %0.15f\r\n", strtod(nmea_split_[11].c_str(), nullptr));
#endif

    double degree, minute;

    degree = (int)strtod(nmea_split_[2].c_str(), nullptr) / 100;
    minute = strtod(nmea_split_[2].c_str(), nullptr) - degree * 100;
    data.latitude = degree + minute / 60.0;
    if (nmea_split_[3] == "S")
    {
        data.latitude = -data.latitude;
    }

    degree = (int)strtod(nmea_split_[4].c_str(), nullptr) / 100;
    minute = strtod(nmea_split_[4].c_str(), nullptr) - degree * 100;
    data.longitude = degree + minute / 60.0;
    if (nmea_split_[5] == "W")
    {
        data.longitude = -data.longitude;
    }

    int loc_status = strtol(nmea_split_[6].c_str(), nullptr, 10);
    data.loc_level = loc_status;
    data.altitude = strtod(nmea_split_[9].c_str(), nullptr) + strtod(nmea_split_[11].c_str(), nullptr);
    data.enable = true;
    if (debug_)
    {
        printf("RTK GGA: %d,%f,%f,%f\r\n", data.loc_level, data.latitude, data.longitude, data.altitude);
    }
}

void Nmea0183::ParserRMC(RMCData &data)
{
#if 0
    printf("RMC: %s\r\n", nmea_.c_str());
    printf("utc hhmmss: %lf\r\n", strtod(nmea_split_[1].c_str(), nullptr));
    printf("utc ddmmyy: %lf\r\n", strtod(nmea_split_[9].c_str(), nullptr));
    printf("loc status: %s\r\n", nmea_split_[2].c_str());
#endif

    if (nmea_split_[2] == "V")
    {
        data.gps_status = 'V';
    }
    else
    {
        data.gps_status = 'A';
        data.utc_hour = strtol(nmea_split_[1].substr(0, 2).c_str(), nullptr, 10);
        data.utc_minute = strtol(nmea_split_[1].substr(2, 2).c_str(), nullptr, 10);
        data.utc_second = strtol(nmea_split_[1].substr(4, 2).c_str(), nullptr, 10);
        data.utc_msecond = (strtol(nmea_split_[1].substr(7, 2).c_str(), nullptr, 10) / 100.0) * 1000; // milliseconds
        data.utc_year = strtol(nmea_split_[9].substr(4, 2).c_str(), nullptr, 10) + 2000;
        data.utc_month = strtol(nmea_split_[9].substr(2, 2).c_str(), nullptr, 10);
        data.utc_day = strtol(nmea_split_[9].substr(0, 2).c_str(), nullptr, 10);
    }

    if (debug_)
    {
        printf("RTK RMC: %s,%d,%d,%d,%d,%d,%d,%d\r\n", data.gps_status == 'V' ? "V" : "A",
               data.utc_year, data.utc_month, data.utc_day,
               data.utc_hour, data.utc_minute, data.utc_second, data.utc_msecond);
    }

    data.enable = true;
}

bool Nmea0183::Parser(const std::string &nmea, NMEAData &data)
{
    if (debug_)
    {
        printf("RKT nmea: %s\r\n", nmea.c_str());
    }
    nmea_.clear();
    nmea_ = nmea;

    bool is_nmea = Checksum(nmea_);
    if (!is_nmea)
    {
        printf("NMEA Checksum fail: %s\r\n", nmea_.c_str());
        return false;
    }
    nmea_split_.clear();

#ifdef USE_BOOST
    boost::split(nmea_split_, nmea_, boost::is_any_of(","));
#else
    nmea_split_ = SpliteString(nmea_, ",");
#endif

    NmeaType type = UNKNOWN;
    if (nmea_split_[0].find("GGA") != std::string::npos)
        type = GGA;
    if (nmea_split_[0].find("RMC") != std::string::npos)
        type = RMC;

    data.type = type;
    switch (type)
    {
    case GGA:
        ParserGGA(data.gga);
        break;
    case RMC:
        ParserRMC(data.rmc);
        break;
    case UNKNOWN:
        break;
    default:
        break;
    }

    return true;
}
