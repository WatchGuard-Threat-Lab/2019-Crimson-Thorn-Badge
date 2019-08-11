#include "BadgeMessage.h"
#include "Arduino.h"
#include "BLERemoteCharacteristic.h"

void BadgeMessage::loadMessage(uint8_t *d) {
    memcpy(message_type, d, 1);
    memcpy(device_id, d + 1, 4);
    memcpy(message_data, d + 5, 12);
    memcpy(hash, d + 17, 3);
}

uint8_t BadgeMessage::getMessageType() {
    return (message_type[0]);
}

unsigned int BadgeMessage::getDeviceID() {
    unsigned int deviceID = ((device_id[0] << 24) 
                            + (device_id[1] << 16) 
                            + (device_id[2] << 8) 
                            + (device_id[3]));
    return deviceID;
}

unsigned int BadgeMessage::getAttackStrength() {
    unsigned int attackStrength = ((message_data[0] << 24) 
                                + (message_data[1] << 16) 
                                + (message_data[2] << 8) 
                                + (message_data[3]));
    return attackStrength;
}

unsigned int BadgeMessage::getAttackAugments() {
    unsigned int attackAugments = ((message_data[4] << 24) 
                                + (message_data[5] << 16) 
                                + (message_data[6] << 8) 
                                + (message_data[7]));
    return attackAugments;
}

unsigned int BadgeMessage::getNewStrength() {
    unsigned int newStrength = ((message_data[4] << 24) 
                            + (message_data[5] << 16) 
                            + (message_data[6] << 8) 
                            + (message_data[7]));
    return newStrength;
}

unsigned int BadgeMessage::buildAttackMessage(
    unsigned int badgeID,
    unsigned int badgeLevel,
    unsigned int strengthMultiplier,
    unsigned int attackAugments,
    uint8_t *secretKey) {

    setDeviceID(badgeID);
    setMessageType(0x05);
    unsigned int attackStrength = badgeLevel * (10 + strengthMultiplier);
    message_data[0] = (int)((attackStrength >> 24) & 0xFF);
    message_data[1] = (int)((attackStrength >> 16) & 0xFF);
    message_data[2] = (int)((attackStrength >> 8) & 0xFF);
    message_data[3] = (int)((attackStrength & 0xFF));
    message_data[4] = (int)((attackAugments >> 24) & 0xFF);
    message_data[5] = (int)((attackAugments >> 16) & 0xFF);
    message_data[6] = (int)((attackAugments >> 8) & 0xFF);
    message_data[7] = (int)((attackAugments & 0xFF));
    message_data[8] = (int)((random(ULONG_MAX) >> 24) & 0xFF);
    message_data[9] = (int)((random(ULONG_MAX) >> 16) & 0xFF);
    message_data[10] = (int)((random(ULONG_MAX) >> 8) & 0xFF);
    message_data[11] = (int)((random(ULONG_MAX) & 0xFF));
    generateHash(secretKey);
    return attackStrength;
}

void BadgeMessage::sendAttack(BLERemoteCharacteristic *pRemoteCharacteristic) {
    uint8_t message[20];
    message[0] = message_type[0];
    for (int i = 0; i < 4; i++) {
        message[i + 1] = device_id[i];
    }
    for (int i = 0; i < 12; i++) {
        message[i + 5] = message_data[i];
    }
    for (int i = 0; i < 3; i++) {
        message[i + 17] = hash[i];
    }
    pRemoteCharacteristic->writeValue(message, 20, false);
}

unsigned int BadgeMessage::getUpdateTime() {
    unsigned int updateTime = message_data[0] + message_data[1] + message_data[2] + message_data[3];
    return updateTime;
}

bool BadgeMessage::validateHash(uint8_t *secretKey) {
    uint8_t temp1, temp2, temp3;
    temp1 = message_data[0] ^ message_data[3] ^ message_data[6] ^ message_data[9] ^ secretKey[0];
    temp2 = message_data[1] ^ message_data[4] ^ message_data[7] ^ message_data[10] ^ secretKey[1];
    temp3 = message_data[2] ^ message_data[5] ^ message_data[8] ^ message_data[11] ^ secretKey[2];
    if (temp1 == hash[0] && temp2 == hash[1] && temp3 == hash[2]) {
        return true;
    } else {
        return false;
    }
}

void BadgeMessage::setDeviceID(unsigned int badgeID) {
    device_id[0] = (int)((badgeID >> 24) & 0xFF);
    device_id[1] = (int)((badgeID >> 16) & 0xFF);
    device_id[2] = (int)((badgeID >> 8) & 0xFF);
    device_id[3] = (int)((badgeID & 0xFF));
}

void BadgeMessage::setMessageType(byte m_type) {
    message_type[0] = m_type;
}

void BadgeMessage::generateHash(uint8_t *secretKey) {
    /* L0L security by obscurity */
    hash[0] = message_data[0] ^ message_data[3] ^ message_data[6] ^ message_data[9] ^ secretKey[0];
    hash[1] = message_data[1] ^ message_data[4] ^ message_data[7] ^ message_data[10] ^ secretKey[1];
    hash[2] = message_data[2] ^ message_data[5] ^ message_data[8] ^ message_data[11] ^ secretKey[2];
}