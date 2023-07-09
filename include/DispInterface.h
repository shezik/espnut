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
#include "proc_nut.h"
#include "SettingsMgr.h"
#include "U8g2lib.h"
#include "Configuration.h"
#include XBM_FONT_FILENAME

class DispInterface {
    protected:
        static DispInterface *context;
        SettingsMgr &sm;
        U8G2_DISPLAY_TYPE &u8g2;
        uint16_t blinkMs = 0;
        bool lowBatAnnOverride = false;
        void drawSegments(segment_bitmap_t *, bool);
        bool blinkTick();
    public:
        DispInterface(SettingsMgr &, U8G2_DISPLAY_TYPE &);
        ~DispInterface();
        bool init();
        static void applySettings();
        void updateDisplay(nut_reg_t *, bool = false);
        void sendCriticalMsg(char *);
        void setLowBatAnnunciator(bool);
        void setLowBatAnnunciatiorBlink(uint16_t);
        U8G2_DISPLAY_TYPE *getU8g2();
        void drawBattery(uint8_t, uint8_t, uint8_t, bool);
        void drawCheckerboard(bool = false);
        void sendLowBattery();
        const uint8_t batteryIconWidth = 11, batteryIconHeight = 5;
};
