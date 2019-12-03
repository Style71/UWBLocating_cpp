#include <iostream>
#include <unistd.h>
#include <time.h>
//#define NDEBUG
#include <cassert>
#include <wiringSerial.h>
#include "UWBFrame.h"

using std::cerr;
using std::cout;
using std::ofstream;

UWBFrame::UWBFrame(/* args */)
{
    fileDescription_UWB = -1;
    iState = 0;
    status = No_Frame;
}

UWBFrame::UWBFrame(const char *device_path)
{
    iState = 0;
    status = No_Frame;
    // Open UWB serial port.
    fileDescription_UWB = serialOpen(device_path, 921600);
    if (fileDescription_UWB < 0)
        cerr << "Failed to open " << device_path << " for UWB device.\n";
    else
    {
        cout << "Open " << device_path << " for UWB device.\n";
        // Clear all data in the serial buffer.
        serialFlush(fileDescription_UWB);

        time_t rawtime;
        struct tm *localTime;
        // Open log file.
        time(&rawtime);
        localTime = localtime(&rawtime);
        UWBRawLog.open("UWB_Raw_Log%04d-%02d-%02d-%02d-%02d-%02d.txt", ofstream::out, localTime->tm_year + 1900, localTime->tm_mon + 1, localTime->tm_mday, localTime->tm_hour, localTime->tm_min, localTime->tm_sec);
    }
}

UWBFrame::~UWBFrame()
{
    // If current class is associated with a serial port, close it first.
    if (fileDescription_UWB >= 0)
    {
        serialClose(fileDescription_UWB);
        fileDescription_UWB = -1;
        UWBRawLog.close();
    }
}

bool UWBFrame::open(const char *device_path)
{
    bool bIsOpened = false;
    if (fileDescription_UWB < 0)
    {
        // Open UWB serial port.
        fileDescription_UWB = serialOpen(device_path, 921600);
        if (fileDescription_UWB < 0)
            cerr << "Failed to open " << device_path << " for UWB device.\n";
        else
        {
            bIsOpened = true;
            cout << "Open " << device_path << " for UWB device.\n";
            // Clear all data in the serial buffer.
            serialFlush(fileDescription_UWB);

            time_t rawtime;
            struct tm *localTime;
            // Open log file.
            time(&rawtime);
            localTime = localtime(&rawtime);
            UWBRawLog.open("UWB_Raw_Log%04d-%02d-%02d-%02d-%02d-%02d.txt", ofstream::out, localTime->tm_year + 1900, localTime->tm_mon + 1, localTime->tm_mday, localTime->tm_hour, localTime->tm_min, localTime->tm_sec);

            iState = 0;
            status = No_Frame;
        }
    }
    else
        cerr << "Unable to open " << device_path << " cause current UWBFrame object is already associated with a serial port.\n";
    return bIsOpened;
}

void UWBFrame::close()
{
    if (fileDescription_UWB >= 0)
    {
        serialClose(fileDescription_UWB);
        fileDescription_UWB = -1;
        UWBRawLog.close();
        status = No_Frame;
    }
    else
        cerr << "Failed to close UWB device port cause current UWBFrame object is not associated with a serial port.\n";
}

uint8_t UWBFrame::updateData()
{
    int iSize, result;
    // obtain data size:
    iSize = serialDataAvail(fileDescription_UWB);
    // If new data is available.
    if (iSize > 0)
    {
        // Truncate data to the maximun size allowed.
        if (iSize > UWB_SERIAL_BUFFER_SIZE)
            iSize = UWB_SERIAL_BUFFER_SIZE;
        // copy the data into the buffer:
        result = read(fileDescription_UWB, buf, iSize);
        UWBRawLog.write(buf, result);

        unsigned char *msg = buf;
        int count = result;
        uint8_t retVal = No_Frame;

        while (count > 0)
        {
            switch (iState)
            {
            case 0:
                // If we get UWB message starting character '$', initialize the state machine and clear the message buffer.
                if (*msg == 0x55)
                {
                    iState = 1;
                    bufIndex = 0;
                    ucChecksum = *msg;
                    buf[bufIndex++] = *msg;
                }
                break;
            case 1:
                // If we get UWB message type character, go to next state.
                if ((*msg == 0x00) || (*msg == 0x01) || (*msg == 0x02) || (*msg == 0x03) || (*msg == 0x04))
                {
                    buf[bufIndex++] = *msg;
                    ucChecksum += *msg;
                    switch (buf[1])
                    {
                    case 0x00:
                        remainingBytes = 894;
                        iState = 4;
                        break;
                    case 0x01:
                        remainingBytes = 126;
                        iState = 4;
                        break;
                    case 0x02:
                    case 0x03:
                    case 0x04:
                        iState = 2;
                        break;

                    default:
                        break;
                    }
                }
                else // Get illegal character, roll back to initial state.
                    iState = 0;
                break;
            case 2:
                buf[bufIndex++] = *msg;
                ucChecksum += *msg;
                FrameLength = (uint16_t)*msg;
                iState = 3;
                break;
            case 3:
                buf[bufIndex++] = *msg;
                ucChecksum += *msg;
                FrameLength |= ((uint16_t)*msg) << 8;
                remainingBytes = FrameLength - 4;
                iState = 4;
                break;
            case 4:
                buf[bufIndex++] = *msg;

                remainingBytes--;
                if (remainingBytes > 0)
                    ucChecksum += *msg;
                else
                {
                    if ((uint8_t)buf[bufIndex - 1] == ucChecksum)
                    {
                        switch (buf[1])
                        {
                        case 0x00:

                            break;
                        case 0x01:

                            break;
                        case 0x02:
                            unpackNodeFrame0Data(buf);
                            // Copy node frame0 to current object's member variable.
                            frame0 = nodeFrame0Data_;
                            cout<<"Get Frame0\n";
                            // Copy data in frame0 to data buffer.
                            for (int i = 0; i < nodeFrame0Data_.framePart.validNodeQuantity; i++)
                            {
                                int id = nodeFrame0Data_.nodeIDList[i];
                                int DataLength = nodeFrame0Data_.node[id]->dataLength;

                                assert(id < MAX_POSSIBLE_NODE_NUM);
                                for (int j = 0; j < DataLength; j++)
                                    UWBDataBuffer[id].push_back(nodeFrame0Data_.node[id]->data[j]);
                            }
                            retVal |= UWB_Node_Frame0;
                            status |= UWB_Node_Frame0;
                            break;
                        case 0x03:

                            break;
                        case 0x04:
                            unpackNodeFrame2Data(buf);
                            // Copy node frame2 to current object's member variable.
                            frame2 = nodeFrame2Data_;
                            cout<<"Get Frame2\n";
                            retVal |= UWB_Node_Frame2;
                            status |= UWB_Node_Frame2;
                            break;
                        default:
                            break;
                        }
                        //printf("Nlink 0x%02hhX frame checksum OK.\n", buf[1]);
                    }
                    else
                    {
                        //printf("Nlink 0x%02hhX frame checksum error, get 0x%02hhX, calculated 0x%02hhX.\n", buf[1], (uint8_t)buf[bufIndex - 1], ucChecksum);
                    }
                    iState = 0;
                }
                break;
            default:
                break;
            }
            msg++;
            count--;
        }

        return retVal;
    }
    else
        return No_Frame;
}

bool UWBFrame::is_getNewFrame(UWBFrame_Type type)
{
    return (type && status);
}

bool UWBFrame::getFrame(UWBFrame_Type type)
{
    if (type && status)
    {
        status &= (~type);
        return true;
    }
    else
        return false;
}