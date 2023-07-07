#include <Arduino.h>
#include "util.h"
#include "U8g2lib.h"
#include "DispInterface.h"
#include "KeyboardMgr.h"
#include "PowerMgr.h"
#include "NutEmuInterface.h"
#include "Menu.h"
#include "MatrixKeyboard.h"
#include "Configuration.h"

U8G2_DISPLAY_TYPE u8g2(U8G2_R2, SPI_CS, SPI_DC, U8G2_RESET_PIN);
DispInterface dispInterface(u8g2);  // Referred to in util.h
KeyboardMgr keyboardMgr(POWER_BUTTON);  // Referred to in util.cpp
PowerMgr powerMgr(keyboardMgr, dispInterface, POWER_BUTTON, LDO_ENABLE, DISPLAY_BACKLIGHT_CONTROL, BAT_LVL_CHK, BAT_CHRG);
NutEmuInterface nutEmuInterface(keyboardMgr, dispInterface, powerMgr);
Menu menu(keyboardMgr, dispInterface, powerMgr, nutEmuInterface);

void appendLog(char *str) {
    
}

static void remapSPIPins() {
    // Reference: https://www.esp32.com/viewtopic.php?t=1929#p9108

    // Connect the pin to the GPIO matrix.
    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[SPI_DATA], PIN_FUNC_GPIO);
    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[SPI_CLK], PIN_FUNC_GPIO);
    // Set the direction. GPIO_MODE_INPUT always makes it an input, GPIO_MODE_OUTPUT always makes it an output, GPIO_MODE_INPUT_OUTPUT lets the peripheral decide what direction it has. You usually want the last one.
    gpio_set_direction((gpio_num_t) SPI_DATA, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction((gpio_num_t) SPI_CLK, GPIO_MODE_INPUT_OUTPUT);
    // Connect the output functionality of a peripheral to the pin you want. This allows a peripheral to set the direction of the pin (in case it's configured as GPIO_INPUT_OUTPUT) and set the output value.
    gpio_matrix_out(SPI_DATA, FSPID_OUT_IDX, false, false);
    gpio_matrix_out(SPI_CLK, FSPICLK_OUT_IDX, false, false);
    // Connect the input functionality of a peripheral to the pin. This allows the peripheral to read the signal indicated from this pin.
    gpio_matrix_in(SPI_DATA, FSPID_IN_IDX, false);
    gpio_matrix_in(SPI_CLK, FSPICLK_IN_IDX, false);
}

void setup() {
    Serial.begin(115200);

    powerMgr.init();  // Reset wakeUpInterruptPin pin mode, detect last deep sleep (ext0), keep LDO enabled, init and turn on backlight, set CPU frequency
    powerMgr.enterModemSleep();
    
    remapSPIPins();
    u8g2.setBusClock(U8G2_BUS_CLK);
    u8g2.begin();

    u8g2.setContrast(FALLBACK_CONTRAST);  // !! TODO: Settings manager
    powerMgr.tick();  // To check battery status

    keyboardMgr.init();  // Init and start matrix keyboard scanner task

    if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED))
        fatal(1, "Failed to init LittleFS.\n");

    bool isRestoreFlagPresent = nutEmuInterface.checkRestoreFlag();
    printf("Restore flag presence: %d\n", isRestoreFlagPresent);
    menu.init(!isRestoreFlagPresent);  // Load user settings into classes, skip showing main menu if restore file is successfully loaded    
}

void loop() {
    powerMgr.tick();
    menu.tick();
    nutEmuInterface.tick();
}
