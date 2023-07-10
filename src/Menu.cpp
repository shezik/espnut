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

#include "menu.h"
#include "Configuration.h"
#include <string>
#include <sstream>

#define peanut_width 24
#define peanut_height 24
static unsigned char peanut_bits[] = {
   0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x0c, 0x03, 0x00, 0x22, 0x04, 0x00,
   0x02, 0x08, 0x00, 0x01, 0x11, 0x00, 0x01, 0x10, 0x00, 0x01, 0x20, 0x00,
   0x21, 0xc0, 0x00, 0x01, 0x00, 0x03, 0x02, 0x00, 0x0c, 0x06, 0x00, 0x10,
   0x0c, 0x00, 0x20, 0x30, 0x00, 0x42, 0xc0, 0x20, 0x40, 0x00, 0x01, 0x40,
   0x00, 0x02, 0x80, 0x00, 0x02, 0x82, 0x00, 0x04, 0x80, 0x00, 0x08, 0x80,
   0x00, 0x30, 0x40, 0x00, 0xc0, 0x31, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00 };

#define TAG "Menu: "

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
    delete aboutBtn; aboutBtn = nullptr;
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
    delete editFilenamePage; editFilenamePage = nullptr;
    delete nameFieldItem; nameFieldItem = nullptr;
    delete acceptNameBtn; acceptNameBtn = nullptr;
    delete confirmDeletePage; confirmDeletePage = nullptr;
    delete cancelDeletionBtn; cancelDeletionBtn = nullptr;
    delete acceptDeletionBtn; acceptDeletionBtn = nullptr;
    pm.registerBatPercentChangedCallback(nullptr);
}

void Menu::init(bool showMenuFlag_) {
    showMenuFlag = showMenuFlag_;

    // Allocate GEM objects here
    // Any one-line function is written as lambda expression
    // It's quite interesting that static class methods can access protected members via the (class member) pointer 'context'.
    mainPage = new GEMPage(generateMainPageTitle(), [](){if (context->emu.isProcessorPresent()) context->exitMenu();});
    resumeBtn = new GEMItem("Resume", exitMenu);
    saveStateBtn = new GEMItem("Save State", [](){context->saveStateButtonCallback();});
    loadStateBtn = new GEMItem("Load State", [](){context->fileSelectedCallback = loadStateFileSelectedCallback; context->enterFileManager("/");});
    loadROMBtn = new GEMItem("Load ROM", [](){context->fileSelectedCallback = loadROMFileSelectedCallback; context->enterFileManager("/");});
    resetCPUBtn = new GEMItem("Reset CPU", [](){context->emu.resetProcessor();});
    obdurateResetCPUBtn = new GEMItem("Reset CPU & Memory", [](){context->emu.resetProcessor(true);});
    deleteFileBtn = new GEMItem("Delete file", [](){context->fileSelectedCallback = [](char *path){context->deleteFileCallback(path);}; context->enterFileManager("/");});
    showLogfileBtn = new GEMItem("Logs", [](){});
    settingsBtn = new GEMItem("Settings", settingsButtonCallback);
    aboutBtn = new GEMItem("About", aboutButtonCallback);
    powerOffBtn = new GEMItem("Power Off", [](){context->pm.enterDeepSleep();});
    mainPage->addMenuItem(*resumeBtn);
    mainPage->addMenuItem(*saveStateBtn);
    mainPage->addMenuItem(*loadStateBtn);
    mainPage->addMenuItem(*loadROMBtn);
    mainPage->addMenuItem(*resetCPUBtn);
    mainPage->addMenuItem(*obdurateResetCPUBtn);
    mainPage->addMenuItem(*deleteFileBtn);
    mainPage->addMenuItem(*showLogfileBtn);
    mainPage->addMenuItem(*settingsBtn);
    mainPage->addMenuItem(*aboutBtn);
    mainPage->addMenuItem(*powerOffBtn);
    settingsPage = new GEMPage("Settings", [](){exitSettingsPageCallback(false);});
    brightnessItem = new GEMItem("Brightness %", *sm.getBrightnessPercent(), [](){context->sm.applySettings();});
    contrastItem = new GEMItem("Contrast", *sm.getContrast(), [](){context->sm.applySettings();});
    backlightTimeoutItem = new GEMItem("Backlight (Sec)", *sm.getBacklightTimeoutSec(), [](){context->sm.applySettings();});
    powerOffTimeoutItem = new GEMItem("Power Off (Min)", *sm.getPowerOffTimeoutMin(), [](){context->sm.applySettings();});
    enablePowerMgmtItem = new GEMItem("Power Mgmt", *sm.getEnablePowerMgmt(), [](){context->sm.applySettings();});
    unlockEmulationSpeedItem = new GEMItem("Unlock Speed", *sm.getUnlockSpeed(), [](){context->sm.applySettings();});
    enableLoggingItem = new GEMItem("Logging", *sm.getEnableLogging(), [](){context->sm.applySettings();});
    clearLogfileBtn = new GEMItem("Clear Logs", [](){});
    saveSettingsBtn = new GEMItem("Exit & Save", [](GEMCallbackData data){context->exitSettingsPageCallback(data.valBool);}, true);
    exitSettingsBtn = new GEMItem("Exit w/o Saving", [](GEMCallbackData data){context->exitSettingsPageCallback(data.valBool);}, false);
    resetSettingsBtn = new GEMItem("Reset All", resetSettingsButtonCallback);
    settingsPage->addMenuItem(*brightnessItem);
    settingsPage->addMenuItem(*contrastItem);
    settingsPage->addMenuItem(*backlightTimeoutItem);
    settingsPage->addMenuItem(*powerOffTimeoutItem);
    settingsPage->addMenuItem(*enablePowerMgmtItem);
    settingsPage->addMenuItem(*unlockEmulationSpeedItem);
    // settingsPage->addMenuItem(*enableLoggingItem);  // !! Not implemented
    settingsPage->addMenuItem(*clearLogfileBtn);
    settingsPage->addMenuItem(*saveSettingsBtn);
    settingsPage->addMenuItem(*exitSettingsBtn);
    settingsPage->addMenuItem(*resetSettingsBtn);
    ramSizePage = new GEMPage("Pick RAM size (80 only for 15C)", [](){context->loadROMRAMSelectedCallback(0);});
    smallRAMBtn = new GEMItem("40", [](GEMCallbackData data){context->loadROMRAMSelectedCallback(data.valInt);}, 40);
    largeRAMBtn = new GEMItem("80", [](GEMCallbackData data){context->loadROMRAMSelectedCallback(data.valInt);}, 80);
    ramSizePage->addMenuItem(*smallRAMBtn);
    ramSizePage->addMenuItem(*largeRAMBtn);
    editFilenamePage = new GEMPage("Edit filename", [](){context->editFilenameConfirmedCallback(false);});
    nameFieldItem = new GEMItem("", editFilenameBuffer);
    acceptNameBtn = new GEMItem("Accept", [](){context->editFilenameConfirmedCallback(true);});
    editFilenamePage->addMenuItem(*nameFieldItem);
    editFilenamePage->addMenuItem(*acceptNameBtn);
    confirmDeletePage = new GEMPage("", [](){context->deleteFileConfirmedCallback(false);});
    cancelDeletionBtn = new GEMItem("Cancel", [](){context->deleteFileConfirmedCallback(false);});
    acceptDeletionBtn = new GEMItem("Accept", [](){context->deleteFileConfirmedCallback(true);});
    confirmDeletePage->addMenuItem(*cancelDeletionBtn);
    confirmDeletePage->addMenuItem(*acceptDeletionBtn);

    applySettings();
    sm.registerApplySettingsCallback(applySettings);

    gem = new GEMProxy(*dp.getU8g2(), GEM_POINTER_ROW, ITEMS_PER_PAGE, ITEM_HEIGHT, PAGE_TOP_OFFSET, VALUES_LEFT_OFFSET);
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
            printf_log(TAG "Keycode: %d\n", keycode);
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
            printf_log(TAG "ON key detected in background\n");
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

void Menu::aboutButtonCallback() {
    const uint8_t copyrightInfo[] = "\fespnut (c) 2023 shezik, licensed under GPLv2\n"
                                    "\fNonpareil (c) 1995, 2003, 2004, 2005 Eric L. Smith, licensed under GPLv2, a MODIFIED version is used\n"
                                    "\farduino-esp32 (c) 2015-2016 Espressif Systems (Shanghai) PTE LTD, licensed under Apache License v2\n"
                                    "\fGEM (c) 2018-2022 Alexander 'Spirik' Spiridonov, licensed under LGPLv3, a MODIFIED version is used\n"
                                    "\fU8g2 (c) 2021 olikraus@gmail.com, licensed under the 2-clause BSD license\n"
                                    "\fu8g2_font_6x12_tr from the X11 distribution\n"
                                    "\fu8g2_font_tom_thumb_4x6_mr (c) 1999 Brian J. Swetland, licensed under CC-BY 3.0\n"
                                    "\fVisit https://github.com/shezik/espnut for further info\n";

    const uint8_t fontWidth = 4, fontHeight = 6;
    uint8_t width = context->dp.getU8g2()->getDisplayWidth() / fontWidth,  height = context->dp.getU8g2()->getDisplayHeight() / fontHeight - 1 /* !! Accommodate for the weird spacing on top of the terminal window */;
    uint8_t *buf = new uint8_t[width * height];
    U8G2LOG *u8g2log = new U8G2LOG;
    context->dp.getU8g2()->setDrawColor(1);
    context->dp.getU8g2()->setFont(u8g2_font_tom_thumb_4x6_mr);
    u8g2log->begin(*context->dp.getU8g2(), width, height, buf);
    u8g2log->setRedrawMode(0);

    std::stringstream ss((char *) copyrightInfo);
    std::string line;
    context->kbdMgr.clear();
    for (;;) {
        bool success = std::getline(ss, line, '\n').operator bool();
        if (!success) {
            vTaskDelay(pdMS_TO_TICKS(1000));  // 1500 ms in total for the last entry
            break;
        }

        if (context->kbdMgr.keysAvailable())
            break;

        line += '\n';
        u8g2log->print(line.c_str());
        printf_log("%s", line.c_str());
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    context->kbdMgr.clear();

    delete u8g2log; u8g2log = nullptr;
    delete buf; buf = nullptr;
    context->gem->drawMenu();
}

void Menu::enterMenu() {
    printf_log(TAG "Executing enterMenu\n");
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

    // gem->reInit();
    // u8g2.setContrast(contrast);  // GEM_uìg2::reInit causes U8g2 to reset contrast
    gem->setMenuValuesLeftOffset(VALUES_LEFT_OFFSET);
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
        fileList[FILE_LIST_LENGTH] = new GEMItem("..", [](GEMCallbackData data){context->enterFileManager((char *) data.valPointer);}, upDirBuf);  // fileList is 'FILE_LIST_LENGTH + 1' long, only this line and freeFileList() should be able to access this last index.
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

        fileList[itemCount] = new GEMItem(filenameBuf[itemCount], [](GEMCallbackData data){context->enterFileManager((char *) data.valPointer);}, filePathBuf[itemCount]);
        fileManagerPage->addMenuItem(*fileList[itemCount]);
        
        itemCount++;
    }

    dir.close();
    gem->setMenuPageCurrent(*fileManagerPage);
    gem->drawMenu();
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

void Menu::deleteFileCallback(char *path) {
    if (!path) {  // Cancelled
        enterMenu();
        return;
    }
    if (strlen(path) >= 4) {
        char *extension = path + strlen(path) - 4;
        if (!strcasecmp(extension, ".obj")) {
            printf_log(TAG "Blocked request to delete OBJ file: %s\n", path);
            dirGoUp(path);
            enterFileManager(path);
            return;
        }
    }
    pendingDeleteFilePath = path;

    static char title[ROM_FILENAME_LENGTH + 8];  // ROM_FILENAME_LENGTH + strlen("Delete ?")
    snprintf(title, sizeof(title), "Delete %s?", pathToFileName(path));
    confirmDeletePage->setTitle(title);
    gem->setMenuPageCurrent(*confirmDeletePage);
    gem->drawMenu();
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

void Menu::saveStateButtonCallback() {
    char randHex[9];  // uint32_t in hex, maximum FFFF FFFF
    char uptime[17];  // int64_t converted to uint64_t in hex, maximum 16 Fs
    snprintf(randHex, sizeof(randHex), "%08lx", esp_random());  // Unfortunately the RF subsystem is disabled, this generates pseudo-random numbers
    snprintf(uptime, sizeof(uptime), "%016llx", get_timer_ms());
    snprintf(editFilenameBuffer, sizeof(editFilenameBuffer), "%4.4s%s", randHex, uptime + 12);  // 8 chars and a terminator

    editFilenameConfirmedCallback = [](bool accepted){context->stateFileRenamedCallback(accepted);};
    editFilenameCallback();
}

void Menu::editFilenameCallback() {
    gem->setMenuValuesLeftOffset(5);
    gem->setMenuPageCurrent(*editFilenamePage);
    gem->drawMenu();
}

void Menu::stateFileRenamedCallback(bool accepted) {
    char stateFilename[ROM_FILENAME_LENGTH + 2 + GEM_STR_LEN - 1];  // ROM filename buffer length + strlen("/_") + Max editable string length in GEM - One excess terminator
    if (accepted) {
        snprintf(stateFilename, sizeof(stateFilename), "/%s_%s", emu.getRomFilename(), editFilenameBuffer);
        emu.saveState(stateFilename);
    }
    enterMenu();
}

void Menu::deleteFileConfirmedCallback(bool accepted) {
    if (accepted)
        LittleFS.remove(pendingDeleteFilePath);
    dirGoUp(pendingDeleteFilePath);  // Should be safe
    enterFileManager(pendingDeleteFilePath);
}

void Menu::drawBatteryCallback() {
    GEMPage *menuPageCurrent = context->gem->getMenuPageCurrent();
    if (!context->showingMenu || !menuPageCurrent || memcmp(menuPageCurrent->getTitle(), "espnut", 6))
        return;  // Not in main menu, don't draw
    context->dp.drawBattery(context->dp.getU8g2()->getDisplayWidth() - context->dp.batteryIconWidth - 1, 1, context->pm.getBatteryPercentage(), context->pm.getBatteryCharging());
    context->dp.getU8g2()->setDrawColor(1);  // Must reset color for GEM!
}
