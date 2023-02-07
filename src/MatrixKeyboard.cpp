#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/dedic_gpio.h"
#include "driver/gpio.h"
#include "MatrixKeyboard.h"

typedef struct {
    dedic_gpio_bundle_handle_t row_bundle;
    dedic_gpio_bundle_handle_t col_bundle;
    uint8_t row_gpios_n;
    uint8_t col_gpios_n;
    uint8_t debounce_ms;
    uint32_t last_row_state;
    uint32_t last_col_state;
} matrix_keyboard_handle_t;

static uint32_t readRow(matrix_keyboard_handle_t *handle) {
    dedic_gpio_bundle_write(handle->col_bundle, COL_1, 0);
    dedic_gpio_bundle_write(handle->row_bundle, ROW_1, ROW_1);
    return (~dedic_gpio_bundle_read_in(handle->row_bundle)) & (ROW_1);
}

static uint32_t readCol(matrix_keyboard_handle_t *handle) {
    dedic_gpio_bundle_write(handle->row_bundle, ROW_1, 0);
    dedic_gpio_bundle_write(handle->col_bundle, COL_1, COL_1);
    return (~dedic_gpio_bundle_read_in(handle->col_bundle)) & (COL_1);
}

// Something must have changed for this to be called
static uint32_t generateKeycode(matrix_keyboard_handle_t *handle, uint32_t rowData, uint32_t colData) {
    uint32_t rowDiff = rowData ^ handle->last_row_state;
    uint32_t colDiff = colData ^ handle->last_col_state;

    // Once this statement is false, the I/O swapping method no longer gives correct results.
    // Shouldn't be a problem if keys are pressed/released one by one.
    // Ghosting may be solved with diodes. (Imagine an NKRO calculator keypad lol)
    if (__builtin_popcount(rowDiff) <= 1 || __builtin_popcount(colDiff) <= 1) {
        if (!rowDiff) {  // All keys on one row, which already contains pressed keys
            uint8_t row = __builtin_ffs(rowData) - 1;
            for (/*omitted*/; colDiff; colDiff &= colDiff - 1) {
                uint8_t col = __builtin_ffs(colDiff) - 1;
                MakeKeycode(colData & (1 << col), row, col);
            }
        } else if (!colDiff) {  // All keys on one column, which already contains pressed keys
            uint8_t col = __builtin_ffs(colData) - 1;
            for (/*omitted*/; rowDiff; rowDiff &= rowDiff - 1) {
                uint8_t row = __builtin_ffs(rowDiff) - 1;
                MakeKeycode(rowData & (1 << row), row, col);
            }
        } else {
            for (/*omitted*/; rowDiff; rowDiff &= rowDiff - 1) {
                uint8_t row = __builtin_ffs(rowDiff) - 1;
                for (/*omitted*/; colDiff; colDiff &= colDiff - 1) {
                    uint8_t col = __builtin_ffs(colDiff) - 1;
                    MakeKeycode(colData & (1 << col), row, col);
                }
            }
        }
    }
}

esp_err_t MatrixKeyboardInit(const matrix_keyboard_config_t *config, matrix_keyboard_handle_t **handle) {
    if (!config)
        return ESP_ERR_INVALID_ARG;
    matrix_keyboard_handle_t *mkhandle = calloc(1, sizeof(matrix_keyboard_handle_t));
    if (!mkhandle)
        return ESP_ERR_NO_MEM;
    mkhandle->row_gpios_n = config->row_gpios_n;
    mkhandle->col_gpios_n = config->col_gpios_n;
    mkhandle->debounce_ms = config->debounce_ms;

    gpio_config_t io_conf = {
        .mode = GPIO_MODE_INPUT_OUTPUT_OD,
        .pull_up_en = 1;
    };
    for (uint8_t i = 0; i < config->row_gpios_n; i++) {
        io_conf.pin_bit_mask = 1ULL << config->row_gpios[i];
        gpio_config(&io_conf);
    }
    for (uint8_t i = 0; i < config->col_gpios_n; i++) {
        io_conf.pin_bit_mask = 1ULL << config->col_gpios[i];
        gpio_config(&io_conf);
    }

    dedic_gpio_bundle_config_t bundle_row_config = {
        .gpio_array = config->row_gpios,
        .array_size = config->row_gpios_n,
        .flags = {
            .in_en = 1,
            .out_en = 1,
        },
    };
    dedic_gpio_bundle_config_t bundle_col_config = {
        .gpio_array = config->col_gpios;
        .array_size = config->col_gpios_n;
        .flags = {
            .in_en = 1,
            .out_en = 1,
        },
    };
    if (!dedic_gpio_new_bundle(&bundle_row_config, &mkhandle->row_bundle) || \
        !dedic_gpio_new_bundle(&bundle_col_config, &mkhandle->col_bundle))
        return ESP_FAIL;

    *handle = mkhandle;
    return ESP_OK;
}

esp_err_t MatrixKeyboardDeinit(matrix_keyboard_handle_t *handle) {
    if (!handle)
        return ESP_ERR_INVALID_ARG;
    dedic_gpio_del_bundle(handle->row_bundle);
    dedic_gpio_del_bundle(handle->col_bundle);
    free(handle);
    return ESP_OK;
}

void MatrixKeyboardStart() {

}

void MatrixKeyboardStop() {

}

void MatrixKeyboardLoop(matrix_keyboard_config_t *config) {
    matrix_keyboard_handle_t *handle = nullptr;
    MatrixKeyboardInit(config, &handle);
    uint32_t row, col;
    for (;;) {
        // vTaskDelay(100 ms);
        if (readRow() == handle->last_row_state && readCol == handle->last_col_state)
            continue;
        // vTaskDelay(Debounce time);
        row = readRow();
        col = readCol();
        generateKeycode(handle, row, col);  // Handles multiple keys, including key status
        handle->last_row_state = row;
        handle->last_col_state = col;
    }
}
