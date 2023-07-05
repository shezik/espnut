#pragma once

#define VERSION "1.0"

#ifdef USE_ESP_DEEP_SLEEP
    #undef USE_ESP_DEEP_SLEEP
#endif

// LittleFS
#define FORMAT_LITTLEFS_IF_FAILED true

// U8g2
#define U8G2_DISPLAY_TYPE U8G2_ST7567_JLX12864_F_4W_SW_SPI
#define SPI_DC 40    // SPI_MISO
#define SPI_DATA 42  // SPI_MOSI
#define SPI_CLK 41
#define SPI_CS 38
#define U8G2_RESET_PIN 39
#define U8G2_BUS_CLK 4000000  // Visit https://github.com/olikraus/u8g2/wiki/u8g2reference#setbusclock

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

// Emulator
#define SIM_RUNS_BEFORE_CHECK 10
#define MAX_WAKEUP_ATTEMPTS 128
#define RESTORE_FLAG_FILENAME "/.RESTORE"
#define RESTORE_STATE_FILENAME "/.RESTORE_STATE"
#define NUT_FREQUENCY_HZ 215000

// Menu
#define ITEMS_PER_PAGE 2
#define CONFIG_FILENAME "/.CONFIG"
#define FALLBACK_CONTRAST 80  // 0~255
#define FALLBACK_ENABLE_POWER_MGMT true
#define FALLBACK_UNLOCK_SPEED false
#define FALLBACK_ENABLE_LOGGING false
#define HOLD_DOWN_LENGTH 800  // ms

// DispInterface
#define XBM_FONT_FILENAME "xbmFont128x32.h"

// KeyboardMgr
#define KEY_SCAN_INTERVAL 100  // ms
#define KEY_QUEUE_LENGTH 32
#define ROW_GPIOS 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
#define COL_GPIOS 14, 15, 16, 17
#define ROW_GPIOS_N 10
#define COL_GPIOS_N 4
