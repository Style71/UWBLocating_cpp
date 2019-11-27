#include <iostream>
#include <signal.h>
#include <atomic>
#include <unistd.h>
#include <cassert>
#include "SDLogger.h"
#include "UWBFrame.h"
#include "systime.h"

using namespace std;

atomic<char> quit(false); // signal flag
void got_signal(int)
{
    quit = true;
}

/*int main(int argc, char *argv[])
{
    SDLogWriter outfile1, outfile2("new2.txt"), outfile3;
    UWB_Node0_t testNode;
    double runTime = 0, last_time, store;
    bool isOpen = false;

    signal(SIGINT, got_signal);
    initSystime();

    cout<<"length: "<<testNode.data.size()<<", capacity: "<<testNode.data.capacity()<<", sizeof(testNode): "<<sizeof(testNode)<<endl;
    for (uint8_t i=0;i<128;i++)
    {
        testNode.data.push_back(i);
        cout<<"length: "<<testNode.data.size()<<", capacity: "<<testNode.data.capacity()<<", sizeof(testNode): "<<sizeof(testNode)<<endl;
    }
    while (true)
    {
        if ((runTime - last_time) > 0.1)
        {
            if (!isOpen)
            {
                outfile1.open("new1.txt", ios_base::out);
                outfile3.open("new3_%.2f.txt", ios_base::out, runTime);
                isOpen = true;
            }
            outfile1 << runTime << '\n';
            outfile3 << runTime << endl;
            cout << runTime << '\n';
            outfile2.putsLogandConsole("Current time: %.2f\n", runTime);
            //outfile2.write((char *)&runTime, sizeof(double));

            last_time = runTime;
        }
        runTime = getSystime();

        if (quit)
            break; // exit normally after SIGINT
    }
    return 0;
}*/

int main(int argc, char *argv[])
{
    time_t rawtime;
    struct tm *localTime;
    int iSize, result;

    signal(SIGINT, got_signal);
    initSystime();

#define MAX_SERIALPORT_PATH_NAME_SIZE 32
    char deviceName_UWB[MAX_SERIALPORT_PATH_NAME_SIZE] = "/dev/ttyS21";
    // If additional arguments are given, set serial port of the device according to the arguments.
    if (argc > 1)
    {
        int i = 0;
        while ((argv[1][i] != '\0') && (i < (MAX_SERIALPORT_PATH_NAME_SIZE - 1)))
        {
            deviceName_UWB[i] = argv[2][i];
            i++;
        }
        deviceName_UWB[i] = '\0';
    }
    // Create a UWBFrame object to open UWB serial port, read and parsing data.
    UWBFrame UWBFrameParser(deviceName_UWB);
    // Open log file.
    time(&rawtime);
    localTime = localtime(&rawtime);
    SDLogWriter binaryLog("binaryLog%04d-%02d-%02d-%02d-%02d-%02d.bin", ofstream::out | ofstream::binary, localTime->tm_year + 1900, localTime->tm_mon + 1, localTime->tm_mday, localTime->tm_hour, localTime->tm_min, localTime->tm_sec);
    SDLogWriter LogFile("Log%04d-%02d-%02d-%02d-%02d-%02d.txt", ofstream::out, localTime->tm_year + 1900, localTime->tm_mon + 1, localTime->tm_mday, localTime->tm_hour, localTime->tm_min, localTime->tm_sec);

    while (true)
    {
        if (UWBFrameParser.updateData() > 0)
        {
            if (UWBFrameParser.is_getNewFrame(UWB_Node_Frame2))
            {
                UWBFrameParser.getFrame(UWB_Node_Frame2);
                binaryLog << UWBFrameParser.frame2;

                LogFile.putsLogandConsole("Get UWB frame2:\n");
                LogFile.putsLogandConsole("%hhu\t%u\t%hhu\t", UWBFrameParser.frame2.framePart.id, UWBFrameParser.frame2.framePart.systemTime, UWBFrameParser.frame2.framePart.validNodeQuantity);

                int NodeExist[MAX_POSSIBLE_NODE_NUM];

                for (int i = 0; i < MAX_POSSIBLE_NODE_NUM; i++)
                    NodeExist[i] = 0;
                for (int i = 0; i < UWBFrameParser.frame2.framePart.validNodeQuantity; i++)
                {
                    if (UWBFrameParser.frame2.nodeIDList[i] < MAX_POSSIBLE_NODE_NUM)
                        NodeExist[UWBFrameParser.frame2.nodeIDList[i]] = 1;
                }
                for (int i = 0; i < MAX_POSSIBLE_NODE_NUM; i++)
                {
                    if (NodeExist[i])
                        LogFile.putsLogandConsole("D%hhu-D%hhu\t%.3f\t", UWBFrameParser.frame2.framePart.id, i, UWBFrameParser.frame2.node[i]->dis);
                    else if ((NodeExist[i] == 0) && (UWBFrameParser.frame2.framePart.id == i))
                        LogFile.putsLogandConsole("D%hhu-D%hhu\t0.000\t", i, i);
                    else
                        LogFile.putsLogandConsole("D%hhu-D%hhu\tNan\t", UWBFrameParser.frame2.framePart.id, i);
                }
                LogFile.putsLogandConsole("\n");
            }
            if (UWBFrameParser.is_getNewFrame(UWB_Node_Frame0))
            {
                UWBFrameParser.getFrame(UWB_Node_Frame0);
                binaryLog << UWBFrameParser.frame0;

                LogFile.putsLogandConsole("Get UWB frame0:\n");
                for (int i=0;i<MAX_POSSIBLE_NODE_NUM;i++)
                {
                    if (UWBFrameParser.UWBDataBuffer[i].size()>0)
                    {
                        LogFile.putsLogandConsole("Get %d chars from node %d: ", UWBFrameParser.UWBDataBuffer[i].size(),i);
                        while(UWBFrameParser.UWBDataBuffer[i].size()>0)
                        {
                            LogFile.putsLogandConsole("0x%02X ", UWBFrameParser.UWBDataBuffer[i].front());
                            UWBFrameParser.UWBDataBuffer[i].pop_front();
                        }
                        LogFile.putsLogandConsole("\n");
                    }
                }
                
            }
        }
        if (quit)
            break; // exit normally after SIGINT
    }
    return 0;
}
