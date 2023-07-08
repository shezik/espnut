#include "menu.h"

#define peanut_width 24
#define peanut_height 24
static unsigned char peanut_bits[] = {
   0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x0c, 0x03, 0x00, 0x22, 0x04, 0x00,
   0x02, 0x08, 0x00, 0x01, 0x11, 0x00, 0x01, 0x10, 0x00, 0x01, 0x20, 0x00,
   0x21, 0xc0, 0x00, 0x01, 0x00, 0x03, 0x02, 0x00, 0x0c, 0x06, 0x00, 0x10,
   0x0c, 0x00, 0x20, 0x30, 0x00, 0x42, 0xc0, 0x20, 0x40, 0x00, 0x01, 0x40,
   0x00, 0x02, 0x80, 0x00, 0x02, 0x82, 0x00, 0x04, 0x80, 0x00, 0x08, 0x80,
   0x00, 0x30, 0x40, 0x00, 0xc0, 0x31, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00 };

Menu *Menu::context = nullptr;

Menu::Menu(SettingsMgr &sm_, KeyboardMgr &kbdMgr_, DispInterface &dp_, PowerMgr &pm_, NutEmuInterface &emu_)
    : sm(sm_)
    , kbdMgr(kbdMgr_)
    , dp(dp_)
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
    delete deleteFileBtn; deleteFileBtn = nullptr;
    delete showLogfileBtn; showLogfileBtn = nullptr;
    delete settingsBtn; settingsBtn = nullptr;
    delete powerOffBtn; powerOffBtn = nullptr;
    delete settingsPage; settingsPage = nullptr;
    delete brightnessItem; brightnessItem = nullptr;
    delete contrastItem; contrastItem = nullptr;
    delete backlightTimeoutItem; backlightTimeoutItem = nullptr;
    delete powerOffTimeoutItem; powerOffTimeoutItem = nullptr;
    delete enablePowerMgmtItem; enablePowerMgmtItem = nullptr;
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
    pm.registerBatPercentChangedCallback(nullptr);
}

void Menu::init(bool showMenuFlag_) {
    showMenuFlag = showMenuFlag_;

    // Allocate GEM objects here
    // Any one-line function is written as lambda expression
    // It's quite interesting that static class methods can access protected members via the (class member) pointer 'context'.
    mainPage = new GEMPage(generateMainPageTitle(), [](){if (context->emu.isProcessorPresent()) context->exitMenu();});
    resumeBtn = new GEMItem("Resume", exitMenu);
    saveStateBtn = new GEMItem("Save State", saveStateButtonCallback);  // These are placeholders.
    loadStateBtn = new GEMItem("Load State", [](){context->fileSelectedCallback = loadStateFileSelectedCallback; context->enterFileManager("/");});
    resetCPUBtn = new GEMItem("Reset CPU", [](){context->emu.resetProcessor();});
    obdurateResetCPUBtn = new GEMItem("Reset CPU & Memory", [](){context->emu.resetProcessor(true);});
    loadROMBtn = new GEMItem("Load ROM", [](){context->fileSelectedCallback = loadROMFileSelectedCallback; context->enterFileManager("/");});
    deleteFileBtn = new GEMItem("Delete file", [](){context->fileSelectedCallback = deleteSelectedFileCallback; context->enterFileManager("/");});
    showLogfileBtn = new GEMItem("Logs", [](){});
    settingsBtn = new GEMItem("Settings", settingsButtonCallback);
    powerOffBtn = new GEMItem("Power Off", [](){context->pm.enterDeepSleep();});
    mainPage->addMenuItem(*resumeBtn);
    mainPage->addMenuItem(*saveStateBtn);
    mainPage->addMenuItem(*loadStateBtn);
    mainPage->addMenuItem(*resetCPUBtn);
    mainPage->addMenuItem(*obdurateResetCPUBtn);
    mainPage->addMenuItem(*loadROMBtn);
    mainPage->addMenuItem(*deleteFileBtn);
    mainPage->addMenuItem(*showLogfileBtn);
    mainPage->addMenuItem(*settingsBtn);
    mainPage->addMenuItem(*powerOffBtn);
    settingsPage = new GEMPage("Settings", [](){exitSettingsPageCallback(false);});
    brightnessItem = new GEMItem("Brightness", *sm.getBrightnessPercent(), [](){context->sm.applySettings();});
    contrastItem = new GEMItem("Contrast", *sm.getContrast(), [](){context->sm.applySettings();});
    backlightTimeoutItem = new GEMItem("Backlight Timeout (sec)", *sm.getBacklightTimeoutSec(), [](){context->sm.applySettings();});
    powerOffTimeoutItem = new GEMItem("Power Off Timeout (min)", *sm.getPowerOffTimeoutMin(), [](){context->sm.applySettings();});
    enablePowerMgmtItem = new GEMItem("Power Mgmt", *sm.getEnablePowerMgmt(), [](){context->sm.applySettings();});
    unlockEmulationSpeedItem = new GEMItem("Unlock Speed", *sm.getUnlockSpeed(), [](){context->sm.applySettings();});
    enableLoggingItem = new GEMItem("Enable Logging", *sm.getEnableLogging(), [](){context->sm.applySettings();});
    clearLogfileBtn = new GEMItem("Clear Logs", [](){});
    saveSettingsBtn = new GEMItem("Exit & Save", exitSettingsPageCallback, true);
    exitSettingsBtn = new GEMItem("Exit w/o Saving", exitSettingsPageCallback, false);
    resetSettingsBtn = new GEMItem("Reset All", resetSettingsButtonCallback);
    settingsPage->addMenuItem(*brightnessItem);
    settingsPage->addMenuItem(*contrastItem);
    settingsPage->addMenuItem(*backlightTimeoutItem);
    settingsPage->addMenuItem(*powerOffTimeoutItem);
    settingsPage->addMenuItem(*enablePowerMgmtItem);
    settingsPage->addMenuItem(*unlockEmulationSpeedItem);
    settingsPage->addMenuItem(*enableLoggingItem);
    settingsPage->addMenuItem(*clearLogfileBtn);
    settingsPage->addMenuItem(*saveSettingsBtn);
    settingsPage->addMenuItem(*exitSettingsBtn);
    settingsPage->addMenuItem(*resetSettingsBtn);
    ramSizePage = new GEMPage("Pick RAM size (80 only for 15C)", [](){context->loadROMRAMSelectedCallback(0);});
    smallRAMBtn = new GEMItem("40", loadROMRAMSelectedCallback, 40);
    largeRAMBtn = new GEMItem("80", loadROMRAMSelectedCallback, 80);
    ramSizePage->addMenuItem(*smallRAMBtn);
    ramSizePage->addMenuItem(*largeRAMBtn);

    applySettings();
    sm.registerApplySettingsCallback(applySettings);

    gem = new GEMProxy(*dp.getU8g2(), GEM_POINTER_ROW, ITEMS_PER_PAGE /*!! More config here*/);
    gem->registerDrawMenuCallback(drawBatteryCallback);
    pm.registerBatPercentChangedCallback([](){context->drawBatteryCallback(); context->dp.getU8g2()->sendBuffer();});
    gem->setSplash(peanut_width, peanut_height, peanut_bits);
    if (!showMenuFlag)
        gem->setSplashDelay(0);
    gem->init();
}

void Menu::applySettings() {
    bool enableLogging = *context->sm.getEnableLogging();
    context->showLogfileBtn->hide(!enableLogging);
    context->clearLogfileBtn->hide(!enableLogging);
    if (!enableLogging) {
        // !! clear logfile here
    }
    // !! enable log saving
}

void Menu::tick() {
    if (showMenuFlag) {
        showMenuFlag = false;
        enterMenu();
    }

    if (showingMenu) {
        if (gem->readyForKey() && kbdMgr.keysAvailable()) {
            uint16_t keycode = GetKeycodeContent(kbdMgr.getPositiveKeycode());  // Key release is of no use here
            printf_log("Menu: Keycode: %d\n", keycode);
            switch (keycode) {
                case 116:  // 2
                    gem->registerKeyPress(GEM_KEY_UP);
                    break;
                case 117:  // .
                    gem->registerKeyPress(GEM_KEY_DOWN);
                    break;
                case 197:  // 0
                    gem->registerKeyPress(GEM_KEY_LEFT);
                    break;
                case 53:  // Sigma+
                    gem->registerKeyPress(GEM_KEY_RIGHT);
                    break;
                case 132:  // ENTER
                    gem->registerKeyPress(GEM_KEY_OK);
                    break;
                case 129:  // <-
                    gem->registerKeyPress(GEM_KEY_CANCEL);
                    break;
                default:
                    gem->registerKeyPress(GEM_KEY_NONE);
            }
        }
    } else {
        // -------- BEGINNING OF CRITICAL SECTION --------
        xSemaphoreTake(kbdMgr.getMutex(), portMAX_DELAY);
        if (kbdMgr.keysAvailable() == 1 && kbdMgr.peekLastKeycode() == MakeKeycodeFromCode(true, 24 /*ON*/)) {
            printf_log("Menu: ON key detected in background\n");
            uint16_t tempKeycode;
            uint16_t onKeycode = MakeKeycodeFromCode(true, 24 /*ON*/);
            QueueHandle_t queue = kbdMgr.getKeyQueue();

            kbdMgr.getLastKeycode();  // Remove 'ON' keycode, emptying the queue
            xSemaphoreGive(kbdMgr.getMutex());
            BaseType_t ret = xQueueReceive(queue, &tempKeycode, pdMS_TO_TICKS(HOLD_DOWN_LENGTH));
            xSemaphoreTake(kbdMgr.getMutex(), portMAX_DELAY);
            // Note that releasing a key also generates a keycode.
            if (ret == errQUEUE_EMPTY)
                enterMenu();
            else {
                // Give them back
                xQueueSendToFront(queue, &tempKeycode, 0);
                xQueueSendToFront(queue, &onKeycode, 0);
            }
        }
    }
}

void Menu::settingsButtonCallback() {
    context->gem->setMenuPageCurrent(*context->settingsPage);
    context->gem->drawMenu();
}

void Menu::resetSettingsButtonCallback() {
    context->sm.loadDefaults();
    context->sm.applySettings();
    context->sm.saveSettings();
    context->gem->drawMenu();
}

void Menu::exitSettingsPageCallback(bool doSave) {
    if (doSave) {
        context->sm.saveSettings();
    } else {
        // Then restore previous settings
        context->sm.loadSettings();
        context->sm.applySettings();
    }
    // Go back to main menu
    context->enterMenu();
}

void Menu::exitSettingsPageCallback(GEMCallbackData callbackData) {
    exitSettingsPageCallback(callbackData.valBool);
}

void Menu::enterMenu() {
    printf_log("Menu: Executing enterMenu\n");
    showingMenu = true;
    emu.pause();
    kbdMgr.clear();
    xSemaphoreGive(kbdMgr.getMutex());

    bool isProcessorPresent = emu.isProcessorPresent();
    resumeBtn->hide(!isProcessorPresent);
    saveStateBtn->hide(!isProcessorPresent);
    resetCPUBtn->hide(!isProcessorPresent);
    obdurateResetCPUBtn->hide(!isProcessorPresent);
    mainPage->setTitle(generateMainPageTitle());
    context->saveStateBtn->setReadonly(false);

    // gem->reInit();
    // u8g2.setContrast(contrast);  // GEM_uìg2::reInit causes U8g2 to reset contrast
    gem->setMenuPageCurrent(*mainPage);
    gem->drawMenu();
}

void Menu::exitMenu() {
    context->showingMenu = false;
    context->dp.getU8g2()->clear();
    context->kbdMgr.clear();
    context->emu.resume();

    // It is okay to leave a copy in the memory.
    // These will be free'd and reallocated upon every call to the enterFileManager function.
    /*
    delete fileManagerPage; fileManagerPage = nullptr;
    freeFileList();
    */
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
    delete fileManagerPage; fileManagerPage = nullptr;  // Must set to nullptr since cancelling will cause the new page not to be allocated.
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

    while (true) {
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
    if (*path != '/' || strlen(path) == 1)
        return;
    
    char *pathEnd = path + strlen(path) - 1;
    pathEnd--;  // Skip the last character
    while (*pathEnd != '/')
        pathEnd--;

    *pathEnd = '\0';
    if (!strlen(path)) {
        *path = '/';
        *(path + 1) = '\0';
    }
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
    if (!path) {  // Cancelled
        context->enterMenu();
        return;
    }

    if (!context->emu.isProcessorPresent()) {
        context->emu.newProcessorFromStatefile(NUT_FREQUENCY_HZ, path);
    }
    context->emu.loadStateFromStatefile(path);  // You can load a mismatching state file for giggles  // !! Show warning?
    exitMenu();
}

void Menu::loadROMFileSelectedCallback(char *path) {
    if (!path) {  // Cancelled
        context->enterMenu();
        return;
    }

    context->selectedROMPath = path;
    context->gem->setMenuPageCurrent(*context->ramSizePage);
    context->gem->drawMenu();
}

void Menu::deleteSelectedFileCallback(char *path) {
    if (!path) {  // Cancelled
        context->enterMenu();
        return;
    }

    LittleFS.remove(path);
    context->dirGoUp(path);  // Should be safe
    context->enterFileManager(path);
}

void Menu::loadROMRAMSelectedCallback(int romSize) {
    if (romSize) {
        emu.newProcessor(NUT_FREQUENCY_HZ, romSize, selectedROMPath);
        exitMenu();
    } else {
        dirGoUp(selectedROMPath);  // This actually modifies filePathBuf in enterFileManager, but I don't think it matters.
        enterFileManager(selectedROMPath);
    }
}

void Menu::loadROMRAMSelectedCallback(GEMCallbackData callbackData) {
    context->loadROMRAMSelectedCallback(callbackData.valInt);
}

void Menu::saveStateButtonCallback() {
    char stateFilename[32];
    char randHex[9];  // uint32_t in hex, maximum FFFF FFFF
    char uptime[17];  // int64_t converted to uint64_t in hex, maximum 16 Fs
    snprintf(randHex, sizeof(randHex), "%08x", esp_random());  // Unfortunately RF subsystem is disabled, this generates pseudo-random numbers
    snprintf(uptime, sizeof(uptime), "%016lx", get_timer_ms());
    snprintf(stateFilename, sizeof(stateFilename), "/%s_%4.4s%s", context->emu.getRomFilename(), randHex, uptime + 12);
    context->emu.saveState(stateFilename);
    context->saveStateBtn->setReadonly(true);
    // !! Redraw?
}

void Menu::drawBatteryCallback() {
    GEMPage *menuPageCurrent = context->gem->getMenuPageCurrent();
    if (!context->showingMenu || !menuPageCurrent || memcmp(menuPageCurrent->getTitle(), "espnut", 6))
        return;  // Not in main menu, don't draw
    context->dp.drawBattery(context->dp.getU8g2()->getDisplayWidth() - context->dp.batteryIconWidth - 1, 1, context->pm.getBatteryPercentage(), context->pm.getBatteryCharging());
    context->dp.getU8g2()->setDrawColor(1);  // Reset color!
}
