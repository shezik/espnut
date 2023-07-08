#pragma once

#include <Arduino.h>
#include "proc_nut.h"
#include "U8g2lib.h"
#include "Configuration.h"
#include XBM_FONT_FILENAME

class DispInterface {
    protected:
        U8G2_DISPLAY_TYPE &u8g2;
        uint16_t blinkMs = 0;
        bool lowBatAnnOverride = false;
        bool powerSaveState = false;
        void drawSegments(segment_bitmap_t *, bool);
        bool blinkTick();
    public:
        DispInterface(U8G2_DISPLAY_TYPE &);
        void updateDisplay(nut_reg_t *, bool = false);
        void sendCriticalMsg(char *);
        void setU8g2PowerSave(uint8_t);
        void setLowBatAnnunciator(bool);
        void setLowBatAnnunciatiorBlink(uint16_t);
        U8G2_DISPLAY_TYPE *getU8g2();
        void drawBattery(uint8_t, uint8_t, uint8_t, bool);
        void drawCheckerboard(bool = false);
        void sendLowBattery();
        const uint8_t batteryIconWidth = 11, batteryIconHeight = 5;
};
