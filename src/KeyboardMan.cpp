#include "KeyboardMan.h"

static void keyboardCallback(void *ptr) {
    KeyboardMan *context = static_cast<KeyboardMan *>(ptr);
    static bool keyIsDown = false;

    if (!digitalRead(34)) {
        if (keyIsDown) {
            context->queueKeycode(-1);  // should we handle multiple keypresses like this? (CH450 doesn't even support that but I'd like to know anyway.)
        } else {
            keyIsDown = true;
        }
        context->queueKeycode(context->keyboard.toKeycode(context->keyboard.getKeyData()));
    }

    if (keyIsDown && !context->keyboard.toState(context->keyboard.getKeyData())) {
        context->queueKeycode(-1);
        keyIsDown = false;
    }
}

KeyboardMan::KeyboardMan(Kbd_8x5_CH450 keyboard_, uint8_t interruptPin_)
    : keyboard(keyboard_)
    , interruptPin(interruptPin_)
{
    // Do nothing
}

bool KeyboardMan::init() {
    pinMode(interruptPin, INPUT);  // CH450 keyboard interrupt (active low)
    return keyboard.init();
}

void KeyboardMan::enableInterrupt() {
    if (!isInterruptEnabled)
        attachInterruptArg(interruptPin, keyboardCallback, this, FALLING);
}

void KeyboardMan::disableInterrupt() {
    if (isInterruptEnabled)
        detachInterrupt(interruptPin);
}

bool KeyboardMan::getInterruptState() {
    return isInterruptEnabled;
}

void KeyboardMan::blockingWaitForKey() {
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
	    }
    }
}
