#include "systime.h"

struct timespec timebase_start, timebase_finish;

void initSystime()
{
    clock_gettime(CLOCK_REALTIME, &timebase_start);
}

double getSystime()
{
    clock_gettime(CLOCK_REALTIME, &timebase_finish);
    return (timebase_finish.tv_sec - timebase_start.tv_sec) + (timebase_finish.tv_nsec - timebase_start.tv_nsec) / 1000000000.0;
}