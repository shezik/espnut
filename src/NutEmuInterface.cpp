#include "NutEmuInterface.h"

NutEmuInterface::NutEmuInterface(KeyboardMan &kbdMan_, DispInterface &disp_)
    : kbdMan(kbdMan_)
    , disp(disp_)
{
    // Do nothing
}

bool NutEmuInterface::newProcessor(int clockFrequency, int ramSize, char *filename) {
    nv = nut_new_processor(ramSize, (void *) this);  // void *nut_reg->display is reused for storing NutEmuInterface *
    nut_read_object_file(nv, filename);
}

void NutEmuInterface::tick() {

}

bool NutEmuInterface::saveState(char *filename) {

}

bool NutEmuInterface::loadState(char *filename) {

}

void NutEmuInterface::updateDisplayCallback() {
    disp.updateDisplay(nv);
}
