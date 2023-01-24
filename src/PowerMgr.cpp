#include "PowerMgr.h"

PowerMgr::PowerMgr(KeyboardMgr &kbdMgr_, uint8_t wakeUpInterruptPin_, uint8_t displayPowerPin_, uint8_t displayBacklightPin_)
    : kbdMgr(kbdMgr_)
    , wakeUpInterruptPin(wakeUpInterruptPin_)
    , displayPowerPin(displayPowerPin_)
    , displayBacklightPin(displayBacklightPin_)
{
    // Do nothing
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
    kbdMgr.disableInterrupt();
    kbdMgr.chipEnterSleep(true);
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
    // Deep Sleep Cleanup
    esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();
    if (wakeupReason == ESP_SLEEP_WAKEUP_EXT0) {
        woken = true;
        rtc_gpio_deinit((gpio_num_t) wakeUpInterruptPin);  // This one doesn't reset on wakeup according to https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/sleep_modes.html
    }

    pinMode(displayPowerPin, OUTPUT);
    pinMode(displayBacklightPin, OUTPUT);
    setDisplayPower(true);
    setBacklightTimeout(FALLBACK_BACKLIGHT_TIMEOUT);  // !! Should be removed after implementing ConfigMgr
    feedBacklightTimeout();
}

void PowerMgr::tick() {
    if (getBacklightPower() && esp_timer_get_time() >= nextBacklightOff)
        setBacklightPower(false);
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

void PowerMgr::setBacklightTimeout(uint16_t ms) {
    nextBacklightOff += (ms - backlightTimeout);  // Modify next backlight off time without feeding. No worries about sign.
    backlightTimeout = ms;
}

// Normally you turn on backlight with this function
// Of course you can manually override with setBacklightPower(true) but why?
void PowerMgr::feedBacklightTimeout() {
    setBacklightPower(true);
    nextBacklightOff = esp_timer_get_time() + backlightTimeout;
}
