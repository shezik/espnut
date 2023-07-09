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
#include "SettingsMgr.h"
#include "KeyboardMgr.h"
#include "DispInterface.h"
#include <esp_adc_cal.h>
#include <driver/ledc.h>
#include "SettingsMgr.h"

// Calculated by hand, not tested with timer initialized on frequency other than 240 MHz.
#define convertTime(ms) ((uint16_t) round(( 61.0 / 15640.0 * getCpuFrequencyMhz() + 25.0 / 391.0) * 240.0 / frequency * ms))

class PowerMgr {
    protected:
        SettingsMgr &sm;
        KeyboardMgr &kbdMgr;
        DispInterface &dp;
        uint8_t wakeUpInterruptPin;
        uint8_t LDOEnablePin;
        uint8_t displayBacklightPin;
        uint8_t batLvlChk;
        uint8_t batChrg;
        esp_adc_cal_characteristics_t *adcCalCharacteristics = nullptr;
        uint32_t backlightTimeout;  // In ms
        int64_t nextBacklightOff = 0;   // In ms
        uint32_t deepSleepTimeout;  // In ms
        int64_t nextDeepSleep = 0;      // In ms
        ledc_timer_config_t ledcTimerConf = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .duty_resolution = LEDC_TIMER_13_BIT,
            .timer_num = LEDC_TIMER_0,
            .freq_hz = 5000,
            .clk_cfg = LEDC_AUTO_CLK
        };
        ledc_channel_config_t ledcChannelConf = {
            .gpio_num = 0,
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel = LEDC_CHANNEL_0,
            .intr_type = LEDC_INTR_DISABLE,
            .timer_sel = LEDC_TIMER_0,
            .duty = 0,
            .hpoint = 0,
            .flags = {
                .output_invert = 0
            }
        };
        uint32_t frequency;
        uint32_t ledDutyCycle;
        bool wokenUp = false;
        void (*deepSleepCallback)() = nullptr;
        void (*batPercentChangedCallback)() = nullptr;
        SemaphoreHandle_t keyPressSignal = NULL;
        static PowerMgr *context;

    public:
        PowerMgr(SettingsMgr &, KeyboardMgr &, DispInterface &, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
        ~PowerMgr();

        bool enterModemSleep();
        void enterDeepSleep() __attribute__ ((noreturn));
        bool wokenUpFromDeepSleep();
        static void applySettings();
        void init();
        void tick();

        void enableLDO(bool);

        void setBacklightPower(bool, bool = false);

        uint32_t getBacklightTimeout();
        void setBacklightTimeout(uint32_t);
        void feedBacklightTimeout();

        uint32_t getDeepSleepTimeout();
        void setDeepSleepTimeout(uint32_t);
        void feedDeepSleepTimeout();

        bool getBatteryCharging();
        uint8_t getBatteryPercentage();

        bool setFrequency(uint32_t);
        bool reduceFrequency();
        bool restoreFrequency();
        bool isFrequencyReduced();

        void setBrightnessPercent(uint8_t);
        
        void registerDeepSleepCallback(void (*)());
        void registerBatPercentChangedCallback(void (*)());
        static void keyPressCallback();
};
