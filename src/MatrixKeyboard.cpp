#include "MatrixKeyboard.h"
#include "util.h"
#include "Configuration.h"
#include "freertos/task.h"

static void writeRow(matrix_keyboard_handle_t *handle, uint8_t index) {
    for (uint8_t i = 0; i < handle->row_gpios_n; i++)
        digitalWrite(handle->row_gpios[i], i == index ? LOW : HIGH);
}

static uint32_t readCol(matrix_keyboard_handle_t *handle) {
    uint32_t result = 0;
    for (uint8_t i = 0; i < handle->col_gpios_n; i++)
        result |= !(digitalRead(handle->col_gpios[i])) << i;
    return result;
}

static void getKeycode(matrix_keyboard_handle_t *handle) {
    for (uint8_t col = 0; col < handle->row_gpios_n; col++) {
        // IO RW section
        writeRow(handle, col);
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
                printf_log(MatrixKeyboardTag "Maximum debounce reset count (%d) exceeded!\n", handle->debounce_reset_max_count);
                break;
            }
            printf_log(MatrixKeyboardTag "Debouncing\n");
            i = 0;
            debounceResetCount++;
            prevColData = colData;
        }

        // Keycode calculation section
        uint32_t colDiff = colData ^ handle->last_col_state[col];
        if (!colDiff)
            continue;
        handle->last_col_state[col] = colData;
        for (/*omitted*/; colDiff; colDiff &= colDiff - 1) {
            uint8_t row = __builtin_ffs(colDiff) - 1;
            uint16_t keycode = MakeKeycode(colData & (1 << row), row, col);
            if (handle->key_event)
                handle->key_event();
            if (handle->skip_key_releases) {
                if (!(colData & (1 << row))) {
                    printf_log(MatrixKeyboardTag "Got keycode %d (row %d, col %d), but skipped\n", keycode, row, col);
                    continue;
                }
                handle->skip_key_releases = false;
            }
            printf_log(MatrixKeyboardTag "Got keycode %d (row %d, col %d), adding to queue\n", keycode, row, col);
            xSemaphoreTake(handle->mutex, portMAX_DELAY);
            xQueueSend(handle->key_queue, (void *) &keycode, 0);
            xSemaphoreGive(handle->mutex);
        }
    }
}

static void checkPowerBtn(matrix_keyboard_handle_t *handle) {
    bool state = !digitalRead(handle->power_btn_pin);
    static bool prevState = state;

    if (state == prevState)
        return;

    prevState = state;
    if (handle->key_event)
        handle->key_event();

    uint16_t keycode = MakeKeycodeFromCode(state, 24 /*ON*/);
    xSemaphoreTake(handle->mutex, portMAX_DELAY);
    xQueueSend(handle->key_queue, (void *) &keycode, 0);
    xSemaphoreGive(handle->mutex);
}

static void matrixKeyboardLoop(void *pvParameters) {
    matrix_keyboard_handle_t *handle = static_cast<matrix_keyboard_handle_t *>(pvParameters);
    for (;;) {
        checkPowerBtn(handle);
        getKeycode(handle);
        vTaskDelay(pdMS_TO_TICKS(KEY_SCAN_INTERVAL));
    }
}

esp_err_t MatrixKeyboardInit(const matrix_keyboard_config_t *config, matrix_keyboard_handle_t **handle) {
    if (!config)
        return ESP_ERR_INVALID_ARG;
    matrix_keyboard_handle_t *mkhandle = static_cast<matrix_keyboard_handle_t *>(calloc(1, sizeof(matrix_keyboard_handle_t)));
    mkhandle->row_gpios = (int *) malloc(config->row_gpios_n * sizeof(int));
    mkhandle->col_gpios = (int *) malloc(config->col_gpios_n * sizeof(int));
    mkhandle->task_name = (char *) malloc(strlen(config->task_name) + 1);
    mkhandle->last_col_state = (uint32_t *) calloc(config->row_gpios_n, sizeof(uint32_t));
    if (!mkhandle || !mkhandle->row_gpios || !mkhandle->col_gpios || !mkhandle->task_name || !mkhandle->last_col_state) {
        free(mkhandle->row_gpios);
        free(mkhandle->col_gpios);
        free(mkhandle->task_name);
        free(mkhandle->last_col_state);
        free(mkhandle);
        return ESP_ERR_NO_MEM;
    }

    memcpy((void *) mkhandle->row_gpios, config->row_gpios, config->row_gpios_n * sizeof(int));
    memcpy((void *) mkhandle->col_gpios, config->col_gpios, config->col_gpios_n * sizeof(int));
    strcpy(mkhandle->task_name, config->task_name);
    mkhandle->row_gpios_n = config->row_gpios_n;
    mkhandle->col_gpios_n = config->col_gpios_n;
    mkhandle->power_btn_pin = config->power_btn_pin;
    mkhandle->debounce_stable_count = config->debounce_stable_count;
    mkhandle->debounce_reset_max_count = config->debounce_reset_max_count;
    mkhandle->key_queue = config->key_queue;
    mkhandle->mutex = config->mutex;
    mkhandle->key_event = config->key_event;
    mkhandle->skip_key_releases = false;
    
    pinMode(config->power_btn_pin, INPUT_PULLUP);
    for (uint8_t i = 0; i < config->row_gpios_n; i++)
        pinMode(config->row_gpios[i], OUTPUT);
    for (uint8_t i = 0; i < config->col_gpios_n; i++)
        pinMode(config->col_gpios[i], INPUT_PULLUP);

    *handle = mkhandle;
    return ESP_OK;
}

esp_err_t MatrixKeyboardDeinit(matrix_keyboard_handle_t *handle) {
    if (!handle)
        return ESP_ERR_INVALID_ARG;
    vTaskDelete(handle->task_handle);
    free(handle->row_gpios);
    free(handle->col_gpios);
    free(handle->task_name);
    free(handle->last_col_state);
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
