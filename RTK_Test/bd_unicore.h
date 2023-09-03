#ifndef BD_UNICORE_H
#define BD_UNICORE_H

#include <string>
#include <vector>

enum class UnicoreType : uint8_t
{
    ENVENT,
    UNKNOWN
};

struct PVTSLNData
{
    std::string time_status = "Unknown"; /* gps time quality */
    unsigned short week = 0;             /* gps weeks */
    unsigned long week_ms = 0;           /* GPS seconds within a week, accurate to ms. */
    int bestpos_type = 0;                /** rtk position type*/
    float bestpos_hgt = 0.0;             /* attitude */
    double bestpos_lat = 0.0;            /*latitude*/
    double bestpos_lon = 0.0;            /*longitude*/
    float bestpos_hgtstd = 0.0;          /*standard error of attitude */
    float bestpos_latstd = 0.0;          /*standard error of latitude */
    float bestpos_lonstd = 0.0;          /*standard error of longitude */
    float bestpos_diffage = 0.0;         /* differential age for rtk*/
    float undulation = 0.0;              /*undulation*/
    int bestpos_svs = 0;                 /*number of satellites*/
    int bestpos_solnsvs = 0;             /*solving the satellites for calculation*/
    float pdop = 0.0;                    /*position dop*/
    float hdop = 0.0;                    /*horizon dop*/
    float vdop = 0.0;                    /*vertical dop*/
    bool enable = false;
};

struct UnicoreData
{
    PVTSLNData pvtsln; // 定位定向信息
};

class BDUnicore
{
public:
    BDUnicore(const bool &debug);
    ~BDUnicore();
    bool Parser(const std::string &unicore_msg, UnicoreData &data);

private:
    bool CheckCRC32(const std::string &unicore_msg);
    bool ParserPVTSLN(const std::vector<std::string> &unicore_splits, UnicoreData &data);

    bool debug_ = false;
};

#endif
