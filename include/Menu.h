#pragma once

#include <Arduino.h>
#include "GEM_u8g2.h"
#include "KeyboardMgr.h"
#include "PowerMgr.h"
#include "NutEmuInterface.h"
#include "Configuration.h"

#define FILE_LIST_LENGTH 64
#define MAIN_PAGE_TITLE_LENGTH 64

class Menu {
    protected:
        KeyboardMgr &kbdMgr;
        U8G2_DISPLAY_TYPE &u8g2;
        PowerMgr &pm;
        NutEmuInterface &emu;

        GEM_u8g2 *gem = nullptr;
        // Main page
        GEMPage *mainPage = nullptr;
        GEMItem *resumeBtn = nullptr;  // HIDE
        GEMItem *saveStateBtn = nullptr;  // HIDE
        GEMItem *loadStateBtn = nullptr;
        GEMItem *resetCPUBtn = nullptr;  // HIDE
        GEMItem *obdurateResetCPUBtn = nullptr;  // HIDE
        GEMItem *loadROMBtn = nullptr;
        GEMItem *showLogfileBtn = nullptr;  // HIDE
        GEMItem *settingsBtn = nullptr;
        GEMItem *powerOffBtn = nullptr;
        // Settings page
        GEMPage *settingsPage = nullptr;
        GEMItem *contrastItem = nullptr;
        GEMItem *backlightTimeoutItem = nullptr;
        GEMItem *powerOffTimeoutItem = nullptr;
        GEMItem *unlockEmulationSpeedItem = nullptr;
        GEMItem *enableLoggingItem = nullptr;
        GEMItem *clearLogfileBtn = nullptr;  // HIDE
        GEMItem *saveSettingsBtn = nullptr;
        GEMItem *resetSettingsBtn = nullptr;
        GEMItem *exitSettingsBtn = nullptr;
        // File manager page
        GEMPage *fileManagerPage = nullptr;
        GEMItem *fileList[FILE_LIST_LENGTH] = {0};

        uint8_t contrast;
        uint8_t backlightTimeoutSec;
        uint8_t powerOffTimeoutMin;
        bool unlockSpeed;
        bool enableLogging;

        uint16_t holdDownCyclesCount = 0;
        bool showingMenu = false;

        static Menu *context;

        char *generateMainPageTitle();
    public:
        Menu(KeyboardMgr &, U8G2_DISPLAY_TYPE &, PowerMgr &, NutEmuInterface &);
        ~Menu();
        void init();
        bool tick();  // Return value decides whether NutEmuInterface should tick
        bool loadSettings();  // Load settings from file to menu
        void applySettings();  // Apply settings in Menu to other classes
        bool saveSettings();
        void loadDefaultSettings();
        void enterMenu();
        static void exitMenu();
        // static void resetCPUCallback(GEMCallbackData);
        // static void clearLogfileCallback();
        static void settingsChangedCallback();
        static void resetSettingsButtonCallback();
        static void exitSettingsPageCallback(bool);
        static void exitSettingsPageCallback(GEMCallbackData);
        // static void fileSelectedCallback();
};
