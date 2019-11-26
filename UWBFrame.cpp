#include <iostream>
#include <unistd.h>
#include <time.h>
#include <wiringSerial.h>
#include "UWBFrame.h"

using std::cerr, std::cout;

UWBFrame::UWBFrame(/* args */)
{
    fileDescription_UWB = -1;
}

UWBFrame::UWBFrame(const char *device_path)
{
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
    if (fileDescription_UWB < 0)
    {
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
    else
        cerr << "Unable to open " << device_path << " cause current UWBFrame object is already associated with a serial port.\n";
}

void UWBFrame::close()
{
    if (fileDescription_UWB >= 0)
    {
        serialClose(fileDescription_UWB);
        fileDescription_UWB = -1;
        UWBRawLog.close();
    }
    else
        cerr << "Failed to close UWB device port cause current UWBFrame object is not associated with a serial port.\n";
}

UWBFrame_Type UWBFrame::updateData()
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
        result = read(fileDescription_UWB, buffer, iSize);
        UWBRawLog.write(buffer, result);

        unsigned char *msg = buffer;
        int count = result;
#define UWB_MESSAGE_BUFFERSIZE 8192
        static int iState = 0; //State: 0 - Idle; 1 - Get UWB message starting symbol '0x55'; 2 - Get function mark '0x02'/'0x03'/'0x04'; 3 - Get fitst byte of framelength; 4 - Get second byte of framelength; 5 - Dispatch message.
        static char buf[UWB_MESSAGE_BUFFERSIZE];
        static int bufIndex;
        static uint8_t ucChecksum;
        static uint16_t FrameLength;
        static int remainingBytes;
        UWBFrame_Type retVal = No_Frame;

        static char isInitializeBuffer = 0;

        // Initialize UWBDataBuffer.
        if (isInitializeBuffer == 0)
        {
            for (int i = 0; i < NODENUM; i++)
            {
                UWBDataBuffer[i].head = 0;
                UWBDataBuffer[i].tail = 0;
            }
            isInitializeBuffer = 1;
        }

        while (count > 0)
        {
            switch (iState)
            {
            case 0:
                // If we get GPS message starting character '$', initialize the state machine and clear the message buffer.
                if (*msg == 0x55)
                {
                    iState = 1;
                    bufIndex = 0;
                    ucChecksum = *msg;
                    buf[bufIndex++] = *msg;
                }
                break;
            case 1:
                // If we get GPS message end character '*', go to checksum state.
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
                            for (int i = 0; i < nodeFrame0Data_.framePart.validNodeQuantity; i++)
                            {
                                int id = nodeFrame0Data_.nodeIDList[i];
                                int DataLength = nodeFrame0Data_.node[id]->dataLength;
                                int size;

                                if ((UWBDataBuffer[id].tail + DataLength) > (QUEUE_BUFFERSIZE - 1))
                                {
                                    size = QUEUE_BUFFERSIZE - UWBDataBuffer[id].tail;
                                    memcpy(UWBDataBuffer[id].buffer + UWBDataBuffer[id].tail, nodeFrame0Data_.node[id]->data, size);
                                    memcpy(UWBDataBuffer[id].buffer, nodeFrame0Data_.node[id]->data + size, DataLength - size);
                                    UWBDataBuffer[id].tail = DataLength - size;
                                }
                                else
                                {
                                    memcpy(UWBDataBuffer[id].buffer + UWBDataBuffer[id].tail, nodeFrame0Data_.node[id]->data, DataLength);
                                    UWBDataBuffer[id].tail += DataLength;
                                }
                            }
                            retVal |= GET_NLINK_FRAME0;
                            break;
                        case 0x03:

                            break;
                        case 0x04:
                            unpackNodeFrame2Data(buf);
                            retVal |= GET_NLINK_FRAME2;
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
}

bool UWBFrame::getFrame(UWBFrame_Type type)
{
}