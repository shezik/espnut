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
    

    if (!loadSettings()) {
        loadDefaultSettings();
        saveSettings();
    }
    applySettings();
}

bool Menu::tick() {
    if (showingMenu) {
        if (gem->readyForKey()) {
            // !! Translate keycode blah blah
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
    context->saveSettingsBtn->setReadonly(false);
    // !! Redraw?
    context->applySettings();
}

void Menu::saveButtonCallback() {
    context->saveSettingsBtn->setReadonly(true);
    // !! Redraw?
    // Settings are already applied on the go
    context->saveSettings();
}

void Menu::resetSettingsButtonCallback() {
    context->loadDefaultSettings();
    context->applySettings();
    context->saveSettings();
    // !! Redraw menu here?
}

void Menu::exitSettingsPageCallback() {
    if (!context->saveSettingsBtn->getReadonly()) {
        // Then restore previous settings
        context->loadSettings();
        context->applySettings();
    }
}

void Menu::enterMenu() {
    showingMenu = true;
    kbdMgr.clear();

    bool isProcessorPresent = emu.isProcessorPresent();
    resumeBtn->hide(!isProcessorPresent);
    saveStateBtn->hide(!isProcessorPresent);
    resetCPUBtn->hide(!isProcessorPresent);
    obdurateResetCPUBtn->hide(!isProcessorPresent);
    if (isProcessorPresent && strcmp(emu.romFilename, romFilename)) {
        // Change page title
        // mainPage->setTitle(blah blah);  // Keep buffer somewhere
    }
}

void Menu::exitMenu() {
    showingMenu = false;
    kbdMgr.clear();
}
