#pragma once

#define VERSION "1.0"

// LittleFS
#define FORMAT_LITTLEFS_IF_FAILED true

// U8g2
#define U8G2_DISPLAY_TYPE U8G2_ST7567_ERC13232_F_4W_SW_SPI
#define SPI_DC 40    // SPI_MISO
#define SPI_DATA 38  // SPI_MOSI
#define SPI_CLK 39
#define SPI_CS 42
#define U8G2_RESET_PIN 41
#define U8G2_BUS_CLK 4000000  // Visit https://github.com/olikraus/u8g2/wiki/u8g2reference#setbusclock

// PowerMgr
#define LDO_ENABLE 21
#define DISPLAY_BACKLIGHT_CONTROL 2
#define FALLBACK_BACKLIGHT_TIMEOUT 10  // Fallback value, in seconds
#define FALLBACK_DEEP_SLEEP_TIMEOUT 5  // Fallback value, in minutes
#define FALLBACK_CPU_FREQUENCY_MHZ 240
#define POWER_BUTTON 18

// Emulator
#define RESTORE_FLAG_FILENAME "/.RESTORE"
#define RESTORE_STATE_FILENAME "/.RESTORE_STATE"
#define NUT_FREQUENCY_HZ 215000

// Menu
#define CONFIG_FILENAME "/.CONFIG"
#define FALLBACK_CONTRAST 191  // 0~255
#define FALLBACK_UNLOCK_SPEED false
#define FALLBACK_ENABLE_LOGGING false
#define HOLD_DOWN_LENGTH 1000  // ms

// DispInterface
#define XBM_FONT_FILENAME "xbmFont128x32.h"

// KeyboardMgr
#define KEY_SCAN_INTERVAL 100  // ms
#define KEY_QUEUE_LENGTH 32
#define ROW_GPIOS 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
#define COL_GPIOS 14, 15, 16, 17
#define ROW_GPIOS_N 10
#define COL_GPIOS_N 4
