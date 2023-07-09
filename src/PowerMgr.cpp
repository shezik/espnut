#include <WiFi.h>
#include <driver/rtc_io.h>
#include <driver/adc.h>
#include "Configuration.h"
#include "PowerMgr.h"
#include "util.h"

#define TAG "PowerMgr: "

PowerMgr *PowerMgr::context = nullptr;  // classic

PowerMgr::PowerMgr(SettingsMgr &sm_, KeyboardMgr &kbdMgr_, DispInterface &dp_, uint8_t wakeUpInterruptPin_, uint8_t LDOEnablePin_, uint8_t displayBacklightPin_, uint8_t batLvlChk_, uint8_t batChrg_)
    : sm(sm_)
    , kbdMgr(kbdMgr_)
    , dp(dp_)
    , wakeUpInterruptPin(wakeUpInterruptPin_)
    , LDOEnablePin(LDOEnablePin_)
    , displayBacklightPin(displayBacklightPin_)
    , batLvlChk(batLvlChk_)
    , batChrg(batChrg_)
{
    context = this;
    ledcChannelConf.gpio_num = displayBacklightPin_;
}

PowerMgr::~PowerMgr() {
    kbdMgr.registerKeyPressCallback(nullptr);
    context = nullptr;
    delete adcCalCharacteristics; adcCalCharacteristics = nullptr;
}

void PowerMgr::keyPressCallback() {
    xSemaphoreGive(context->keyPressSignal);
}

bool PowerMgr::enterModemSleep() {
    bool result = true;
    result &= WiFi.disconnect(true, true);
    result &= WiFi.setSleep(WIFI_PS_MAX_MODEM);
    result &= WiFi.mode(WIFI_OFF);
    result &= btStop();
    return result;
}

void PowerMgr::enterDeepSleep() {
    setBacklightPower(false);

    if (deepSleepCallback)
        deepSleepCallback();

    #ifdef USE_ESP_DEEP_SLEEP
        // kbdMgr.disableInterrupt();
        setBacklightPower(false);
        adc_power_release();
        rtc_gpio_pullup_en((gpio_num_t) wakeUpInterruptPin);
        esp_sleep_enable_ext0_wakeup((gpio_num_t) wakeUpInterruptPin, 0);
        esp_deep_sleep_start();  // Does not return
    #else
        enableLDO(false);  // Turn off 3V3 power supply
        vTaskDelay(pdMS_TO_TICKS(1000));
        fatal(5, "Failed to power off LDO power supply!\n");
    #endif
}

bool PowerMgr::wokenUpFromDeepSleep() {
    return wokenUp;
}

void PowerMgr::applySettings() {
    context->setFrequency(*context->sm.getCpuFrequency());
    context->setBacklightTimeout(*context->sm.getBacklightTimeoutSec() * 1000);
    context->setBrightnessPercent(*context->sm.getBrightnessPercent());  // Get ledDutyCycle variable ready
    context->feedBacklightTimeout();
    context->setDeepSleepTimeout(*context->sm.getPowerOffTimeoutMin() * 1000 * 60);
    context->feedDeepSleepTimeout();
}

void PowerMgr::init() {
    pinMode(LDOEnablePin, OUTPUT);
    
    #ifdef USE_ESP_DEEP_SLEEP
        // Deep Sleep Cleanup
        esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();
        if (wakeupReason == ESP_SLEEP_WAKEUP_EXT0) {
            wokenUp = true;
            rtc_gpio_deinit((gpio_num_t) wakeUpInterruptPin);  // This one doesn't reset on wakeup according to https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/sleep_modes.html
        }
    #else
        enableLDO(true);  // Hold LDO 3V3 output
    #endif

    pinMode(batChrg, INPUT);
    pinMode(batLvlChk, ANALOG);
    esp_err_t ret = esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP_FIT);
    if (ret == ESP_OK) {
        printf_log(TAG "Enabling ADC software calibration\n");
        adcCalCharacteristics = new esp_adc_cal_characteristics_t;
        esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_11db, (adc_bits_width_t) ADC_WIDTH_BIT_DEFAULT, 0, adcCalCharacteristics);
    } else {
        printf_log(TAG "ADC software calibration not supported, voltage readout may be inaccurate\n");
    }
    adc1_config_width((adc_bits_width_t) ADC_WIDTH_BIT_DEFAULT);
    adc1_config_channel_atten(BAT_LVL_CHK_ADC_CHANNEL, ADC_ATTEN_11db);  // 0 mV ~ 3100 mV

    ledc_timer_config(&ledcTimerConf);
    ledc_channel_config(&ledcChannelConf);
    ledc_fade_func_install(0);

    applySettings();
    sm.registerApplySettingsCallback(applySettings);

    keyPressSignal = xSemaphoreCreateBinary();
    kbdMgr.registerKeyPressCallback(keyPressCallback);
}

void PowerMgr::tick() {
    if (xSemaphoreTake(keyPressSignal, 0) == pdPASS) {
        restoreFrequency();
        feedBacklightTimeout();
        feedDeepSleepTimeout();
    }

    int64_t timeNow = get_timer_ms();

    static int64_t nextBatteryCheck = 0;
    static uint8_t prevBatPercent = -1;  // Force update
    uint8_t batPercentNow;

    // printf_log(TAG "timeNow: %lld, nextBacklightOff: %lld, nextDeepSleep: %lld\n", timeNow, nextBacklightOff, nextDeepSleep);

    if (timeNow >= nextBatteryCheck) {
        batPercentNow = getBatteryPercentage();
        nextBatteryCheck = timeNow + 1000;
        if (batPercentNow != prevBatPercent) {
            prevBatPercent = batPercentNow;
            printf_log(TAG "Battery status updated: %d\n", batPercentNow);
            if (batPercentChangedCallback)
                batPercentChangedCallback();
            if (!batPercentNow) {
                printf_log(TAG "Low battery, going to deep sleep\n");
                dp.sendLowBattery();
                vTaskDelay(pdMS_TO_TICKS(LOW_BAT_SHUTDOWN_DELAY));
                enterDeepSleep();
            }
        }
    }

    if (timeNow >= nextBacklightOff || !backlightTimeout)
        setBacklightPower(false);
    
    if (deepSleepTimeout && timeNow >= nextDeepSleep)
        enterDeepSleep();

    // printf_log(TAG "Charging: %d, Battery: %d%, Calibrated: %d\n", getBatteryCharging(), getBatteryPercentage(), (bool) adcCalCharacteristics);
}

void PowerMgr::enableLDO(bool state) {
    digitalWrite(LDOEnablePin, state ? HIGH : LOW);
}

void PowerMgr::setBacklightPower(bool state, bool force) {
    // digitalWrite(displayBacklightPin, state ? HIGH : LOW);
    static bool prevState = false;
    if (!force && state == prevState)
        return;
    prevState = state;

    // Works the same as ledc_fade_stop() which is not available yet
    ledc_fade_func_uninstall();
    ledc_fade_func_install(0);

    printf_log(TAG "Freq: %d, orig. freq: %d; Fade time: 100 -> %d, 350 -> %d\n", getCpuFrequencyMhz(), frequency, convertTime(100), convertTime(350));
    ledc_set_fade_with_time(ledcChannelConf.speed_mode, ledcChannelConf.channel, state ? ledDutyCycle : 0, state ? convertTime(100) : convertTime(350));
    ledc_fade_start(ledcChannelConf.speed_mode, ledcChannelConf.channel, LEDC_FADE_NO_WAIT);
}

uint16_t PowerMgr::getBacklightTimeout() {
    return backlightTimeout;
}

void PowerMgr::setBacklightTimeout(uint32_t ms) {
    nextBacklightOff += (ms - backlightTimeout);  // Modify next backlight off time without feeding. No worries about sign.
    backlightTimeout = ms;
}

// Normally you turn on backlight with this function
// Of course you can manually override with setBacklightPower(true) but why?
void PowerMgr::feedBacklightTimeout() {
    if (!backlightTimeout)
        return;
    
    nextBacklightOff = get_timer_ms() + backlightTimeout;
    setBacklightPower(true);
}

uint32_t PowerMgr::getDeepSleepTimeout() {
    return deepSleepTimeout;
}

void PowerMgr::setDeepSleepTimeout(uint32_t ms) {
    nextDeepSleep += (ms - deepSleepTimeout);
    deepSleepTimeout = ms;
    // printf_log(TAG "nextDeepSleep set to %lld, deepSleepTimeout set to %lu\n", nextDeepSleep, deepSleepTimeout);
}

void PowerMgr::feedDeepSleepTimeout() {
    nextDeepSleep = get_timer_ms() + deepSleepTimeout;
}

bool PowerMgr::getBatteryCharging() {
    return !digitalRead(batChrg);
}

uint8_t PowerMgr::getBatteryPercentage() {
    float voltage;
    if (adcCalCharacteristics)
        voltage = esp_adc_cal_raw_to_voltage(adc1_get_raw(BAT_LVL_CHK_ADC_CHANNEL), adcCalCharacteristics) * (1 + RESISTOR_RATIO);
    else
        voltage = 3100 / 4095 * adc1_get_raw(BAT_LVL_CHK_ADC_CHANNEL) * (1 + RESISTOR_RATIO);
    int16_t percentage = round((voltage - BAT_MIN_VOLTAGE) / (BAT_MAX_VOLTAGE - BAT_MIN_VOLTAGE) * 100);
    return percentage > 100 ? 100 : percentage < 0 ? 0 : percentage;
}

bool PowerMgr::setFrequency(uint32_t freq) {
    if (setCpuFrequencyMhz(freq)) {
        frequency = freq;
        return true;
    }
    return false;
}

bool PowerMgr::reduceFrequency() {
    static uint32_t cachedTargetFreq = 0;

    if (!cachedTargetFreq) {
        switch (getXtalFrequencyMhz()) {
        case 40:
            cachedTargetFreq = 10;
            break;
        case 26:
            cachedTargetFreq = 13;
            break;
        case 24:
            cachedTargetFreq = 12;
            break;
        default:
            cachedTargetFreq = 80;
            break;
        }
    }

    if (getCpuFrequencyMhz() <= cachedTargetFreq)
        return true;
    
    return setCpuFrequencyMhz(cachedTargetFreq);
}

bool PowerMgr::restoreFrequency() {
    if (getCpuFrequencyMhz() == frequency)
        return true;

    return setCpuFrequencyMhz(frequency);
}

bool PowerMgr::isFrequencyReduced() {
    return getCpuFrequencyMhz() < frequency;
}

void PowerMgr::setBrightnessPercent(uint8_t brightness_) {
    ledDutyCycle = brightness_ / 100.0 * (1 << (uint8_t) ledcTimerConf.duty_resolution - 1);
    setBacklightPower((bool) backlightTimeout, true);
}

void PowerMgr::registerDeepSleepCallback(void (*callback)()) {
    deepSleepCallback = callback;
}

void PowerMgr::registerBatPercentChangedCallback(void (*callback)()) {
    batPercentChangedCallback = callback;
}
