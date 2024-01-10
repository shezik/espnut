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

// Modifications to GEM_u8g2.cpp in spirik/GEM 1.5.1:
// Line 251: `// _u8g2.clear();`

// Modifications to GEM_u8g2.h in spirik/GEM 1.5.1:
// Line 151: `virtual GEM_u8g2& drawMenu();`
// Line 158: `protected:`
class GEMProxy : public GEM_u8g2 {
    protected:
        void (*drawMenuCallback)() = nullptr;

    public:
        using GEM_u8g2::GEM_u8g2;

        void registerDrawMenuCallback(void (*callback)()) {
            drawMenuCallback = callback;
        };

        GEM_u8g2& drawMenu() override {
            _u8g2.clearBuffer();

            drawTitleBar();
            printMenuItems();
            drawMenuPointer();
            drawScrollbar();
            if (drawMenuCallback)
                drawMenuCallback();

            _u8g2.sendBuffer();
            return *this;
        }

        GEMAppearance* getCurrentAppearance() {
            return GEM_u8g2::getCurrentAppearance();
        }
};

// Modifications to GEMPage.h in spirik/GEM 1.5.1:
// Line 76: `protected:`
class GEMPageProxy : public GEMPage {
    public:
        using GEMPage::GEMPage;

        void setCurrentItemNum(uint8_t num) {
            currentItemNum = num;
        }
        // Excluding hidden ones
        uint8_t getItemsCount() {
            return itemsCount;
        }
        // Including hidden ones
        uint8_t getItemsCountTotal() {
            return itemsCountTotal;
        }
};
