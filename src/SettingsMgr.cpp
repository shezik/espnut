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

#include "SettingsMgr.h"
#include "util.h"
#include "Configuration.h"

#define TAG "SettingsMgr: "

SettingsMgr::SettingsMgr(char *filePath_) {
    filePath = strdup(filePath_);
}

SettingsMgr::~SettingsMgr() {
    free(filePath); filePath = nullptr;
}

bool SettingsMgr::init() {
    if (!loadSettings())
        saveSettings();
    return true;
}

void SettingsMgr::loadDefaults() {
    brightness = FALLBACK_BRIGHTNESS;
    contrast = FALLBACK_CONTRAST;
    backlightTimeoutSec = FALLBACK_BACKLIGHT_TIMEOUT;
    powerOffTimeoutMin = FALLBACK_DEEP_SLEEP_TIMEOUT;
    enablePowerMgmt = FALLBACK_ENABLE_POWER_MGMT;
    unlockSpeed = FALLBACK_UNLOCK_SPEED;
    enableLogging = FALLBACK_ENABLE_LOGGING;
    cpuFrequency = FALLBACK_CPU_FREQUENCY_MHZ;
}

bool SettingsMgr::loadSettings() {
    File file = LittleFS.open(filePath, "r");
    if (!file) {
        printf_log(TAG "loadSettings: Failed to open file %s for reading, loading defaults\n", filePath);
        loadDefaults();
        return false;
    }

    brightness = file.read();
    printf_log(TAG "loadSettings: brightness: %d\n", brightness);
    contrast = file.read();
    printf_log(TAG "loadSettings: contrast: %d\n", contrast);
    backlightTimeoutSec = file.read();
    printf_log(TAG "loadSettings: backlightTimeoutSec: %d\n", backlightTimeoutSec);
    powerOffTimeoutMin = file.read();
    printf_log(TAG "loadSettings: powerOffTimeoutMin: %d\n", powerOffTimeoutMin);
    enablePowerMgmt = (bool) file.read();
    printf_log(TAG "loadSettings: enablePowerMgmt: %d\n", enablePowerMgmt);
    unlockSpeed = (bool) file.read();
    printf_log(TAG "loadSettings: unlockSpeed: %d\n", unlockSpeed);
    enableLogging = (bool) file.read();
    printf_log(TAG "loadSettings: enableLogging: %d\n", enableLogging);
    file.read((uint8_t *) &cpuFrequency, sizeof(uint32_t));
    printf_log(TAG "loadSettings: cpuFrequency: %d\n", cpuFrequency);

    file.close();
    return true;
}

bool SettingsMgr::saveSettings() {
    File file = LittleFS.open(filePath, "w");
    if (!file) {
        printf_log(TAG "saveSettings: Failed to open file %s for writing\n", filePath);
        return false;
    }

    file.write(brightness);
    file.write(contrast);
    file.write(backlightTimeoutSec);
    file.write(powerOffTimeoutMin);
    file.write((uint8_t) enablePowerMgmt);
    file.write((uint8_t) unlockSpeed);
    file.write((uint8_t) enableLogging);
    file.write((uint8_t *) &cpuFrequency, sizeof(uint32_t));
    
    file.close();
    return true;
}

void SettingsMgr::applySettings() {
    brightness = brightness > 100 ? 100 : brightness;
    contrast = contrast < 4 ? 4 : contrast;
    powerOffTimeoutMin = powerOffTimeoutMin < 1 ? 1 : powerOffTimeoutMin;
    for (listIterator = applySettingsCallbackList.begin(); listIterator != applySettingsCallbackList.end(); listIterator++) {
        (*listIterator)();
    }
}

bool SettingsMgr::registerApplySettingsCallback(void (*callback)()) {
    applySettingsCallbackList.push_back(callback);
    return true;
}

uint8_t *SettingsMgr::getBrightnessPercent() {
    return &brightness;
}

uint8_t *SettingsMgr::getContrast() {
    return &contrast;
}

uint8_t *SettingsMgr::getBacklightTimeoutSec() {
    return &backlightTimeoutSec;
}

uint8_t *SettingsMgr::getPowerOffTimeoutMin() {
    return &powerOffTimeoutMin;
}

bool *SettingsMgr::getEnablePowerMgmt() {
    return &enablePowerMgmt;
}

bool *SettingsMgr::getUnlockSpeed() {
    return &unlockSpeed;
}

bool *SettingsMgr::getEnableLogging() {
    return &enableLogging;
}

uint32_t *SettingsMgr::getCpuFrequency() {
    return &cpuFrequency;
}
