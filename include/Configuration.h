#pragma once

#define VERSION "1.0"

// LittleFS
#define FORMAT_LITTLEFS_IF_FAILED true

// U8g2
#define U8G2_DISPLAY_TYPE U8G2_ST7565_JLX12864_F_4W_SW_SPI
#define VSPI_DC 19    // VSPI_MISO
#define VSPI_DATA 23  // VSPI_MOSI
#define VSPI_CLK 18
#define VSPI_CS 5
#define U8G2_RESET_PIN U8X8_PIN_NONE
#define U8G2_BUS_CLK 4000000  // Visit https://github.com/olikraus/u8g2/wiki/u8g2reference#setbusclock

// PowerMgr
#define DISPLAY_POWER_CONTROL 32  // !! Arbitrary value
#define DISPLAY_BACKLIGHT_CONTROL 33  // !! Arbitrary value
#define FALLBACK_BACKLIGHT_TIMEOUT 10  // Fallback value, in seconds
#define FALLBACK_DEEP_SLEEP_TIMEOUT 5  // Fallback value, in minutes
#define FALLBACK_CPU_FREQUENCY_MHZ 240
#define POWER_BUTTON 8  // !! Arbitrary value

// Emulator
#define RESTORE_FLAG_FILENAME "/.RESTORE"
#define RESTORE_STATE_FILENAME "/.RESTORE_STATE"
#define NUT_FREQUENCY_HZ 215000

// Menu
#define CONFIG_FILENAME "/.CONFIG"
#define FALLBACK_CONTRAST 191  // 0~255
#define FALLBACK_UNLOCK_SPEED false
#define FALLBACK_ENABLE_LOGGING false
#define HOLD_DOWN_CYCLES 64  // !! ?

// DispInterface
#define XBM_FONT_FILENAME "xbmFont128x32.h"

// KeyboardMgr
#define KEY_QUEUE_LENGTH 32
#define ROW_GPIOS 1, 2  // !! Arbitrary value
#define COL_GPIOS 3, 4
#define ROW_GPIOS_N 2
#define COL_GPIOS_N 2
