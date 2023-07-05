#pragma once

#include "GEM_u8g2.h"

// Modifications on GEM_u8g2.h of spirik/GEM@^1.4.0:
// Line 134: virtual void drawMenu();
// Line 141: protected:
class GEMProxy : public GEM_u8g2 {
    public:
        using GEM_u8g2::GEM_u8g2;

        void registerDrawMenuCallback(void (*callback)()) {
            drawMenuCallback = callback;
        };

        GEMPage *getMenuPageCurrent() {
            return _menuPageCurrent;
        };

        void drawMenu() override {
            _u8g2.clearBuffer();

            drawTitleBar();
            printMenuItems();
            drawMenuPointer();
            drawScrollbar();
            if (drawMenuCallback)
                drawMenuCallback();

            _u8g2.sendBuffer();
        }

    private:
        void (*drawMenuCallback)() = nullptr;
};
