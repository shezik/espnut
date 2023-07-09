#include <Arduino.h>
#include "util.h"
#include "U8g2lib.h"
#include "DispInterface.h"
#include "KeyboardMgr.h"
#include "PowerMgr.h"
#include "NutEmuInterface.h"
#include "Menu.h"
#include "MatrixKeyboard.h"
#include "SettingsMgr.h"
#include "Configuration.h"

SettingsMgr settingsMgr(CONFIG_FILENAME);
U8G2_DISPLAY_TYPE u8g2(U8G2_R2, SPI_CS, SPI_DC, U8G2_RESET_PIN);
DispInterface dispInterface(settingsMgr, u8g2);  // Referred to in util.h
KeyboardMgr keyboardMgr(POWER_BUTTON);  // Referred to in util.cpp
PowerMgr powerMgr(settingsMgr, keyboardMgr, dispInterface, POWER_BUTTON, LDO_ENABLE, DISPLAY_BACKLIGHT_CONTROL, BAT_LVL_CHK, BAT_CHRG);
NutEmuInterface nutEmuInterface(settingsMgr, keyboardMgr, dispInterface, powerMgr);
Menu menu(settingsMgr, keyboardMgr, dispInterface, powerMgr, nutEmuInterface);

void appendLog(char *str) {
    
}

void setup() {
    Serial.begin(115200);

    if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
        printf_log("Failed to init LittleFS.\n");
        exit(1);
    }
    settingsMgr.init();

    powerMgr.init();  // Reset wakeUpInterruptPin pin mode, detect last deep sleep (ext0), keep LDO enabled, init and turn on backlight, set CPU frequency
    powerMgr.enterModemSleep();

    dispInterface.init();
    powerMgr.tick();  // To check battery status

    keyboardMgr.init();  // Init and start matrix keyboard scanner task
    nutEmuInterface.init();

    bool isRestoreFlagPresent = nutEmuInterface.checkRestoreFlag();
    printf("Restore flag presence: %d\n", isRestoreFlagPresent);
    if (isRestoreFlagPresent)
        nutEmuInterface.resume();
    menu.init(!isRestoreFlagPresent);  // Load user settings into classes, skip showing main menu if restore file is successfully loaded
}

void loop() {
    powerMgr.tick();
    menu.tick();
    nutEmuInterface.tick();
}
