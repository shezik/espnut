#pragma once

#include <Arduino.h>
#include "proc_nut.h"
#include "U8g2lib.h"
#include "Configuration.h"
#include XBM_FONT_FILENAME

class DispInterface {
    protected:
        U8G2_DISPLAY_TYPE &u8g2;
        bool lowBat = false;
        void drawSegments(segment_bitmap_t *);
    public:
        DispInterface(U8G2_DISPLAY_TYPE &);
        void updateDisplay(nut_reg_t *, bool = false);
        void drawAndSendDialog(char *);
        void setU8g2PowerSave(uint8_t);
        void setLowBatAnnunciator(bool);
};
