/*
 Copyright (C) 2023  shezik
 
 espnut is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License version 2 as
 published by the Free Software Foundation.  Note that the permission
 to redistribute or modify espnut under the terms of any later
 version of the General Public License is denied by the author
 of Nonpareil, Eric L. Smith, according to his notice.
 
 espnut is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program (in the file "COPYING" or "LICENSE"); if not,
 write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 Boston, MA 02111, USA.
*/

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
