#pragma once

#include <Arduino.h>
#include "KeyboardMan.h"
#include "DispInterface.h"
#include "proc_nut.h"

class NutEmuInterface {
    protected:
        KeyboardMan &kbdMan;  // Named differently to avoid collision with keyboardMan in main.cpp
        DispInterface &disp;
        nut_reg_t *nv;
    public:
        NutEmuInterface(KeyboardMan &, DispInterface &);
        bool newProcessor(int, int, char *);
        void tick();
        bool saveState(char *);
        bool loadState(char *);
        void updateDisplayCallback();
};
