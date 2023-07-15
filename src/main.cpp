/*
 Copyright (C) 2023  shezik
 
 espnut is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License version 2 as
 published by the Free Software Foundation.  Note that the permission
 to redistribute or modify espnut under the terms of any later
 version of the General Public License is denied by the author
 of Nonpareil, Eric L. Smith, according to his notice.
 
 espnut is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program (in the file "COPYING" or "LICENSE"); if not,
 write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 Boston, MA 02111, USA.
*/

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
    if (isRestoreFlagPresent) {
        nutEmuInterface.resume();
        
        uint16_t keycode;
        xSemaphoreTake(keyboardMgr.getMutex(), 0);
        for (;;) {
            keycode = GetKeycodeContent(keyboardMgr.getPositiveKeycode());
            if (keycode == 24 /*ON*/)
                continue;
            break;
        }
        xSemaphoreGive(keyboardMgr.getMutex());
        if (keycode != INVALID_KEYCODE) {
            nutEmuInterface.handleONKeySequence(keycode);
        }
    }
    menu.init(!isRestoreFlagPresent);  // Load user settings into classes, skip showing main menu if restore file is successfully loaded
}

void loop() {
    powerMgr.tick();
    menu.tick();
    nutEmuInterface.tick();
}
