#include "KeyboardMgr.h"

KeyboardMgr::KeyboardMgr() {
    // Do nothing
}

KeyboardMgr::~KeyboardMgr() {
    // MatrixKeyboardStop(mkHandle);
    MatrixKeyboardDeinit(mkHandle);
    mkHandle = nullptr;
}

void KeyboardMgr::init() {
    *keyQueue = xQueueCreate(KEY_QUEUE_LENGTH, sizeof(uint8_t));
    int rowGPIOs[ROW_GPIOS_N] = {ROW_GPIOS};
    int colGPIOs[COL_GPIOS_N] = {COL_GPIOS};
    matrix_keyboard_config_t mkConf = {
        .row_gpios = rowGPIOs,
        .col_gpios = colGPIOs,
        .row_gpios_n = ROW_GPIOS_N,
        .col_gpios_n = COL_GPIOS_N,
        .debounce_stable_count = 8,
        .debounce_reset_max_count = 8,  // ?
        .key_queue = keyQueue,
        .task_name = "MatrixKbd",
        .key_event = keyPressCallback
    };
    MatrixKeyboardInit(&mkConf, &mkHandle);
    MatrixKeyboardStart(mkHandle);
}

void KeyboardMgr::blockingWaitForKey() {
    clear();
    uint8_t keycode;
    while (!(xQueueReceive(*keyQueue, &keycode, portMAX_DELAY) == pdPASS && GetKeycodeStatus(keycode)));  // !! vTaskDelay() if required.
    clear();
}

void KeyboardMgr::skipReleaseCheck() {
    MatrixKeyboardSkipKeyReleases(mkHandle);
}

uint8_t KeyboardMgr::getPositiveKeycode() {
    uint8_t keycode;
    for (;;) {
        if (xQueueReceive(*keyQueue, &keycode, 0) != pdPASS)
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
    xQueueReset(*keyQueue);
    skipReleaseCheck();
}

uint8_t KeyboardMgr::getLastKeycode() {
    uint8_t keycode;
    if (xQueueReceive(*keyQueue, &keycode, 0) != pdPASS)
        return INVALID_KEYCODE;
    return keycode;
}

uint8_t KeyboardMgr::peekLastKeycode() {
    uint8_t keycode;
    if (xQueuePeek(*keyQueue, &keycode, 0) != pdPASS)
        return INVALID_KEYCODE;
    return keycode;
}

uint8_t KeyboardMgr::keysAvailable() {
    return KEY_QUEUE_LENGTH - uxQueueSpacesAvailable(*keyQueue);
}
