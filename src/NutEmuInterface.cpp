#include "NutEmuInterface.h"

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
    // Does not need to persist value between calls.
    // Static'd simply to save time (how long?)
    static int64_t currentTime;
    static int64_t deltaMs;
    static int instructionCount;

    currentTime = esp_timer_get_time();
    if (currentTime >= NEXT_RUN_TIME) {
        deltaMs = currentTime - lastRunTime;
        if (deltaMs > 2000)
            deltaMs = 2000;  // According to proc.c, this should be 1000 but it feels wrong?

        instructionCount = deltaMs * wordsPerMs;
        if (instructionCount > MAX_INST_BURST)
            instructionCount = MAX_INST_BURST;

        while (instructionCount--) {
            if (!(instructionCount % 100))  // yield() when divisible by 100
                yield();
            if (!nut_execute_instruction(nv))
                break;
        }

        // nv->awake  displayStateStabilized
        //     1                0             Display toggle? Maybe another type of 'light sleep'.
        //     1                1             Normal state
        //     0                1             Light sleep
        //     0                0             Deep sleep, should only respond to ON key.
        if (!nv->awake && !kbdMgr.count() && !kbdMgr.pendingRelease) {
            if (displayStateStabilized) {
                frequencyReduced = pm.reduceFrequency();
                // CH450 should not sleep since only a few dedicated rows of keys are able to bring it up
                // Polling will work but why the hassle?
            } else {
                deepSleepPrepare();
                pm.enterDeepSleep();
            }
        } else if (frequencyReduced) {
            pm.restoreFrequency();
        }

        lastRunTime = esp_timer_get_time();
    }
}

// Pass nullptr or left out filename to load from romFilename
bool NutEmuInterface::newProcessor(int clockFrequency, int ramSize_, char *filename) {
    lastRunTime = esp_timer_get_time() - JIFFY_MSEC;  // In ms. Could be negative but that does not matter.
    wordsPerMs = clockFrequency / (1.0E3 * ARCH_NUT_WORD_LENGTH);

    deinit();
    if (ramSize_) {
        ramSize = ramSize_;
    }
    if (filename) {
        strncpy(romFilename, filename, sizeof(romFilename) - 1);
        romFilename[sizeof(romFilename)] = '\0';
    }
    nv = nut_new_processor(ramSize, (void *) this);  // void *nut_reg->display is reused for storing NutEmuInterface *
    pm.registerDeepSleepCallback(deepSleepCallback);
    return nut_read_object_file(nv, romFilename);
}

void NutEmuInterface::deinit() {
    if (nv) {
        nut_free_processor(nv); nv = nullptr;  // !! Check if there's any memory leak
    }
    pm.registerDeepSleepCallback(nullptr);
}

void NutEmuInterface::tick() {
    static int keycode;

    if (nv) {
        if (kbdMgr.count()) {
            kbdMgr.busy = true;
            keycode = kbdMgr.getLastKeycode();
            kbdMgr.removeLastKeycode();
            kbdMgr.busy = false;
            if (keycode < 0) {
                nut_release_key(nv);
            } else {
                nut_press_key(nv, keycode);
            }
        }

        // !! check for run flag here
        sim_run();
    }
}

bool NutEmuInterface::saveState(char *filename) {
    if (!nv)
        return false;

    File file = LittleFS.open(filename, "w");
    if (!file)
        return false;

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

bool NutEmuInterface::loadState(char *filename, bool onlyUpdateMetadata) {
    char magic[6];
    char romFilename_[32];
    uint8_t size;

    File file = LittleFS.open(filename, "r");
    if (!file)
        return false;

    file.readBytes(magic, 5);  // Magic string is 'STATE'
    magic[6] = '\0';
    if (strcmp(magic, "STATE")) {
        file.close();
        return false;
    }

    size = file.readBytesUntil('\0', romFilename_, sizeof(romFilename_) - 1);
    romFilename_[size] = '\0';
    if (size == sizeof(romFilename_) - 1) {  // Need to skip one '\0' in stream
        if (!file.available() || file.read() != '\0') {
            file.close();
            return false;  // you've got a very, very long filename...
        }
    }
    strcpy(romFilename, romFilename_);

    file.read((uint8_t *) &ramSize, sizeof(int));

    if (onlyUpdateMetadata) {
        file.close();
        return true;
    }

    if (!nv)
        file.close();
        return false;

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
    return true;
}

void NutEmuInterface::updateDisplayCallback() {
    yield();  // dunno if necessary

    displayStateStabilized = nv->display_chip->enable;  // The value is the most reliable NOW

    if (displayStateStabilized) {
        setDisplayPowerSave(false);
        disp.updateDisplay(nv);
    } else
        setDisplayPowerSave(true);  // I believe this counts as updating display rather than managing power ;)
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
    return loadState(RESTORE_STATE_FILENAME, true);
}

void NutEmuInterface::resetProcessor(bool obdurate) {
    if (nv) {
        do_event(nv, event_reset);
        if (obdurate)
            do_event(nv, event_clear_memory);
    }
}

void NutEmuInterface::deepSleepPrepare() {
    if (nv) {
        saveState(RESTORE_STATE_FILENAME);
        File flag = LittleFS.open(RESTORE_FLAG_FILENAME, "w");
        flag.close();
    }
}
