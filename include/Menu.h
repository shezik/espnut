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
#include "PowerMgr.h"
#include "NutEmuInterface.h"
#include "DispInterface.h"
#include "GEMProxy.h"

#define FILE_LIST_LENGTH 64
#define MAIN_PAGE_TITLE_LENGTH 64

#define FILE_PATH_LENGTH 128

class Menu {
    protected:
        SettingsMgr &sm;
        KeyboardMgr &kbdMgr;
        DispInterface &dp;
        PowerMgr &pm;
        NutEmuInterface &emu;

        GEMProxy *gem = nullptr;
        // Main page
        GEMPage *mainPage = nullptr;
        GEMItem *resumeBtn = nullptr;  // HIDE
        GEMItem *saveStateBtn = nullptr;  // HIDE
        GEMItem *loadStateBtn = nullptr;
        GEMItem *resetCPUBtn = nullptr;  // HIDE
        GEMItem *obdurateResetCPUBtn = nullptr;  // HIDE
        GEMItem *loadROMBtn = nullptr;
        GEMItem *deleteFileBtn = nullptr;
        GEMItem *showLogfileBtn = nullptr;  // HIDE
        GEMItem *settingsBtn = nullptr;
        GEMItem *powerOffBtn = nullptr;
        // Settings page
        GEMPage *settingsPage = nullptr;
        GEMItem *brightnessItem = nullptr;
        GEMItem *contrastItem = nullptr;
        GEMItem *backlightTimeoutItem = nullptr;
        GEMItem *powerOffTimeoutItem = nullptr;
        GEMItem *enablePowerMgmtItem = nullptr;
        GEMItem *unlockEmulationSpeedItem = nullptr;
        GEMItem *enableLoggingItem = nullptr;
        GEMItem *clearLogfileBtn = nullptr;  // HIDE
        GEMItem *saveSettingsBtn = nullptr;
        GEMItem *resetSettingsBtn = nullptr;
        GEMItem *exitSettingsBtn = nullptr;
        // File manager page
        GEMPage *fileManagerPage = nullptr;
        GEMItem *fileList[FILE_LIST_LENGTH + 1] = {0};  // The '+1' is reserved for the 'go up' button
        // RAM size picker
        GEMPage *ramSizePage = nullptr;
        GEMItem *smallRAMBtn = nullptr;
        GEMItem *largeRAMBtn = nullptr;
        
        bool showingMenu = false;
        bool showMenuFlag;

        void (*fileSelectedCallback)(char *) = nullptr;
        char *selectedROMPath;  // Used after selecting ROM file and before selecting RAM size.

        static Menu *context;

        void dirGoUp(char *);
        char *generateMainPageTitle();
        void freeFileList();

    public:
        Menu(SettingsMgr &, KeyboardMgr &, DispInterface &, PowerMgr &, NutEmuInterface &);
        ~Menu();
        void init(bool = true);
        static void applySettings();
        void tick();
        void enterMenu();
        void enterFileManager(char *);
        static void enterFileManager(GEMCallbackData);
        static void exitMenu();
        // static void clearLogfileCallback();
        static void settingsButtonCallback();
        static void resetSettingsButtonCallback();
        static void exitSettingsPageCallback(bool);
        static void exitSettingsPageCallback(GEMCallbackData);
        static void loadStateFileSelectedCallback(char *);
        static void loadROMFileSelectedCallback(char *);
        static void deleteSelectedFileCallback(char *);
        void loadROMRAMSelectedCallback(int);
        static void loadROMRAMSelectedCallback(GEMCallbackData);
        static void saveStateButtonCallback();
        // static void fileSelectedCallback();
        static void drawBatteryCallback();
};
