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

#define VERSION "1.0"

#ifdef USE_ESP_DEEP_SLEEP
    #undef USE_ESP_DEEP_SLEEP
#endif

// Definitions used by both Menu and NutEmu
#define FILENAME_LENGTH 32
#define FILE_PATH_LENGTH 128
#define FILE_LIST_LENGTH 64
#define MAIN_PAGE_TITLE_LENGTH 64

// LittleFS
#define FORMAT_LITTLEFS_IF_FAILED true

// U8g2
#define U8G2_DISPLAY_TYPE U8G2_ST7567_LW12832_F_4W_HW_SPI
#define SPI_DC 40    // SPI_MISO
#define SPI_DATA 42  // SPI_MOSI
#define SPI_CLK 41
#define SPI_CS 38
#define U8G2_RESET_PIN 39
#define U8G2_BUS_CLK 4E6  // Visit https://github.com/olikraus/u8g2/wiki/u8g2reference#setbusclock

// PowerMgr
#define LDO_ENABLE 21
#define DISPLAY_BACKLIGHT_CONTROL 2
#define FALLBACK_BACKLIGHT_TIMEOUT 10  // Fallback value, in seconds
#define FALLBACK_DEEP_SLEEP_TIMEOUT 5  // Fallback value, in minutes
#define FALLBACK_CPU_FREQUENCY_MHZ 240
#define POWER_BUTTON 18
#define BAT_LVL_CHK 1
#define BAT_LVL_CHK_ADC_CHANNEL ADC1_CHANNEL_0  // Must be one ADC1 channel if not modifying code
#define BAT_CHRG 47
#define BAT_MAX_VOLTAGE 4200  // TP4054, mV
#define BAT_MIN_VOLTAGE 3700
#define RESISTOR_RATIO (1 / 1)  // R1 divided by R2 in this schematic: BAT_VCC---R1---BAT_LVL_CHK---R2---GND. In the Gen 1 PCB design this would be R7 / R9.
#define LOW_BAT_SHUTDOWN_DELAY 2000  // ms

// Emulator
#define SIM_RUNS_BEFORE_CHECK 10
#define MAX_WAKEUP_ATTEMPTS 128
#define RESTORE_FLAG_FILENAME "/.RESTORE"
#define RESTORE_STATE_FILENAME "/.RESTORE_STATE"
#define NUT_FREQUENCY_HZ 215000

// Menu
#define ITEMS_PER_PAGE 2
#define ITEM_HEIGHT 10
#define PAGE_TOP_OFFSET 10
#define VALUES_LEFT_OFFSET (/* Default value */ 86 + /* distance between rightmost uint8_t editing cursor and scroll bar */ 23 - /* minimum value of that */ 5)
#define CONFIG_FILENAME "/.CONFIG"
#define FALLBACK_BRIGHTNESS 100
#define FALLBACK_CONTRAST 80  // 0~255
#define FALLBACK_ENABLE_POWER_MGMT true
#define FALLBACK_UNLOCK_SPEED false
#define FALLBACK_ENABLE_LOGGING false
#define HOLD_DOWN_LENGTH 800  // ms

// DispInterface
#define XBM_FONT_FILENAME "xbmFont128x32.h"

// KeyboardMgr
#define KEY_SCAN_INTERVAL 10  // ms
#define KEY_QUEUE_LENGTH 32
#define ROW_GPIOS 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
#define COL_GPIOS 14, 15, 16, 17
#define ROW_GPIOS_N 10
#define COL_GPIOS_N 4
