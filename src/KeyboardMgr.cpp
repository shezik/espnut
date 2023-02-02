#include "KeyboardMgr.h"

void KeyboardMgr::keyboardCallback(void *ptr) {
    KeyboardMgr *context = static_cast<KeyboardMgr *>(ptr);

    if (!context->getBusy()) {
        context->handleKeyPress();
    }
}

KeyboardMgr::KeyboardMgr(Kbd_8x5_CH450 &keyboard_, uint8_t interruptPin_)
    : keyboard(keyboard_)
    , interruptPin(interruptPin_)
{
    // Do nothing
}

void KeyboardMgr::init() {
    pinMode(interruptPin, INPUT_PULLUP);  // CH450 keyboard interrupt (active low)
}

void KeyboardMgr::enableInterrupt() {
    if (!isInterruptEnabled)
        attachInterruptArg(interruptPin, KeyboardMgr::keyboardCallback, this, FALLING);
}

void KeyboardMgr::disableInterrupt() {
    if (isInterruptEnabled)
        detachInterrupt(interruptPin);
}

bool KeyboardMgr::getInterruptState() {
    return isInterruptEnabled;
}

void KeyboardMgr::blockingWaitForKey() {
    bool prevBusy = getBusy();
    setBusy(true);

	clear();
	while (1) {
        if (!digitalRead(interruptPin))
                handleKeyPress();  // We poll instead of using interrupts to circumvent inadequate interruption

		if (count()) {
			if (getLastKeycode() != -1) {
                clear();
                skipReleaseCheck();
                setBusy(prevBusy);
				return;
			} else {
				removeLastKeycode();
			}
		}
        yield();
	}
}

void KeyboardMgr::tick() {
    busy = true;

    if (isInterruptEnabled && !digitalRead(interruptPin))
        // This happens because ISR could be executed while busy == true (I guess.)
        // tick() should be called frequently, even though setBusy(false) also checks for this situation.
        handleKeyPress();
    else
        checkForRelease();

    busy = false;
}

void KeyboardMgr::checkForRelease() {
    if (pendingRelease && !keyboard.toState(keyboard.getKeyData())) {
        queueKeycode(-1);
        pendingRelease = false;
        if (keyPressCallback)
            keyPressCallback();
    }
}

// To wake up, press any key between SEG0 and SEG3 (keycode 02H+40H to 1FH+40H) or send any command to the chip. Either triggers interrupt. (?)
bool KeyboardMgr::chipEnterSleep() {
    setBusy(true);
    bool result = keyboard.sendConfig(true);
    setBusy(false);
    return result;
}

void KeyboardMgr::skipReleaseCheck() {
    // There's no need to tackle with ISR to get this feature working.
    // The ISR either returns with one keycode (key wasn't released immediately) or with the keycode and a '-1' (which can be cleared with clear()).
    // We just need to make sure after clearing the queue, checkForRelease() doesn't add a '-1' to it later.
    // Maybe call clear() after this?
    pendingRelease = false;
}

int KeyboardMgr::getPositiveKeycode() {
    setBusy(true);
    int keycode;

    while (1) {
        if (count()) {
            keycode = getLastKeycode();
            removeLastKeycode();
            if (keycode > 0) {
                break;
            }  // else continue;
        } else {
            keycode = -1;
            break;
        }
    }

    setBusy(false);
    return keycode;
}

void KeyboardMgr::handleKeyPress() {
    static uint8_t keyData;

    if (pendingRelease) {
        queueKeycode(-1);  // handle simultaneous keypresses  * CH450 doesn't even support that but I'd like to implement it anyway.
                           //                                 * Q: Does the original calculator support simultaneous keypresses?
                           //                                   A: Yes. "The real hardware has two-key rollover", according to nonpareil-0.78/src/keyboard.c.
                           //                                      ~And we are going to implement that later.~  Actually that's not possible with CH450 if you read the datasheet. :(
    }

    keyData = keyboard.getKeyData();
    queueKeycode(keyboard.toKeycode(keyData));
    if (!keyboard.toState(keyData)) {
        queueKeycode(-1);
        pendingRelease = false;
    } else {
        pendingRelease = true;
    }

    if (keyPressCallback)
        keyPressCallback();
}

void KeyboardMgr::registerKeyPressCallback(void (*callback)()) {
    keyPressCallback = callback;
}

bool KeyboardMgr::getPendingRelease() {
    return pendingRelease;
}

bool KeyboardMgr::getBusy() {
    return busy;
}

bool KeyboardMgr::setBusy(bool busy_) {
    if (busy_) {
        busy = true;
        return false;
    }

    busy = true;
    if (isInterruptEnabled && !digitalRead(interruptPin)) {
        handleKeyPress();
        setBusy(false);  // See if it happened again
        return true;
    }
    
    busy = false;  // There's a slight chance that an interrupt would occur between busy = true and busy = false, if not returned.
    return false;
}
