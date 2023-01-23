#include <Arduino.h>
#include <LittleFS.h>
#include <WiFi.h>
#include "util.h"
#include "Kbd_8x5_CH450.h"
#include "KeyboardMan.h"
#include "DispInterface.h"
#include "NutEmuInterface.h"
#include "PowerMgr.h"
#include "U8g2lib.h"
#include "GEM.h"
#include "Configuration.h"

U8G2_DISPLAY_TYPE u8g2(U8G2_R2, VSPI_CLK, VSPI_DATA, VSPI_CS, VSPI_DC, U8G2_RESET_PIN);
DispInterface dispInterface(u8g2);  // Referred to in util.h
Kbd_8x5_CH450 keyboard(CH450_SDA, CH450_SCL, CH450_DELAY);
KeyboardMan keyboardMan(keyboard, CH450_INT);  // Referred to in util.cpp
PowerMgr powerMgr(keyboardMan, CH450_INT);
NutEmuInterface nutEmuInterface(keyboardMan, dispInterface, powerMgr);

void appendLog(char *str) {
    
}

void setup() {
    setCpuFrequencyMhz(getXtalFrequencyMhz() == 40 ? 40 : 80);
    Serial.begin(115200);

    powerMgr.deepSleepCleanup();
    powerMgr.enterModemSleep();
    
    u8g2.setBusClock(U8G2_BUS_CLK);
    u8g2.begin();
    keyboard.init();
    keyboardMan.init();
    keyboardMan.enableInterrupt();

    if (!LittleFS.begin(/*FORMAT_LITTLEFS_IF_FAILED*/))
        fatal(1, "Failed to init LittleFS.\n");

}

void loop() {
    keyboardMan.checkForRelease();
}