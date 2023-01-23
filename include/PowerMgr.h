#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include "KeyboardMan.h"
#include <driver/rtc_io.h>

class PowerMgr {
    protected:
        KeyboardMan &kbdMan;
        uint8_t wakeUpInterruptPin;
    public:
        PowerMgr(KeyboardMan &, uint8_t);
        void deepSleepCleanup();
        bool enterModemSleep();
        void enterDeepSleep();
};
