#pragma once

#include <Arduino.h>
#include <list>

class SettingsMgr {
    protected:
        char *filePath;
        std::list<void (*)()> applySettingsCallbackList;
        std::list<void (*)()>::iterator listIterator;

        uint8_t brightness;
        uint8_t contrast;
        uint8_t backlightTimeoutSec;
        uint8_t powerOffTimeoutMin;
        bool enablePowerMgmt;
        bool unlockSpeed;
        bool enableLogging;
        uint32_t cpuFrequency;
    public:
        SettingsMgr(char *);
        ~SettingsMgr();
        bool init();
        void loadDefaults();
        bool loadSettings();
        bool saveSettings();
        void applySettings();
        bool registerApplySettingsCallback(void (*)());

        uint8_t *getBrightnessPercent();
        uint8_t *getContrast();
        uint8_t *getBacklightTimeoutSec();
        uint8_t *getPowerOffTimeoutMin();
        bool *getEnablePowerMgmt();
        bool *getUnlockSpeed();
        bool *getEnableLogging();
        uint32_t *getCpuFrequency();
};
