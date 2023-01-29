#include "menu.h"

Menu *Menu::context = nullptr;

Menu::Menu(KeyboardMgr &kbdMgr_, U8G2_DISPLAY_TYPE &u8g2_, PowerMgr &pm_, NutEmuInterface &emu_)
    : kbdMgr(kbdMgr_)
    , u8g2(u8g2_)
    , pm(pm_)
    , emu(emu_)
{
    context = this;
}

Menu::~Menu() {
    context = nullptr;
    // Free GEM objects here
    delete gem; gem = nullptr;
    delete mainPage; mainPage = nullptr;
    delete resumeBtn; resumeBtn = nullptr;
    delete saveStateBtn; saveStateBtn = nullptr;
    delete loadStateBtn; loadStateBtn = nullptr;
    delete resetCPUBtn; resetCPUBtn = nullptr;
    delete obdurateResetCPUBtn; obdurateResetCPUBtn = nullptr;
    delete loadROMBtn; loadROMBtn = nullptr;
    delete showLogfileBtn; showLogfileBtn = nullptr;
    delete settingsBtn; settingsBtn = nullptr;
    delete powerOffBtn; powerOffBtn = nullptr;
    delete settingsPage; settingsPage = nullptr;
    delete contrastItem; contrastItem = nullptr;
    delete backlightTimeoutItem; backlightTimeoutItem = nullptr;
    delete powerOffTimeoutItem; powerOffTimeoutItem = nullptr;
    delete unlockEmulationSpeedItem; unlockEmulationSpeedItem = nullptr;
    delete enableLoggingItem; enableLoggingItem = nullptr;
    delete clearLogfileBtn; clearLogfileBtn = nullptr;
    delete saveSettingsBtn; saveSettingsBtn = nullptr;
    delete resetSettingsBtn; resetSettingsBtn = nullptr;
    delete exitSettingsBtn; exitSettingsBtn = nullptr;
    delete fileManagerPage; fileManagerPage = nullptr;
    for (int i = 0; i < sizeof(fileList) / sizeof(GEMItem *); i++) {
        if (fileList[i]) {
            delete fileList[i];
            fileList[i] = nullptr;
        }
    }
}

void Menu::init() {
    // Allocate GEM objects here
    // Any one-line function is written as lambda expression
    gem = new GEM_u8g2(u8g2 /*!! More config here*/);
    mainPage = new GEMPage(generateMainPageTitle(), exitMenu);
    resumeBtn = new GEMItem("Resume", exitMenu);
    saveStateBtn = new GEMItem("Save State", [](){});  // These are placeholders.
    loadStateBtn = new GEMItem("Load State", [](){});
    resetCPUBtn = new GEMItem("Reset CPU", [](){context->emu.resetProcessor();});
    obdurateResetCPUBtn = new GEMItem("Reset CPU & Memory", [](){context->emu.resetProcessor(true);});
    loadROMBtn = new GEMItem("Load ROM", [](){});
    showLogfileBtn = new GEMItem("Logs", [](){});
    settingsBtn = new GEMItem("Settings", [](){});
    powerOffBtn = new GEMItem("Power Off", [](){context->pm.enterDeepSleep();});
    settingsPage = new GEMPage("Settings", [](){exitSettingsPageCallback(false);});
    contrastItem = new GEMItem("Contrast", contrast);
    backlightTimeoutItem = new GEMItem("Backlight Timeout (sec)", backlightTimeoutSec);
    powerOffTimeoutItem = new GEMItem("Power Off Timeout (min)", powerOffTimeoutMin);
    unlockEmulationSpeedItem = new GEMItem("Unlock Speed", unlockSpeed);
    enableLoggingItem = new GEMItem("Logging", enableLogging);
    clearLogfileBtn = new GEMItem("Clear Logs", [](){});
    saveSettingsBtn = new GEMItem("Exit & Save", exitSettingsPageCallback, true);
    exitSettingsBtn = new GEMItem("Exit w/o Saving", exitSettingsPageCallback, false);
    resetSettingsBtn = new GEMItem("Reset All", resetSettingsButtonCallback);
    fileManagerPage = new GEMPage("Pick a file...", [](){});

    if (!loadSettings()) {
        loadDefaultSettings();
        saveSettings();
    }
    applySettings();
}

bool Menu::tick() {
    static int keycode;

    if (showingMenu) {
        if (gem->readyForKey()) {
            keycode = kbdMgr.getPositiveKeycode();  // Key release is of no use here
            switch (keycode) {
                // !! Translate keycode blah blah
                default:
                    ;
            }
        }
    } else if (kbdMgr.count() && kbdMgr.getLastKeycode() == 24) {  // 'ON' keycode
        holdDownCyclesCount++;
        if (holdDownCyclesCount == HOLD_DOWN_CYCLES) {
            holdDownCyclesCount = 0;
            enterMenu();
        } else {
            return false;  // Pause emulator execution for now
        }
    } else if (holdDownCyclesCount) {
        holdDownCyclesCount = 0;
    }

    return !showingMenu;  // Acts as emulator run flag
}

bool Menu::loadSettings() {
    File file = LittleFS.open(CONFIG_FILENAME, "r");
    if (!file)
        return false;

    contrast = file.read();
    backlightTimeoutSec = file.read();
    powerOffTimeoutMin = file.read();
    unlockSpeed = (bool) file.read();
    enableLogging = (bool) file.read();

    file.close();
    return true;
}

void Menu::applySettings() {
    u8g2.setContrast(contrast);
    pm.setBacklightTimeout(backlightTimeoutSec * 1000);
    pm.setDeepSleepTimeout(powerOffTimeoutMin * 1000 * 60);
    // !! emu.setUnlockSpeed(unlockSpeed);
    showLogfileBtn->hide(!enableLogging);
    clearLogfileBtn->hide(!enableLogging);
    if (!enableLogging) {
        // !! clear logfile here
    }
    // !! enable log saving
}

bool Menu::saveSettings() {
    File file = LittleFS.open(CONFIG_FILENAME, "w");
    if (!file)
        return false;

    file.write(contrast);
    file.write(backlightTimeoutSec);
    file.write(powerOffTimeoutMin);
    file.write((uint8_t) unlockSpeed);
    file.write((uint8_t) enableLogging);
    
    file.close();
    return true;
}

void Menu::loadDefaultSettings() {
    contrast = FALLBACK_CONTRAST;
    backlightTimeoutSec = FALLBACK_BACKLIGHT_TIMEOUT;
    powerOffTimeoutMin = FALLBACK_DEEP_SLEEP_TIMEOUT;
    unlockSpeed = FALLBACK_UNLOCK_SPEED;
    enableLogging = FALLBACK_ENABLE_LOGGING;
}

void Menu::settingsChangedCallback() {
    context->applySettings();
}

void Menu::resetSettingsButtonCallback() {
    context->loadDefaultSettings();
    context->applySettings();
    context->saveSettings();
    // !! Redraw menu here?
}

void Menu::exitSettingsPageCallback(bool doSave) {
    if (doSave) {
        context->saveSettings();
    } else {
        // Then restore previous settings
        context->loadSettings();
        context->applySettings();
    }
    // !! Go back to main menu
}

void Menu::exitSettingsPageCallback(GEMCallbackData callbackData) {
    exitSettingsPageCallback(callbackData.valBool);
}

void Menu::enterMenu() {
    showingMenu = true;
    kbdMgr.clear();
    kbdMgr.skipReleaseCheck();

    bool isProcessorPresent = emu.isProcessorPresent();
    resumeBtn->hide(!isProcessorPresent);
    saveStateBtn->hide(!isProcessorPresent);
    resetCPUBtn->hide(!isProcessorPresent);
    obdurateResetCPUBtn->hide(!isProcessorPresent);
    mainPage->setTitle(generateMainPageTitle());
}

void Menu::exitMenu() {
    context->showingMenu = false;
    context->u8g2.clear();
    context->kbdMgr.clear();
    context->kbdMgr.skipReleaseCheck();
}

char *Menu::generateMainPageTitle() {
    static char title[MAIN_PAGE_TITLE_LENGTH];

    if (emu.isProcessorPresent())
        snprintf(title, sizeof(title), "espnut v" VERSION " - %s", emu.getRomFilename());
    else
        snprintf(title, sizeof(title), "espnut v" VERSION);

    return title;
}
