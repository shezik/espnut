/*
    GPIO bundle interrupt is NOT AVAILABLE on ESP32-S3, how sad. (What for??)
    Poll, then.

    Keycode structure: (16x8)
        uint8_t 1        001        0001
                ^_Status ^^^_Row    ^^^^_Column
*/

#pragma once

#include <stdint.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "queue.h"
#include "driver/dedic_gpio.h"
#include "driver/gpio.h"

#define MakeKeycode(status, row, col) (((status & 0x01) << 7) | ((row & 0x07) << 4) | (col & 0x0F))
#define GetKeycodeStatus(keycode)         ((keycode >> 7) & 0x01)
#define GetKeycodeRow(keycode)            ((keycode >> 4) & 0x07)
#define GetKeycodeCol(keycode)            (keycode & 0x0F)

#define ROW_1 ((1 << handle->row_gpios_n) - 1)
#define COL_1 ((1 << handle->col_gpios_n) - 1)

#define MatrixKeyboardTag "MatrixKeyboard: "

// Works best when columns > rows.
typedef struct {
    const int *row_gpios;
    const int *col_gpios;
    uint8_t row_gpios_n;
    uint8_t col_gpios_n;
    uint8_t debounce_stable_count;
    uint8_t debounce_reset_max_count;
    QueueHandle_t *key_queue;
    char *task_name;
} matrix_keyboard_config_t;

typedef struct {
    dedic_gpio_bundle_handle_t row_bundle;
    dedic_gpio_bundle_handle_t col_bundle;
    uint8_t row_gpios_n;
    uint8_t col_gpios_n;
    uint8_t debounce_stable_count;
    uint8_t debounce_reset_max_count;
    QueueHandle_t *key_queue;
    char *task_name;
    TaskHandle_t task_handle;
    uint32_t last_col_state[0];
} matrix_keyboard_handle_t;

esp_err_t MatrixKeyboardInit(const matrix_keyboard_config_t *config, matrix_keyboard_handle_t **handle);
esp_err_t MatrixKeyboardDeinit(matrix_keyboard_handle_t *handle);
void MatrixKeyboardStart(matrix_keyboard_handle_t *handle_);
void MatrixKeyboardStop(matrix_keyboard_handle_t *handle);
