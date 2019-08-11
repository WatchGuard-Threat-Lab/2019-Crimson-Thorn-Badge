#pragma once
#ifndef HEADER_FreezeWatch
#define HEADER_FreezeWatch
#include <TimeLib.h>
#include <Esp.h>


class FreezeWatcher{
public:
    FreezeWatcher();
    FreezeWatcher(unsigned int *loopCtr);
    void checkIn();
    void watch();
    void setLoopCtr(unsigned int *loopCtr);
    static void start(void * self);
    void setTaskHandle(TaskHandle_t * handle);

private:
    unsigned int lastTime = 0;
    unsigned int lastLoopCtr = 0;
    unsigned int *loopCtr;
    unsigned int timeOfLastCheckup = 0;
    TaskHandle_t * watcher;
};
#ifndef NOFREEZE
#define NOFREEZE
static FreezeWatcher nofreezeplz;
#endif
#endif