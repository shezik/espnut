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

#include "GEM_u8g2.h"

// Modifications to GEM_u8g2.h in spirik/GEM@^1.4.0:
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

        void setMenuValuesLeftOffset(uint8_t offset) {
            _menuValuesLeftOffset = offset;
            // Line 169 - 170, GEM_u8g2.cpp in spirik/GEM@^1.4.0
            _menuItemTitleLength = (_menuValuesLeftOffset - 5) / _menuItemFont[_menuItemFontSize].width;
            _menuItemValueLength = (_u8g2.getDisplayWidth() - _menuValuesLeftOffset - 6) / _menuItemFont[_menuItemFontSize].width;
        }

    private:
        void (*drawMenuCallback)() = nullptr;
};
