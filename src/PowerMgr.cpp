#include "PowerMgr.h"

PowerMgr::PowerMgr(KeyboardMan &kbdMan_, uint8_t wakeUpInterruptPin_, uint8_t displayPowerPin_)
    : kbdMan(kbdMan_)
    , wakeUpInterruptPin(wakeUpInterruptPin_)
    , displayPowerPin(displayPowerPin_)
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
    kbdMan.disableInterrupt();
    kbdMan.chipEnterSleep(true);
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
    setDisplayPower(true);
}

bool PowerMgr::getDisplayPower() {
    return digitalRead(displayPowerPin);
}

void PowerMgr::setDisplayPower(bool state) {
    digitalWrite(displayPowerPin, state ? HIGH : LOW);
}
