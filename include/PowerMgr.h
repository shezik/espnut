#pragma once

#include <Arduino.h>
#include "KeyboardMgr.h"
#include "DispInterface.h"
#include <esp_adc_cal.h>
#include <driver/ledc.h>

class PowerMgr {
    protected:
        KeyboardMgr &kbdMgr;
        DispInterface &dp;
        uint8_t wakeUpInterruptPin;
        uint8_t LDOEnablePin;
        uint8_t displayBacklightPin;
        uint8_t batLvlChk;
        uint8_t batChrg;
        esp_adc_cal_characteristics_t *adcCalCharacteristics = nullptr;
        uint32_t backlightTimeout;  // In ms
        int64_t nextBacklightOff;   // In ms
        uint32_t deepSleepTimeout;  // In ms
        int64_t nextDeepSleep;      // In ms
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
        bool wokenUp = false;
        void (*deepSleepCallback)() = nullptr;
        void (*batPercentChangedCallback)() = nullptr;
        SemaphoreHandle_t keyPressSignal = NULL;
        static PowerMgr *context;
    public:
        PowerMgr(KeyboardMgr &, DispInterface &, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
        ~PowerMgr();

        bool enterModemSleep();
        void enterDeepSleep() __attribute__ ((noreturn));
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

        bool getBatteryCharging();
        uint8_t getBatteryPercentage();

        bool setFrequency(uint32_t);
        bool reduceFrequency();
        bool restoreFrequency();
        bool isFrequencyReduced();
        
        void registerDeepSleepCallback(void (*)());
        void registerBatPercentChangedCallback(void (*)());
        static void keyPressCallback();
};
