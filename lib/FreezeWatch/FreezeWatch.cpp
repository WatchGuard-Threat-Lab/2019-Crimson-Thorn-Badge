
#include "FreezeWatch.h"

FreezeWatcher::FreezeWatcher()
{
    lastTime = millis();
    timeOfLastCheckup = millis();
    loopCtr = nullptr;
}

FreezeWatcher::FreezeWatcher(unsigned int *loopCtr)
{
    this->loopCtr = loopCtr;
    lastTime = millis();
    timeOfLastCheckup = millis();
}

void FreezeWatcher::checkIn()
{
    timeOfLastCheckup = millis();
}

void FreezeWatcher::watch()
{
    Serial.print("FreezeWatcher CORE ID : ");
    Serial.println(xPortGetCoreID());
    while(true){
        if (millis() - timeOfLastCheckup > 10000){
            Serial.printf("WE ARE PAUSED FOR TOO LONG \n ITS BEEN %f.2", (double)millis() - (double)timeOfLastCheckup);
            Serial.println("**RESTARTING BADGE**");
            delay(1000);
            ESP.restart();
        }
        if (*loopCtr > (UINT_MAX - 1000) && *loopCtr % 10 == 0)
            *loopCtr = 0;
        delay(10);
    }
}

void FreezeWatcher::setLoopCtr(unsigned int *loopCtr)
{
    this->loopCtr = loopCtr;
}
void FreezeWatcher::start(void *self)
{
    ((FreezeWatcher *)self)->watch();
}
void FreezeWatcher::setTaskHandle(TaskHandle_t * handle){
    watcher = handle;
}
extern FreezeWatcher nofreeze();