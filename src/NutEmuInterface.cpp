#include "NutEmuInterface.h"

NutEmuInterface::NutEmuInterface(KeyboardMan &kbdMan_, DispInterface &disp_, PowerMgr &pm_)
    : kbdMan(kbdMan_)
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

        if (instructionCount) {  // instructionCount > 0 indicates that the processor went to bed
            pm.enterDeepSleep();
        }

        lastRunTime = esp_timer_get_time();
    }
}

bool NutEmuInterface::newProcessor(int clockFrequency, int ramSize, char *filename) {
    lastRunTime = esp_timer_get_time();  // In ms
    wordsPerMs = clockFrequency / (1.0E3 * ARCH_NUT_WORD_LENGTH);

    nv = nut_new_processor(ramSize, (void *) this);  // void *nut_reg->display is reused for storing NutEmuInterface *
    nut_read_object_file(nv, filename);
}

void NutEmuInterface::tick() {
    static int keycode;
    if (kbdMan.count()) {
        kbdMan.busy = true;
        keycode = kbdMan.getLastKeycode();
        kbdMan.removeLastKeycode();
        kbdMan.busy = false;
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
    yield();
    disp.updateDisplay(nv);
}
