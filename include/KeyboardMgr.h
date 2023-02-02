#pragma once

// Keyboard Manager, handles interrupts and key queue

#include "Kbd_8x5_CH450.h"
#include "KeyQueue.h"

class KeyboardMgr : public KeyQueue {
    protected:
        uint8_t interruptPin;
        bool isInterruptEnabled = false;
        void checkForRelease();
        Kbd_8x5_CH450 &keyboard;
        void (*keyPressCallback)() = nullptr;
        bool busy = false;
        bool pendingRelease = false;
    public:
        KeyboardMgr(Kbd_8x5_CH450 &, uint8_t);
        // Don't call this outside if interrupt is enabled.
        void handleKeyPress();

        void init();
        void tick();
        void enableInterrupt();
        void disableInterrupt();
        bool getInterruptState();
        void blockingWaitForKey();
        bool chipEnterSleep();
        void skipReleaseCheck();
        int getPositiveKeycode();
        // Callback should be protected from the interrupt (if you are not using handleKeyPress())
        // Key release also counts!
        void registerKeyPressCallback(void (*)());

        bool getPendingRelease();
        bool getBusy();
        // Returns true if a key was pressed while busy == true and was fixed
        bool setBusy(bool);

        static void keyboardCallback(void *ptr);
};