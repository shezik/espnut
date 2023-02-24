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
#include <stdint.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "queue.h"

/*
// uint8_t, 0 ~ 127
#define MakeKeycode(status, row, col) (((status & 0x01) << 7) | ((row & 0x07) << 4) | (col & 0x0F))
#define GetKeycodeStatus(keycode)         ((keycode >> 7) & 0x01)
#define GetKeycodeRow(keycode)            ((keycode >> 4) & 0x07)
#define GetKeycodeCol(keycode)            (keycode & 0x0F)
*/

// uint16_t, 0 ~ 32767
#define MakeKeycode(status, row, col) (((status & 0x01) << 15) | keycodeMap[row][col] & 0x7FFF)
#define MakeKeycodeFromContent(status, keycodeContent) (((status & 0x01) << 15) | keycodeContent & 0x7FFF)
#define GetKeycodeStatus(keycode)     ((keycode >> 15) & 0x01)
#define GetKeycodeContent(keycode)    (keycode & 0x7FFF)

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
    uint8_t debounce_stable_count;
    uint8_t debounce_reset_max_count;
    QueueHandle_t key_queue;
    char *task_name;
    void (*key_event)();
} matrix_keyboard_config_t;

typedef struct {
    int *row_gpios = nullptr;
    int *col_gpios = nullptr;
    uint8_t row_gpios_n;
    uint8_t col_gpios_n;
    uint8_t debounce_stable_count;
    uint8_t debounce_reset_max_count;
    QueueHandle_t key_queue;
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
