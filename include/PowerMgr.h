#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include "KeyboardMgr.h"
#include <driver/rtc_io.h>
#include <driver/adc.h>
#include "Configuration.h"

class PowerMgr {
    protected:
        KeyboardMgr &kbdMgr;
        uint8_t wakeUpInterruptPin;
        uint8_t displayPowerPin;
        uint8_t displayBacklightPin;
        uint16_t backlightTimeout;  // In ms
        int64_t nextBacklightOff;  // In ms
        uint32_t frequency;
        bool woken = false;
    public:
        PowerMgr(KeyboardMgr &, uint8_t, uint8_t, uint8_t);
        bool enterModemSleep();
        void enterDeepSleep();
        bool wokenUpFromDeepSleep();
        void init();
        void tick();
        bool getDisplayPower();
        void setDisplayPower(bool);
        bool getBacklightPower();
        void setBacklightPower(bool);
        uint16_t getBacklightTimeout();
        void setBacklightTimeout(uint16_t);
        void feedBacklightTimeout();
        bool setFrequency(uint32_t);
        bool reduceFrequency();
        bool restoreFrequency();
};
