#include "NutEmuInterface.h"

NutEmuInterface::NutEmuInterface(KeyboardMgr &kbdMgr_, DispInterface &disp_, PowerMgr &pm_)
    : kbdMgr(kbdMgr_)
    , disp(disp_)
    , pm(pm_)
{
    // Do nothing
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
        if (!nv->awake) {
            if (displayStateStabilized) {
                frequencyReduced = pm.reduceFrequency();
                // CH450 should not sleep since only a few dedicated rows of keys are able to bring it up
                // Polling will work but why the hassle?
            } else {
                pm.enterDeepSleep();
            }
        } else if (frequencyReduced) {
            pm.restoreFrequency();
        }

        lastRunTime = esp_timer_get_time();
    }
}

bool NutEmuInterface::newProcessor(int clockFrequency, int ramSize, char *filename) {
    lastRunTime = esp_timer_get_time() - JIFFY_MSEC;  // In ms. Could be negative but that does not matter.
    wordsPerMs = clockFrequency / (1.0E3 * ARCH_NUT_WORD_LENGTH);

    if (nv) {
        nut_free_processor(nv); nv = NULL;  // !! Check if there's any memory leak
    }
    nv = nut_new_processor(ramSize, (void *) this);  // void *nut_reg->display is reused for storing NutEmuInterface *
    return nut_read_object_file(nv, filename);
}

void NutEmuInterface::tick() {
    static int keycode;
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

    sim_run();
}

bool NutEmuInterface::saveState(char *filename) {

}

bool NutEmuInterface::loadState(char *filename) {

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

}
