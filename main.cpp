#include <iostream>
#include <signal.h>
#include <atomic>
#include <unistd.h>
#include <cassert>
#include "SDLogger.h"
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
    int fileDescription_UWB;
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

    // Open UWB serial port.
    fileDescription_UWB = serialOpen(deviceName_UWB, 921600);
    if (fileDescription_UWB < 0)
    {
        printf("Failed to open %s for UWB device.\n", deviceName_UWB);
        return 1;
    }
    else
        printf("Open %s for UWB device.\n", deviceName_UWB);
    // Clear all data in the serial buffer.
    serialFlush(fileDescription_UWB);

    // Open log file.
    time(&rawtime);
    localTime = localtime(&rawtime);
    SDLogWriter binaryLog("binaryLog.txt", ofstream::out | ofstream::binary);
    SDLogWriter UWBRawLog("UWB_Raw_Log%04d-%02d-%02d-%02d-%02d-%02d.txt", ofstream::out, localTime->tm_year + 1900, localTime->tm_mon + 1, localTime->tm_mday, localTime->tm_hour, localTime->tm_min, localTime->tm_sec);

#define SERIAL_BUFFERSIZE 8192
    char buffer[SERIAL_BUFFERSIZE];
    while (true)
    {
        // obtain data size:
        iSize = serialDataAvail(fileDescription_UWB);
        // If new data is available.
        if (iSize > 0)
        {
            // Truncate data to the maximun size allowed.
            if (iSize > SERIAL_BUFFERSIZE)
                iSize = SERIAL_BUFFERSIZE;
            // copy the data into the buffer:
            result = read(fileDescription_UWB, buffer, iSize);
            binaryLog.write(buffer, result);
            isGetNewFrame = UWBMessageProcess(buffer, result);
        }

        if (quit)
            break; // exit normally after SIGINT
    }
    return 0;
}
