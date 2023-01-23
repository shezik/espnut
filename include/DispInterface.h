#pragma once

#include <Arduino.h>
#include "proc_nut.h"
#include "U8g2lib.h"
#include "Configuration.h"

class DispInterface {
    protected:
        U8G2_DISPLAY_TYPE &u8g2;
    public:
        DispInterface(U8G2_DISPLAY_TYPE &);
        void updateDisplay(nut_reg_t *);
        void drawAndSendDialog(char *);
};
