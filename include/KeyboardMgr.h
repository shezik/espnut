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
