#pragma once

#include <Arduino.h>
#include "GEM_u8g2.h"
#include "KeyboardMgr.h"
#include "PowerMgr.h"
#include "NutEmuInterface.h"
#include "Configuration.h"

#define FILE_LIST_LENGTH 64
#define MAIN_PAGE_TITLE_LENGTH 64

#define FILE_PATH_LENGTH 128

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
        GEMItem *fileList[FILE_LIST_LENGTH + 1] = {0};  // The '+1' is reserved for the 'go up' button
        // RAM size picker
        GEMPage *ramSizePage = nullptr;
        GEMItem *smallRAMBtn = nullptr;
        GEMItem *largeRAMBtn = nullptr;

        uint8_t contrast;
        uint8_t backlightTimeoutSec;
        uint8_t powerOffTimeoutMin;
        bool unlockSpeed;
        bool enableLogging;

        uint16_t holdDownCyclesCount = 0;
        bool showingMenu = false;
        bool showMenuFlag;

        void (*fileSelectedCallback)(char *) = nullptr;
        char *selectedROMPath;  // Used after selecting ROM file and before selecting RAM size.

        static Menu *context;

        void dirGoUp(char *);
        char *generateMainPageTitle();
        void freeFileList();
    public:
        Menu(KeyboardMgr &, U8G2_DISPLAY_TYPE &, PowerMgr &, NutEmuInterface &);
        ~Menu();
        void init(bool = false);
        bool tick();  // Return value decides whether NutEmuInterface should tick
        bool loadSettings();  // Load settings from file to menu
        void applySettings();  // Apply settings in Menu to other classes
        bool saveSettings();
        void loadDefaultSettings();
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
        static void loadROMRAMSelectedCallback(GEMCallbackData);
        static void saveStateButtonCallback();
        // static void fileSelectedCallback();
};
