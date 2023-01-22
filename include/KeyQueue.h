#pragma once

#define KeyQueueMaxSize 32

#include <Arduino.h>  // for uint8_t definition

class KeyQueue
{
    protected:
        int queue[KeyQueueMaxSize];
        uint8_t availableKeys = 0;
    public:
        bool isRingBuffer = false;
        int getLastKeycode();
        void removeLastKeycode();
        bool queueKeycode(int newcode);
        uint8_t count();
        void clear();
};
