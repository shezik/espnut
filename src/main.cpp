#include <Arduino.h>
#include "util.h"
#include "U8g2lib.h"
#include "DispInterface.h"
#include "Kbd_8x5_CH450.h"
#include "KeyboardMgr.h"
#include "PowerMgr.h"
#include "NutEmuInterface.h"
#include "Menu.h"
#include "Configuration.h"

U8G2_DISPLAY_TYPE u8g2(U8G2_R2, VSPI_CLK, VSPI_DATA, VSPI_CS, VSPI_DC, U8G2_RESET_PIN);
DispInterface dispInterface(u8g2);  // Referred to in util.h
Kbd_8x5_CH450 keyboard(CH450_SDA, CH450_SCL, CH450_DELAY);
KeyboardMgr keyboardMgr(keyboard, CH450_INT);  // Referred to in util.cpp
PowerMgr powerMgr(keyboardMgr, CH450_INT, DISPLAY_POWER_CONTROL, DISPLAY_BACKLIGHT_CONTROL);
NutEmuInterface nutEmuInterface(keyboardMgr, dispInterface, powerMgr);
Menu menu(keyboardMgr, u8g2, powerMgr, nutEmuInterface);

void appendLog(char *str) {
    
}

void setup() {
    Serial.begin(115200);

    powerMgr.init();  // Reset wakeUpInterruptPin pin mode, detect last deep sleep (ext0), power up display, 
                      // init and turn on backlight, set CPU frequency
    powerMgr.enterModemSleep();
    
    u8g2.setBusClock(U8G2_BUS_CLK);
    u8g2.begin();
    keyboard.init(!powerMgr.wokenUpFromDeepSleep());  // Set CH450_SDA/SCL pin mode & init CH450. !! DO NOT INIT IF WOKEN UP BY IT, TO GET THE WAKING KEYCODE!
    keyboardMgr.init();  // Set keyboard interrupt pin mode
    menu.init();  // Load user settings into classes
    
    if (powerMgr.wokenUpFromDeepSleep())
        keyboardMgr.handleKeyPress();  // Get waking keycode
    keyboardMgr.enableInterrupt();  // Start watching for key presses

    if (!LittleFS.begin(/*FORMAT_LITTLEFS_IF_FAILED*/))
        fatal(1, "Failed to init LittleFS.\n");

    nutEmuInterface.checkRestoreFlag();
}

void loop() {
    keyboardMgr.tick();
    powerMgr.tick();  // Check for backlight timeout
    if (menu.tick())
        nutEmuInterface.tick();
}