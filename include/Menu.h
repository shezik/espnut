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
