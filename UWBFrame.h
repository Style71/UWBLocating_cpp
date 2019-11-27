#ifndef UWBFRAME_H
#define UWBFRAME_H

#include <deque>
#include "nlink_linktrack_node_frame0.h"
#include "nlink_linktrack_node_frame2.h"
#include "SDLogger.h"

#define MAX_POSSIBLE_NODE_NUM 6

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
    char buf[UWB_SERIAL_BUFFER_SIZE];
    int iState; //State: 0 - Idle; 1 - Get UWB message starting symbol '0x55'; 2 - Get function mark '0x02'/'0x03'/'0x04'; 3 - Get fitst byte of framelength; 4 - Get second byte of framelength; 5 - Dispatch message.
    int bufIndex;
    uint8_t ucChecksum;
    int remainingBytes;
    uint16_t FrameLength;

    int status;

public:
    UWBFrame();
    UWBFrame(const char *device_path);
    ~UWBFrame();

    bool open(const char *device_path);
    void close();
    UWBFrame_Type updateData();
    bool is_getNewFrame(UWBFrame_Type type);
    bool getFrame(UWBFrame_Type type);

    SDLogWriter UWBRawLog;
    NodeFrame0Data frame0;
    NodeFrame2Data frame2;
    std::deque<uint8_t> UWBDataBuffer[MAX_POSSIBLE_NODE_NUM];
};

#endif