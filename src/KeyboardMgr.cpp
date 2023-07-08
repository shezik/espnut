#include "KeyboardMgr.h"
#include "freertos/portmacro.h"
#include "Configuration.h"

KeyboardMgr::KeyboardMgr(uint8_t powerButtonPin_)
    : powerButtonPin(powerButtonPin_)
{
    // Do nothing
}

KeyboardMgr::~KeyboardMgr() {
    // MatrixKeyboardStop(mkHandle);
    MatrixKeyboardDeinit(mkHandle);
    mkHandle = nullptr;
}

void KeyboardMgr::init() {
    keyQueue = xQueueCreate(KEY_QUEUE_LENGTH, sizeof(uint16_t));
    mutex = xSemaphoreCreateMutex();
    int rowGPIOs[ROW_GPIOS_N] = {ROW_GPIOS};
    int colGPIOs[COL_GPIOS_N] = {COL_GPIOS};
    matrix_keyboard_config_t mkConf = {
        .row_gpios = rowGPIOs,
        .col_gpios = colGPIOs,
        .row_gpios_n = ROW_GPIOS_N,
        .col_gpios_n = COL_GPIOS_N,
        .power_btn_pin = powerButtonPin,
        .debounce_stable_count = 8,
        .debounce_reset_max_count = 2,  // ?
        .key_queue = keyQueue,
        .mutex = mutex,
        .task_name = "MatrixKbd",
        .key_event = keyPressCallback
    };
    MatrixKeyboardInit(&mkConf, &mkHandle);
    MatrixKeyboardStart(mkHandle);
}

void KeyboardMgr::blockingWaitForKey() {
    clear();
    uint16_t keycode;
    while (!(xQueueReceive(keyQueue, &keycode, portMAX_DELAY) == pdPASS && GetKeycodeStatus(keycode)))
        vTaskDelay(pdMS_TO_TICKS(10));
    clear();
}

void KeyboardMgr::skipReleaseCheck() {
    MatrixKeyboardSkipKeyReleases(mkHandle);
}

uint16_t KeyboardMgr::getPositiveKeycode() {
    uint16_t keycode;
    for (;;) {
        if (xQueueReceive(keyQueue, &keycode, 0) != pdPASS)
            return INVALID_KEYCODE;
        if (GetKeycodeStatus(keycode))
            return keycode;
    }
}

void KeyboardMgr::registerKeyPressCallback(void (*callback)()) {
    keyPressCallback = callback;
}

bool KeyboardMgr::isKeyboardClear() {
    for (uint8_t i = 0; i < mkHandle->col_gpios_n; i++) {
        if (mkHandle->last_col_state[i])
            return false;
    }
    return true;
}

void KeyboardMgr::clear() {
    xQueueReset(keyQueue);
    skipReleaseCheck();
}

uint16_t KeyboardMgr::getLastKeycode() {
    uint16_t keycode;
    if (xQueueReceive(keyQueue, &keycode, 0) != pdPASS)
        return INVALID_KEYCODE;
    return keycode;
}

uint16_t KeyboardMgr::peekLastKeycode() {
    uint16_t keycode;
    if (xQueuePeek(keyQueue, &keycode, 0) != pdPASS)
        return INVALID_KEYCODE;
    return keycode;
}

uint8_t KeyboardMgr::keysAvailable() {
    return KEY_QUEUE_LENGTH - uxQueueSpacesAvailable(keyQueue);
}

QueueHandle_t KeyboardMgr::getKeyQueue() {
    return keyQueue;
}

SemaphoreHandle_t KeyboardMgr::getMutex() {
    return mutex;
}
