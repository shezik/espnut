#include <Arduino.h>
#include <LittleFS.h>
#include "util.h"
#include "Kbd_8x5_CH450.h"
#include "KeyboardMan.h"
#include "U8g2lib.h"
#include "GEM.h"
#include "Configuration.h"

U8G2_DISPLAY_TYPE u8g2(U8G2_R2, VSPI_CLK, VSPI_DATA, VSPI_CS, VSPI_DC, U8G2_RESET_PIN);
Kbd_8x5_CH450 keyboard(CH450_SDA, CH450_SCL, CH450_DELAY);
KeyboardMan keyboardMan(keyboard, CH450_INT);  // Referred to in util.cpp

void appendLog(char *str) {
    
}

void U8g2DrawAndSendDialog(char *message) {
    
}

void setup() {
    Serial.begin(115200);
    u8g2.setBusClock(U8G2_BUS_CLK);
    u8g2.begin();
    keyboardMan.init();
    keyboardMan.enableInterrupt();

    if (!LittleFS.begin(/*FORMAT_LITTLEFS_IF_FAILED*/))
        fatal(1, "Failed to init LittleFS.\n");

}

void loop() {

}