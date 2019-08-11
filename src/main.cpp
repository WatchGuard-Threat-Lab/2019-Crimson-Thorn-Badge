#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_IS31FL3731.h>
#include <TimeLib.h>
#include <Preferences.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#include <FreezeWatch.h>
#include <BadgeMessage.h>
#include <Botnet.h>
#include <MenuSystem.h>

const int BLUE = 0;
const int GREEN = 1;
const int RED = 2;

const int SW1 = 33;
const int SW2 = 32;
const int SWM = 25;
const int SWA = 12;
const int SWB = 14;
const int SWC = 27;
const int SWD = 26;

Adafruit_IS31FL3731 ledmatrix = Adafruit_IS31FL3731();

unsigned int loopCtr = 0;

int upButton = 0;
int downButton = 0;
int leftButton = 0;
int rightButton = 0;
int acceptButton = 0;
int backButton = 0;
int centerButton = 0;

unsigned int badgeID = 0;
unsigned int botState = 0;
unsigned int botLvl = 1;
unsigned int faction = 1;
unsigned int numAS = 0; //number attack success
unsigned int numAF = 0; //number attack fail
unsigned int numDS = 0;
unsigned int numDF = 0;
int eyeColor = RED;

typedef struct {
  unsigned int badgeID;
  unsigned int time;
} Attacked_Badge;

Attacked_Badge attacked_badges[550];

int numAttackedBadges = 0;

int attackwait = 0;

short int ledArr[16][9];

TaskHandle_t * watcher;

void writeLEDs() {
  for (int x = 0; x < 16; x++) {
    for (int y = 0; y < 9; y++) {
      ledmatrix.drawPixel(x, y, ledArr[x][y]);
    }
  }
}

void clearLEDs() {
  for (int x = 0; x < 16; x++) {
    for (int y = 0; y < 9; y++) {
      ledArr[x][y] = 0;
    }
  }
}

void showEyes() {
  if (faction == 1) {
    ledArr[0][RED] = 128;
    ledArr[1][RED] = 128;
  }
  else if (faction == 2) {
    ledArr[0][BLUE] = 128;
    ledArr[1][BLUE] = 128;
  }
  else if (faction == 3) {
    ledArr[0][GREEN] = 128;
    ledArr[1][GREEN] = 128;
  }
}

void showBacklights() {
  for (int y = 0; y < 7; y++) {
    ledArr[8][y] = 255;
  }
}

void tracingLEDS() {
  for (int x = 8; x < 16; x++) {
    ledArr[x][loopCtr % 9] = 255;
    ledArr[x][(loopCtr + 2) % 9] = 255;
    ledArr[x][(loopCtr + 4) % 9] = 255;
    ledArr[x][(loopCtr + 6) % 9] = 255;
  }
}

void blingMode() {
  for (int x = 8; x < 16; x++) {
    ledArr[x][loopCtr % 9] = 255;
    ledArr[x][(loopCtr+1) % 9] = 175;
    ledArr[x][(loopCtr+2) % 9] = 75;
  }
}

void adminBling() {
  for (int x = 8; x < 16; x++) {
    ledArr[x][loopCtr % 8] = 255;
    ledArr[x][8-(loopCtr % 8)] = 255;
  }
}

void fullLEDS() {
  for (int x = 8; x < 16; x++) {
    for (int y = 0; y < 9; y++) {
      ledArr[x][y] = 255;
    }
  }
}

void setFaction (unsigned int f) {
  faction = f;
  Botnet::setFaction(f);
}

unsigned int getLoopCtr() {
  return loopCtr;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Initialization Starting...");
  Serial.println("Starting up the FreezeWatcher");
  nofreezeplz.setLoopCtr(&loopCtr);
  nofreezeplz.setTaskHandle(watcher);
  xTaskCreatePinnedToCore(nofreezeplz.start,"Watcher",8*1024,(void*)(&nofreezeplz),1,watcher,0);

  randomSeed(analogRead(16));

  // Setup Buttons
  pinMode(SW1, INPUT);
  pinMode(SW2, INPUT);
  pinMode(SWM, INPUT);
  pinMode(SWA, INPUT);
  pinMode(SWB, INPUT);
  pinMode(SWC, INPUT);
  pinMode(SWD, INPUT);

  Botnet::initBotnet();

  Serial.print("Badge ID: 0x");
  badgeID = Botnet::getBadgeID();
  Serial.println(badgeID, HEX);
  Serial.print("Badge Level: ");
  botLvl = Botnet::getBadgeLevel();
  Serial.println(botLvl);
  Serial.print("Faction: ");
  faction = Botnet::getFaction();
  Serial.println(faction);
  Serial.print("Attack Count (success|fail): ");
  numAS = Botnet::getNumAttacksSucceed();
  Serial.print(numAS);
  Serial.print(" | ");
  numAF = Botnet::getNumAttacksFailed();
  Serial.println(numAF);
  Serial.print("Defense Count (success|fail): ");
  numDS = Botnet::getNumDefenseSucceed();
  numDF = Botnet::getNumDefenseFailed();
  Serial.print(numDS);
  Serial.print(" | ");
  Serial.println(numDF);
  Serial.println("Initializing LED Controller...");
  nofreezeplz.checkIn();
  if (!ledmatrix.begin()) {
    Serial.println("LED Controller Not Found");
    while (1);
  }
  nofreezeplz.checkIn();
  Serial.println("LED Controller Initialized");
  nofreezeplz.checkIn();
  MenuSystem::initMenuSystem(
    Botnet::getBadgeID(),
    Botnet::getBadgeLevel(),
    Botnet::getBadgeXP(),
    Botnet::getXPPercentage(),
    Botnet::getFaction(),
    Botnet::getStandbyBling(),
    Botnet::startAttack,
    Botnet::stopAttack,
    Botnet::resetBadge,
    setFaction,
    Botnet::toggleAttackAugment,
    Botnet::toggleDefenseAugment,
    Botnet::toggleStandbyBling,
    Botnet::ackAttackorUpdate,
    Botnet::getAttackAugments,
    Botnet::getDefenseAugments,
    Botnet::getNumAugments,
    Botnet::getBadgeLevel,
    Botnet::getBadgeXP,
    Botnet::checkCode,
    Botnet::isFaction1Unlocked,
    Botnet::isFaction2Unlocked,
    getLoopCtr);



    nofreezeplz.checkIn();
    Serial.println("Badge Ready, Have Fun :)");
}

void loop() {
  nofreezeplz.checkIn();
  clearLEDs();
  showEyes();
  showBacklights();


  upButton = digitalRead(SWD);
  downButton = digitalRead(SWA);
  leftButton = digitalRead(SWC);
  rightButton = digitalRead(SWB);
  acceptButton = digitalRead(SW1);
  backButton = digitalRead(SW2);
  centerButton = digitalRead(SWM);

  if (Botnet::isSearching()) {
    tracingLEDS();
    if (MenuSystem::getBotState() != 1) {
      MenuSystem::setBotState(1);
    }
  }else if (Botnet::readyConnect() || Botnet::isConnected()) {
    fullLEDS();
    if (Botnet::readyConnect() && MenuSystem::getBotState() != 2 ){
      MenuSystem::setBotState(2);
    }
    else if (Botnet::isConnected() && MenuSystem::getBotState() != 3) {
      MenuSystem::setBotState(3);
    }
  } else {
    if (MenuSystem::isOnMainMenu()) {
      if (Botnet::isUnderattack() && MenuSystem::getBotState() != 4) {
        fullLEDS();
        writeLEDs();
        MenuSystem::setBotState(4);
        delay(1000);
        Botnet::ackAttackorUpdate();
      }
      else if (Botnet::isBeingUpdated() && MenuSystem::getBotState() != 11) {
        fullLEDS();
        writeLEDs();
        MenuSystem::setBotState(11);
        delay(2000);
        Botnet::ackAttackorUpdate();
      } else {
        if (MenuSystem::getBotState() != 0) {
          fullLEDS();
          writeLEDs();
          if (Botnet::getNumDefenseSucceed() > numDS) {
            numDS += 1;
            MenuSystem::setBotState(7);
            delay(2000);
          } else if (Botnet::getNumDefenseFailed() > numDF) {
            numDF += 1;
            MenuSystem::setBotState(8);
            delay(2000);
          }
          MenuSystem::setBotState(0);
        }
      }

    }
    else if (MenuSystem::isOnBotMenu()) {
      if (MenuSystem::getBotState() != 0) {
        MenuSystem::setBadgeXP(Botnet::getBadgeXP());
        MenuSystem::setBadgeLevel(Botnet::getBadgeLevel());
        MenuSystem::setBadgeXPProgress(Botnet::getXPPercentage());
        if (Botnet::getNumAttacksSucceed() > numAS) {
          numAS += 1;
          MenuSystem::setBotState(5);
          delay(2000);
          if (Botnet::getBadgeLevel() > botLvl) {
            botLvl = Botnet::getBadgeLevel();
            MenuSystem::setBotState(9);
            delay(2000);
          }
        } else if (Botnet::getNumAttacksFailed() > numAF) {
          numAF += 1;
          MenuSystem::setBotState(6);
          delay(2000);
          if (Botnet::getBadgeLevel() > botLvl) {
            botLvl = Botnet::getBadgeLevel();
            MenuSystem::setBotState(9);
            delay(2000);
          }
        }
        MenuSystem::setBotState(0);
        Botnet::startBLEServer();
      }
    }
    if (Botnet::getStandbyBling() && botLvl >= 100) {
      adminBling();
    }
    if (loopCtr % 600  >= 1 && loopCtr %600 < 60) {
      if (Botnet::getStandbyBling()) {
        blingMode();
      }
    } 
  }

  if (MenuSystem::isOnWiFiScan()) {
    MenuSystem::printWiFiScan(loopCtr);
  }

  if (loopCtr%10 == 0){
    if(Botnet::readyConnect()) {
      if(Botnet::connectToBadge()) { //CHANGE
        Serial.println("Successfully Connected To Other Badge");
      } else {
        Serial.println("Failed To Connect To Other Badge");
      }
      Botnet::setReadyConnect(false);
    }

    if (Botnet::isConnected()) {
      Serial.println("Currently Connected To Other Badge");
      if(Botnet::getRemoteID() && !Botnet::isAttacking()) {
        Serial.print("Remote ID: ");
        unsigned int remoteID = Botnet::getRemoteID();
        Serial.println(remoteID,HEX); 
        Serial.print("Remote Level: ");
        Serial.println(Botnet::getRemoteLevel());
        Serial.print("Remote Strength: ");
        Serial.println(Botnet::getRemoteStrength());
        bool inArray = false;
        int arrayIndex;
        unsigned int lastAttacked = 0;
        if (numAttackedBadges > 0) {
          for (int i = 0; i < numAttackedBadges; i++) {
            if(remoteID == attacked_badges[i].badgeID) {
              inArray = true;
              arrayIndex = i;
              lastAttacked = attacked_badges[i].time;
            }
          }
        }
        if (inArray && (millis() - lastAttacked) < 300000) { // TODO: CHANGE TO 3000
          // Badge in already attacked badge list and already attacked in the last 10 minutes
          Serial.println("Already Attacked Badge Too Recently");
          Botnet::stopAttack();
          MenuSystem::setBotState(12);
          delay(2000);
        }
        else if (inArray) {
          // Badge is in attacked badge list but was attacked more than 10 minutes ago
          Botnet::attackBadge();
          unsigned int t = millis();
          attacked_badges[arrayIndex] = {remoteID,t};
        } else {
          // Badge is not in attacked badge list
          Botnet::attackBadge();
          unsigned int t = millis();
          attacked_badges[numAttackedBadges] = {remoteID,t};
          numAttackedBadges += 1;
        }
      } else if (Botnet::getRemoteID()) {
        Serial.println("Attacking...");
        if (attackwait > 5) {
          Serial.println("No Reply Received, Assuming Attack Failed");
          Botnet::handleNoNotify();
          attackwait = 0;
        }
        attackwait++;
      } else {
        Botnet::getRemoteInfo();
      }
    }
  }

  if (upButton) {
    MenuSystem::moveUp();
  }
  else if (downButton) {
    MenuSystem::moveDown();
  } 
  else if (leftButton) {
    MenuSystem::moveLeft();
  } 
  else if (rightButton) {
    MenuSystem::moveRight();
  }
  else if (acceptButton) {
    MenuSystem::accept();
  } 
  else if (backButton) {
    MenuSystem::back();
  }
  else if (centerButton) {
  }
  
  loopCtr++;
  writeLEDs();
  delay(50);//100
}
