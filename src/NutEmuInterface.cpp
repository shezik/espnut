#include "NutEmuInterface.h"

#define EMU_TAG "NutEmuInterface: "

NutEmuInterface *NutEmuInterface::context = nullptr;

NutEmuInterface::NutEmuInterface(KeyboardMgr &kbdMgr_, DispInterface &disp_, PowerMgr &pm_)
    : kbdMgr(kbdMgr_)
    , disp(disp_)
    , pm(pm_)
{
    context = this;
}

NutEmuInterface::~NutEmuInterface() {
    deinit();
    context = nullptr;
}

void NutEmuInterface::deepSleepCallback() {
    if (context)
        context->deepSleepPrepare();
}

void NutEmuInterface::sim_run() {
    int64_t currentTime;
    int64_t deltaMs;
    int instructionCount;

    currentTime = get_timer_ms();
    if (currentTime < NEXT_RUN_TIME)
        return;

    deltaMs = currentTime - lastRunTime;
    if (deltaMs > 2000)
        deltaMs = 2000;  // According to proc.c, this should be 1000 but it feels wrong?

    instructionCount = deltaMs * wordsPerMs;
    if (instructionCount > MAX_INST_BURST)
        instructionCount = MAX_INST_BURST;

    while (instructionCount--) {
        if (!nut_execute_instruction(nv)) {
            // printf_log(EMU_TAG "Failed to execute instruction at instructionCount %d\n", instructionCount);
            break;
        }
    }

    // nv->awake  displayEnabled
    //     1                0             Display toggle? Maybe another type of 'light sleep'.
    //     1                1             Normal state
    //     0                1             Light sleep
    //     0                0             Deep sleep, should only respond to ON key.
    if (!nv->awake && !kbdMgr.keysAvailable() && kbdMgr.isKeyboardClear() && !tickActionOverride) {
        if (displayEnabled) {
            if (!frequencyReduced) {
                printf_log(EMU_TAG "Entering light sleep\n");
                frequencyReduced = pm.reduceFrequency();
            }
        } else {
            printf_log(EMU_TAG "Entering deep sleep\n");
            pm.enterDeepSleep();  // deepSleepPrepare() is registered as callback
        }
    } else if (frequencyReduced) {
        printf_log(EMU_TAG "Quitting light sleep\n");
        frequencyReduced = !pm.restoreFrequency();
    }

    lastRunTime = get_timer_ms();
}

bool NutEmuInterface::newProcessor(int clockFrequency, int ramSize_, char *filename) {
    lastRunTime = get_timer_ms() - JIFFY_MSEC;  // In ms. Could be negative but that does not matter.
    wordsPerMs = clockFrequency / (1.0E3 * ARCH_NUT_WORD_LENGTH);

    deinit();
    if (ramSize_)
        ramSize = ramSize_;

    if (filename) {
        strncpy(romFilename, filename, sizeof(romFilename) - 1);
        romFilename[sizeof(romFilename)] = '\0';
    }
    nv = nut_new_processor(ramSize, (void *) this);  // (void *)nut_reg->display is reused for storing (NutEmuInterface *)
    pm.registerDeepSleepCallback(deepSleepCallback);
    wakeUpOnTick();
    return nut_read_object_file(nv, romFilename);
}

bool NutEmuInterface::newProcessorFromStatefile(int clockFrequency, char *filename) {
    if (loadMetadataFromStatefile(filename)) {
        return newProcessor(clockFrequency, 0, nullptr);
    }
    return false;
}

void NutEmuInterface::deinit() {
    if (nv) {
        nut_free_processor(nv); nv = nullptr;  // !! Check if there's any memory leak
    }
    pm.registerDeepSleepCallback(nullptr);
    keysPressedSet.clear();
    tickActionOverride = nullptr;
}

void NutEmuInterface::keyPressed(uint16_t keycodeContent) {
    keysPressedSet.insert(keycodeContent);
    if (keysPressedSet.size() == 1) {
        keyPressedFirst = keycodeContent;
        nut_press_key(nv, keycodeContent);
    } else
        printf_log(EMU_TAG "additional key press, keycode %d\n", keycodeContent);
}

void NutEmuInterface::keyReleased(uint16_t keycodeContent) {
    uint16_t keycode;

    keysPressedSet.erase(keycodeContent);
    switch (keysPressedSet.size()) {
        case 0:
            printf_log(EMU_TAG "last key release, keycode %d\n", keycodeContent);
            nut_release_key(nv);
            break;
        case 1:
            printf_log(EMU_TAG "next-to-last key release, keycode %d\n", keycodeContent);
            keycode = *keysPressedSet.begin();
            if (keycode != keyPressedFirst) {
                printf_log(EMU_TAG "rollover pressing keycode %d\n", keycode);
                nut_release_key(nv);
                keyPressedFirst = keycode;
                // The following function will be called once on the next tick.
                tickActionOverride = [](){nut_press_key(context->nv, context->keyPressedFirst);
                                          context->tickActionOverride = nullptr;};
            }
            break;
        default:
            printf_log(EMU_TAG "key release, keycode %d\n", keycodeContent);
    }
}

void NutEmuInterface::tick() {
    static bool keycodeCheckedByMenu = true;

    if (!nv || !emulatorRunFlag)
        return;

    if (tickActionOverride) {
        printf_log(EMU_TAG "Executing overriding tick action\n");
        tickActionOverride();
    } else {
        // This is the default 'tick action'.
        uint8_t keysAvailable = kbdMgr.keysAvailable();
        if (keysAvailable == 1 && kbdMgr.peekLastKeycode() == MakeKeycodeFromCode(true, 24 /*ON*/)) {
            keycodeCheckedByMenu = !keycodeCheckedByMenu;
        } else {
            keycodeCheckedByMenu = true;
        }
        if (keysAvailable && keycodeCheckedByMenu) {
            uint16_t keycode = kbdMgr.getLastKeycode();
            if (GetKeycodeStatus(keycode))
                keyPressed(GetKeycodeContent(keycode));
            else
                keyReleased(GetKeycodeContent(keycode));
        }
    }

    sim_run();
}

void NutEmuInterface::pause() {
    emulatorRunFlag = false;
    printf_log(EMU_TAG "Quitting light sleep\n");
    frequencyReduced = !pm.restoreFrequency();
}

void NutEmuInterface::resume() {
    if (emulatorRunFlag)
        return;
        
    emulatorRunFlag = true;
    lastRunTime = get_timer_ms() - JIFFY_MSEC;
    keysPressedSet.clear();
    if (nv)
        nut_release_key(nv);

    wakeUpOnTick();  // Overwrite tickActionOverride in case it is the rollover pressing one
}

void NutEmuInterface::wakeUpOnTick() {
    // Repeatedly press the power button until nv->awake becomes true, one action per tick()
    tickActionOverride = [](){
        static uint8_t count = 0;
        static uint8_t stage = 0;
        switch (stage++) {
            case 10:
                if (context->nv->awake)
                    printf_log(EMU_TAG "wakeUpOnTick: Emulator awake, forcing display update\n");
                else if (count == MAX_WAKEUP_ATTEMPTS)
                    printf_log(EMU_TAG "wakeUpOnTick: Reached max wakeup attempts!\n");
                    
                if (context->nv->awake || count == MAX_WAKEUP_ATTEMPTS) {
                    count = 0;
                    stage = 0;
                    context->tickActionOverride = nullptr;
                    context->disp.updateDisplay(context->nv, true);
                    break;
                }
                printf_log(EMU_TAG "wakeUpOnTick: Pressing ON\n");
                nut_press_key(context->nv, 24 /*ON*/);
                break;
            case 20:
                printf_log(EMU_TAG "wakeUpOnTick: Releasing keys\n");
                nut_release_key(context->nv);
                count++;
                stage = 0;
                break;
            default:
                break;

        }
        // !! Clear key queue here?
    };
}

bool NutEmuInterface::saveState(char *filename) {
    if (!nv)
        return false;

    File file = LittleFS.open(filename, "w");
    if (!file) {
        printf_log(EMU_TAG "Failed to open file %s for writing\n", filename);
        return false;
    }

    file.write((uint8_t *) "STATE", 5);  // Magic
    file.write((uint8_t *) romFilename, strlen(romFilename));
    file.write('\0');
    file.write((uint8_t *) &ramSize, sizeof(int));
    file.write((uint8_t *)  nv->a,   sizeof(reg_t));
    file.write((uint8_t *)  nv->b,   sizeof(reg_t));
    file.write((uint8_t *)  nv->c,   sizeof(reg_t));
    file.write((uint8_t *)  nv->m,   sizeof(reg_t));
    file.write((uint8_t *)  nv->n,   sizeof(reg_t));
    file.write((uint8_t *)  nv->g,   sizeof(digit_t) * 2);
    file.write((uint8_t)    nv->p);
    file.write((uint8_t)    nv->q);
    file.write((uint8_t)    nv->q_sel);
    file.write(             nv->fo);
    file.write((uint8_t)    nv->decimal);
    file.write((uint8_t)    nv->carry);
    file.write((uint8_t)    nv->prev_carry);
    file.write((uint8_t *) &nv->prev_tef_last, sizeof(int));
    file.write((uint8_t *)  nv->s,             sizeof(bool) * SSIZE);
    file.write((uint8_t *) &nv->pc,            sizeof(rom_addr_t));
    file.write((uint8_t *) &nv->prev_pc,       sizeof(rom_addr_t));
    file.write((uint8_t *)  nv->stack,         sizeof(rom_addr_t) * STACK_DEPTH);
    file.write((uint8_t *) &nv->cxisa_addr,    sizeof(rom_addr_t));
    file.write((uint8_t *) &nv->inst_state,    sizeof(inst_state_t));
    file.write((uint8_t *) &nv->first_word,    sizeof(rom_word_t));
    file.write((uint8_t)    nv->long_branch_carry);
    file.write((uint8_t)    nv->key_down);
    file.write((uint8_t *) &nv->kb_state,                  sizeof(keyboard_state_t));
    file.write((uint8_t *) &nv->kb_debounce_cycle_counter, sizeof(int));
    file.write((uint8_t *) &nv->key_buf,                   sizeof(int));
    file.write((uint8_t)    nv->awake);
    file.write(             nv->active_bank, MAX_PAGE);
    file.write((uint8_t *) &nv->ram_addr,    sizeof(uint16_t));
    file.write((uint8_t *)  nv->ram,         sizeof(reg_t) * nv->max_ram);
    file.write(             nv->pf_addr);
    file.write(             nv->selprf);
    file.write((uint8_t *)  nv->display_segments, sizeof(uint32_t) * MAX_DIGIT_POSITION);
    file.write((uint8_t *) &nv->cycle_count,      sizeof(uint64_t));

    file.close();
    return true;
}

bool NutEmuInterface::loadState(char *filename, bool doUpdateMetadata, bool doLoadState) {
    char magic[6];
    char romFilename_[ROM_FILENAME_LENGTH];
    uint8_t size;

    File file = LittleFS.open(filename, "r");
    if (!file) {
        printf_log(EMU_TAG "loadState: Failed to open file %s for reading\n", filename);
        return false;
    }

    file.readBytes(magic, 5);  // Magic string is 'STATE'
    magic[5] = '\0';
    if (strcmp(magic, "STATE")) {
        file.close();
        printf_log(EMU_TAG "loadState: Bad file magic: %s\n", magic);
        return false;
    }

    size = file.readBytesUntil('\0', romFilename_, sizeof(romFilename_) - 1);
    romFilename_[size] = '\0';
    if (size == sizeof(romFilename_) - 1  // Need to skip one '\0' in stream
        && (!file.available() || file.read() != '\0')) {
            
        file.close();
        printf_log(EMU_TAG "loadState: Filename buffer depleted!\n");
        return false;  // you've got a very, very long filename...
    }
    
    if (doUpdateMetadata) {
        strcpy(romFilename, romFilename_);
        file.read((uint8_t *) &ramSize, sizeof(int));
        printf_log(EMU_TAG "loadState: Metadata: RAM size: %d, ROM filename: %s\n", ramSize, romFilename);
    } else
        file.seek(sizeof(int), SeekCur);

    if (!doLoadState) {
        file.close();
        return true;
    }

    if (!nv) {
        file.close();
        return false;
    }

    file.read((uint8_t *) nv->a, sizeof(reg_t));
    file.read((uint8_t *) nv->b, sizeof(reg_t));
    file.read((uint8_t *) nv->c, sizeof(reg_t));
    file.read((uint8_t *) nv->m, sizeof(reg_t));
    file.read((uint8_t *) nv->n, sizeof(reg_t));
    file.read((uint8_t *) nv->g, sizeof(digit_t) * 2);
    nv->p          = (digit_t) file.read();
    nv->q          = (digit_t) file.read();
    nv->q_sel      = (bool)    file.read();
    nv->fo         =           file.read();
    nv->decimal    = (bool)    file.read();
    nv->carry      = (bool)    file.read();
    nv->prev_carry = (bool)    file.read();
    file.read((uint8_t *) &nv->prev_tef_last, sizeof(int));
    file.read((uint8_t *)  nv->s,             sizeof(bool) * SSIZE);
    file.read((uint8_t *) &nv->pc,            sizeof(rom_addr_t));
    file.read((uint8_t *) &nv->prev_pc,       sizeof(rom_addr_t));
    file.read((uint8_t *)  nv->stack,         sizeof(rom_addr_t) * STACK_DEPTH);
    file.read((uint8_t *) &nv->cxisa_addr,    sizeof(rom_addr_t));
    file.read((uint8_t *) &nv->inst_state,    sizeof(inst_state_t));
    file.read((uint8_t *) &nv->first_word,    sizeof(rom_word_t));
    nv->long_branch_carry = (bool) file.read();
    nv->key_down          = (bool) file.read();
    file.read((uint8_t *) &nv->kb_state,                  sizeof(keyboard_state_t));
    file.read((uint8_t *) &nv->kb_debounce_cycle_counter, sizeof(int));
    file.read((uint8_t *) &nv->key_buf,                   sizeof(int));
    nv->awake = (bool) file.read();
    file.read(             nv->active_bank, MAX_PAGE);
    file.read((uint8_t *) &nv->ram_addr,    sizeof(uint16_t));
    file.read((uint8_t *)  nv->ram,         sizeof(reg_t) * nv->max_ram);
    nv->pf_addr = file.read();
    nv->selprf  = file.read();
    file.read((uint8_t *)  nv->display_segments, sizeof(uint32_t) * MAX_DIGIT_POSITION);
    file.read((uint8_t *) &nv->cycle_count,      sizeof(uint64_t));

    file.close();

    wakeUpOnTick();
    return true;
}

bool NutEmuInterface::loadMetadataFromStatefile(char *filename) {
    return loadState(filename, true, false);
}

bool NutEmuInterface::loadStateFromStatefile(char *filename) {
    return loadState(filename, false, true);
}

bool NutEmuInterface::isProcessorPresent() {
    return nv;
}

char *NutEmuInterface::getRomFilename() {
    static char romFilename_[ROM_FILENAME_LENGTH];

    strcpy(romFilename_, romFilename);
    return romFilename_;
}

void NutEmuInterface::updateDisplayCallback() {
    displayEnabled = nv->display_chip->enable;  // The value is the most reliable FOR NOW

    static bool lastState = !displayEnabled;
    if (displayEnabled != lastState) {
        lastState = displayEnabled;
        printf_log(EMU_TAG "updateDisplayCallback: displayEnabled: %d\n", displayEnabled);
    }

    if (displayEnabled) {
        setDisplayPowerSave(false);
        disp.updateDisplay(nv);
    }// else
        // setDisplayPowerSave(true);  // I believe this counts as updating display rather than managing power ;)
}

void NutEmuInterface::setDisplayPowerSave(bool state) {
    disp.setU8g2PowerSave(state);
    if (state)  // Actively turn off backlight, don't actively turn on
        pm.setBacklightPower(false);
}

bool NutEmuInterface::checkRestoreFlag() {
    if (!LittleFS.exists(RESTORE_FLAG_FILENAME))
        return false;
    
    LittleFS.remove(RESTORE_FLAG_FILENAME);
    printf_log(EMU_TAG "Checking restore flag\n");
    newProcessorFromStatefile(NUT_FREQUENCY_HZ, RESTORE_STATE_FILENAME);
    loadStateFromStatefile(RESTORE_STATE_FILENAME);  // Load state
    printf_log(EMU_TAG "State loaded from file.\n");
    return true;
}

void NutEmuInterface::resetProcessor(bool obdurate) {
    if (!nv)
        return;

    do_event(nv, event_reset);
    if (obdurate)
        do_event(nv, event_clear_memory);
}

void NutEmuInterface::deepSleepPrepare() {
    if (!nv)
        return;

    saveState(RESTORE_STATE_FILENAME);
    File flag = LittleFS.open(RESTORE_FLAG_FILENAME, "w");
    flag.close();
}
