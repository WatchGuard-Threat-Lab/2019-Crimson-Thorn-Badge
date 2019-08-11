#include "Botnet.h"

#include <Preferences.h>
#include <TimeLib.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEValue.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#include "BadgeMessage.h"

#define SERVICE_UUID    "9edf2ca5-470d-4266-a867-a2edf5b750ef"
#define MSG_CHAR_UUID   "5b56f9b7-df9e-4d23-b989-29832a39884c"
#define BADGE_INFO_UUID "7329c8b9-d332-484a-94b8-a524e9e66f78"

#define SECRET_KEY_1 = (0x55)
#define SECRET_KEY_2 = (0xF3)
#define SECRET_KEY_3 = (0xA2)

Preferences preferences;

uint8_t SECRET_KEY[3] = {0x55, 0xF3, 0xA2};

const int ATTACK_SUCCESS_XP = 10;
const int ATTACK_FAIL_XP = 1;
const int DEFENSE_SUCCESS_XP = 2;

static BLEUUID serviceUUID(SERVICE_UUID);
static BLEUUID msgUUID(MSG_CHAR_UUID);
static BLEUUID badgeInfoUUID(BADGE_INFO_UUID);

const unsigned int XP_REQUIREMENTS[100] = {
    0, 100, 300, 600, 1000, 1500, 2100, 2800, 3600, 4500,
    5500, 6600, 7800, 9100, 10500, 12000, 13600, 15300, 17100, 19000,
    21000, 23100, 25300, 27600, 30000, 32500, 35100, 37800, 40600, 43500,
    46500, 49600, 52800, 56100, 59500, 63000, 66600, 70300, 74100, 78000,
    82000, 86100, 90300, 94600, 99000, 103500, 108100, 112800, 117600, 122500,
    127500,132600,137800,143100,148500,154000,159600,165300,171100,177000,
    183000,189100,195300,201600,208000,214500,221100, 227800, 234600, 241500, 
    248500, 255600, 262800, 270100, 277500, 285000, 292600, 300300, 308100, 316000, 
    324000, 332100, 340300, 348600, 357000, 365500, 374100, 382800, 391600, 400500, 
    409500, 418600, 427800, 437100, 446500, 456000, 465600, 475300, 485100, 495000};

unsigned int Botnet::badgeID;
unsigned int Botnet::badgeLevel;
unsigned int Botnet::badgeXP;
unsigned int Botnet::faction;
unsigned int Botnet::strengthMultiplier;
unsigned int Botnet::numAugmentsEnabled;
unsigned int Botnet::attackAugments;
unsigned int Botnet::defenseAugments;
unsigned int Botnet::numAttacksFailed;
unsigned int Botnet::numAttacksSucceed;
unsigned int Botnet::numDefenseFailed;
unsigned int Botnet::numDefenseSucceed;
unsigned int Botnet::lastStrengthUpdate;

bool Botnet::unlockedFaction1;
bool Botnet::unlockedFaction2;
bool Botnet::code1Redeemed;
bool Botnet::code2Redeemed;
bool Botnet::code3Redeemed;
bool Botnet::code4Redeemed;
bool Botnet::code5Redeemed;
bool Botnet::standbyBling;

unsigned int Botnet::remoteID;
unsigned int Botnet::remoteLVL;
unsigned int Botnet::remoteSTR;

BLEServer *Botnet::pServer;
BLEAdvertisedDevice *Botnet::remoteBadge;
BLEScan *Botnet::pBLEScan;
BLEClient *Botnet::pClient;
BLERemoteCharacteristic *Botnet::pRemoteMSGCharacteristic;
BLERemoteCharacteristic *Botnet::pRemoteINFOCharacteristic;
BLEService *Botnet::pService;

boolean Botnet::doConnect;
boolean Botnet::connected;
boolean Botnet::searching;
boolean Botnet::attacking;
boolean Botnet::beingAttacked;
boolean Botnet::beingUpdated;

void (*resetFunc)(void) = 0;

void Botnet::resetBadge() {
    preferences.clear();
    resetFunc();
}

void Botnet::initBotnet() {
    doConnect = false;
    connected = false;
    searching = false;
    attacking = false;
    beingAttacked = false;
    beingUpdated = false;
    initPreferences();
    initBLEServer();
}

void Botnet::initPreferences() {
    preferences.begin("crimsonthorn", false);

    // Uncomment to clear preferences
    //preferences.clear();

    // Get badge info from preferences, initialize if not set

    Botnet::badgeID = preferences.getUInt("badge_id", random(LONG_MAX));

    Botnet::badgeLevel = preferences.getUInt("badge_level", 1);
    Botnet::badgeXP = preferences.getUInt("badge_xp", 0);
    Botnet::faction = preferences.getUInt("faction", 1);
    Botnet::strengthMultiplier = preferences.getUInt("badge_multi", 1);
    Botnet::numAugmentsEnabled = preferences.getUInt("num_aug", 0);
    Botnet::attackAugments = preferences.getUInt("attack_aug", 0);
    Botnet::defenseAugments = preferences.getUInt("defense_aug", 0);
    Botnet::numAttacksFailed = preferences.getUInt("num_att_fail", 0);
    Botnet::numAttacksSucceed = preferences.getUInt("num_att_suc", 0);
    Botnet::numDefenseFailed = preferences.getUInt("num_def_fail", 0);
    Botnet::numDefenseSucceed = preferences.getUInt("num_def_suc", 0);
    Botnet::lastStrengthUpdate = preferences.getLong("last_str_up", 0);
    Botnet::unlockedFaction1 = preferences.getBool("unlock_f_1", false);
    Botnet::unlockedFaction2 = preferences.getBool("unlock_f_2", false);
    Botnet::code1Redeemed = preferences.getBool("code_1_r",false);
    Botnet::code2Redeemed = preferences.getBool("code_2_r",false);
    Botnet::code3Redeemed = preferences.getBool("code_3_r",false);
    Botnet::code4Redeemed = preferences.getBool("code_4_r",false);
    Botnet::code5Redeemed = preferences.getBool("code_5_r",false);
    Botnet::standbyBling = preferences.getBool("standbyBling",true);

    // Save values back
    preferences.putUInt("badge_id", Botnet::badgeID);
    preferences.putUInt("badge_level", Botnet::badgeLevel);
    preferences.putUInt("badge_xp", Botnet::badgeXP);
    preferences.putUInt("faction", Botnet::faction);
    preferences.putUInt("badge_multi", Botnet::strengthMultiplier);
    preferences.putUInt("num_aug", Botnet::numAugmentsEnabled);
    preferences.putUInt("attack_aug", Botnet::attackAugments);
    preferences.putUInt("defense_aug", Botnet::defenseAugments);
    preferences.putUInt("num_att_fail", Botnet::numAttacksFailed);
    preferences.putUInt("num_att_suc", Botnet::numAttacksSucceed);
    preferences.putUInt("num_def_fail", Botnet::numDefenseFailed);
    preferences.putUInt("num_def_suc", Botnet::numDefenseSucceed);
    preferences.putLong("last_str_up", Botnet::lastStrengthUpdate);
    preferences.putBool("unlock_f_1", Botnet::unlockedFaction1);
    preferences.putBool("unlock_f_2", Botnet::unlockedFaction2);
    preferences.putBool("code_1_r", Botnet::code1Redeemed);
    preferences.putBool("code_2_r", Botnet::code2Redeemed);
    preferences.putBool("code_3_r", Botnet::code3Redeemed);
    preferences.putBool("code_4_r", Botnet::code4Redeemed);
    preferences.putBool("code_5_r", Botnet::code5Redeemed);
    preferences.putBool("standbyBling", Botnet::standbyBling);
}

void Botnet::initBLEServer() {
    BLEDevice::init(String(badgeID, HEX).c_str());
    pServer = BLEDevice::createServer();

    pService = pServer->createService(SERVICE_UUID);

    BLECharacteristic *pMSGCharacteristic = pService->createCharacteristic(
        MSG_CHAR_UUID,
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY);

    BLECharacteristic *pINFOCharacteristic = pService->createCharacteristic(
        BADGE_INFO_UUID,
        BLECharacteristic::PROPERTY_READ);

    // Load badge info characteristic
    uint8_t badgeInfo[12];
    badgeInfo[0] = (int)((badgeID >> 24) & 0xFF);
    badgeInfo[1] = (int)((badgeID >> 16) & 0xFF);
    badgeInfo[2] = (int)((badgeID >> 8) & 0xFF);
    badgeInfo[3] = (int)((badgeID & 0xFF));
    badgeInfo[4] = (int)((badgeLevel >> 24) & 0xFF);
    badgeInfo[5] = (int)((badgeLevel >> 16) & 0xFF);
    badgeInfo[6] = (int)((badgeLevel >> 8) & 0xFF);
    badgeInfo[7] = (int)((badgeLevel & 0xFF));
    badgeInfo[8] = (int)((strengthMultiplier >> 24) & 0xFF);
    badgeInfo[9] = (int)((strengthMultiplier >> 16) & 0xFF);
    badgeInfo[10] = (int)((strengthMultiplier >> 8) & 0xFF);
    badgeInfo[11] = (int)((strengthMultiplier & 0xFF));

    pINFOCharacteristic->setValue(badgeInfo, 12);
    pMSGCharacteristic->setCallbacks(new MessageCallback());

    pService->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    BLEDevice::startAdvertising();
};

bool Botnet::isSearching() { return searching; }

bool Botnet::readyConnect() { return doConnect; }

bool Botnet::isConnected() { return connected; }

bool Botnet::isUnderattack() { return beingAttacked; }

bool Botnet::isBeingUpdated() { return beingUpdated; }

bool Botnet::isAttacking() { return attacking; }

bool Botnet::isFaction1Unlocked() { return unlockedFaction1; }

bool Botnet::isFaction2Unlocked() { return unlockedFaction2; }

void Botnet::unlockFaction1() {
    unlockedFaction1 = true;
    preferences.putBool("unlock_f_1", true);
}

void Botnet::unlockFaction2() {
    unlockedFaction2 = true;
    preferences.putBool("unlock_f_2", true);
}

unsigned int Botnet::getBadgeID() { return badgeID; }

unsigned int Botnet::getBadgeLevel() { return badgeLevel; }

unsigned int Botnet::getBadgeXP() { return badgeXP; }

unsigned int Botnet::getFaction() { return faction; }

bool Botnet::getStandbyBling() { return standbyBling; }

unsigned int Botnet::getXPPercentage() {
    // Get grogress towards next level
    unsigned int newXP = badgeXP - XP_REQUIREMENTS[badgeLevel - 1];
    unsigned int requiredXP = XP_REQUIREMENTS[badgeLevel] - XP_REQUIREMENTS[badgeLevel - 1];
    float ans = ((float)newXP / (float)requiredXP) * 100;
    return static_cast<unsigned int>(ans);
}

unsigned int Botnet::getNumAttacksSucceed() { return numAttacksSucceed; }

unsigned int Botnet::getNumAttacksFailed() { return numAttacksFailed; }

unsigned int Botnet::getNumDefenseSucceed() { return numDefenseSucceed; }

unsigned int Botnet::getNumDefenseFailed() { return numDefenseFailed; }

unsigned int Botnet::getRemoteID() { return remoteID; }

unsigned int Botnet::getRemoteLevel() { return remoteLVL; }

unsigned int Botnet::getRemoteStrength() { return remoteSTR; }

unsigned int Botnet::getAttackAugments() { return attackAugments; }

unsigned int Botnet::getDefenseAugments() { return defenseAugments; }

unsigned int Botnet::getNumAugments() { return numAugmentsEnabled; }


void Botnet::setSearching(bool value) {
    searching = value;
}

void Botnet::setConnected(bool value) {
    connected = value;
}

void Botnet::setAttacking(bool value) {
    attacking = value;
}

void Botnet::setRemoteBadge(BLEAdvertisedDevice *rBadge) {
    remoteBadge = rBadge;
}

void Botnet::setReadyConnect(bool value) {
    doConnect = value;
}

void Botnet::setFaction(unsigned int f) {
    faction = f;
    preferences.putUInt("faction", Botnet::faction);
}


bool Botnet::toggleAttackAugment(int augNum) {
    Serial.print("Toggling Augment: ");
    Serial.println(augNum);
    // Check current toggle status of augment
    if ((attackAugments >> augNum) & 1U) {
        // Augment currently enabled, disable and decrement augment count
        attackAugments ^= 1UL << augNum;
        numAugmentsEnabled -= 1;
    } else {
        // Augment currentl disabled, check if we can enable and enable int
        if (numAugmentsEnabled < badgeLevel / 5) {
            // Can enable another augment
            attackAugments ^= 1UL << augNum;
            numAugmentsEnabled += 1;
        } else {
            // Already at max augments
            return false;
        }
    }
    preferences.putUInt("attack_aug", Botnet::attackAugments);
    preferences.putUInt("num_aug", Botnet::numAugmentsEnabled);
    return true;
}

bool Botnet::toggleDefenseAugment(int augNum) {
    // Check current toggle status of augment
    if ((defenseAugments >> augNum) & 1U) {
        // Augment currently enabled, disable and decrement augment count
        defenseAugments ^= 1UL << augNum;
        numAugmentsEnabled -= 1;
    } else {
        // Augment currentl disabled, check if we can enable and enable int
        if (numAugmentsEnabled < badgeLevel / 5) {
            // Can enable another augment
            defenseAugments ^= 1UL << augNum;
            numAugmentsEnabled += 1;
        } else {
            // Already at max augments
            return false;
        }
    }
    preferences.putUInt("defense_aug", Botnet::defenseAugments);
    preferences.putUInt("num_aug", Botnet::numAugmentsEnabled);
    return true;
}

void Botnet::toggleStandbyBling(bool enabled) {
    standbyBling = enabled;
    preferences.putBool("standbyBling", standbyBling);
}

void Botnet::giveXP(unsigned int newXP) {
    boolean levelUp = false;
    boolean newAugment = false;
    // Give XP
    badgeXP += newXP;
    preferences.putUInt("badge_xp", badgeXP);
    // Check if enough XP to level up
    if (badgeLevel < 100) {
        while (true) {
            // Handle gaining multiple levels in one go
            if (badgeXP >= XP_REQUIREMENTS[badgeLevel]) {
                badgeLevel += 1;
                levelUp = true;
                if (badgeLevel % 5 == 0) {
                    // New augment if new level is a multiple of 5
                    newAugment = true;
                }
                if (badgeLevel == 100) {
                    break;
                }
            } else {
                break;
            }
        }
    }
    if (levelUp) {
        // User leveled up, update BLE characteristic
        // Get service
        Serial.println("*** LEVEL UP! ***");
        Serial.print("New Level: ");
        Serial.println(badgeLevel);
        preferences.putUInt("badge_level", badgeLevel);
        if (newAugment) {
            Serial.println("*** NEW AUGMENT SLOT UNLOCKED! **");
        }
        BLEService *pService = pServer->getServiceByUUID(SERVICE_UUID);
        // Get Info Charactersitic
        BLECharacteristic *pINFOCharacteristic = pService->getCharacteristic(BADGE_INFO_UUID);
        uint8_t *badgeInfo = pINFOCharacteristic->getData();

        badgeInfo[4] = (int)((badgeLevel >> 24) & 0xFF);
        badgeInfo[5] = (int)((badgeLevel >> 16) & 0xFF);
        badgeInfo[6] = (int)((badgeLevel >> 8) & 0xFF);
        badgeInfo[7] = (int)((badgeLevel & 0xFF));
    }
}

/*
Roll a random number to decide if attack was succesful
*/
bool Botnet::rollAttack(
    unsigned int attackStrength,
    unsigned int defenseStrength) {

    // Generate random number, lower bound attack strength upper bound defense
    if (random((attackStrength * -1), (defenseStrength + 1)) <= 0) {
        return true;
    } else {
        return false;
    }
}

bool Botnet::handleAttack(
    unsigned int attackStrength,
    unsigned int clientAugments) {
    nofreezeplz.checkIn();

    // Calculate strength of defense
    unsigned int defenseStrength = badgeLevel * (10 + strengthMultiplier);
    unsigned int sAttackRolls = 0;
    unsigned int fAttackRolls = 0;

    // Create bitmask of uncountered augments
    unsigned int counteredAugments = clientAugments & defenseAugments;
    unsigned int uncounteredAugments = clientAugments ^ counteredAugments;
    unsigned int numUncountered = 0;

    // Read number of bits to calculate uncountered augments
    for (int i = 0; i < 32; i++) {
        if (bitRead(uncounteredAugments, i)) {
            numUncountered++;
        }
    }

    for (int i = 0; i <= numUncountered; i++) {
        // Roll for each uncountered attack augment, minimum one roll.
        if (rollAttack(attackStrength, defenseStrength)) {
            sAttackRolls++;
        } else {
            fAttackRolls++;
        }
    }

    // Compare succesful attack rolls vs failed attack rolls
    if (sAttackRolls >= fAttackRolls) {
        // Attack was succesful
        return true;
    } else {
        // Attack failed
        return false;
    }
}

void Botnet::handleMessage(BLECharacteristic *pCharacteristic, uint8_t *data) {
    nofreezeplz.checkIn();
    BadgeMessage Message;
    Message.loadMessage(data);
    // Validate super secret message hash
    if (Message.validateHash(SECRET_KEY)) {
        // Message is valid
        uint8_t msg_type = Message.getMessageType();
        if (msg_type == 0x03) {
            beingUpdated = true;
            //Strength Update Message
            unsigned int msgUpdateTime = Message.getUpdateTime();
            if (msgUpdateTime > lastStrengthUpdate) {
                unsigned int newStrengthMultiplier = Message.getNewStrength();
                if (newStrengthMultiplier > strengthMultiplier) {
                    Serial.println("Your Badge Is Now Stronger!");
                    Serial.print("Setting Strength To: ");
                    Serial.println(newStrengthMultiplier);
                    strengthMultiplier = newStrengthMultiplier;
                    preferences.putUInt("badge_multi", strengthMultiplier);
                    if (strengthMultiplier == 5) {
                        Serial.println("Unlock Faction 2!");
                        unlockFaction2();
                    }
                } else {
                    Serial.println("Your Strength Has Not Increased");
                    if (strengthMultiplier == 5) {
                        unlockFaction2();
                    }
                }
                lastStrengthUpdate = msgUpdateTime;
                preferences.putLong("last_str_up", lastStrengthUpdate);
            } else {
                Serial.println("Stale Strength Update Received");
            }
            // Build reply message
            uint8_t notifyData[12];
            notifyData[0] = (int)((badgeID >> 24) & 0xFF);
            notifyData[1] = (int)((badgeID >> 16) & 0xFF);
            notifyData[2] = (int)((badgeID >> 8) & 0xFF);
            notifyData[3] = (int)((badgeID & 0xFF));
            notifyData[4] = 0x01;
            notifyData[5] = (int)((badgeLevel >> 24) & 0xFF);
            notifyData[6] = (int)((badgeLevel >> 16) & 0xFF);
            notifyData[7] = (int)((badgeLevel >> 8) & 0xFF);
            notifyData[8] = (int)((badgeLevel & 0xFF));
            pCharacteristic->setValue(notifyData, 12);
            pCharacteristic->notify(true);
        }
        else if (msg_type == 0x05) {
            //Attack message!
            beingAttacked = true;
            Serial.println("Valid Attack Message, Calculating Defense");
            unsigned int attackStrength = Message.getAttackStrength();
            unsigned int clientAttackAugments = Message.getAttackAugments();
            if (handleAttack(attackStrength, clientAttackAugments)) {
                // Attack successful
                Serial.println("Attack Successful!");

                // Build reply message
                unsigned int clientDeviceID = Message.getDeviceID();
                uint8_t notifyData[12];
                notifyData[0] = (int)((clientDeviceID >> 24) & 0xFF);
                notifyData[1] = (int)((clientDeviceID >> 16) & 0xFF);
                notifyData[2] = (int)((clientDeviceID >> 8) & 0xFF);
                notifyData[3] = (int)((clientDeviceID & 0xFF));
                notifyData[4] = 0x01;
                notifyData[5] = (int)((defenseAugments >> 24) & 0xFF);
                notifyData[6] = (int)((defenseAugments >> 16) & 0xFF);
                notifyData[7] = (int)((defenseAugments >> 8) & 0xFF);
                notifyData[8] = (int)((defenseAugments & 0xFF));
                pCharacteristic->setValue(notifyData, 12);

                numDefenseFailed++;
                preferences.putUInt("num_def_fail", numDefenseFailed);
            } else {
                //Attack failed
                Serial.println("Attack Failed!");

                // Award XP for succesfully defending attack
                Botnet::giveXP(attackStrength * DEFENSE_SUCCESS_XP);

                // Build reply message
                unsigned int clientDeviceID = Message.getDeviceID();
                uint8_t notifyData[12];
                notifyData[0] = (int)((clientDeviceID >> 24) & 0xFF);
                notifyData[1] = (int)((clientDeviceID >> 16) & 0xFF);
                notifyData[2] = (int)((clientDeviceID >> 8) & 0xFF);
                notifyData[3] = (int)((clientDeviceID & 0xFF));
                notifyData[4] = 0x00;
                notifyData[5] = (int)((defenseAugments >> 24) & 0xFF);
                notifyData[6] = (int)((defenseAugments >> 16) & 0xFF);
                notifyData[7] = (int)((defenseAugments >> 8) & 0xFF);
                notifyData[8] = (int)((defenseAugments & 0xFF));
                pCharacteristic->setValue(notifyData, 12);

                numDefenseSucceed += 1;
                preferences.putUInt("num_def_suc", numDefenseSucceed);
            }
            // Send notify
            Serial.println("Sending Notify");
            pCharacteristic->notify(true);
        }
    } else {
        // Invalid message
        Serial.println("Invalid Message, Ignore");
    }
    nofreezeplz.checkIn();
}

void Botnet::ackAttackorUpdate() {
    beingAttacked = false;
    beingUpdated = false;
}

void Botnet::stopBLEServer() {
    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->stop();
    pService->stop();
}

void Botnet::startBLEServer() {
    pService->start();
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    BLEDevice::startAdvertising();
}

void Botnet::stopAttack() {
    nofreezeplz.checkIn();
    if (pClient && pClient->isConnected()) {
        pClient->disconnect();
    }
    if (pBLEScan) {
        pBLEScan->stop();
    }
    searching = false;
    attacking = false;
    connected = false;
    remoteID = 0;
    remoteLVL = 0;
    remoteSTR = 0;
}

void Botnet::handleNoNotify() {
    Serial.println("Attack Failed!");
    unsigned int experienceEarned = remoteLVL * (10 + strengthMultiplier) * ATTACK_FAIL_XP;
    Serial.print("Experience Earned: ");
    Serial.println(experienceEarned);
    Botnet::giveXP(experienceEarned);
    numAttacksFailed += 1;
    preferences.putUInt("num_att_fail", numAttacksFailed); 
    Serial.println("Stopping Attack");
    Botnet::stopAttack();
    attacking = false; 
}

void Botnet::notifyCallback(
    BLERemoteCharacteristic *pBLERemoteCharacteristic,
    uint8_t *pData,
    size_t length,
    bool isNotify) {
    nofreezeplz.checkIn();
    Serial.println("Notify Message Received");
    if (length == 12) {
        // Correct length, attempt to parse
        unsigned int notifyBadgeID;
        notifyBadgeID = ((pData[0] << 24) 
                        + (pData[1] << 16) 
                        + (pData[2] << 8) 
                        + (pData[3]));
        if (notifyBadgeID == badgeID) {
            // Notify meant for us, check if attack was success
            if (pData[4] == 0x01) {
                // Attack was succesful!
                Serial.println("Attack Successful!");
                unsigned int experienceEarned = remoteLVL * (10 + remoteSTR) * ATTACK_SUCCESS_XP;
                Serial.print("Experience Earned: ");
                Serial.println(experienceEarned);
                Botnet::giveXP(experienceEarned);
                numAttacksSucceed += 1;
                preferences.putUInt("num_att_suc", numAttacksSucceed);
            } else {
                // Attack failed!
                Serial.println("Attack Failed!");
                unsigned int experienceEarned = remoteLVL * (10 + strengthMultiplier) * ATTACK_FAIL_XP;
                Serial.print("Experience Earned: ");
                Serial.println(experienceEarned);
                Botnet::giveXP(experienceEarned);
                numAttacksFailed += 1;
                preferences.putUInt("num_att_fail", numAttacksFailed);
            }
        } else {
            Serial.println("Incorrect Notification Target");
        }
    } else {
        Serial.println("Attack Response Length Not Correct");
    }

    Serial.println("Stopping Attack");
    Botnet::stopAttack();
    attacking = false;
}

void Botnet::attackBadge() {
    nofreezeplz.checkIn();
    attacking = true;
    BLERemoteService *pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
        pClient->disconnect();
        connected = false;
        return;
    }
    pRemoteMSGCharacteristic = pRemoteService->getCharacteristic(msgUUID);
    if (pRemoteMSGCharacteristic == nullptr) {
        pClient->disconnect();
        connected = false;
        return;
    }

    pRemoteMSGCharacteristic->registerForNotify(Botnet::notifyCallback);
    BadgeMessage Message;
    unsigned int attackStrength = Message.buildAttackMessage(
        badgeID,
        badgeLevel,
        strengthMultiplier,
        attackAugments,
        SECRET_KEY);
    Serial.print("Attack Strength: ");
    Serial.println(attackStrength);
    Message.sendAttack(pRemoteMSGCharacteristic);
    nofreezeplz.checkIn();
    return;
}

void Botnet::scanComplete(BLEScanResults results) {
    searching = false;
    
};

void Botnet::getRemoteInfo() {
    uint8_t remoteINFO[12];
    BLERemoteService *pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
        pClient->disconnect();
        connected = false;
        return;
    }
    pRemoteINFOCharacteristic = pRemoteService->getCharacteristic(badgeInfoUUID);

    if (pRemoteINFOCharacteristic == nullptr) {
        pClient->disconnect();
        connected = false;
        return;
    }

    if (pRemoteINFOCharacteristic->canRead()) {
        String rINFO = pRemoteINFOCharacteristic->readValue().c_str();
        uint8_t *rDATA = pRemoteINFOCharacteristic->readRawData();

        int ctr = 0;
        while (rDATA == nullptr) {
            ctr++;
            delay(10);
            if (ctr > 200) {
                break;
            }
        }
        if (rDATA == nullptr) { //got to use protection!
            Serial.println("No Data Read, Stopping Attack");
            stopAttack();
        } else {
            rINFO.getBytes(remoteINFO, 12);
        }

        remoteID = ((rDATA[0] << 24) 
                + (rDATA[1] << 16) 
                + (rDATA[2] << 8) 
                + (rDATA[3]));

        remoteLVL = ((rDATA[4] << 24) 
                + (rDATA[5] << 16) 
                + (rDATA[6] << 8) 
                + (rDATA[7]));

        remoteSTR = ((rDATA[8] << 24) 
                + (rDATA[9] << 16) 
                + (rDATA[10] << 8) 
                + (rDATA[11]));
    }
    return;
}

void Botnet::startAttack() {
    Serial.println("Searching for other badges...");
    stopBLEServer();
    searching = true;
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new ScanCallback());
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);
    pBLEScan->start(15, scanComplete, false);
}

bool Botnet::connectToBadge() {
    Serial.print("Forming a connection to ");
    Serial.println(remoteBadge->getAddress().toString().c_str());

    pClient = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new ClientCallback());

    pClient->connect(remoteBadge);
    Serial.println(" - Connected to server");
    connected = true;
    return true;
}

bool Botnet::checkCode(String code) {
    String code1 = "STRENGTH";
    if (code == code1) {
        unlockFaction1();
        return true;
    }
    else{
        char *p;
        const char *c = code.c_str();
        unsigned long ul;
        ul = strtoul(c,&p,16);

        if ((ul ^ badgeID) == 320017171) { //13131313 in hex
            if (!code1Redeemed) {
                Serial.println("Code Not Yet Redeemed");
                code1Redeemed = true;
                preferences.putBool("code_1_r",code1Redeemed);
                unsigned int newXP = XP_REQUIREMENTS[badgeLevel] - XP_REQUIREMENTS[badgeLevel-1];
                Serial.print("Giving: ");
                Serial.print(newXP);
                Serial.println(" new XP");
                giveXP(newXP);
                return true;
            } else {
                Serial.println("Code Already Redeemed.... Cheater");
                return false;
            }
        } else if ((ul ^ badgeID) == 322376503) { //13371337 in hex
            if (!code2Redeemed) {
                Serial.println("Code Not Yet Redeemed");
                code2Redeemed = true;
                preferences.putBool("code_2_r",code2Redeemed);
                unsigned int newXP = XP_REQUIREMENTS[badgeLevel] - XP_REQUIREMENTS[badgeLevel-1];
                Serial.print("Giving: ");
                Serial.print(newXP);
                Serial.println(" new XP");
                giveXP(newXP);
                return true;
            } else {
                Serial.println("Code Already Redeemed.... Cheater");
                return false;
            }
        } else if ((ul ^ badgeID) == 3134958867) { //badbad13 in hex
            if (!code3Redeemed) {
                Serial.println("Code Not Yet Redeemed");
                code3Redeemed = true;
                preferences.putBool("code_3_r",code3Redeemed);
                unsigned int newXP = XP_REQUIREMENTS[badgeLevel] - XP_REQUIREMENTS[badgeLevel-1];
                Serial.print("Giving: ");
                Serial.print(newXP);
                Serial.println(" new XP");
                giveXP(newXP);
                return true;
            } else {
                Serial.println("Code Already Redeemed.... Cheater");
                return false;
            }
        } else if ((ul ^ badgeID) == 322376467) { //13371313 in hex
            if (!code4Redeemed) {
                Serial.println("Code Not Yet Redeemed");
                code4Redeemed = true;
                preferences.putBool("code_4_r",code4Redeemed);
                unsigned int newXP = XP_REQUIREMENTS[badgeLevel] - XP_REQUIREMENTS[badgeLevel-1];
                Serial.print("Giving: ");
                Serial.print(newXP);
                Serial.println(" new XP");
                giveXP(newXP);
                return true;
            } else {
                Serial.println("Code Already Redeemed.... Cheater");
                return false;
            }
        } else if ((ul ^ badgeID) == 2953510931) { //b00b0013 in hex
            if (!code5Redeemed) {
                Serial.println("Code Not Yet Redeemed");
                code5Redeemed = true;
                preferences.putBool("code_5_r",code5Redeemed);
                unsigned int newXP = XP_REQUIREMENTS[badgeLevel] - XP_REQUIREMENTS[badgeLevel-1];
                Serial.print("Giving: ");
                Serial.print(newXP);
                Serial.println(" new XP");
                giveXP(newXP);
                return true;
            } else {
                Serial.println("Code Already Redeemed.... Cheater");
                return false;
            }
        }
        return false;      
    } 
}

void MessageCallback::onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    size_t size = value.length();

    uint8_t *data = pCharacteristic->getData();
    for (uint8_t i = 0; i < size; ++i) {
    }

    if (size == 20) {
        Botnet::handleMessage(pCharacteristic, data);
    }
}

/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
void ScanCallback::onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
        Serial.println("Found A Badge!");
        Botnet::setSearching(false);
        BLEDevice::getScan()->stop();
        Botnet::setRemoteBadge(new BLEAdvertisedDevice(advertisedDevice));
        Botnet::setReadyConnect(true);
    } // Found Server
} // onResult

void ClientCallback::onConnect(BLEClient *pclient) {}

void ClientCallback::onDisconnect(BLEClient *pclient) {
    Botnet::setConnected(false);
    Botnet::setAttacking(false);
}