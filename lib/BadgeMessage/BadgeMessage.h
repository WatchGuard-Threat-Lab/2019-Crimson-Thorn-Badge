#ifndef HEADER_BadgeMessage
#define HEADER_BadgeMessage
#include "stdint.h"
#include "TimeLib.h"
#include "Arduino.h"
#include "BLERemoteCharacteristic.h"

class BadgeMessage {
public:
    void loadMessage(uint8_t *d);
    uint8_t getMessageType();
    unsigned int getDeviceID();
    unsigned int getAttackStrength();
    unsigned int getAttackAugments();
    unsigned int buildAttackMessage(
        unsigned int badgeID,
        unsigned int badgeLevel,
        unsigned int strengthMultiplier,
        unsigned int attackAugments,
        uint8_t *secretKey);
    void sendAttack(BLERemoteCharacteristic *pRemoteCharacteristic);
    unsigned int getUpdateTime();
    unsigned int getNewStrength();
    bool validateHash(uint8_t *secretKey);

private:
    uint8_t message_type[1];
    uint8_t device_id[4];
    uint8_t message_data[12];
    uint8_t hash[3];
    void setDeviceID(unsigned int badgeID);
    void setMessageType(byte m_type);
    void generateHash(uint8_t *secretKey);
};

#endif