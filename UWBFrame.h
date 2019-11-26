#ifndef UWBFRAME_H
#define UWBFRAME_H

#include "nlink_linktrack_node_frame0.h"
#include "nlink_linktrack_node_frame2.h"
#include "SDLogger.h"

enum UWBFrame_Type : uint8_t
{
    No_Frame = 0x00,
    //UWB_Anchor_Frame0 = 0x01,
    //UWB_Tag_Frame0 = 0x02,
    UWB_Node_Frame0 = 0x04,
    //UWB_Node_Frame1 = 0x08,
    UWB_Node_Frame2 = 0x10,
};

class UWBFrame
{
private:
    int fileDescription_UWB;
    #define UWB_SERIAL_BUFFER_SIZE 8192
    char buffer[UWB_SERIAL_BUFFER_SIZE];

    SDLogWriter UWBRawLog;

    NodeFrame0Data frame0;
    NodeFrame2Data frame2;
public:
    UWBFrame();
    UWBFrame(const char *device_path);
    ~UWBFrame();

    bool open(const char *device_path);
    void close();
    UWBFrame_Type updateData();
    bool is_getNewFrame(UWBFrame_Type type);
    bool getFrame(UWBFrame_Type type);
    void renameLog(const char *pcString, ...);
};

#endif