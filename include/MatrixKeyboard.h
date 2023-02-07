/*
    GPIO bundle interrupt is NOT AVAILABLE on ESP32-S3, how sad. (What for??)
    Poll, then.

    Keycode structure: (16x8)
        uint8_t 1        001        0001
                ^_Status ^^^_Row    ^^^^_Column
*/

#pragma once

#include <stdint.h>
#include <esp_err.h>

#define MakeKeycode(status, row, col) (((status & 0x01) << 7) | ((row & 0x07) << 4) | (col & 0x0F))
#define GetKeycodeStatus(keycode)         ((keycode >> 7) & 0x01)
#define GetKeycodeRow(keycode)            ((keycode >> 4) & 0x07)
#define GetKeycodeCol(keycode)            (keycode & 0x0F)

#define ROW_1 ((1 << handle->row_gpios_n) - 1)
#define COL_1 ((1 << handle->col_gpios_n) - 1)

typedef struct {
    const uint8_t *row_gpios;
    const uint8_t *col_gpios;
    uint8_t row_gpios_n;
    uint8_t col_gpios_n;
    uint8_t debounce_ms;
} matrix_keyboard_config_t;
