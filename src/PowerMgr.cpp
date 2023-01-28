#include "PowerMgr.h"

PowerMgr *PowerMgr::context = nullptr;  // classic

PowerMgr::PowerMgr(KeyboardMgr &kbdMgr_, uint8_t wakeUpInterruptPin_, uint8_t displayPowerPin_, uint8_t displayBacklightPin_)
    : kbdMgr(kbdMgr_)
    , wakeUpInterruptPin(wakeUpInterruptPin_)
    , displayPowerPin(displayPowerPin_)
    , displayBacklightPin(displayBacklightPin_)
{
    context = this;
}

PowerMgr::~PowerMgr() {
    kbdMgr.registerKeyPressCallback(nullptr);
    context = nullptr;
}

void PowerMgr::keyPressCallback() {
    if (context) {
        context->feedBacklightTimeout();
        context->feedDeepSleepTimeout();
    }
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
    if (deepSleepCallback)
        deepSleepCallback();
    kbdMgr.disableInterrupt();
    kbdMgr.chipEnterSleep();
    setDisplayPower(false);
    setBacklightPower(false);
    adc_power_release();
    rtc_gpio_pullup_en((gpio_num_t) wakeUpInterruptPin);
    esp_sleep_enable_ext0_wakeup((gpio_num_t) wakeUpInterruptPin, 0);
    esp_deep_sleep_start();  // Does not return
}

bool PowerMgr::wokenUpFromDeepSleep() {
    return woken;
}

void PowerMgr::init() {
    setFrequency(FALLBACK_CPU_FREQUENCY_MHZ);  // !! Only if we had a config manager...

    // Deep Sleep Cleanup
    esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();
    if (wakeupReason == ESP_SLEEP_WAKEUP_EXT0) {
        woken = true;
        rtc_gpio_deinit((gpio_num_t) wakeUpInterruptPin);  // This one doesn't reset on wakeup according to https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/sleep_modes.html
    }

    pinMode(displayPowerPin, OUTPUT);
    pinMode(displayBacklightPin, OUTPUT);
    setDisplayPower(true);
    setBacklightTimeout(FALLBACK_BACKLIGHT_TIMEOUT * 1000);  // !! Should be removed after implementing ConfigMgr
    feedBacklightTimeout();
    setDeepSleepTimeout(FALLBACK_DEEP_SLEEP_TIMEOUT * 1000 * 60);  // !! Should be removed after implementing ConfigMgr
    feedDeepSleepTimeout();
    kbdMgr.registerKeyPressCallback(keyPressCallback);
}

void PowerMgr::tick() {
    int64_t timeNow = esp_timer_get_time();

    if (getBacklightPower() && timeNow >= nextBacklightOff)
        setBacklightPower(false);
    
    if (timeNow >= nextDeepSleep) {
        enterDeepSleep();
    }
}

bool PowerMgr::getDisplayPower() {
    return digitalRead(displayPowerPin);
}

void PowerMgr::setDisplayPower(bool state) {
    digitalWrite(displayPowerPin, state ? HIGH : LOW);
}

bool PowerMgr::getBacklightPower() {
    return digitalRead(displayBacklightPin);
}

void PowerMgr::setBacklightPower(bool state) {
    digitalWrite(displayBacklightPin, state ? HIGH : LOW);
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
    setBacklightPower(true);
    nextBacklightOff = esp_timer_get_time() + backlightTimeout;
}

uint32_t PowerMgr::getDeepSleepTimeout() {
    return deepSleepTimeout;
}

void PowerMgr::setDeepSleepTimeout(uint32_t ms) {
    nextDeepSleep += (ms - deepSleepTimeout);
    deepSleepTimeout = ms;
}

void PowerMgr::feedDeepSleepTimeout() {
    nextDeepSleep = esp_timer_get_time() + deepSleepTimeout;
}

bool PowerMgr::setFrequency(uint32_t freq) {
    if (setCpuFrequencyMhz(freq)) {
        frequency = freq;
        return true;
    } else
        return false;
}

bool PowerMgr::reduceFrequency() {
    bool result;
    switch (getXtalFrequencyMhz()) {
        case 40:
            result = setCpuFrequencyMhz(10);
            break;
        case 26:
            result = setCpuFrequencyMhz(13);
            break;
        case 24:
            result = setCpuFrequencyMhz(12);
            break;
        default:
            result = setCpuFrequencyMhz(80);
            break;
    }
    return result;
}

bool PowerMgr::restoreFrequency() {
    return setCpuFrequencyMhz(frequency);
}

void PowerMgr::registerDeepSleepCallback(void (*callback)()) {
    deepSleepCallback = callback;
}
