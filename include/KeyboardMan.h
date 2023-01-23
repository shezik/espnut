#pragma once

// Keyboard Manager, handles interrupts and key queue

#include "Kbd_8x5_CH450.h"
#include "KeyQueue.h"

void keyboardCallback();

class KeyboardMan : public KeyQueue {
    private:
        uint8_t interruptPin;
        bool isInterruptEnabled = false;
        bool pendingRelease = false;
        bool busy = false;
    public:
        Kbd_8x5_CH450 &keyboard;
        KeyboardMan(Kbd_8x5_CH450, uint8_t);
        bool init();
        void enableInterrupt();
        void disableInterrupt();
        bool getInterruptState();
        void blockingWaitForKey();
        void checkForRelease();
};