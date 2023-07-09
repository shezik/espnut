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

/*
    GPIO bundle interrupt is NOT AVAILABLE on ESP32-S3, how sad. (What for??)
    Poll, then.

    Keycode structure:
        uint8_t 1        001        0001
                ^_Status ^^^_Row    ^^^^_Column
       uint16_t 1        010 1010 1010 1010
                ^_Status ^^^^^^^^^^^^^^^^^^_Translated keycode
*/

#pragma once

#include <Arduino.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "queue.h"

/*
// uint8_t, 0 ~ 127
#define MakeKeycode(status, row, col) ((((bool) (status)) << 7) | (((row) & 0x07) << 4) | ((col) & 0x0F))
#define GetKeycodeStatus(keycode)         (((keycode) >> 7) & 0x01)
#define GetKeycodeRow(keycode)            (((keycode) >> 4) & 0x07)
#define GetKeycodeCol(keycode)            ((keycode) & 0x0F)
*/

// uint16_t, 0 ~ 32767
#define MakeKeycode(status, row, col) ((((bool) (status)) << 15) | keycodeMap[row][col] & 0x7FFF)
#define MakeKeycodeFromCode(status, keycode) ((((bool) (status)) << 15) | (keycode) & 0x7FFF)
#define GetKeycodeStatus(keycode)     (((keycode) >> 15) & 0x01)
#define GetKeycodeContent(keycode)    ((keycode) & 0x7FFF)

const uint16_t keycodeMap[4][10] = {
    {19, 51, 115, 195, 131, 130, 194, 114, 50, 18},
    {16, 48, 112, 192, 128, 135, 199, 119, 55, 23},
    {17, 49, 113, 193, 129, 132, 196, 116, 52, 20},
    {24, 56, 120, 200, 136, 132, 197, 117, 53, 21}
};

#define MatrixKeyboardTag "MatrixKeyboard: "

// Works best when columns > rows.
typedef struct {
    const int *row_gpios;
    const int *col_gpios;
    uint8_t row_gpios_n;
    uint8_t col_gpios_n;
    uint8_t power_btn_pin;
    uint8_t debounce_stable_count;
    uint8_t debounce_reset_max_count;
    QueueHandle_t key_queue;
    SemaphoreHandle_t mutex;
    char *task_name;
    void (*key_event)();
} matrix_keyboard_config_t;

typedef struct {
    int *row_gpios = nullptr;
    int *col_gpios = nullptr;
    uint8_t row_gpios_n;
    uint8_t col_gpios_n;
    uint8_t power_btn_pin;
    uint8_t debounce_stable_count;
    uint8_t debounce_reset_max_count;
    QueueHandle_t key_queue;
    SemaphoreHandle_t mutex;
    char *task_name = nullptr;
    void (*key_event)();

    TaskHandle_t task_handle;
    bool skip_key_releases;
    uint32_t *last_col_state = nullptr;
} matrix_keyboard_handle_t;

esp_err_t MatrixKeyboardInit(const matrix_keyboard_config_t *config, matrix_keyboard_handle_t **handle);
esp_err_t MatrixKeyboardDeinit(matrix_keyboard_handle_t *handle);
void MatrixKeyboardStart(matrix_keyboard_handle_t *handle);
void MatrixKeyboardStop(matrix_keyboard_handle_t *handle);
// Skip adding release keycodes to queue until a press down keycode is found
void MatrixKeyboardSkipKeyReleases(matrix_keyboard_handle_t *handle);
