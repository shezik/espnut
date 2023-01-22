/*
    Simple FIFO key queue
*/

#include "KeyQueue.h"

KeyQueue::KeyQueue(){

}

int KeyQueue::getLastKeycode() {
    return availableKeys ? queue[availableKeys - 1] : -1;
        // Please, check if there is any keycode left in the queue
        // before calling this, to avoid getting false results!
}

void KeyQueue::removeLastKeycode() {
    if (availableKeys)
        availableKeys--;
}

bool KeyQueue::queueKeycode(int newcode) {
    if (!isRingBuffer && availableKeys == KeyQueueMaxSize)
        return false;

    for (uint8_t i = (availableKeys < KeyQueueMaxSize ? availableKeys++ : KeyQueueMaxSize - 1); i > 0; i--)
        queue[i] = queue[i - 1];

    queue[0] = newcode;

    return true;
}

uint8_t KeyQueue::count() {
    return availableKeys;
}
