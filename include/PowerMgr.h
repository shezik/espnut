#pragma once

#include <Arduino.h>
#include "KeyboardMgr.h"

class PowerMgr {
    protected:
        KeyboardMgr &kbdMgr;
        uint8_t wakeUpInterruptPin;
        uint8_t LDOEnablePin;
        uint8_t displayBacklightPin;
        uint32_t backlightTimeout;  // In ms
        int64_t nextBacklightOff;   // In ms
        uint32_t deepSleepTimeout;   // In ms
        int64_t nextDeepSleep;       // In ms
        uint32_t frequency;
        bool wokenUp = false;
        void (*deepSleepCallback)() = nullptr;
        static PowerMgr *context;
    public:
        PowerMgr(KeyboardMgr &, uint8_t, uint8_t, uint8_t);
        ~PowerMgr();

        bool enterModemSleep();
        void enterDeepSleep();
        bool wokenUpFromDeepSleep();
        void init();
        void tick();

        void enableLDO(bool);

        bool getBacklightPower();
        void setBacklightPower(bool);

        uint16_t getBacklightTimeout();
        void setBacklightTimeout(uint32_t);
        void feedBacklightTimeout();

        uint32_t getDeepSleepTimeout();
        void setDeepSleepTimeout(uint32_t);
        void feedDeepSleepTimeout();

        bool setFrequency(uint32_t);
        bool reduceFrequency();
        bool restoreFrequency();
        
        void registerDeepSleepCallback(void (*)());
        static void keyPressCallback();
};
