#pragma once

// LittleFS
#define FORMAT_LITTLEFS_IF_FAILED true

// Kbd_8x5_CH450 (Software I2C)
#define CH450_SDA 21
#define CH450_SCL 22
#define CH450_INT 34
#define CH450_FREQ 1000000

// U8g2
#define U8G2_DISPLAY_TYPE U8G2_ST7565_JLX12864_F_4W_SW_SPI
#define VSPI_DC 19    // VSPI_MISO
#define VSPI_DATA 23  // VSPI_MOSI
#define VSPI_CLK 18
#define VSPI_CS 5
#define U8G2_RESET_PIN U8X8_PIN_NONE
#define U8G2_BUS_CLK 4000000  // Visit https://github.com/olikraus/u8g2/wiki/u8g2reference#setbusclock
