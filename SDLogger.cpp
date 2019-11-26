#include <iostream>
#include <fstream>
#include <signal.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <cstddef>

//#define NDEBUG
#include <cassert>

#include "SDLogger.h"
#include "systime.h"

using namespace std;

list<SDLogWriter *> SDLogWriter::logArray;
bool SDLogWriter::isInit = false;

SDLogWriter::SDLogWriter(/* args */) : ofstream()
{
    // If this is the first SDlog object, initialize all the static variable.
    if (!isInit)
    {
        pthread_t tid;
        pthread_create(&tid, NULL, thread_SaveLog, NULL);
        isInit = true;
    }
}

SDLogWriter::SDLogWriter(const char *pcString, ios_base::openmode mode, ...) : ofstream(), openMode(mode)
{
    // If this is the first SDlog object, initialize all the static variable.
    if (!isInit)
    {
        pthread_t tid;
        pthread_create(&tid, NULL, thread_SaveLog, NULL);
        isInit = true;
    }

    //Acquire a pointer to the argument list.
    va_list ap;
    // Parse the logfile name.
    // Initialize the list pointer.
    va_start(ap, mode);
    // Formating the output string.
    vsnprintf(fileName, MAX_LOGFILENAME_SIZE, pcString, ap);
    va_end(ap);

    ofstream::open(fileName, mode);

    if (is_open())
        logArray.push_back(this);
    else
        cout << "Error opening file " << fileName << ".\n";
}

void SDLogWriter::open(const char *pcString, ios_base::openmode mode, ...)
{
    //Acquire a pointer to the argument list.
    va_list ap;
    // Parse the logfile name.
    // Initialize the list pointer.
    va_start(ap, mode);
    // Formating the output string.
    vsnprintf(fileName, MAX_LOGFILENAME_SIZE, pcString, ap);
    va_end(ap);

    ofstream::open(fileName, mode);
    openMode = mode;

    if (is_open())
        logArray.push_back(this);
    else
        cout << "Error opening file " << fileName << ".\n";
}

void SDLogWriter::rename(const char *pcString, ...)
{
    if (is_open())
    {
        int result;
        char newFilename[MAX_LOGFILENAME_SIZE];

        // Close file first.
        ofstream::close();

        //Acquire a pointer to the argument list.
        va_list ap;
        // Parse the logfile name.
        // Initialize the list pointer.
        va_start(ap, pcString);
        // Formating the output string.
        vsnprintf(newFilename, MAX_LOGFILENAME_SIZE, pcString, ap);
        va_end(ap);

        result = std::rename(fileName, newFilename);
        if (result == 0)
        {
            cout << fileName << " successfully renamed to " << newFilename << ".\n";
            strcpy(fileName, newFilename);
        }
        else
            cout << "Error renaming file " << fileName << ".\n";

        ofstream::open(fileName, openMode | std::fstream::app);
    }
    else
    {
        cout << "Current SDLog object is not associated with file " << fileName << ".\n";
    }
}

void SDLogWriter::close()
{
    ofstream::close();
    logArray.remove(this);
}

SDLogWriter::~SDLogWriter()
{
    // If the log stream is associated with a file, close it first.
    if (is_open())
        close();
}

/*int SDLogWriter::nameFormatter(char *pcString, char *filename, va_list arg)
{
    int length;
    // Formating the output string.
    length = vsnprintf(filename, MAX_LOGFILENAME_SIZE, pcString, arg);
    length = (length < MAX_LOGFILENAME_SIZE) ? length : MAX_LOGFILENAME_SIZE - 1;

    return length;
}*/

void *SDLogWriter::thread_SaveLog(void *arg)
{
    pthread_detach(pthread_self());
    while (true)
    {
        for (list<SDLogWriter *>::iterator it = logArray.begin(); it != logArray.end(); ++it)
            (*it)->flush();
        sleep(LOG_SAVE_INTERVAL); // Save logfile in a period of LOG_SAVE_INTERVAL.
    }
}

int SDLogWriter::putsLog(const char *pcString, ...)
{
    int length;
    char logData[PRINTF_BUFFER_SIZE];
    //Acquire a pointer to the argument list.
    va_list ap;
    // Parse the logfile name.
    // Initialize the list pointer.
    va_start(ap, pcString);
    // Formating the output string.
    length = vsnprintf(logData, PRINTF_BUFFER_SIZE, pcString, ap);
    length = (length < PRINTF_BUFFER_SIZE) ? length : PRINTF_BUFFER_SIZE - 1;
    va_end(ap);

    write(logData, length);

    return length;
}

int SDLogWriter::putsLogandConsole(const char *pcString, ...)
{
    int length;
    char logData[PRINTF_BUFFER_SIZE];
    //Acquire a pointer to the argument list.
    va_list ap;
    // Parse the logfile name.
    // Initialize the list pointer.
    va_start(ap, pcString);
    // Formating the output string.
    length = vsnprintf(logData, PRINTF_BUFFER_SIZE, pcString, ap);
    length = (length < PRINTF_BUFFER_SIZE) ? length : PRINTF_BUFFER_SIZE - 1;
    va_end(ap);

    write(logData, length);
    cout << logData;

    return length;
}

SDLogWriter &operator<<(SDLogWriter &logWriter, const NodeFrame2Data &frame)
{
    double timebase = getSystime();
    enum SDLog_Type usHeader = UWB_Frame2;
    unsigned int length = sizeof(enum SDLog_Type) + sizeof(unsigned int) + sizeof(double) + sizeof(NLink_LinkTrack_Node_Frame2_Part) + 13 * sizeof(float) + frame.framePart.validNodeQuantity * sizeof(Node2_t) + sizeof(uint8_t);
    unsigned int pos = 0;
    uint8_t ucChecksum = 0;
    uint8_t *buffer = new uint8_t[length];

    memcpy((void *)(buffer + pos), (void *)&usHeader, sizeof(usHeader));
    pos += sizeof(usHeader);

    memcpy((void *)(buffer + pos), (void *)&length, sizeof(length));
    pos += sizeof(length);

    memcpy((void *)(buffer + pos), (void *)&timebase, sizeof(timebase));
    pos += sizeof(timebase);

    memcpy((void *)(buffer + pos), (void *)&frame.framePart, sizeof(frame.framePart));
    pos += sizeof(frame.framePart);

    memcpy((void *)(buffer + pos), (void *)&frame.pos, sizeof(frame.pos));
    pos += sizeof(frame.pos);

    memcpy((void *)(buffer + pos), (void *)&frame.eop, sizeof(frame.eop));
    pos += sizeof(frame.eop);

    memcpy((void *)(buffer + pos), (void *)&frame.vel, sizeof(frame.vel));
    pos += sizeof(frame.vel);

    memcpy((void *)(buffer + pos), (void *)&frame.angle, sizeof(frame.angle));
    pos += sizeof(frame.angle);

    memcpy((void *)(buffer + pos), (void *)&frame.supplyVoltage, sizeof(frame.supplyVoltage));
    pos += sizeof(frame.supplyVoltage);

    for (uint8_t i = 0; i < frame.framePart.validNodeQuantity; i++)
    {
        memcpy((void *)(buffer + pos), (void *)&frame.node[frame.nodeIDList[i]], sizeof(Node2_t));
        pos += sizeof(Node2_t);
    }
    for (int i = 0; i < pos; i++)
        ucChecksum += buffer[i];

    buffer[pos++] = ucChecksum;

    assert(pos == length);

    logWriter.write((const char *)buffer, length);

    delete[] buffer;

    return logWriter;
}

SDLogWriter &operator<<(SDLogWriter &logWriter, const NodeFrame0Data &frame)
{
    double timebase = getSystime();
    enum SDLog_Type usHeader = UWB_Frame0;
    unsigned int length = sizeof(enum SDLog_Type) + sizeof(unsigned int) + sizeof(double) + sizeof(frame.framePart) + sizeof(uint8_t);

    for (uint8_t i = 0; i < frame.framePart.validNodeQuantity; i++)
        length += frame.node[frame.nodeIDList[i]]->dataLength + 2 * sizeof(uint8_t) + sizeof(uint16_t);

    unsigned int pos = 0;
    uint8_t ucChecksum = 0;
    uint8_t *buffer = new uint8_t[length];

    memcpy((void *)(buffer + pos), (void *)&usHeader, sizeof(usHeader));
    pos += sizeof(usHeader);

    memcpy((void *)(buffer + pos), (void *)&length, sizeof(length));
    pos += sizeof(length);

    memcpy((void *)(buffer + pos), (void *)&timebase, sizeof(timebase));
    pos += sizeof(timebase);

    memcpy((void *)(buffer + pos), (void *)&frame.framePart, sizeof(frame.framePart));
    pos += sizeof(frame.framePart);

    for (uint8_t i = 0; i < frame.framePart.validNodeQuantity; i++)
    {
        uint16_t dataLength = frame.node[frame.nodeIDList[i]]->dataLength;
        buffer[pos++] = frame.node[frame.nodeIDList[i]]->role;
        buffer[pos++] = frame.node[frame.nodeIDList[i]]->id;
        memcpy((void *)(buffer + pos), (void *)&dataLength, sizeof(uint16_t));
        pos += sizeof(uint16_t);
        memcpy((void *)(buffer + pos), (void *)&frame.node[frame.nodeIDList[i]]->data, dataLength);
        pos += dataLength;
    }
    for (int i = 0; i < pos; i++)
        ucChecksum += buffer[i];

    buffer[pos++] = ucChecksum;

    assert(pos == length);

    logWriter.write((const char *)buffer, length);

    delete[] buffer;

    return logWriter;
}

SDLogWriter &operator<<(SDLogWriter &logWriter, const nmea_msg &msg)
{
    GPS_Raw_SDLog tempLog;

    tempLog.timebase_rp3 = getSystime();
    tempLog.usHeader = GPS_Raw;
    tempLog.length = sizeof(tempLog) - offsetof(GPS_Raw_SDLog, timebase_rp3) + sizeof(tempLog.usHeader) + sizeof(tempLog.length);
    tempLog.msg = msg;

    uint8_t *p = (uint8_t *)&tempLog.usHeader;
    for (int i = 0; i < sizeof(tempLog.usHeader); i++)
        tempLog.ucChecksum += *(p + i);
    logWriter.write((const char *)p, sizeof(tempLog.usHeader));

    p = (uint8_t *)&tempLog.length;
    for (int i = 0; i < sizeof(tempLog.length); i++)
        tempLog.ucChecksum += *(p + i);
    logWriter.write((const char *)p, sizeof(tempLog.length));

    p = (uint8_t *)&tempLog.timebase_rp3;
    for (int i = 0; i < (offsetof(GPS_Raw_SDLog, ucChecksum) - offsetof(GPS_Raw_SDLog, timebase_rp3)); i++)
        tempLog.ucChecksum += *(p + i);
    logWriter.write((const char *)p, offsetof(GPS_Raw_SDLog, ucChecksum) - offsetof(GPS_Raw_SDLog, timebase_rp3));
    logWriter.put(tempLog.ucChecksum);

    return logWriter;
}

bool SDLogReader::ReadLog(const char *filename)
{
    ifstream logfile(filename, ifstream::binary | ifstream::in);

    if (!logfile.is_open())
    {
        cout << "Failed to open " << filename << " while reading log.\n";
        return false;
    }
    else
    {
        // get length of file:
        logfile.seekg(0, logfile.end);
        int length = logfile.tellg();
        logfile.seekg(0, logfile.beg);

        char *buffer = new char[length];

        // read data as a block:
        logfile.read(buffer, length);
        logfile.close();

        int head = 0, tail = 0;
        UWB_Frame0_SDLog UWB_Frame0_temp;
        UWB_Frame2_SDLog UWB_Frame2_temp;
        GPS_Raw_SDLog GPS_Raw_temp;
        enum SDLog_Type usHeader;
        unsigned int loglength;
        uint8_t ucChecksum;

        while (head < (length - sizeof(enum SDLog_Type) - sizeof(unsigned int)))
        {
            memcpy((void *)&usHeader, buffer + head, sizeof(usHeader));
            tail = head + sizeof(usHeader);
            switch (usHeader)
            {
            case UWB_Frame0:
            case UWB_Frame2:
            case GPS_Raw:
                memcpy((void *)&loglength, buffer + tail, sizeof(loglength));
                tail += sizeof(loglength);
                // See if there is enough data indicate by loglength in the buffer.
                if ((head + loglength) <= length)
                {
                    // See if the checksum is correct.
                    for (int i = 0; i < (loglength - 1); i++)
                        ucChecksum += buffer[head + i];
                    if (ucChecksum == buffer[head + loglength - 1])
                    {
                        switch (usHeader)
                        {
                        case UWB_Frame0:
                            UWB_Frame0_temp.usHeader = usHeader;
                            UWB_Frame0_temp.length = loglength;
                            memcpy((void *)&UWB_Frame0_temp.timebase_rp3, buffer + tail, sizeof(UWB_Frame0_temp.timebase_rp3));
                            tail += sizeof(UWB_Frame0_temp.timebase_rp3);
                            memcpy((void *)&UWB_Frame0_temp.framePart, buffer + tail, sizeof(UWB_Frame0_temp.framePart));
                            tail += sizeof(UWB_Frame0_temp.framePart);
                            for (uint8_t i = 0; i < UWB_Frame0_temp.framePart.validNodeQuantity; i++)
                            {
                                UWB_Node0_t tempNode0;
                                uint16_t dataLength;

                                tempNode0.role = buffer[tail++];
                                tempNode0.id = buffer[tail++];
                                memcpy((void *)&dataLength, buffer + tail, sizeof(uint16_t));
                                tail += sizeof(uint16_t);
                                tempNode0.data.assign(buffer + tail, buffer + tail + dataLength);
                                tail += dataLength;
                                assert(tempNode0.data.size() == dataLength);
                                UWB_Frame0_temp.nodes.push_back(tempNode0);
                            }
                            UWB_Frame0_temp.ucChecksum = buffer[tail++];
                            assert(tail == (head + loglength));
                            head = tail;
                            break;
                        case UWB_Frame2:
                            UWB_Frame2_temp.usHeader = usHeader;
                            UWB_Frame2_temp.length = loglength;
                            memcpy((void *)&UWB_Frame2_temp.timebase_rp3, buffer + tail, sizeof(UWB_Frame2_temp.timebase_rp3));
                            tail += sizeof(UWB_Frame2_temp.timebase_rp3);
                            memcpy((void *)&UWB_Frame2_temp.framePart, buffer + tail, sizeof(UWB_Frame2_temp.framePart));
                            tail += sizeof(UWB_Frame2_temp.framePart);
                            memcpy((void *)&UWB_Frame2_temp.pos, buffer + tail, sizeof(UWB_Frame2_temp.pos));
                            tail += sizeof(UWB_Frame2_temp.pos);
                            memcpy((void *)&UWB_Frame2_temp.eop, buffer + tail, sizeof(UWB_Frame2_temp.eop));
                            tail += sizeof(UWB_Frame2_temp.eop);
                            memcpy((void *)&UWB_Frame2_temp.vel, buffer + tail, sizeof(UWB_Frame2_temp.vel));
                            tail += sizeof(UWB_Frame2_temp.vel);
                            memcpy((void *)&UWB_Frame2_temp.angle, buffer + tail, sizeof(UWB_Frame2_temp.angle));
                            tail += sizeof(UWB_Frame2_temp.angle);
                            memcpy((void *)&UWB_Frame2_temp.supplyVoltage, buffer + tail, sizeof(UWB_Frame2_temp.supplyVoltage));
                            tail += sizeof(UWB_Frame2_temp.supplyVoltage);

                            for (uint8_t i = 0; i < UWB_Frame2_temp.framePart.validNodeQuantity; i++)
                            {
                                Node2_t tempNode2;
                                uint16_t dataLength;

                                memcpy((void *)&tempNode2, buffer + tail, sizeof(tempNode2));
                                tail += sizeof(tempNode2);

                                UWB_Frame2_temp.nodes.push_back(tempNode2);
                            }
                            UWB_Frame2_temp.ucChecksum = buffer[tail++];
                            assert(tail == (head + loglength));
                            head = tail;
                            break;
                        case GPS_Raw:
                            GPS_Raw_temp.usHeader = usHeader;
                            GPS_Raw_temp.length = loglength;
                            memcpy((void *)&GPS_Raw_temp.timebase_rp3, buffer + tail, offsetof(GPS_Raw_SDLog, ucChecksum) - offsetof(GPS_Raw_SDLog, timebase_rp3));
                            tail += offsetof(GPS_Raw_SDLog, ucChecksum) - offsetof(GPS_Raw_SDLog, timebase_rp3);
                            GPS_Raw_temp.ucChecksum = buffer[tail++];
                            assert(tail == (head + loglength));
                            head = tail;
                            break;
                        }
                    }
                    else // Checksum failed, skip this character.
                        head++;
                }
                else // Incomplete log, skip this character.
                    head++;

                break;
            default:
                // No match for any SDlog header, skip this character.
                head++;
                break;
            }
        }

        delete[] buffer;

        return true;
    }
}
