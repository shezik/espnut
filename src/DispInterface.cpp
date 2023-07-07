#include "DispInterface.h"

DispInterface::DispInterface(U8G2_DISPLAY_TYPE &u8g2_)
    : u8g2(u8g2_)
{
    // Do nothing
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

void DispInterface::drawAndSendDialog(char *message) {
    u8g2.setFontMode(0);
    u8g2.setDrawColor(1);
    u8g2.setFont(u8g2_font_squeezed_r6_tr);

    for (uint8_t y = 0; y < u8g2.getDisplayHeight(); y++)
        for (uint8_t x = (y % 2); x < u8g2.getDisplayWidth(); x += 2)
            u8g2.drawPixel(x, y);

    u8g2.drawStr(0, 0, message);  // !! Beautify this later
    u8g2.sendBuffer();
}

void DispInterface::setU8g2PowerSave(uint8_t state) {
    u8g2.setPowerSave(state);
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
