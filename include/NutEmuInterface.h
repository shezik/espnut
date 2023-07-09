/*
 Copyright (C) 2023  shezik
 
 espnut is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License version 2 as
 published by the Free Software Foundation.  Note that the permission
 to redistribute or modify espnut under the terms of any later
 version of the General Public License is denied by the author
 of Nonpareil, Eric L. Smith, according to his notice.
 
 espnut is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program (in the file "COPYING" or "LICENSE"); if not,
 write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 Boston, MA 02111, USA.
*/

#pragma once

#include <Arduino.h>
#include "SettingsMgr.h"
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
        SettingsMgr &sm;
        KeyboardMgr &kbdMgr;  // Named differently to avoid collision with keyboardMgr in main.cpp
        DispInterface &disp;
        PowerMgr &pm;
        nut_reg_t *nv = nullptr;
        double wordsPerMs;
        int64_t lastRunTime;
        bool displayEnabled = false;
        int ramSize = 0;
        static NutEmuInterface *context;
        char romFilename[ROM_FILENAME_LENGTH] = {0};
        bool emulatorRunFlag = false;
        bool enablePowerMgmt = true;
        bool unlockSpeed = false;

        void sim_run();
        bool loadState(char *, bool, bool);
        bool loadMetadataFromStatefile(char *);

        // 2-key rollover
        uint16_t keyPressedFirst;
        void keyPressed(uint16_t);
        void keyReleased(uint16_t);
        std::set<uint16_t> keysPressedSet;
        // Executed every tick before sim_run(), overrides key detection, and keeps emulator from sleeping if not nullptr.
        void (*tickActionOverride)() = nullptr;
    public:
        NutEmuInterface(SettingsMgr &, KeyboardMgr &, DispInterface &, PowerMgr &);
        ~NutEmuInterface();
        bool init();
        static void applySettings();
        bool newProcessor(int, int, char *);
        bool newProcessorFromStatefile(int, char *);
        void tick();
        void pause();
        void resume();
        void wakeUpOnTick();
        bool saveState(char *);
        bool loadStateFromStatefile(char *);
        bool isProcessorPresent();
        char *getRomFilename();

        void updateDisplayCallback();
        void setEnablePowerMgmt(bool);
        void setUnlockSpeed(bool);

        // Attempt to load deep sleep restore file
        bool checkRestoreFlag();
        void resetProcessor(bool = false);
        void deepSleepPrepare();

        void deinit();
        static void deepSleepCallback();
};
