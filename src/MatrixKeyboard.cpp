#include "MatrixKeyboard.h"
#include "Configuration.h"

static void writeRow(matrix_keyboard_handle_t *handle, uint8_t index) {
    dedic_gpio_bundle_write(handle->row_bundle, ROW_1, (~(1 << index)) & ROW_1);
}

static uint32_t readCol(matrix_keyboard_handle_t *handle) {
    return (~dedic_gpio_bundle_read_in(handle->col_bundle)) & (COL_1);
}

static void getKeycode(matrix_keyboard_handle_t *handle) {
    for (uint8_t row = 0; row < handle->row_gpios_n; row++) {
        // IO RW section
        writeRow(handle, row);
        uint32_t colData = readCol(handle);

        // Debounce section
        uint32_t prevColData = colData;
        uint8_t debounceResetCount = 0;
        for (uint8_t i = 0; i < handle->debounce_stable_count; i++) {
            colData = readCol(handle);
            if (colData == prevColData)
                continue;
            // Otherwise reset counter
            if (debounceResetCount == handle->debounce_reset_max_count) {
                printf(MatrixKeyboardTag "Maximum debounce reset count (%d) exceeded!\n", handle->debounce_reset_max_count);
                break;
            }
            printf(MatrixKeyboardTag "Debouncing\n");
            i = 0;
            debounceResetCount++;
            prevColData = colData;
        }

        // Keycode calculation section
        uint32_t colDiff = colData ^ handle->last_col_state[row];
        if (!colDiff)
            continue;
        handle->last_col_state[row] = colData;
        for (/*omitted*/; colDiff; colDiff &= colDiff - 1) {
            uint8_t col = __builtin_ffs(colDiff) - 1;
            uint16_t keycode = MakeKeycode(colData & (1 << col), row, col);
            if (handle->key_event)
                handle->key_event();
            if (handle->skip_key_releases) {
                if (!(colData & (1 << col))) {
                    printf(MatrixKeyboardTag "Got keycode %d (row %d, col %d), but skipped\n", keycode, row, col);
                    continue;
                }
                handle->skip_key_releases = false;
            }
            printf(MatrixKeyboardTag "Got keycode %d (row %d, col %d), adding to queue\n", keycode, row, col);
            xQueueSend(*handle->key_queue, (void *) &keycode, 0);
        }
    }
}

static void matrixKeyboardLoop(void *pvParameters) {
    matrix_keyboard_handle_t *handle = static_cast<matrix_keyboard_handle_t *>(pvParameters);
    for (;;) {
        getKeycode(handle);
        vTaskDelay(pdMS_TO_TICKS(KEY_SCAN_INTERVAL));
    }
}

esp_err_t MatrixKeyboardInit(const matrix_keyboard_config_t *config, matrix_keyboard_handle_t **handle) {
    if (!config)
        return ESP_ERR_INVALID_ARG;
    matrix_keyboard_handle_t *mkhandle = static_cast<matrix_keyboard_handle_t *>(calloc(1, sizeof(matrix_keyboard_handle_t) + config->col_gpios_n * sizeof(uint8_t)));
    if (!mkhandle)
        return ESP_ERR_NO_MEM;
    mkhandle->row_gpios_n = config->row_gpios_n;
    mkhandle->col_gpios_n = config->col_gpios_n;
    mkhandle->debounce_stable_count = config->debounce_stable_count;
    mkhandle->debounce_reset_max_count = config->debounce_reset_max_count;
    mkhandle->key_queue = config->key_queue;
    mkhandle->task_name = config->task_name;
    mkhandle->key_event = config->key_event;
    mkhandle->skip_key_releases = false;

    gpio_config_t row_conf = {
        .mode = GPIO_MODE_OUTPUT_OD,
        // .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    for (uint8_t i = 0; i < config->row_gpios_n; i++) {
        row_conf.pin_bit_mask = 1ULL << config->row_gpios[i];
        gpio_config(&row_conf);
    }
    gpio_config_t col_conf = {
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    for (uint8_t i = 0; i < config->col_gpios_n; i++) {
        col_conf.pin_bit_mask = 1ULL << config->col_gpios[i];
        gpio_config(&col_conf);
    }

    dedic_gpio_bundle_config_t bundle_row_config = {
        .gpio_array = config->row_gpios,
        .array_size = config->row_gpios_n,
        .flags = {
            .out_en = 1,
        },
    };
    dedic_gpio_bundle_config_t bundle_col_config = {
        .gpio_array = config->col_gpios,
        .array_size = config->col_gpios_n,
        .flags = {
            .in_en = 1,
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
    vTaskDelete(handle->task_handle);
    dedic_gpio_del_bundle(handle->row_bundle);
    dedic_gpio_del_bundle(handle->col_bundle);
    free(handle);
    return ESP_OK;
}

void MatrixKeyboardStart(matrix_keyboard_handle_t *handle) {
    static matrix_keyboard_handle_t *handle_ = handle;
    xTaskCreatePinnedToCore(matrixKeyboardLoop, handle_->task_name, 8192, handle_, 1, &handle_->task_handle, 0);
}

void MatrixKeyboardStop(matrix_keyboard_handle_t *handle) {
    vTaskSuspend(handle->task_handle);
}

void MatrixKeyboardSkipKeyReleases(matrix_keyboard_handle_t *handle) {
    handle->skip_key_releases = true;
}
