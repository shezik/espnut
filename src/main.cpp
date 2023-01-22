#include <Arduino.h>
#include <LittleFS.h>
#include "util.h"

#define FORMAT_LITTLEFS_IF_FAILED true

void appendLog(char *str) {
    
}

void U8g2DrawAndSendDialog(char *message) {
    
}

void setup() {
    Serial.begin(115200);
    if (!LittleFS.begin(/*FORMAT_LITTLEFS_IF_FAILED*/))
        fatal(1, "Failed to init LittleFS.\n");
}

void loop() {

}