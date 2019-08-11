#ifndef HEADER_Botnet
#define HEADER_Botnet
#include "stdint.h"
#include "Arduino.h"
#include <string>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEValue.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include "FreezeWatch.h"

class Botnet {
public:
    static void initBotnet();
    static void handleMessage(
        BLECharacteristic *pCharacteristic,
        uint8_t *data);
    static bool rollAttack(
        unsigned int attackStrength,
        unsigned int defenseStrength);
    static bool handleAttack(
        unsigned int attackStrength,
        unsigned int clientAugments);
    static void resetBadge();
    static void getRemoteInfo();
    static void stopBLEServer();
    static void startBLEServer();
    static void handleNoNotify();
    static void stopAttack();
    static void startAttack();
    static void attackBadge();
    static bool isSearching();
    static bool readyConnect();
    static bool isConnected();
    static bool isUnderattack();
    static bool isBeingUpdated();
    static bool isAttacking();
    static void ackAttackorUpdate();
    static bool isFaction1Unlocked();
    static bool isFaction2Unlocked();
    static void unlockFaction1();
    static void unlockFaction2();
    static void setSearching(bool value);
    static void setConnected(bool value);
    static void setAttacking(bool value);
    static void setFaction(unsigned int faction);
    static void setRemoteBadge(BLEAdvertisedDevice *rBadge);
    static void setReadyConnect(bool value);
    static void toggleStandbyBling(bool enabled);
    static bool toggleAttackAugment(int augmentNum);
    static bool toggleDefenseAugment(int augmentNum);
    static unsigned int getBadgeID();
    static unsigned int getBadgeLevel();
    static unsigned int getBadgeXP();
    static unsigned int getFaction();
    static unsigned int getXPPercentage();
    static unsigned int getNumAttacksSucceed();
    static unsigned int getNumAttacksFailed();
    static unsigned int getNumDefenseSucceed();
    static unsigned int getNumDefenseFailed();
    static unsigned int getRemoteID();
    static unsigned int getRemoteLevel();
    static unsigned int getRemoteStrength();
    static unsigned int getAttackAugments();
    static unsigned int getDefenseAugments();
    static unsigned int getNumAugments();
    static bool getStandbyBling();
    static bool checkCode(String code);
    static bool connectToBadge();

private:
    static void initPreferences();
    static void initBLEServer();
    static void scanComplete(BLEScanResults results);
    static void giveXP(unsigned int newXP);
    static void notifyCallback(
        BLERemoteCharacteristic *pBLERemoteCharacteristic,
        uint8_t *pData,
        size_t length,
        bool isNotify);
    static unsigned int badgeID;
    static unsigned int badgeLevel;
    static unsigned int badgeXP;
    static unsigned int faction;
    static unsigned int strengthMultiplier;
    static unsigned int numAugmentsEnabled;
    static unsigned int attackAugments;
    static unsigned int defenseAugments;
    static unsigned int numAttacksFailed;
    static unsigned int numAttacksSucceed;
    static unsigned int numDefenseFailed;
    static unsigned int numDefenseSucceed;
    static unsigned int lastStrengthUpdate;

    static bool unlockedFaction1;
    static bool unlockedFaction2;
    static bool code1Redeemed;
    static bool code2Redeemed;
    static bool code3Redeemed;
    static bool code4Redeemed;
    static bool code5Redeemed;
    static bool standbyBling;

    static unsigned int remoteID;
    static unsigned int remoteLVL;
    static unsigned int remoteSTR;

    static BLEServer *pServer;
    static BLEAdvertisedDevice *remoteBadge;
    static BLEScan *pBLEScan;
    static BLEClient *pClient;
    static BLERemoteCharacteristic *pRemoteMSGCharacteristic;
    static BLERemoteCharacteristic *pRemoteINFOCharacteristic;
    static BLEService *pService;

    static boolean doConnect;
    static boolean connected;
    static boolean searching;
    static boolean attacking;
    static boolean beingAttacked;
    static boolean beingUpdated;
};

class MessageCallback : public BLECharacteristicCallbacks {
public:
    void onWrite(BLECharacteristic *pCharacteristic);
};

class ScanCallback : public BLEAdvertisedDeviceCallbacks {
public:
    void onResult(BLEAdvertisedDevice advertisedDevice);
};

class ClientCallback : public BLEClientCallbacks {
public:
    void onConnect(BLEClient *pclient);
    void onDisconnect(BLEClient *pclient);
};

#endif