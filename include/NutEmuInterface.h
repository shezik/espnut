#pragma once

#include <Arduino.h>
#include "KeyboardMgr.h"
#include "DispInterface.h"
#include "proc_nut.h"
#include "PowerMgr.h"

// In arch.c
#define ARCH_NUT_WORD_LENGTH 56
// In proc.c
#define JIFFY_PER_SEC 30
#define JIFFY_MSEC (1.0E3 / JIFFY_PER_SEC)  // i.e. ms per jiffy
#define MAX_INST_BURST 5000

#define NEXT_RUN_TIME (lastRunTime + JIFFY_MSEC)

class NutEmuInterface {
    protected:
        KeyboardMgr &kbdMgr;  // Named differently to avoid collision with keyboardMgr in main.cpp
        DispInterface &disp;
        PowerMgr &pm;
        nut_reg_t *nv;
        void sim_run();
        double wordsPerMs;
        int64_t lastRunTime;
    public:
        NutEmuInterface(KeyboardMgr &, DispInterface &, PowerMgr &);
        bool newProcessor(int, int, char *);
        void tick();
        bool saveState(char *);
        bool loadState(char *);
        void updateDisplayCallback();
};
