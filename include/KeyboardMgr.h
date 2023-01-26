#pragma once

// Keyboard Manager, handles interrupts and key queue

#include "Kbd_8x5_CH450.h"
#include "KeyQueue.h"

static void keyboardCallback(void *ptr);

class KeyboardMgr : public KeyQueue {
    protected:
        uint8_t interruptPin;
        bool isInterruptEnabled = false;
    public:
        Kbd_8x5_CH450 &keyboard;                // We are not using getter and setters
        KeyboardMgr(Kbd_8x5_CH450 &, uint8_t);  // on these variables because function
        bool busy = false;                      // calls take time and these are being
        bool pendingRelease = false;            // used in our time-sensivive ISR.
        void init();
        void enableInterrupt();
        void disableInterrupt();
        bool getInterruptState();
        void blockingWaitForKey();
        void checkForRelease();
        bool chipEnterSleep();
        void skipReleaseCheck();
        int getPositiveKeycode();
};