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

class Menu {
    protected:
        SettingsMgr &sm;
        KeyboardMgr &kbdMgr;
        DispInterface &dp;
        PowerMgr &pm;
        NutEmuInterface &emu;

        GEMProxy *gem = nullptr;
        // Main page
        GEMPageProxy *mainPage = nullptr;
        GEMItem *resumeBtn = nullptr;  // HIDE
        GEMItem *saveStateBtn = nullptr;  // HIDE
        GEMItem *loadStateBtn = nullptr;
        GEMItem *resetCPUBtn = nullptr;  // HIDE
        GEMItem *obdurateResetCPUBtn = nullptr;  // HIDE
        GEMItem *loadROMBtn = nullptr;
        GEMItem *deleteFileBtn = nullptr;
        GEMItem *showLogfileBtn = nullptr;  // HIDE
        GEMItem *settingsBtn = nullptr;
        GEMItem *aboutBtn = nullptr;
        GEMItem *powerOffBtn = nullptr;
        // Settings page
        GEMPageProxy *settingsPage = nullptr;
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
        GEMPageProxy *fileManagerPage = nullptr;
        GEMItem *fileList[FILE_LIST_LENGTH + 1] = {0};  // The '+1' is reserved for the 'go up' button
        uint8_t upDirBtnCursorPos = 0;
        // RAM size picker
        GEMPageProxy *ramSizePage = nullptr;
        GEMItem *smallRAMBtn = nullptr;
        GEMItem *largeRAMBtn = nullptr;
        // Filename editor
        GEMAppearance *editFilenamePageAppearance = nullptr;
        GEMPageProxy *editFilenamePage = nullptr;
        GEMItem *nameFieldItem = nullptr;
        GEMItem *acceptNameBtn = nullptr;
        // File delete confirmation page
        GEMPageProxy *confirmDeletePage = nullptr;
        GEMItem *cancelDeletionBtn = nullptr;
        GEMItem *acceptDeletionBtn = nullptr;
        
        bool showingMenu = false;
        bool showMenuFlag;

        void (*fileSelectedCallback)(char *) = nullptr;
        char *selectedROMPath;  // Used after selecting ROM file and before selecting RAM size.

        void (*editFilenameConfirmedCallback)(bool) = nullptr;
        char editFilenameBuffer[GEM_STR_LEN] = {0};

        char *pendingDeleteFilePath = nullptr;

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
        void enterFileManager(char *, bool = false, uint8_t = 0, uint8_t = 0);
        static void exitMenu();
        // static void clearLogfileCallback();
        static void settingsButtonCallback();
        static void resetSettingsButtonCallback();
        static void exitSettingsPageCallback(bool);
        static void aboutButtonCallback();
        static void loadStateFileSelectedCallback(char *);
        static void loadROMFileSelectedCallback(char *);
        void deleteFileCallback(char *);
        void loadROMRAMSelectedCallback(int);
        void saveStateButtonCallback();
        void editFilenameCallback();
        void stateFileRenamedCallback(bool);
        void deleteFileConfirmedCallback(bool);
        static void drawBatteryCallback();
};
