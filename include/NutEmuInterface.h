#pragma once

#include <Arduino.h>
#include "KeyboardMgr.h"
#include "DispInterface.h"
#include "proc_nut.h"
#include "PowerMgr.h"
#include <set>

// In arch.c
#define ARCH_NUT_WORD_LENGTH 56
// In proc.c
#define JIFFY_PER_SEC 30
#define JIFFY_MSEC (1.0E3 / JIFFY_PER_SEC)  // i.e. ms per jiffy
#define MAX_INST_BURST 5000

#define NEXT_RUN_TIME (lastRunTime + JIFFY_MSEC)
#define ROM_FILENAME_LENGTH 32

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
        int ramSize = NULL;
        static NutEmuInterface *context;
        char romFilename[ROM_FILENAME_LENGTH] = {0};

        // 2-key rollover
        uint16_t keyPressedFirst;
        void keyPressed(uint16_t);
        void keyReleased(uint16_t);
        std::set<uint16_t> keysPressedSet;
        void (*postponedKeyAction)() = nullptr;
    public:
        NutEmuInterface(KeyboardMgr &, DispInterface &, PowerMgr &);
        ~NutEmuInterface();
        bool newProcessor(int, int = NULL, char * = nullptr);
        void tick();
        // Call resume() when exiting menu and resuming emulator to avoid potential key detection related bugs
        void resume();
        bool saveState(char *);
        bool loadState(char *, bool, bool);
        bool isProcessorPresent();
        char *getRomFilename();

        void updateDisplayCallback();
        void setDisplayPowerSave(bool);

        bool checkRestoreFlag();
        void resetProcessor(bool = false);
        void deepSleepPrepare();

        void deinit();
        static void deepSleepCallback();
};
