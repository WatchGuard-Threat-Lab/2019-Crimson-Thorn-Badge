
#ifndef HEADER_MenuSystem
#define HEADER_MenuSystem
#include "stdint.h"
#include "Arduino.h"
#include <string>
#include <Adafruit_GFX.h>
#include "FreezeWatch.h"
//#include <Adafruit_ST7735.h>

class MenuSystem {
public:
    static void initMenuSystem(
        unsigned int badgeID,
        unsigned int badgeLevel,
        unsigned int badgeXP,
        unsigned int badgeXPProgress,
        unsigned int faction,
        bool standbyBling,
        void (*startAttack)(),
        void (*stopAttack)(),
        void (*resetBadge)(),
        void (*setFaction)(unsigned int faction),
        bool (*toggleAttackAugment)(int augNum),
        bool (*toggleDefenseAugment)(int augNum),
        void (*toggleStandbyBling)(bool enabled),
        void (*ackAttackOrUpdate)(),
        unsigned int (*getAttackAugments)(),
        unsigned int (*getDefenseAugments)(),
        unsigned int (*getNumAugments)(),
        unsigned int (*getBadgeLevel)(),
        unsigned int (*getBadgeXP)(),
        bool (*checkCode)(String code),
        bool (*isFaction1Unlocked)(),
        bool (*isFaction2Unlocked)(),
        unsigned int (*getLoopCtr)());
    static void moveRight();
    static void moveLeft();
    static void moveDown();
    static void moveUp();
    static void accept();
    static void back();
    static void printMainMenu();
    static void printBotnetMenu(int specialState);
    static void printSettingsMenu();
    static void printAugmentsMenu(int specialState);
    static void printSecretsMenu();
    static void printExtrasMenu();
    static void printWiFiScan(unsigned int loopCtr);

    static void secretAccept();
    static void secretReject();

    static void setBadgeID(unsigned int id);
    static void setBadgeXP(unsigned int xp);
    static void setBadgeLevel(unsigned int level);
    static void setBadgeXPProgress(unsigned int xpProgress);
    static void setFaction(unsigned int faction);

    static void setBotState(int botState);
    static int getBotState();

    static bool isOnBotMenu();
    static bool isOnMainMenu();
    static bool isOnWiFiScan();

private:
    static int _currentMenu;
    static int _currentMenuState;
    static int _currentMenuPage;
    static int _currentMenuOption;
    static int _botState;
    static int _currentLetter;
    static int _numWiFiNetworks;
    static String _currentCode;
    static int _currentCodeLength;
    static bool _canMoveRight;
    static bool _canMoveLeft;
    static bool _canMoveUp;
    static bool _canMoveDown;
    static unsigned int _badgeID;
    static unsigned int _badgeXP;
    static unsigned int _badgeLevel;
    static unsigned int _badgeXPProgress;
    static unsigned int _faction;
    static unsigned short menuColor;
    static bool _standbyBling;
    static void printNavArrows();
    static void jpegRender(int xpos, int ypos);
    static void drawJpeg(const char *filename, int xpos, int ypos);
    static void drawRocketShip();
    static void (*_startAttack)();
    static void (*_stopAttack)();
    static void (*_resetBadge)();
    static void (*_setFaction)(unsigned int faction);
    static bool (*_toggleAttackAugment)(int augNum);
    static bool (*_toggleDefenseAugment)(int augNum);
    static unsigned int (*_getAttackAugments)();
    static unsigned int (*_getDefenseAugments)();
    static unsigned int (*_getNumAugments)();
    static unsigned int (*_getBadgeLevel)();
    static unsigned int (*_getBadgeXP)();
    static bool (*_checkCode)(String code);
    static bool (*_isFaction1Unlocked)();
    static bool (*_isFaction2Unlocked)();
    static unsigned int (*_getLoopCtr)();
    static void (*_toggleStandbyBling)(bool enabled);
    static void (*_ackAttackorUpdate)();
};

#endif