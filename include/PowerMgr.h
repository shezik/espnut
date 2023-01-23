#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include "KeyboardMan.h"
#include <driver/rtc_io.h>
#include <driver/adc.h>

class PowerMgr {
    protected:
        KeyboardMan &kbdMan;
        uint8_t wakeUpInterruptPin;
        uint8_t displayPowerPin;
        bool woken = false;
    public:
        PowerMgr(KeyboardMan &, uint8_t, uint8_t);
        bool enterModemSleep();
        void enterDeepSleep();
        bool wokenUpFromDeepSleep();
        void init();
        bool getDisplayPower();
        void setDisplayPower(bool);
};
