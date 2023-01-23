#include "KeyboardMgr.h"

static void keyboardCallback(void *ptr) {
    KeyboardMgr *context = static_cast<KeyboardMgr *>(ptr);

    if (!context->busy) {
        context->busy = true;

        if (context->pendingRelease) {
            context->queueKeycode(-1);  // handle simultaneous keypresses  * CH450 doesn't even support that but I'd like to implement it anyway.
                                        //                                 * Q: Does the original calculator support simultaneous keypresses?
                                        //                                   A: Yes. "The real hardware has two-key rollover", according to nonpareil-0.78/src/keyboard.c.
                                        //                                      ~And we are going to implement that later. !!~  Actually that's not possible with CH450 if you read the datasheet. :(
        } else {
            context->pendingRelease = true;
        }
        context->queueKeycode(context->keyboard.toKeycode(context->keyboard.getKeyData()));
        
        context->busy = false;
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
        attachInterruptArg(interruptPin, keyboardCallback, this, FALLING);
}

void KeyboardMgr::disableInterrupt() {
    if (isInterruptEnabled)
        detachInterrupt(interruptPin);
}

bool KeyboardMgr::getInterruptState() {
    return isInterruptEnabled;
}

void KeyboardMgr::blockingWaitForKey() {
    if (isInterruptEnabled) {
	    clear();
	    while (1) {
	    	if (count()) {  // Keys are registered via interrupts
	    		if (getLastKeycode() != -1) {
                    clear();
	    			return;
	    		} else {
	    			removeLastKeycode();
	    		}
	    	}
            yield();
	    }
    }
}

void KeyboardMgr::checkForRelease() {
    busy = true;

    if (pendingRelease && !keyboard.toState(keyboard.getKeyData())) {
        queueKeycode(-1);
        pendingRelease = false;
    }

    busy = false;
}

// To wake up, press any key between SEG0 and SEG3 (keycode 02H+40H to 1FH+40H) or send any command to the chip. Either triggers interrupt. (?)
// Setting sendInterruptOnWake to false disables the internal keyboard scanner so no interrupt will be sent on wake.
// However the chip still wakes up on key press even if the keyboard scanner is turned off.
// You can re-enable the keyboard scanner by calling Kbd_8x5_CH450::init().
bool KeyboardMgr::chipEnterSleep(bool sendInterruptOnWake) {
    busy = true;
    bool result = keyboard.sendConfig(true, sendInterruptOnWake);
    busy = false;
    return result;
}
