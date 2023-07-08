#include "DispInterface.h"

DispInterface *DispInterface::context = nullptr;

DispInterface::DispInterface(SettingsMgr &sm_, U8G2_DISPLAY_TYPE &u8g2_)
    : sm(sm_)
    , u8g2(u8g2_)
{
    context = this;
}

DispInterface::~DispInterface() {
    context = nullptr;
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

bool DispInterface::init() {
    remapSPIPins();
    u8g2.setBusClock(U8G2_BUS_CLK);
    bool result = u8g2.begin();
    applySettings();
    return sm.registerApplySettingsCallback(applySettings) && result;
}

void DispInterface::applySettings() {
    context->getU8g2()->setContrast(*context->sm.getContrast());
}

void DispInterface::drawSegments(segment_bitmap_t *ds, bool lowBat) {
    u8g2.setDrawColor(1);
    u8g2.setBitmapMode(1);
    u8g2.clearBuffer();

    for (uint8_t i = 0; i < VOYAGER_DISPLAY_DIGITS; i++) {
        if (!ds[i])
            continue;

        uint8_t xOffset = (DIGIT_X_SPACING + DIGIT_WIDTH) * i - (DIGIT_WIDTH - minus_width);  // Every digit is 11 pixels wide except for the minus sign which is 8 pixels wide.
        if (i) {  // i != 0
            for (uint8_t ptr = 0; ptr < 9; ptr++)
                if (ds[i] & (1 << ptr))
                    switch (ptr) {
                        case 0: u8g2.drawXBM(xOffset, 0, a_width, a_height, a_bits); break;
                        case 1: u8g2.drawXBM(xOffset, 0, b_width, b_height, b_bits); break;
                        case 2: u8g2.drawXBM(xOffset, 0, c_width, c_height, c_bits); break;
                        case 3: u8g2.drawXBM(xOffset, 0, d_width, d_height, d_bits); break;
                        case 4: u8g2.drawXBM(xOffset, 0, e_width, e_height, e_bits); break;
                        case 5: u8g2.drawXBM(xOffset, 0, f_width, f_height, f_bits); break;
                        case 6: u8g2.drawXBM(xOffset, 0, g_width, g_height, g_bits); break;
                        case 7: u8g2.drawXBM(xOffset, 0, h_width, h_height, h_bits); break;
                        case 8: u8g2.drawXBM(xOffset, 0, i_width, i_height, i_bits); break;
                    }
        } else if (ds[i] & (1 << 6))  // i == 0
            u8g2.drawXBM(0, 0, minus_width, minus_height, minus_bits);
    
        xOffset = ANNUNCIATOR_X_SPACING * (i >= 2 ? i - 2 : 0);  // First two digits do not contain annunciators
        if (ds[i] & SEGMENT_ANN)
            switch (i) {
                case  2: u8g2.drawXBM(xOffset,                                                                                                                          DIGIT_HEIGHT + ANNUNCIATOR_DIGIT_SPACING, user_width,  user_height,  user_bits);  break;
                case  3: u8g2.drawXBM(xOffset + user_width,                                                                                                             DIGIT_HEIGHT + ANNUNCIATOR_DIGIT_SPACING, f_fn_width,  f_fn_height,  f_fn_bits);  break;
                case  4: u8g2.drawXBM(xOffset + user_width + f_fn_width,                                                                                                DIGIT_HEIGHT + ANNUNCIATOR_DIGIT_SPACING, g_fn_width,  g_fn_height,  g_fn_bits);  break;
                case  5: u8g2.drawXBM(xOffset + user_width + f_fn_width + g_fn_width,                                                                                   DIGIT_HEIGHT + ANNUNCIATOR_DIGIT_SPACING, begin_width, begin_height, begin_bits); break;
                case  6: u8g2.drawXBM(xOffset + user_width + f_fn_width + g_fn_width + begin_width,                                                                     DIGIT_HEIGHT + ANNUNCIATOR_DIGIT_SPACING, g__width,    g__height,    g__bits);    break;
                case  7: u8g2.drawXBM(xOffset + user_width + f_fn_width + g_fn_width + begin_width - ANNUNCIATOR_DIGIT_SPACING,                                         DIGIT_HEIGHT + ANNUNCIATOR_DIGIT_SPACING, _rad_width,  _rad_height,  _rad_bits);  break;
                case  8: u8g2.drawXBM(xOffset + user_width + f_fn_width + g_fn_width + begin_width - ANNUNCIATOR_DIGIT_SPACING +  _rad_width,                           DIGIT_HEIGHT + ANNUNCIATOR_DIGIT_SPACING, dmy_width,   dmy_height,   dmy_bits);   break;
                case  9: u8g2.drawXBM(xOffset + user_width + f_fn_width + g_fn_width + begin_width - ANNUNCIATOR_DIGIT_SPACING +  _rad_width + dmy_width,               DIGIT_HEIGHT + ANNUNCIATOR_DIGIT_SPACING, c_ann_width, c_ann_height, c_ann_bits); break;
                case 10: u8g2.drawXBM(xOffset + user_width + f_fn_width + g_fn_width + begin_width - ANNUNCIATOR_DIGIT_SPACING +  _rad_width + dmy_width + c_ann_width, DIGIT_HEIGHT + ANNUNCIATOR_DIGIT_SPACING, prgm_width,  prgm_height,  prgm_bits);  break;
            }
    }

    if (lowBat)
        u8g2.drawXBM(0, 0, asterisk_width, asterisk_height, asterisk_bits);
}

// Also kicks display out of power save mode
void DispInterface::updateDisplay(nut_reg_t *nv, bool force) {
    static segment_bitmap_t lastSegments[VOYAGER_DISPLAY_DIGITS] = {0};
    bool lowBat = lowBatAnnOverride || blinkTick();
    static bool lastLowBat = false;
    bool doUpdate = (lastLowBat != lowBat) ? true : memcmp(lastSegments, nv->display_segments, sizeof(lastSegments));
    memcpy(lastSegments, nv->display_segments, sizeof(lastSegments));
    
    if (doUpdate || force) {
        lastLowBat = lowBat;
        drawSegments(nv->display_segments, lowBat);
        u8g2.sendBuffer();
    }
}

void DispInterface::sendCriticalMsg(char *message) {
    drawCheckerboard();  // Includes setDrawColor(1);

    u8g2.setFontMode(0);
    u8g2.setFont(u8g2_font_tom_thumb_4x6_mr);
    u8g2.drawStr(0, 0, message);  // !! Beautify this later
    u8g2.sendBuffer();
}

// Overrides blinking (does not clear it)
void DispInterface::setLowBatAnnunciator(bool lowBat_) {
    lowBatAnnOverride = lowBat_;
}

bool DispInterface::blinkTick() {
    int64_t timeNow = get_timer_ms();
    static int64_t lastTime = 0;
    static bool lastState = false;

    if (!blinkMs)
        return false;

    if (timeNow - lastTime >= blinkMs) {
        lastTime = timeNow;
        lastState = !lastState;
    }
    return lastState;
}

// Set to 0 to disable blinking
void DispInterface::setLowBatAnnunciatiorBlink(uint16_t ms) {
    blinkMs = ms;
}

U8G2_DISPLAY_TYPE *DispInterface::getU8g2() {
    return &u8g2;
}

void DispInterface::drawBattery(uint8_t x, uint8_t y, uint8_t percentage, bool charging) {
    u8g2.setDrawColor(1);
    u8g2.drawLine(x, y + 1, x, y + 3);
    u8g2.drawBox(x + 1, y, 10, 5);
    u8g2.setDrawColor(0);
    if (charging)
        percentage = 0;
    uint8_t batLvlWidth = floor((100 - percentage) / 12);
    u8g2.drawBox(x + 2, y + 1, batLvlWidth, 3);
    if (charging) {
        u8g2.setDrawColor(1);
        u8g2.drawLine(x + 5, y + 1, x + 5, y + 2);
        u8g2.drawLine(x + 6, y + 2, x + 6, y + 3);
    }
}

void DispInterface::drawCheckerboard(bool polarity) {
    u8g2.setDrawColor(1);
    for (uint8_t y = 0; y < u8g2.getDisplayHeight(); y++)
        for (uint8_t x = ((y + (uint8_t) polarity) % 2); x < u8g2.getDisplayWidth(); x += 2)
            u8g2.drawPixel(x, y);
}

void DispInterface::sendLowBattery() {
    drawCheckerboard();
    uint8_t x = floor((u8g2.getDisplayWidth() - batteryIconWidth) / 2), y = floor((u8g2.getDisplayHeight() - batteryIconHeight) / 2);
    u8g2.setDrawColor(0);
    u8g2.drawBox(x - 1, y - 1, batteryIconWidth + 2, batteryIconHeight + 2);
    drawBattery(x, y, 0, false);
    u8g2.sendBuffer();
}
