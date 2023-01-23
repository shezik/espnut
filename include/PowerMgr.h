#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include "KeyboardMgr.h"
#include <driver/rtc_io.h>
#include <driver/adc.h>

class PowerMgr {
    protected:
        KeyboardMgr &kbdMgr;
        uint8_t wakeUpInterruptPin;
        uint8_t displayPowerPin;
        bool woken = false;
    public:
        PowerMgr(KeyboardMgr &, uint8_t, uint8_t);
        bool enterModemSleep();
        void enterDeepSleep();
        bool wokenUpFromDeepSleep();
        void init();
        bool getDisplayPower();
        void setDisplayPower(bool);
};
