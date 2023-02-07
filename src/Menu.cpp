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
    freeFileList();
    delete ramSizePage; ramSizePage = nullptr;
    delete smallRAMBtn; smallRAMBtn = nullptr;
    delete largeRAMBtn; largeRAMBtn = nullptr;
}

void Menu::init(bool skipMenu) {
    showMenuFlag = !skipMenu;

    // Allocate GEM objects here
    // Any one-line function is written as lambda expression
    // It's quite interesting that static class methods can access protected members via the (class member) pointer 'context'.
    gem = new GEM_u8g2(u8g2 /*!! More config here*/);
    gem->init();

    mainPage = new GEMPage(generateMainPageTitle(), exitMenu);
    resumeBtn = new GEMItem("Resume", exitMenu);
    saveStateBtn = new GEMItem("Save State", saveStateButtonCallback);  // These are placeholders.
    loadStateBtn = new GEMItem("Load State", [](){context->fileSelectedCallback = loadStateFileSelectedCallback; context->enterFileManager("/");});
    resetCPUBtn = new GEMItem("Reset CPU", [](){context->emu.resetProcessor();});
    obdurateResetCPUBtn = new GEMItem("Reset CPU & Memory", [](){context->emu.resetProcessor(true);});
    loadROMBtn = new GEMItem("Load ROM", [](){context->fileSelectedCallback = loadROMFileSelectedCallback; context->enterFileManager("/");});
    showLogfileBtn = new GEMItem("Logs", [](){});
    settingsBtn = new GEMItem("Settings", settingsButtonCallback);
    powerOffBtn = new GEMItem("Power Off", [](){context->pm.enterDeepSleep();});
    mainPage->addMenuItem(*resumeBtn);
    mainPage->addMenuItem(*saveStateBtn);
    mainPage->addMenuItem(*loadStateBtn);
    mainPage->addMenuItem(*resetCPUBtn);
    mainPage->addMenuItem(*obdurateResetCPUBtn);
    mainPage->addMenuItem(*loadROMBtn);
    mainPage->addMenuItem(*showLogfileBtn);
    mainPage->addMenuItem(*settingsBtn);
    mainPage->addMenuItem(*powerOffBtn);
    settingsPage = new GEMPage("Settings", [](){exitSettingsPageCallback(false);});
    contrastItem = new GEMItem("Contrast", contrast, [](){context->applySettings();});
    backlightTimeoutItem = new GEMItem("Backlight Timeout (sec)", backlightTimeoutSec, [](){context->applySettings();});
    powerOffTimeoutItem = new GEMItem("Power Off Timeout (min)", powerOffTimeoutMin, [](){context->applySettings();});
    unlockEmulationSpeedItem = new GEMItem("Unlock Speed", unlockSpeed, [](){context->applySettings();});
    enableLoggingItem = new GEMItem("Enable Logging", enableLogging, [](){context->applySettings();});
    clearLogfileBtn = new GEMItem("Clear Logs", [](){});
    saveSettingsBtn = new GEMItem("Exit & Save", exitSettingsPageCallback, true);
    exitSettingsBtn = new GEMItem("Exit w/o Saving", exitSettingsPageCallback, false);
    resetSettingsBtn = new GEMItem("Reset All", resetSettingsButtonCallback);
    settingsPage->addMenuItem(*contrastItem);
    settingsPage->addMenuItem(*backlightTimeoutItem);
    settingsPage->addMenuItem(*powerOffTimeoutItem);
    settingsPage->addMenuItem(*unlockEmulationSpeedItem);
    settingsPage->addMenuItem(*enableLoggingItem);
    settingsPage->addMenuItem(*clearLogfileBtn);
    settingsPage->addMenuItem(*saveSettingsBtn);
    settingsPage->addMenuItem(*exitSettingsBtn);
    settingsPage->addMenuItem(*resetSettingsBtn);
    ramSizePage = new GEMPage("Select RAM size (80 for 15C, 40 otherwise)" /*!! Add cancel operation here*/);
    smallRAMBtn = new GEMItem("40", loadROMRAMSelectedCallback, 40);
    largeRAMBtn = new GEMItem("80", loadROMRAMSelectedCallback, 80);
    ramSizePage->addMenuItem(*smallRAMBtn);
    ramSizePage->addMenuItem(*largeRAMBtn);

    if (!loadSettings()) {
        loadDefaultSettings();
        saveSettings();
    }
    applySettings();
}

bool Menu::tick() {
    static uint8_t keycode;

    if (showMenuFlag) {
        showMenuFlag = false;
        enterMenu();
    }

    if (showingMenu)
        if (gem->readyForKey()) {
            keycode = kbdMgr.getPositiveKeycode();  // Key release is of no use here
            switch (keycode) {
                // !! Translate keycode blah blah
                default:
                    ;
            }
        }
    else if (kbdMgr.available() && kbdMgr.peekLastKeycode() == 24) {  // 'ON' keycode
        holdDownCyclesCount++;
        if (holdDownCyclesCount == HOLD_DOWN_CYCLES) {
            holdDownCyclesCount = 0;
            enterMenu();
        } else
            return false;  // Pause emulator execution for now
    } else if (holdDownCyclesCount)
        holdDownCyclesCount = 0;

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
    if (powerOffTimeoutMin < 1)
        powerOffTimeoutMin = 1;
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

void Menu::settingsButtonCallback() {
    context->gem->setMenuPageCurrent(*context->settingsPage);
    context->gem->drawMenu();
}

void Menu::resetSettingsButtonCallback() {
    context->loadDefaultSettings();
    context->applySettings();
    context->saveSettings();
    // !! Redraw menu here?
    context->gem->drawMenu();
}

void Menu::exitSettingsPageCallback(bool doSave) {
    if (doSave) {
        context->saveSettings();
    } else {
        // Then restore previous settings
        context->loadSettings();
        context->applySettings();
    }
    // Go back to main menu
    context->gem->setMenuPageCurrent(*context->mainPage);
    context->gem->drawMenu();
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
    context->saveStateBtn->setReadonly(false);

    gem->reInit();
    gem->setMenuPageCurrent(*mainPage);
    gem->drawMenu();
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

void Menu::enterFileManager(char *path) {
    // Two static char buffers to store data for callback functions
    static char filenameBuf[FILE_LIST_LENGTH][ROM_FILENAME_LENGTH] = {0};
    static char filePathBuf[FILE_LIST_LENGTH][FILE_PATH_LENGTH] = {0};
    static char upDirBuf[FILE_PATH_LENGTH] = {0};
    uint8_t itemCount = 0;

    // Clean up previous items
    delete fileManagerPage;
    freeFileList();

    if (!path) {  // Cancelled
        if (fileSelectedCallback)
            fileSelectedCallback(nullptr);
        return;
    }
    if (!LittleFS.exists(path)) {
        return;  // wtf
    }
    File dir = LittleFS.open(path, "r");
    if (!dir.isDirectory()) {
        dir.close();
        if (fileSelectedCallback)
            fileSelectedCallback(path);
        return;
    }

    fileManagerPage = new GEMPage("Pick a file...", [](){context->enterFileManager(nullptr);});

    if (strcmp("/", path)) {  // If path and "/" are not equal
        strncpy(upDirBuf, path, sizeof(upDirBuf) - 1);
        upDirBuf[sizeof(upDirBuf)] = '\0';
        dirGoUp(upDirBuf);
        fileList[FILE_LIST_LENGTH] = new GEMItem("..", enterFileManager, upDirBuf);  // fileList is 'FILE_LIST_LENGTH + 1' long, only this line and freeFileList() should be able to access this last index.
        fileManagerPage->addMenuItem(*fileList[FILE_LIST_LENGTH]);
    }

    while (1) {
        String path = dir.getNextFileName();  // Full path
        String name = pathToFileName(path.c_str());  // Short filename
        if (itemCount >= FILE_LIST_LENGTH || !name.length())
            break;
        
        strncpy(filenameBuf[itemCount], name.c_str(), sizeof(filenameBuf[itemCount]) - 1);
        filenameBuf[itemCount][sizeof(filenameBuf[itemCount])] = '\0';
        strncpy(filePathBuf[itemCount], path.c_str(), sizeof(filePathBuf[itemCount]) - 1);
        filePathBuf[itemCount][sizeof(filePathBuf[itemCount])] = '\0';

        fileList[itemCount] = new GEMItem(filenameBuf[itemCount], enterFileManager, filePathBuf[itemCount]);
        fileManagerPage->addMenuItem(*fileList[itemCount]);
        
        itemCount++;
    }

    dir.close();
    gem->setMenuPageCurrent(*fileManagerPage);
    gem->drawMenu();
}

void Menu::enterFileManager(GEMCallbackData callbackData) {
    context->enterFileManager((char *) callbackData.valPointer);
}

void Menu::dirGoUp(char *path) {  // First character must be '/'
    if (*path != '/' || strlen(path) == 1) {
        return;
    }
    char *pathEnd = path + strlen(path) - 1;
    pathEnd--;  // Skip the last character
    while (*pathEnd != '/')
        pathEnd--;

    *pathEnd = '\0';
    if (!strlen(path))
        *path = '/';
}

void Menu::freeFileList() {
    for (int i = 0; i < sizeof(fileList) / sizeof(GEMItem *); i++) {
        if (fileList[i]) {
            delete fileList[i];
            fileList[i] = nullptr;
        }
    }
}

void Menu::loadStateFileSelectedCallback(char *path) {
    if (!context->emu.isProcessorPresent()) {
        context->emu.loadState(path, true, false);
        context->emu.newProcessor(NUT_FREQUENCY_HZ);
    }
    context->emu.loadState(path, false, true);  // You can load a mismatching state file for giggles
    exitMenu();
}

void Menu::loadROMFileSelectedCallback(char *path) {
    context->selectedROMPath = path;
    context->gem->setMenuPageCurrent(*context->ramSizePage);
    context->gem->drawMenu();
}

void Menu::loadROMRAMSelectedCallback(GEMCallbackData callbackData) {
    context->emu.newProcessor(NUT_FREQUENCY_HZ, callbackData.valInt, context->selectedROMPath);
    exitMenu();
}

void Menu::saveStateButtonCallback() {
    char stateFilename[32];
    char randHex[9];  // uint32_t in hex, maximum FFFF FFFF
    char uptime[17];  // int64_t converted to uint64_t in hex, maximum 16 Fs
    snprintf(randHex, sizeof(randHex), "%08x", esp_random());  // Unfortunately RF subsystem is disabled, this generates pseudo-random numbers
    snprintf(uptime, sizeof(uptime), "%016lx", esp_timer_get_time());
    snprintf(stateFilename, sizeof(stateFilename), "/%s_%4.4s%s", context->emu.getRomFilename(), randHex, uptime + 12);
    context->emu.saveState(stateFilename);
    context->saveStateBtn->setReadonly(true);
    // !! Redraw?
}
