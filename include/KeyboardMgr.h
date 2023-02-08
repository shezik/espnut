#pragma once

#include "MatrixKeyboard.h"
#include "Configuration.h"

// Actually it reduces available matrix size from 16x8 down to 15x7
#define INVALID_KEYCODE 0xFF
#define ValidateKeycode(keycode) (keycode != INVALID_KEYCODE && keycode != INVALID_KEYCODE >> 1)

class KeyboardMgr {
    protected:
        void (*keyPressCallback)() = nullptr;
        matrix_keyboard_handle_t *mkHandle = nullptr;
        QueueHandle_t *keyQueue = nullptr;
    public:
        KeyboardMgr();
        ~KeyboardMgr();
        void init();
        void blockingWaitForKey();
        void skipReleaseCheck();
        uint8_t getPositiveKeycode();
        // Call before init(). skipReleaseCheck() does not affect the callback.
        // Key release also counts! 
        void registerKeyPressCallback(void (*)());
        // Check if no keys are pressed down
        bool isKeyboardClear();
        void clear();
        uint8_t getLastKeycode();
        uint8_t peekLastKeycode();
        uint8_t keysAvailable();
};
