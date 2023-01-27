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
        nut_reg_t *nv = nullptr;
        void sim_run();
        double wordsPerMs;
        int64_t lastRunTime;
        bool displayStateStabilized = false;
        bool frequencyReduced = false;
        char romFilename[32] = {0};
        int ramSize = NULL;
        static NutEmuInterface *context;
    public:
        NutEmuInterface(KeyboardMgr &, DispInterface &, PowerMgr &);
        ~NutEmuInterface();
        bool newProcessor(int, int = NULL, char * = nullptr);
        void tick();
        bool saveState(char *);
        bool loadState(char *, bool = false);

        void updateDisplayCallback();
        void setDisplayPowerSave(bool);

        bool checkRestoreFlag();
        void resetProcessor(bool);
        void deepSleepPrepare();

        void deinit();
        static void deepSleepCallback();
};
