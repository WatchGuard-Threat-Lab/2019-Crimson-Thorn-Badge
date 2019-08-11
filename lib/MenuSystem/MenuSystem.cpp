#include "MenuSystem.h"
#define FS_NO_GLOBALS
#include <FS.h>
#include "SPIFFS.h"
#include "JPEGDecoder.h"
#include "WiFi.h"
#include <BLEDevice.h>
#include <TFT_eFEX.h> // Include the function extension library
#include <TFT_eSPI.h> // Hardware-specific library

TFT_eSPI tft = TFT_eSPI();

const int MainMenu = 0;
const int BotnetMenu = 1;
const int SecretsMenu = 2;
const int SettingsMenu = 3;
const int ExtrasMenu = 4;
const int AugmentsMenu = 5;

const char ALPHABET[45] = {
    ' ', ' ', ' ', ' ',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K',
    'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
    'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6',
    '7', '8', '9',
    ' ', ' ', ' ', ' ', ' '};

int MenuSystem::_currentMenu;
int MenuSystem::_currentMenuPage;
int MenuSystem::_currentMenuOption;
int MenuSystem::_botState;
int MenuSystem::_currentLetter;
String MenuSystem::_currentCode;
int MenuSystem::_currentCodeLength;
int MenuSystem::_numWiFiNetworks;
bool MenuSystem::_canMoveLeft;
bool MenuSystem::_canMoveRight;
bool MenuSystem::_canMoveUp;
bool MenuSystem::_canMoveDown;
unsigned int MenuSystem::_badgeID;
unsigned int MenuSystem::_badgeLevel;
unsigned int MenuSystem::_badgeXP;
unsigned int MenuSystem::_badgeXPProgress;
unsigned int MenuSystem::_faction;
bool MenuSystem::_standbyBling;
void (*MenuSystem::_startAttack)();
void (*MenuSystem::_stopAttack)();
void (*MenuSystem::_resetBadge)();
void (*MenuSystem::_setFaction)(unsigned int faction);
bool (*MenuSystem::_toggleAttackAugment)(int augNum);
bool (*MenuSystem::_toggleDefenseAugment)(int augNum);
void (*MenuSystem::_toggleStandbyBling)(bool enabled);
void (*MenuSystem::_ackAttackorUpdate)();
unsigned int (*MenuSystem::_getAttackAugments)();
unsigned int (*MenuSystem::_getDefenseAugments)();
unsigned int (*MenuSystem::_getNumAugments)();
unsigned int (*MenuSystem::_getBadgeLevel)();
unsigned int (*MenuSystem::_getBadgeXP)();
bool (*MenuSystem::_checkCode)(String code);
bool (*MenuSystem::_isFaction1Unlocked)();
bool (*MenuSystem::_isFaction2Unlocked)();
unsigned int (*MenuSystem::_getLoopCtr)();

unsigned short MenuSystem::menuColor;

// Return the minimum of two values a and b
#define minimum(a, b) (((a) < (b)) ? (a) : (b))

void MenuSystem::jpegRender(int xpos, int ypos) {

    uint16_t *pImg;
    int16_t mcu_w = JpegDec.MCUWidth;
    int16_t mcu_h = JpegDec.MCUHeight;
    int32_t max_x = JpegDec.width;
    int32_t max_y = JpegDec.height;

    int32_t min_w = minimum(mcu_w, max_x % mcu_w);
    int32_t min_h = minimum(mcu_h, max_y % mcu_h);

    int32_t win_w = mcu_w;
    int32_t win_h = mcu_h;

    uint32_t drawTime = millis();

    max_x += xpos;
    max_y += ypos;

    while (JpegDec.readSwappedBytes()) {

        pImg = JpegDec.pImage;

        int mcu_x = JpegDec.MCUx * mcu_w + xpos;
        int mcu_y = JpegDec.MCUy * mcu_h + ypos;

        if (mcu_x + mcu_w <= max_x)
            win_w = mcu_w;
        else
            win_w = min_w;

        if (mcu_y + mcu_h <= max_y)
            win_h = mcu_h;
        else
            win_h = min_h;

        if (win_w != mcu_w) {
            for (int h = 1; h < win_h - 1; h++) {
                memcpy(pImg + h * win_w, pImg + (h + 1) * mcu_w, win_w << 1);
            }
        }

        if (mcu_x < tft.width() && mcu_y < tft.height()) {
            tft.pushImage(mcu_x, mcu_y, win_w, win_h, pImg);
        }

        else if ((mcu_y + win_h) >= tft.height())
            JpegDec.abort();
    }

    drawTime = millis() - drawTime;
}

void MenuSystem::drawJpeg(const char *filename, int xpos, int ypos) {

    File jpegFile = SPIFFS.open(filename, "r");
    if (!jpegFile) {
        Serial.print("ERROR: File \"");
        Serial.print(filename);
        Serial.println("\" not found!");
        return;
    }

    boolean decoded = JpegDec.decodeFsFile(filename);

    if (decoded) {
        MenuSystem::jpegRender(xpos, ypos);
    } else {
        Serial.println("Jpeg file format not supported!");
    }
}

void MenuSystem::drawRocketShip() {
    MenuSystem::drawJpeg("/land.jpg", 0, 146);
    MenuSystem::drawJpeg("/rocket2.jpg", 20, 124);
    delay(200);
    for (int i = 124; i >= 95; i--) {

        MenuSystem::drawJpeg("/rocket2.jpg", 20, i);
        delay(20);
    }

    TFT_eSprite spr = TFT_eSprite(&tft);
    TFT_eFEX fex = TFT_eFEX(&tft);
    spr.createSprite(16, 49);
    fex.drawJpeg("/rocket2.jpg", 0, 0, &spr);
    tft.setPivot(64, 80);
    spr.setPivot(40, -20);
    int xangle;
    xangle = 2;
    for (int16_t angle = 4; angle <= 75; angle += 1) {
        xangle += 1;
        spr.setPivot(40, -20 + xangle);
        spr.pushRotated(angle);
        delay(30);
        yield(); // Stop watchdog triggering
    }

    for (int i = 0; i >= -111; i -= 2) {
        drawJpeg("/cloud.jpg", i + 128, 60);
        drawJpeg("/land.jpg", i, 146);
    }

    tft.setPivot(109, 27);
    spr.setPivot(0, 0);
    xangle = 1;
    for (int16_t angle = 75; angle <= 150; angle += 1) {
        spr.pushRotated(angle);
        xangle += 2;
        tft.setPivot(112, 27 + xangle);
        delay(20);
        yield(); // Stop watchdog triggering
    }
    drawJpeg("/boom.jpg", 0, 0);
    delay(1000);
    tft.println("    DEAD");
    delay(1000);
    printExtrasMenu();
}

void MenuSystem::initMenuSystem(
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
    void (*ackAttackorUpdate)(),
    unsigned int (*getAttackAugments)(),
    unsigned int (*getDefenseAugments)(),
    unsigned int (*getNumAugments)(),
    unsigned int (*getBadgeLevel)(),
    unsigned int (*getBadgeXP)(),
    bool (*checkCode)(String code),
    bool (*isFaction1Unlocked)(),
    bool (*isFaction2Unlocked)(),
    unsigned int(*getLoopCtr)()) {
    Serial.println("Initializing Screen...");

    delay(10);

    tft.init();
    tft.setRotation(0); // 0 & 2 Portrait. 1 & 3 landscape
    tft.writecommand(TFT_MAD_RGB);
    SPIFFS.begin(true);

    tft.fillScreen(TFT_BLACK);

    delay(20);

    if (!SPIFFS.begin()) {
        Serial.println("SPIFFS initialisation failed!");
        while (1)
            yield(); // Stay here twiddling thumbs waiting
    }
    Serial.println("\r\nInitialisation done.");
    MenuSystem::_badgeID = badgeID;
    MenuSystem::_badgeLevel = badgeLevel;
    MenuSystem::_badgeXP = badgeXP;
    MenuSystem::_badgeXPProgress = badgeXPProgress;
    MenuSystem::_faction = faction;
    MenuSystem::_standbyBling = standbyBling;
    MenuSystem::_startAttack = startAttack;
    MenuSystem::_stopAttack = stopAttack;
    MenuSystem::_resetBadge = resetBadge;
    MenuSystem::_setFaction = setFaction;
    MenuSystem::_toggleAttackAugment = toggleAttackAugment;
    MenuSystem::_toggleDefenseAugment = toggleDefenseAugment;
    MenuSystem::_toggleStandbyBling = toggleStandbyBling;
    MenuSystem::_ackAttackorUpdate = ackAttackorUpdate;
    MenuSystem::_getAttackAugments = getAttackAugments;
    MenuSystem::_getDefenseAugments = getDefenseAugments;
    MenuSystem::_getNumAugments = getNumAugments;
    MenuSystem::_getBadgeLevel = getBadgeLevel;
    MenuSystem::_getBadgeXP = getBadgeXP;
    MenuSystem::_getLoopCtr = getLoopCtr;
    MenuSystem::_checkCode = checkCode;
    MenuSystem::_isFaction1Unlocked = isFaction1Unlocked;
    MenuSystem::_isFaction2Unlocked = isFaction2Unlocked;
    MenuSystem::_currentMenu = 0;
    MenuSystem::_currentMenuPage = 0;
    MenuSystem::_currentMenuOption = 0;
    MenuSystem::_botState = 0;
    MenuSystem::_currentLetter = 4;
    MenuSystem::_currentCodeLength = 0;
    MenuSystem::_numWiFiNetworks = 0;
    MenuSystem::_canMoveLeft = false;
    MenuSystem::_canMoveRight = true;
    MenuSystem::_canMoveUp = false;
    MenuSystem::_canMoveDown = false;
    switch (faction) {
        case 1:
            menuColor = TFT_RED;
            break;
        case 2:
            menuColor = TFT_BLUE;
            break;
        case 3:
            menuColor = TFT_GREEN;
            break;
    }
    MenuSystem::printMainMenu();
}

void MenuSystem::setBadgeID(unsigned int badgeID) { MenuSystem::_badgeID = badgeID; }

void MenuSystem::setBadgeLevel(unsigned int level) { MenuSystem::_badgeLevel = level; }

void MenuSystem::setBadgeXP(unsigned int xp) { MenuSystem::_badgeXP = xp; }

void MenuSystem::setBadgeXPProgress(unsigned int xpProgress) { MenuSystem::_badgeXPProgress = xpProgress; }

void MenuSystem::setBotState(int botState) {
    nofreezeplz.checkIn();
    int curState = _botState;
    MenuSystem::_botState = botState;
    if (curState == 7 || curState == 8 || curState == 11) {
        printMainMenu();
    } else {
        printBotnetMenu(0);
    }
}

int MenuSystem::getBotState() { return MenuSystem::_botState; }

bool MenuSystem::isOnBotMenu() {
    if (MenuSystem::_currentMenu == BotnetMenu) {
        return true;
    } else {
        return false;
    }
}

bool MenuSystem::isOnMainMenu() {
    if (MenuSystem::_currentMenu == MainMenu) {
        return true;
    } else {
        return false;
    }
}

bool MenuSystem::isOnWiFiScan() {
    if (_currentMenu == ExtrasMenu && _currentMenuPage == 0 && _currentMenuOption == 1) {
        return true;
    } else {
        return false;
    }
}

void MenuSystem::moveRight() {
    switch (MenuSystem::_currentMenu) {
    case MainMenu:
        if (MenuSystem::_currentMenuPage < 4) {
            MenuSystem::_currentMenuPage += 1;
            if (MenuSystem::_currentMenuPage > 0) {
                MenuSystem::_canMoveLeft = true;
            }
            if (MenuSystem::_currentMenuPage == 4) {
                MenuSystem::_canMoveRight = false;
            }
            MenuSystem::printMainMenu();
        }
        break;
    case BotnetMenu:
        if (MenuSystem::_botState > 0) {
            break;
        }
        if (MenuSystem::_currentMenuPage < 1) {
            MenuSystem::_currentMenuPage += 1;
            if (MenuSystem::_currentMenuPage > 0) {
                MenuSystem::_canMoveLeft = true;
            }
            if (MenuSystem::_currentMenuPage == 1) {
                MenuSystem::_canMoveRight = false;
            }
            MenuSystem::printBotnetMenu(0);
        }
        break;
    case SecretsMenu:
        switch (_currentMenuOption) {
        case 0:
            if (_currentLetter < 39) {
                _currentLetter += 1;
                printSecretsMenu();
            }
            break;
        case 1:
            _currentMenuOption = 2;
            printSecretsMenu();
            break;
        case 2:
            break;
        }
        break;
    case SettingsMenu:
        if (_currentMenuPage <= 2) {
            _currentMenuPage += 1;
            if (_currentMenuPage == 1) {
                _canMoveLeft = true;
            }
            if (_currentMenuPage == 3) {
                _canMoveRight = false;
            }
            _currentMenuOption = 0;
            printSettingsMenu();
        }
        else if (_currentMenuPage == 4 && _currentMenuOption == 0) {
            _currentMenuOption = 1;
            printSettingsMenu();
        }
        break;
    case ExtrasMenu:
        if (_currentMenuPage == 0) {
            _currentMenuPage += 1;
            _canMoveLeft = true;
            _canMoveRight = false;
            printExtrasMenu();
        }
        break;
    case AugmentsMenu:
        if (_currentMenuPage >= 10 && _currentMenuPage < 19) {
            // In attack menu, can move right
            _currentMenuPage += 1;
            _canMoveLeft = true;
            if (_currentMenuPage == 19) {
                _canMoveRight = false;
            }
            printAugmentsMenu(0);
        }
        else if (_currentMenuPage >= 20 && _currentMenuPage < 27) {
            // In defense menu, can move right
            _currentMenuPage += 1;
            _canMoveLeft = true;
            if (_currentMenuPage == 27) {
                _canMoveRight = false;
            }
            printAugmentsMenu(0);
        }
        break;
    }
}

void MenuSystem::moveLeft() {
    switch (MenuSystem::_currentMenu) {
    case MainMenu:
        if (MenuSystem::_currentMenuPage > 0) {
            MenuSystem::_currentMenuPage -= 1;
            if (MenuSystem::_currentMenuPage < 4) {
                MenuSystem::_canMoveRight = true;
            }
            if (MenuSystem::_currentMenuPage == 0) {
                MenuSystem::_canMoveLeft = false;
            }
            MenuSystem::printMainMenu();
        }
        break;
    case BotnetMenu:
        if (MenuSystem::_botState > 0) {
            break;
        }
        if (MenuSystem::_currentMenuPage > 0) {
            MenuSystem::_currentMenuPage -= 1;
            if (MenuSystem::_currentMenuPage < 1) {
                MenuSystem::_canMoveRight = true;
            }
            if (MenuSystem::_currentMenuPage == 0) {
                MenuSystem::_canMoveLeft = false;
            }
            MenuSystem::printBotnetMenu(0);
        }
        break;
    case SecretsMenu:
        switch (_currentMenuOption) {
        case 0:
            if (_currentLetter > 4) {
                _currentLetter -= 1;
                printSecretsMenu();
            }
            break;
        case 1:
            break;
        case 2:
            _currentMenuOption = 1;
            printSecretsMenu();
            break;
        }
        break;
    case SettingsMenu:
        if (_currentMenuPage == 1 || _currentMenuPage == 2 || _currentMenuPage == 3) {
            _currentMenuPage -= 1;
            if (_currentMenuPage == 2) {
                _canMoveRight = true;
            }
            if (_currentMenuPage == 0) {
                _canMoveLeft = false;
            }
            _currentMenuOption = 0;
            printSettingsMenu();
        }
        else if (_currentMenuPage == 4 && _currentMenuOption == 1) {
            _currentMenuOption = 0;
            printSettingsMenu();
        }
        break;
    case ExtrasMenu:
        if (_currentMenuPage == 1) {
            _currentMenuPage -= 1;
            _canMoveLeft = false;
            _canMoveRight = true;
            printExtrasMenu();
        }
        break;
    case AugmentsMenu:
        if (_currentMenuPage > 10 && _currentMenuPage < 20) {
            // In attack menu, can move left
            _currentMenuPage -= 1;
            _canMoveRight = true;
            if (_currentMenuPage == 10) {
                _canMoveLeft = false;
            }
            printAugmentsMenu(0);
        }
        else if (_currentMenuPage > 20 && _currentMenuPage < 28) {
            // In defense menu, can move left
            _currentMenuPage -= 1;
            _canMoveRight = true;
            if (_currentMenuPage == 20) {
                _canMoveLeft = false;
            }
            printAugmentsMenu(0);
        }
        break;
    }
}

void MenuSystem::moveUp() {
    if (_currentMenu == AugmentsMenu && _currentMenuPage == 1) {
        _currentMenuPage = 0;
        printAugmentsMenu(0);
    }
    else if (_currentMenu == SettingsMenu) {
        if (_currentMenuPage == 1) {
            if (_currentMenuOption == 1){
                _currentMenuOption -= 1;
            } else if (_currentMenuOption == 2) {
                if (_isFaction1Unlocked()) {
                    _currentMenuOption -= 1;
                } else {
                    _currentMenuOption -=2;
                }
                printSettingsMenu();
            }
        }
        else if (_currentMenuPage == 2) {
            if (_currentMenuOption == 1) {
                _currentMenuOption -= 1;
                printSettingsMenu();
            }      
        }
    }
    else if (_currentMenu == SecretsMenu && _currentMenuOption > 0) {
        _currentMenuOption = 0;
        printSecretsMenu();
    }
}

void MenuSystem::moveDown() {
    if (_currentMenu == AugmentsMenu && _currentMenuPage == 0) {
        _currentMenuPage = 1;
        printAugmentsMenu(0);
    }
    else if (_currentMenu == SettingsMenu) {
        if (_currentMenuPage == 1) {
            if (_currentMenuOption == 0 && _isFaction1Unlocked()) {
                _currentMenuOption += 1;
                printSettingsMenu();
            } else if (_currentMenuOption == 0 && _isFaction2Unlocked()) {
                _currentMenuOption += 2;
                printSettingsMenu();
            }
            else if (_currentMenuOption == 1 && _isFaction2Unlocked()) {
                _currentMenuOption += 1;
                printSettingsMenu();
            }
        }
        else if (_currentMenuPage == 2) {
            if (_currentMenuOption == 0) {
                _currentMenuOption += 1;
                printSettingsMenu();
            }
        }
    }
    else if (_currentMenu == SecretsMenu && _currentMenuOption == 0) {
        _currentMenuOption = 1;
        printSecretsMenu();
    }
}

void MenuSystem::accept() {
    switch (MenuSystem::_currentMenu) {
    case MainMenu:
        switch (_currentMenuPage) {
        case 0:
            break;
        case 1:
            _currentMenu = BotnetMenu;
            _currentMenuPage = 0;
            _canMoveLeft = false;
            _badgeLevel = _getBadgeLevel();
            _badgeXP = _getBadgeXP();
            printBotnetMenu(0);
            delay(500);
            break;
        case 2:
            _currentMenu = SecretsMenu;
            _currentMenuPage = 0;
            _currentMenuOption = 0;
            _currentLetter = 4;
            for (int i = 0; i < 8; i++) {
                _currentCode = "";
                _currentCodeLength = 0;
            }
            printSecretsMenu();
            delay(500);
            break;
        case 3:
            _currentMenu = SettingsMenu;
            _currentMenuPage = 0;
            _canMoveLeft = false;
            _canMoveRight = true;
            printSettingsMenu();
            delay(500);
            break;
        case 4:
            _currentMenu = ExtrasMenu;
            _currentMenuPage = 0;
            _currentMenuOption = 0;
            _canMoveRight = true;
            _canMoveLeft = false;
            printExtrasMenu();
            delay(500);
            break;
        }
        break;
    case BotnetMenu:
        if (_botState > 0) {
            break;
        }
        switch (_currentMenuPage) {
        case 0:
            //START ATTACK
            _startAttack();
            printBotnetMenu(1);
            delay(500);
            break;
        case 1:
            // Open Augment Menu
            _currentMenu = AugmentsMenu;
            _currentMenuPage = 0;
            _canMoveLeft = false;
            _canMoveRight = false;
            printAugmentsMenu(0);
            delay(500);
            break;
        }
        break;
    case SecretsMenu:
        switch (_currentMenuOption) {
        case 0:
            // Add letter to code
            if (_currentCode.length() < 8) {
                _currentCode += ALPHABET[_currentLetter];
                _currentCodeLength += 1;
                printSecretsMenu();
                delay(500);
            }
            break;
        case 1:
            // Enter completed code
            if (_checkCode(_currentCode)) {
                // Code Accepted
                secretAccept();
                delay(1500);
                if (_getBadgeLevel() > _badgeLevel) {
                    // User entered a badge level code
                    _badgeLevel = _getBadgeLevel();
                    _badgeXP = _getBadgeXP();
                    drawJpeg("/LevelUp.jpg", 0, 0);
                    delay(2000);
                    if (_badgeLevel %5 == 0) {
                        drawJpeg("/NewAugment.jpg", 0, 0);
                        delay(2000);
                    }
                }
                _currentCode = "";
                _currentCodeLength = 0;
                tft.fillScreen(TFT_BLACK);
                printSecretsMenu();
            } else {
                secretReject();
                delay(1500);
                tft.fillScreen(TFT_BLACK);
                printSecretsMenu();
                // Entered an invalid code
            }
            break;
        case 2:
            // Remove letter from code
            if (_currentCode.length() > 0) {
                _currentCode.remove(_currentCode.length() - 1);
                _currentCodeLength -= 1;
                printSecretsMenu();
                delay(500);
            }
            break;
        }
        break;
    case SettingsMenu:
        switch (_currentMenuPage) {
            case 1:
                switch (_currentMenuOption) {
                case 0:
                    _faction = 1;
                    menuColor = TFT_RED;
                    _setFaction(1);
                    delay(500);
                    break;
                case 1:
                    _faction = 2;
                    menuColor = TFT_BLUE;
                    _setFaction(2);
                    delay(500);
                    break;
                case 2:
                    _faction = 3;
                    menuColor = TFT_GREEN;
                    _setFaction(3);
                    delay(500);
                    break;
                }
                _currentMenuOption = 0;
                _currentMenuPage = 0;
                printSettingsMenu();
                delay(500);
                break;
            case 2:
                if (_currentMenuOption == 0) {
                    _standbyBling = true;
                    _toggleStandbyBling(true);
                    printSettingsMenu();
                    delay(500);
                } else {
                    _standbyBling = false;
                    _toggleStandbyBling(false);
                    printSettingsMenu();
                    delay(500);
                }
                break;
            case 3:
                _currentMenuPage = 4;
                _canMoveLeft = false;
                _canMoveRight = false;
                printSettingsMenu();
                delay(500);
                break;
            case 4:
                if (_currentMenuOption == 0) {
                    _currentMenuPage = 3;
                    _canMoveLeft = true;
                    _canMoveRight = false;
                    printSettingsMenu();
                    delay(500);
                }
                else if (_currentMenuOption == 1) {
                    _resetBadge();
                }
                break;
        }
        break;
    case ExtrasMenu:
        if(_currentMenuPage == 0) {
            _currentMenuOption = 0;
            printWiFiScan(_getLoopCtr());
            delay(500);
            break;
        } else if (_currentMenuPage == 1) {
            drawRocketShip();
        }
        break;
    case AugmentsMenu:
        if (_currentMenuPage >= 10 && _currentMenuPage < 20) {
            if (MenuSystem::_toggleAttackAugment(_currentMenuPage - 10)) {
                printAugmentsMenu(0);
                delay(500);
            } else {
                printAugmentsMenu(1);
                delay(500);
            }
        }
        else if (_currentMenuPage >= 20 && _currentMenuPage < 30) {
            if (MenuSystem::_toggleDefenseAugment(_currentMenuPage - 20)) {
                printAugmentsMenu(0);
                delay(500);
            } else {
                printAugmentsMenu(1);
                delay(500);
            }
        }
        switch (_currentMenuPage) {
        case 0:
            _currentMenuPage = 10;
            _canMoveRight = true;
            printAugmentsMenu(0);
            delay(500);
            break;
        case 1:
            _currentMenuPage = 20;
            _canMoveRight = true;
            printAugmentsMenu(0);
            delay(500);
            break;
        }
        break;
    }
}

void MenuSystem::back() {
    switch (_currentMenu) {
    case MainMenu:
        break;
    case BotnetMenu:
        if (_botState > 0) {
            _stopAttack();
            delay(500);
            break;
        }
        _currentMenu = MainMenu;
        _currentMenuPage = 1;
        _canMoveLeft = true;
        _canMoveRight = true;
        _ackAttackorUpdate();
        printMainMenu();
        delay(500);
        break;
    case SecretsMenu:
        switch (_currentMenuOption) {
        case 0:
            _currentMenu = MainMenu;
            _currentMenuPage = 2;
            _canMoveLeft = true;
            _canMoveRight = true;
            _currentLetter = 4;
            _currentMenuOption = 0;
            _currentCode = "";
            _ackAttackorUpdate();
            printMainMenu();
            delay(500);
            break;
        case 1:
            _currentMenuOption = 0;
            printSecretsMenu();
            delay(500);
            break;
        case 2:
            _currentMenuOption = 0;
            printSecretsMenu();
            delay(500);
            break;
        }
        break;
    case SettingsMenu:
        switch (_currentMenuPage) {
        case 0:
        case 1:
        case 2:
        case 3:
            _currentMenu = MainMenu;
            _currentMenuPage = 3;
            _currentMenuOption = 0;
            _canMoveLeft = true;
            _canMoveRight = true;
            _currentMenuOption = 0;
            _numWiFiNetworks = 0;
            _ackAttackorUpdate();       
            printMainMenu();
            delay(500);
            break;
        case 4:
            _currentMenuPage = 3;
            _currentMenuOption = 0;
            _canMoveLeft = true;
            _canMoveRight = false;
            printSettingsMenu();
            delay(500);
            break;
        }
        break;
    case ExtrasMenu:
        _currentMenu = MainMenu;
        _currentMenuPage = 4;
        _canMoveLeft = true;
        _canMoveRight = false;
        _ackAttackorUpdate();
        printMainMenu();
        delay(500);
        break;
    case AugmentsMenu:
        switch (_currentMenuPage) {
        case 0:
        case 1:
            _currentMenu = BotnetMenu;
            _currentMenuPage = 1;
            _canMoveLeft = true;
            _canMoveRight = false;
            printBotnetMenu(0);
            delay(500);
            break;
        default:
            _currentMenuPage = 0;
            _canMoveLeft = false;
            _canMoveRight = false;
            printAugmentsMenu(0);
            delay(500);
            break;
        }
        break;
    }
}

void MenuSystem::printNavArrows() {
    if (MenuSystem::_canMoveLeft) {
        tft.fillTriangle(1, 80, 8, 72, 8, 88, TFT_RED);
    }

    if (MenuSystem::_canMoveRight) {
        tft.fillTriangle(127, 80, 120, 72, 120, 88, TFT_RED);
    }

    if (MenuSystem::_canMoveUp) {
        tft.fillTriangle(64, 1, 57, 9, 71, 9, TFT_RED);
    }

    if (MenuSystem::_canMoveDown) {
        tft.fillTriangle(64, 159, 57, 150, 71, 150, TFT_RED);
    }
}

void MenuSystem::printMainMenu() {
    // Menu System
    switch (MenuSystem::_currentMenuPage) {
    case 0:
        MenuSystem::drawJpeg("/MenuSystem.jpg", 0, 0);
        break;
    case 1:
        MenuSystem::drawJpeg("/Botnet.jpg", 0, 0);
        break;
    case 2:
        MenuSystem::drawJpeg("/Secrets.jpg", 0, 0);
        break;
    case 3:
        MenuSystem::drawJpeg("/Settings.jpg", 0, 0);
        break;
    case 4:
        MenuSystem::drawJpeg("/Extras.jpg", 0, 0);
        break;
    }
    // Nav Arrows
    MenuSystem::printNavArrows();
}

void MenuSystem::printBotnetMenu(int specialState) {
    String botStateTable[11] = {"", "SEARCHING", "  TARGET     FOUND", " ATTACKING", "  UNDER    ATTACK",
                                "  ATTACK  SUCCESSFUL", "  ATTACK    FAILED!", " DEFENSE  SUCCESFUL",
                                " DEFENSE    FAILED!", " LEVEL UP!", "   NEW      AUGMENT!"};

    // Menu System
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 55);
    tft.setTextWrap(true);
    tft.setTextSize(2);
    switch (MenuSystem::_botState) {
    case 0:
        if (MenuSystem::_currentMenuPage == 0) {
            MenuSystem::drawJpeg("/StartAttack.jpg", 0, 0);
        }
        else if (MenuSystem::_currentMenuPage == 1) {
            MenuSystem::drawJpeg("/SetAugments.jpg", 0, 0);
        }
        MenuSystem::printNavArrows();
        break;
    case 1:
        MenuSystem::drawJpeg("/Searching.jpg", 0, 0);
        break;
    case 2:
        MenuSystem::drawJpeg("/TargetFound.jpg", 0, 0);
        break;
    case 3:
        MenuSystem::drawJpeg("/Attacking.jpg", 0, 0);
        break;
    case 4:
        MenuSystem::drawJpeg("/UnderAttack.jpg", 0, 0);
        break;
    case 5:
        MenuSystem::drawJpeg("/AttackSuccess.jpg", 0, 0);
        break;
    case 6:
        MenuSystem::drawJpeg("/AttackFailed.jpg", 0, 0);
        break;
    case 7:
        MenuSystem::drawJpeg("/DefenseSuccess.jpg", 0, 0);
        break;
    case 8:
        MenuSystem::drawJpeg("/DefenseFailed.jpg", 0, 0);
        break;
    case 9:
        MenuSystem::drawJpeg("/LevelUp.jpg", 0, 0);
        break;
    case 10:
        MenuSystem::drawJpeg("/NewAugment.jpg", 0, 0);
        break;
    case 11:
        MenuSystem::drawJpeg("/Updating.jpg", 0, 0);
        break;
    case 12:
        MenuSystem::drawJpeg("/TooRecent.jpg", 0, 0);
        break;
    }
    // Stats Banner
    tft.setTextWrap(false);
    tft.setCursor(0, 0);
    tft.setTextColor(TFT_RED);
    tft.setTextSize(1);
    tft.fillRect(0, 0, 128, 16, TFT_BLACK);
    tft.print("LVL: ");
    tft.setTextColor(TFT_BLUE);
    tft.print(MenuSystem::_badgeLevel);
    tft.setCursor(48, 0);
    tft.setTextColor(TFT_RED);
    tft.print("XP: ");
    tft.setTextColor(TFT_BLUE);
    tft.println(MenuSystem::_badgeXP);
    tft.setTextColor(TFT_RED);
    tft.print("NXT ");
    tft.fillRect(28, 9, MenuSystem::_badgeXPProgress, 5, TFT_GREEN);
}

void MenuSystem::printSettingsMenu() {
    tft.setTextWrap(false);
    //MenuSystem::drawJpeg("/Settings.jpg", 0 , 0);
    // Badge ID
    tft.setCursor(0, 0);
    tft.setTextColor(menuColor);
    tft.setTextSize(2);
    tft.fillScreen(TFT_BLACK);
    switch (MenuSystem::_currentMenuPage) {
    case 0:
        tft.setTextSize(1);
        tft.println("Badge ID");
        tft.print(" ");
        tft.println(MenuSystem::_badgeID, HEX);
        tft.println("");
        tft.println("Current Faction");
        tft.print(" ");
        switch (MenuSystem::_faction) {
        case 1:
            tft.println("WiltedRose");
            break;
        case 2:
            tft.println("MorningGlory");
            break;
        case 3:
            tft.println("Prefect");
            break;
        }
        tft.println(" ");
        tft.println("BL MAC");
        tft.print(" ");
        tft.println(BLEDevice::getAddress().toString().c_str());
        MenuSystem::printNavArrows();
        break;
    case 1:
        tft.setTextSize(1);
        tft.println("   Select Faction");
        tft.setTextSize(1);
        tft.setCursor(0, 50);
        switch (_currentMenuOption) {
            case 0:
                tft.fillRect(20, 50, 80, 7, TFT_WHITE);
                break;
            case 1:
                tft.fillRect(20, 58, 80, 7, TFT_WHITE);
                break;
            case 2:
                if (_isFaction1Unlocked()) {
                    tft.fillRect(20, 66, 80, 7, TFT_WHITE);
                } else {
                   tft.fillRect(20, 58, 80, 7, TFT_WHITE); 
                }
                break;
        }
        tft.println("     WiltedRose");
        if (_isFaction1Unlocked()) {
            tft.println("    MorningGlory");
        }
        if (_isFaction2Unlocked()) {
            tft.println("       Prefect");
        }
        MenuSystem::printNavArrows();
        break;
    case 2:
        tft.setTextSize(1);
        tft.println("  Enable Idle Bling");
        tft.setTextSize(1);
        tft.setCursor(0, 50);
        switch (_currentMenuOption) {
            case 0:
                tft.fillRect(20, 50, 80, 7, TFT_WHITE);
                break;
            case 1:
                tft.fillRect(20, 58, 80, 7, TFT_WHITE);
                break;
        }
        if (_standbyBling) {
            tft.println("    [x]Enabled");
            tft.println("    [ ]Disabled");            
        } else {
            tft.println("    [ ]Enabled");
            tft.println("    [x]Disabled");
        }
        MenuSystem::printNavArrows();
        break;
    case 3:
        tft.println("***RESET***");
        tft.setTextSize(1);
        tft.setCursor(0, 30);
        tft.setTextColor(TFT_WHITE);
        tft.println("   This will reset");
        tft.println("     EVERYTHING!");
        tft.println(" ");
        tft.println("   You will need to");
        tft.println("  re-sync your badge");
        tft.println("   with the website.");
        tft.setTextColor(menuColor);
        tft.setTextSize(2);
        tft.setCursor(0, 145);
        tft.println("   [OK]   ");
        MenuSystem::printNavArrows();
        break;
    case 4:
        switch (MenuSystem::_currentMenuOption) {
        case 0:
            tft.println("***RESET***");
            tft.setCursor(0, 108);
            tft.println(" Confirm?");
            tft.fillRect(24, 123, 24, 15, TFT_WHITE);
            tft.println("  NO YES  ");
            break;
        case 1:
            tft.println("***RESET***");
            tft.setCursor(0, 108);
            tft.println(" Confirm?");
            tft.fillRect(60, 123, 36, 15, TFT_WHITE);
            tft.println("  NO YES  ");
            break;
        }
    }
}

void MenuSystem::printAugmentsMenu(int specialState) {
    String attackAugNames[10] = {" Portscan ", " Encrypted    C2", "  Network    Worm", " Keylogger",
                                 "  Rootkit", "  Crypto    Miner", "  Evasive    Code", "Flash 0Day",
                                 "  Exploit     Kit", "  400lb     Hacker   Sidekick"};
    String defenseAugNames[8] = {"   Basic    Firewall", "  Network Decryption", " Signature    AV",
                                 " Sandboxed    AV", "  Process Monitoring", "    IPS", " Patching  Schedule",
                                 " Friend at the FBI"};
    String attackAugDesc[10] = {"Countered by Basic        Firewall", "   Countered by      Network Decryption", " Countered by IPS",
                                "   Countered by         Signature AV", "   Countered by      Process Monitoring",
                                "   Countered by      Process Monitoring", "   Countered by         Sandboxed AV",
                                "   Countered by         Sandboxed AV", "   Countered by      Patching Schedule",
                                "   Countered by      Friend at the FBI"};
    String defenseAugDesc[8] = {"Counters Portscan", "    Counters            Encrypted C2", "Counters Keylogger",
                                "  Counters Flash    0Day & Evasive Code", " Counters Rootkit      & Crypto Miner",
                                " Counters Network           Worm", " Counters Exploit            Kit", "  Counters 400lb       Hacker Sidekick"};
    // Menu System
    tft.setCursor(0, 3);
    tft.setTextColor(menuColor);
    tft.setTextSize(2);
    tft.fillScreen(TFT_BLACK);
    //    drawJpeg("/Menu System .jpg", 0 , 0);

    if (_currentMenuPage == 0) {
        tft.fillRect(0, 122, 128, 15, TFT_WHITE);
    }
    else if (_currentMenuPage == 1) {
        tft.fillRect(0, 138, 128, 15, TFT_WHITE);
    }
    else if (_currentMenuPage >= 10 && _currentMenuPage < 20) {
        tft.setTextWrap(true);
        tft.println(attackAugNames[_currentMenuPage - 10]);
        unsigned int attackAugs = _getAttackAugments();
        if ((attackAugs >> (_currentMenuPage - 10)) & 1U) {
            tft.drawRect(0, 0, 128, 160, TFT_GREEN);
            tft.setCursor(0, 140);
            tft.setTextColor(menuColor);
            tft.print(" [DISABLE]");
        } else {
            tft.drawRect(0, 0, 128, 160, TFT_RED);
            tft.setCursor(0, 140);
            tft.setTextColor(menuColor);
            tft.print(" [ENABLE]");
        }
    }
    else if (_currentMenuPage >= 20 && _currentMenuPage < 30) {
        tft.setTextWrap(true);
        tft.println(defenseAugNames[_currentMenuPage - 20]);
        unsigned int defenseAugs = _getDefenseAugments();
        if ((defenseAugs >> (_currentMenuPage - 20)) & 1U) {
            tft.drawRect(0, 0, 128, 160, TFT_GREEN);
            tft.setCursor(0, 140);
            tft.setTextColor(menuColor);
            tft.print(" [DISABLE]");
        } else {
            tft.drawRect(0, 0, 128, 160, TFT_RED);
            tft.setCursor(0, 140);
            tft.setTextColor(menuColor);
            tft.print(" [ENABLE]");
        }
    }

    if (specialState == 1) {
        tft.setCursor(0, 80);
        tft.setTextColor(TFT_CYAN);
        tft.print("  At Max   Augments");
    } else {
        tft.setCursor(10, 85);
        tft.setTextColor(TFT_WHITE);
        tft.setTextSize(1);
        if (_currentMenuPage >= 10 && _currentMenuPage < 20) {
            tft.println(attackAugDesc[_currentMenuPage - 10]);
        }
        else if (_currentMenuPage >= 20 && _currentMenuPage < 30) {
            tft.println(defenseAugDesc[_currentMenuPage - 20]);
        }
    }

    if (MenuSystem::_currentMenuPage == 0 || MenuSystem::_currentMenuPage == 1) {
        tft.setCursor(0, 5);
        tft.setTextSize(1);
        tft.setTextColor(TFT_WHITE);
        tft.println("Augments increase");
        tft.println("your botnet's attack");
        tft.println("& defense strenghth.");
        tft.println("You unlock a new");
        tft.println("augment slot every 5");
        tft.println("levels.");
        tft.setTextColor(menuColor);
        tft.print("Augment Slots: ");
        tft.setTextColor(TFT_WHITE);
        tft.println(MenuSystem::_badgeLevel / 5);
        tft.setTextColor(menuColor);
        tft.print("Enabled: ");
        tft.setTextColor(TFT_WHITE);
        tft.print(_getNumAugments());
        tft.setCursor(0, 105);
        tft.setTextColor(menuColor);
        tft.setTextSize(2);
        tft.println("*Augments*");
        tft.setCursor(0,122);
        tft.println("  Attack");
        tft.println("  Defense");
    }
    MenuSystem::printNavArrows();
}

void MenuSystem::printSecretsMenu() {
    tft.setTextWrap(false);
    tft.setCursor(5, 0);
    tft.setTextColor(menuColor);
    tft.setTextSize(2);
    if (_currentLetter == 4) {
        tft.fillScreen(TFT_BLACK);
    }
    tft.println("ENTER CODE");

    tft.fillRect(3, 58, 122, 18, TFT_BLACK);
    tft.drawRect(3, 58, 122, 18, TFT_WHITE);

    tft.setCursor(16, 60);
    //if (_currentCode.length > 0) {
    tft.print(_currentCode);
    //}
    tft.fillRect(3,84,122,18,TFT_BLACK);
    switch (_currentMenuOption) {
    case 0:
        tft.fillRect(3,109,122,18,TFT_BLACK);
        tft.drawRect(52, 84, 12, 16, TFT_GREEN);
        break;
    case 1:
        tft.fillRect(3,109,122,18,TFT_BLACK);
        tft.drawRect(22, 109, 26, 16, TFT_GREEN);
        break;
    case 2:
        tft.fillRect(3,109,122,18,TFT_BLACK);
        tft.drawRect(58, 109, 38, 16, TFT_GREEN);
        break;
    }

    tft.println("");
    tft.setCursor(5, 85);
    for (int i = _currentLetter - 4; i < _currentLetter + 6; i++) {
        tft.print(ALPHABET[i]);
    }
    tft.println("");
    tft.setCursor(0, 110);
    tft.print("  OK DEL  ");
}

void MenuSystem::secretAccept() {
    tft.setCursor(16, 35);
    tft.setTextColor(TFT_GREEN);
    tft.print("ACCEPTED");
}

void MenuSystem::secretReject() {
    tft.setCursor(16, 35);
    tft.setTextColor(TFT_RED);
    tft.print("REJECTED");
}

void MenuSystem::printExtrasMenu() {
    tft.setTextWrap(false);
    tft.setCursor(5, 0);
    tft.setTextColor(menuColor);
    tft.setTextSize(2);
    tft.fillScreen(TFT_BLACK);
    if (_currentMenuPage == 0) {
        MenuSystem::drawJpeg("/Wi-Fi-Scanner.jpg", 0, 0);
        printNavArrows();
    } else if (_currentMenuPage == 1) {
        MenuSystem::drawJpeg("/SVD.jpg", 0, 0);
        printNavArrows();
    }
}


void MenuSystem::printWiFiScan(unsigned int loopCtr) {
    tft.setTextWrap(false);
    tft.setCursor(5,0);
    tft.setTextColor(menuColor);
    tft.setTextSize(2);
    if (_currentMenuOption == 0) {
        _numWiFiNetworks = 0;
        tft.fillScreen(TFT_BLACK);
        tft.println("Scanning..");
        WiFi.mode(WIFI_STA);
        WiFi.disconnect();
        delay(100);
        Serial.println("Scanning Networks");
        _numWiFiNetworks = WiFi.scanNetworks();
        Serial.println("Scan Done");
        _currentMenuOption = 1;
        tft.fillScreen(TFT_BLACK);
        printWiFiScan(_getLoopCtr());
    }else if (_currentMenuOption == 1) {
        if (_numWiFiNetworks == 0) {
            Serial.println("No Networks Found");
            tft.println("No Networks");       
        } else {
            tft.println("Networks:");  
            Serial.print(_numWiFiNetworks);
            Serial.println(" networks found");
            for (int i = 0; i < _numWiFiNetworks; ++i) {
                String text = WiFi.SSID(i) + " (" + WiFi.RSSI(i) + ")";
                (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? text+" " : text+"*";
                if (text.length() > 17) {
                    unsigned int l = text.length();
                    tft.fillRect(24,16+(i*8),104,8,TFT_BLACK);
                    text += "   " + text;
                    text = text.substring(loopCtr % (l+3));
                }
                tft.setTextSize(1);
                if (i < 9) {
                    tft.print(0);
                    tft.print(i + 1);   
                } else {
                    tft.print(i + 1);
                }
                tft.print(": ");
                tft.println(text);
            }
        }
    }
}