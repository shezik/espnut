#pragma once

#include <Arduino.h>
#include "MatrixKeyboard.h"

#define INVALID_KEYCODE 0xFFFF
#define ValidateKeycode(keycode) (keycode != INVALID_KEYCODE && keycode != INVALID_KEYCODE >> 1)

class KeyboardMgr {
    protected:
        uint8_t powerButtonPin;
        void (*keyPressCallback)() = nullptr;
        matrix_keyboard_handle_t *mkHandle = nullptr;
        QueueHandle_t keyQueue = NULL;
        SemaphoreHandle_t mutex = NULL;
    public:
        KeyboardMgr(uint8_t);
        ~KeyboardMgr();
        void init();
        void blockingWaitForKey();
        void skipReleaseCheck();
        uint16_t getPositiveKeycode();
        // Call before init(). skipReleaseCheck() does not affect the callback.
        // Key release also counts! 
        void registerKeyPressCallback(void (*)());
        // Check if no keys are pressed down
        bool isKeyboardClear();
        void clear();
        uint16_t getLastKeycode();
        uint16_t peekLastKeycode();
        uint8_t keysAvailable();
        QueueHandle_t getKeyQueue();
        SemaphoreHandle_t getMutex();
};
