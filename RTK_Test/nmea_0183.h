#ifndef NMEA_0183_H
#define NMEA_0183_H

#include <string>
#include <vector>

enum NmeaType
{
    GGA,
    TRA,
    RMC,
    UNKNOWN
};

struct GGAData
{
    double latitude = 0;  // 纬度
    double longitude = 0; // 经度
    double altitude = 0;  // 高程

    double pdop = 99.9;
    double hdop = 99.9;
    double vdop = 99.9;
    unsigned int loc_level = 0; // 定位状态

    bool enable = false;
};

struct RMCData
{
    unsigned int utc_year = 0;
    unsigned int utc_month = 0;
    unsigned int utc_day = 0;
    unsigned int utc_hour = 0;
    unsigned int utc_minute = 0;
    unsigned int utc_second = 0;
    unsigned int utc_msecond = 0;
    unsigned char gps_status = 'V'; // gps 状态

    bool enable = false;
};

struct NMEAData
{
    enum NmeaType type;
    GGAData gga;
    RMCData rmc;
};

class Nmea0183
{
public:
    Nmea0183(const bool &debug);
    ~Nmea0183();
    bool Parser(const std::string &nmea, NMEAData &data);
    bool Checksum(const std::string &nmea);
    void ParserGGA(GGAData &data);
    void ParserRMC(RMCData &data);

private:
    std::string nmea_;
    std::vector<std::string> nmea_split_;
    bool debug_ = false;
};
#endif
