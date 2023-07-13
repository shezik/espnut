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

#include "NutEmuInterface.h"

#define EMU_TAG "NutEmuInterface: "

NutEmuInterface *NutEmuInterface::context = nullptr;

NutEmuInterface::NutEmuInterface(SettingsMgr &sm_, KeyboardMgr &kbdMgr_, DispInterface &disp_, PowerMgr &pm_)
    : sm(sm_)
    , kbdMgr(kbdMgr_)
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
    // nv->awake  displayEnabled
    //     1                0             Processing key press
    //     1                1             Normal state
    //     0                1             Light sleep
    //     0                0             Deep sleep, should only respond to ON key.
    bool frequencyReduced = pm.isFrequencyReduced();
    if (!nv->awake && !kbdMgr.keysAvailable() && kbdMgr.isKeyboardClear() && !tickActionOverride) {
        if (displayEnabled) {
            if (!frequencyReduced && enablePowerMgmt) {
                // Don't light sleep with enablePowerMgmt == true
                printf_log(EMU_TAG "Entering light sleep\n");
                pm.reduceFrequency();
            }
        } else {
            if (enablePowerMgmt) {
                printf_log(EMU_TAG "Entering deep sleep\n");
                pm.enterDeepSleep();  // deepSleepPrepare() is registered as callback
            } else {
                // Actively update display to show a blank screen
                disp.updateDisplay(nv);
            }
        }
    } else if (frequencyReduced) {
        printf_log(EMU_TAG "Quitting light sleep\n");
        pm.restoreFrequency();
    }

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
    if (instructionCount > MAX_INST_BURST || unlockSpeed)
        instructionCount = MAX_INST_BURST;

    while (instructionCount--) {
        if (!nut_execute_instruction(nv)) {
            // printf_log(EMU_TAG "Failed to execute instruction at instructionCount %d\n", instructionCount);
            break;
        }
        static bool prevAwake = !nv->awake;
        static bool prevDisplayEnabled = !displayEnabled;
        static bool rawDisplayEnabled = !nv->display_chip->enable;
        if (nv->awake != prevAwake || displayEnabled != prevDisplayEnabled) {
            printf_log(EMU_TAG "awake: %d -> %d, displayEnabled: %d -> %d, rawDisplayEnabled: %d -> %d\n", prevAwake, nv->awake, prevDisplayEnabled, displayEnabled, rawDisplayEnabled, nv->display_chip->enable);
            prevAwake = nv->awake;
            prevDisplayEnabled = displayEnabled;
            rawDisplayEnabled = nv->display_chip->enable;
        }
    }


    lastRunTime = get_timer_ms();
}

bool NutEmuInterface::init() {
    applySettings();
    return sm.registerApplySettingsCallback(applySettings);
}

void NutEmuInterface::applySettings() {
    context->setEnablePowerMgmt(*context->sm.getEnablePowerMgmt());
    context->setUnlockSpeed(*context->sm.getUnlockSpeed());
}

bool NutEmuInterface::newProcessor(int clockFrequency, int ramSize_, char *filepath) {
    lastRunTime = get_timer_ms() - JIFFY_MSEC;  // In ms. Could be negative but that does not matter.
    wordsPerMs = clockFrequency / (1.0E3 * ARCH_NUT_WORD_LENGTH);

    deinit();
    if (ramSize_)
        ramSize = ramSize_;

    if (filepath) {
        strncpy(romFilePath, filepath, sizeof(romFilePath) - 1);
        romFilePath[sizeof(romFilePath)] = '\0';
    }
    nv = nut_new_processor(ramSize, (void *) this);  // (void *)nut_reg->display is reused for storing (NutEmuInterface *)
    pm.registerDeepSleepCallback(deepSleepCallback);
    wakeUpOnTick();
    return nut_read_object_file(nv, romFilePath);
}

bool NutEmuInterface::newProcessorFromStatefile(int clockFrequency, char *filepath) {
    if (loadMetadataFromStatefile(filepath)) {
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

#pragma region

extern bool nut_read_ram (nut_reg_t *, int, uint64_t *);
extern bool nut_write_ram (nut_reg_t *, int, uint64_t *);

static void nut_read_reg(reg_t reg, uint64_t *val) {
	uint64_t data = 0;

	// pack reg into data
	for (int8_t i = WSIZE - 1; i >= 0; i--) {
		data <<= 4;
		data += reg[i];
    }

	*val = data;
}

static void nut_write_reg(reg_t reg, uint64_t *val) {
	uint64_t data;
	data = *val;

	// now unpack data into reg
	for (uint8_t i = 0; i <= WSIZE; i++) {
		reg[i] = data & 0x0f;
		data >>= 4;
    }
}

#pragma endregion

void NutEmuInterface::keyPressed(uint16_t keycodeContent) {
    keysPressedSet.insert(keycodeContent);
    
    if (keysPressedSet.size() == 1) {  // First key pressed
        keyPressedFirst = keycodeContent;
        nut_press_key(nv, keycodeContent);
    } else {
        printf_log(EMU_TAG "additional key press, keycode %d\n", keycodeContent);
        if (nv->awake || displayEnabled)
            return;  // Emulator is not powered off

        if (keycodeContent == 24) {
            printf_log(EMU_TAG "ON key power on sequence detected, pressing ON then %d\n", keyPressedFirst);
            keysPressedSet.erase(keycodeContent);  // Don't interfere with 2-key rollover
            if (keyPressedFirst != 195 /*y^x on 15C*/) {
                // nut_release_key(nv);             // It is okay to leave the first key pressed.
                nut_press_key(nv, keycodeContent);  // Press ON
                // Wait a jiffy between registrations
                tickActionOverride = [](){
                    static uint8_t stage = 0;
                    switch (stage) {
                        case 0:
                            nut_release_key(context->nv);
                            break;
                        case 1:
                            nut_press_key(context->nv, context->keyPressedFirst);  // This key will be released later (it is actually pressed)
                            context->tickActionOverride = nullptr;
                            stage = 0;
                            return;
                    }
                    stage++;
                };
            } else {
                uint64_t x_register;
                uint64_t rotated_x_register = 0;
                printf_log(EMU_TAG "y^x + ON detected, performing magic\n");

                // nut_read_ram(nv, 0x04, &x_register);
                // rotated_x_register = (x_register >> 22 | x_register << (WSIZE * 4 - 22)) & ((uint64_t) -1 >> 8);
                // nut_write_ram(nv, 0x04, &rotated_x_register);
                // printf_log(EMU_TAG "RAM addr 0x04: %llx, rotated right by 22 bits: %llx\n", x_register, rotated_x_register);

                // nut_read_reg(nv->a, &x_register);
                // rotated_x_register = (x_register >> 22 | x_register << (WSIZE * 4 - 22)) & ((uint64_t) -1 >> 8);
                // nut_write_reg(nv->a, &rotated_x_register);
                // printf_log(EMU_TAG "Register a: %llx, rotated right by 22 bits: %llx\n", x_register, rotated_x_register);

                // nut_read_reg(nv->b, &x_register);
                // rotated_x_register = (x_register >> 22 | x_register << (WSIZE * 4 - 22)) & ((uint64_t) -1 >> 8);
                // nut_write_reg(nv->b, &rotated_x_register);
                // printf_log(EMU_TAG "Register a: %llx, rotated right by 22 bits: %llx\n", x_register, rotated_x_register);

                nut_read_reg(nv->n, &x_register);
                rotated_x_register = (x_register >> 22 | x_register << (WSIZE * 4 - 22)) & ((uint64_t) -1 >> 8);
                nut_write_reg(nv->n, &rotated_x_register);
                printf_log(EMU_TAG "Register n: %llx, rotated right by 22 bits: %llx\n", x_register, rotated_x_register);

                tickActionOverride = [](){
                    nut_release_key(context->nv);  // Release keyPressedFirst (will be removed from set later)
                    context->wakeUpOnTick();       // Do ON key's job
                };
            }
        }
    }
}

void NutEmuInterface::keyReleased(uint16_t keycodeContent) {
    uint16_t remainingKeycode;

    keysPressedSet.erase(keycodeContent);
    switch (keysPressedSet.size()) {
        case 0:
            printf_log(EMU_TAG "last key release, keycode %d\n", keycodeContent);
            nut_release_key(nv);
            break;
        case 1:
            printf_log(EMU_TAG "next-to-last key release, keycode %d\n", keycodeContent);
            remainingKeycode = *keysPressedSet.begin();
            if (remainingKeycode != keyPressedFirst) {
                printf_log(EMU_TAG "rollover pressing keycode %d\n", remainingKeycode);
                nut_release_key(nv);  // Register keyPressedFirst
                keyPressedFirst = remainingKeycode;
                // The remaining key will be registered on the next tick.
                tickActionOverride = [](){nut_press_key(context->nv, context->keyPressedFirst);
                                          context->tickActionOverride = nullptr;};
            }
            break;
        default:
            printf_log(EMU_TAG "key release, keycode %d\n", keycodeContent);
    }
}

void NutEmuInterface::tick() {
    if (!nv || !emulatorRunFlag) {
        if (emulatorRunFlag)
            xSemaphoreGive(kbdMgr.getMutex());
        return;
    }

    disp.setLowBatAnnunciatiorBlink((pm.getBatteryPercentage() <= 28) ? 500 : 0);
    disp.setLowBatAnnunciator(pm.getBatteryCharging());

    if (tickActionOverride) {
        printf_log(EMU_TAG "Executing overriding tick action\n");
        tickActionOverride();
    } else {
        // This is the default 'tick action'.
        if (kbdMgr.keysAvailable()) {
            uint16_t keycode = kbdMgr.getLastKeycode();
            if (GetKeycodeStatus(keycode))
                keyPressed(GetKeycodeContent(keycode));
            else
                keyReleased(GetKeycodeContent(keycode));
        }
    }
    
    xSemaphoreGive(kbdMgr.getMutex());
    // -------- END OF CRITICAL SECTION --------

    sim_run();
}

void NutEmuInterface::pause() {
    emulatorRunFlag = false;
    printf_log(EMU_TAG "Quitting light sleep for pausing\n");
    pm.restoreFrequency();
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
    // Repeatedly press the power button until displayEnabled becomes true, one action per tick()
    tickActionOverride = [](){
        static uint8_t count = 0;
        static uint8_t stage = 0;
        static auto deinit = [](){
            count = 0;
            stage = 0;
            context->tickActionOverride = nullptr;
        };
        switch (stage++) {
            case SIM_RUNS_BEFORE_CHECK:
                printf_log(EMU_TAG "wakeUpOnTick: Stage %d\n", stage - 1);
                if (context->displayEnabled) {
                    printf_log(EMU_TAG "wakeUpOnTick: Emulator awake, forcing display update\n");
                    deinit();
                    context->disp.updateDisplay(context->nv, true);
                    break;
                }
                printf_log(EMU_TAG "wakeUpOnTick: Pressing ON\n");
                nut_press_key(context->nv, 24 /*ON*/);
                break;

            case SIM_RUNS_BEFORE_CHECK + 1:
                printf_log(EMU_TAG "wakeUpOnTick: Stage %d\n", stage - 1);
                if (context->nv->awake) {
                    printf_log(EMU_TAG "wakeUpOnTick: ON press registered\n");
                    count = 0;
                    break;  // Proceed to next stage
                } else {
                    stage--;  // Stay in this stage
                    count++;
                    if (count == MAX_WAKEUP_ATTEMPTS) {
                        printf_log(EMU_TAG "wakeUpOnTick: Reached max wakeup attempts waiting for ON press registration\n");
                        deinit();
                    }
                }
                break;

            case SIM_RUNS_BEFORE_CHECK + 2:
                printf_log(EMU_TAG "wakeUpOnTick: Stage %d\n", stage - 1);
                printf_log(EMU_TAG "wakeUpOnTick: Releasing ON\n");
                nut_release_key(context->nv);
                break;

            case SIM_RUNS_BEFORE_CHECK + 3:
                printf_log(EMU_TAG "wakeUpOnTick: Stage %d\n", stage - 1);
                if (!context->nv->awake) {
                    printf_log(EMU_TAG "wakeUpOnTick: ON release registered\n");
                    count = 0;
                    stage = 0;  // Go back to beginnning and wait for SIM_RUNS_BEFORE_CHECK
                } else {
                    stage--;
                    count++;
                    if (count == MAX_WAKEUP_ATTEMPTS) {
                        printf_log(EMU_TAG "wakeUpOnTick: Reached max wakeup attempts waiting for ON release registration\n");
                        deinit();
                    }
                }
                break;

            default:
                printf_log(EMU_TAG "wakeUpOnTick: Stage %d\n", stage - 1);
                break;
        }
        // !! Clear key queue here?
    };
}

bool NutEmuInterface::saveState(char *filepath) {
    if (!nv)
        return false;

    File file = LittleFS.open(filepath, "w");
    if (!file) {
        printf_log(EMU_TAG "Failed to open file %s for writing\n", filepath);
        return false;
    }

    file.write((uint8_t *) "STATE", 5);  // Magic
    file.write((uint8_t *) romFilePath, strlen(romFilePath));
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

bool NutEmuInterface::loadState(char *filepath, bool doUpdateMetadata, bool doLoadState) {
    char magic[6];
    char romFilePath_[FILE_PATH_LENGTH];
    uint8_t size;

    File file = LittleFS.open(filepath, "r");
    if (!file) {
        printf_log(EMU_TAG "loadState: Failed to open file %s for reading\n", filepath);
        return false;
    }

    file.readBytes(magic, 5);  // Magic string is 'STATE'
    magic[5] = '\0';
    if (strcmp(magic, "STATE")) {
        file.close();
        printf_log(EMU_TAG "loadState: Bad file magic: %s\n", magic);
        return false;
    }

    size = file.readBytesUntil('\0', romFilePath_, sizeof(romFilePath_) - 1);
    romFilePath_[size] = '\0';
    if (size == sizeof(romFilePath_) - 1  // Need to skip one '\0' in stream
        && (!file.available() || file.read() != '\0')) {
            
        file.close();
        printf_log(EMU_TAG "loadState: File path buffer depleted!\n");
        return false;  // you've got a very, very long path...
    }
    
    if (doUpdateMetadata) {
        strcpy(romFilePath, romFilePath_);
        file.read((uint8_t *) &ramSize, sizeof(int));
        printf_log(EMU_TAG "loadState: Metadata: RAM size: %d, ROM path: %s\n", ramSize, romFilePath);
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

bool NutEmuInterface::loadMetadataFromStatefile(char *filepath) {
    return loadState(filepath, true, false);
}

bool NutEmuInterface::loadStateFromStatefile(char *filepath) {
    return loadState(filepath, false, true);
}

bool NutEmuInterface::isProcessorPresent() {
    return nv;
}

// Create your own copy!
char *NutEmuInterface::getRomFilePath() {
    return romFilePath;
}

void NutEmuInterface::updateDisplayCallback() {
    displayEnabled = nv->display_chip->enable;  // The value is the most reliable FOR NOW
    static bool lastState = !displayEnabled;

    if (displayEnabled) {
        disp.updateDisplay(nv, !lastState);
    } else if (/*!unlockSpeed && */lastState)  // For the sake of accuracy
        disp.getU8g2()->clear();  // !! What about the low bat annunciator?

    lastState = displayEnabled;
}

void NutEmuInterface::setEnablePowerMgmt(bool enablePowerMgmt_) {
    enablePowerMgmt = enablePowerMgmt_;
}

void NutEmuInterface::setUnlockSpeed(bool unlockSpeed_) {
    unlockSpeed = unlockSpeed_;
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
