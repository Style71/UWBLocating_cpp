#ifndef SDLOGGER_H
#define SDLOGGER_H

#include <fstream>
#include <list>
#include <vector>
#include <cstdarg>
#include "nlink_linktrack_node_frame0.h"
#include "nlink_linktrack_node_frame2.h"
#include "GPSDecode.h"

using std::list;
using std::vector;

#define LOG_SAVE_INTERVAL 10
#define MAX_LOGFILENAME_SIZE 128
#define PRINTF_BUFFER_SIZE 1024

enum SDLog_Type : uint16_t
{
    UWB_Frame0 = 0x5501,
    UWB_Frame2 = 0x5502,
    GPS_Raw = 0x5503,
};

// Struct definition of various SD log type.
typedef struct
{
    enum SDLog_Type usHeader;
    unsigned int length;
    double timebase_rp3;

    NLink_LinkTrack_Node_Frame2_Part framePart;
    float pos[3];
    float eop[3];
    float vel[3];
    float angle[3];
    float supplyVoltage;
    vector<Node2_t> nodes;

    uint8_t ucChecksum;
} UWB_Frame2_SDLog;

typedef struct
{
    uint8_t role;
    uint8_t id;
    vector<uint8_t> data;
} UWB_Node0_t;

typedef struct
{
    enum SDLog_Type usHeader;
    unsigned int length;
    double timebase_rp3;

    NLink_LinkTrack_Node_Frame0_Part framePart;
    vector<UWB_Node0_t> nodes;

    uint8_t ucChecksum;
} UWB_Frame0_SDLog;

typedef struct
{
    enum SDLog_Type usHeader;
    unsigned int length;
    double timebase_rp3;

    nmea_msg msg;

    uint8_t ucChecksum;
} GPS_Raw_SDLog;

// Class SDLogWriter declarition.
class SDLogWriter : public std::ofstream
{
private:
    static bool isInit;
    static list<SDLogWriter *> logArray;

    // Save logfile in a period of LOG_SAVE_INTERVAL.
    static void *thread_SaveLog(void *arg);

    //int nameFormatter(char* pcString, char* filename, va_list arg);
    char fileName[MAX_LOGFILENAME_SIZE];
    std::ios_base::openmode openMode;
    /* data */
public:
    SDLogWriter(/* args */);
    SDLogWriter(const char *pcString, std::ios_base::openmode mode = std::ios_base::out, ...);

    ~SDLogWriter();

    void rename(const char *pcString, ...);
    void open(const char *pcString, std::ios_base::openmode mode = std::ios_base::out, ...);
    void close();
    int putsLog(const char *pcString, ...);
    int putsLogandConsole(const char *pcString, ...);

    friend SDLogWriter &operator<<(SDLogWriter &logWriter, const NodeFrame2Data &frame);
    friend SDLogWriter &operator<<(SDLogWriter &logWriter, const NodeFrame0Data &frame);
    friend SDLogWriter &operator<<(SDLogWriter &logWriter, const nmea_msg &msg);
};

class SDLogReader
{
private:
    /* data */
public:
    SDLogReader() {}
    SDLogReader(const char *filename) { ReadLog(filename); }
    ~SDLogReader() {}

    bool ReadLog(const char *filename);

    vector<UWB_Frame2_SDLog> UWB_Frame2_logArray;
    vector<UWB_Frame0_SDLog> UWB_Frame0_logArray;
    vector<GPS_Raw_SDLog> GPS_Raw__logArray;
};

#endif