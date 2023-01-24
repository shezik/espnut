#pragma once

#include <Arduino.h>
#include <Wire.h>

class Kbd_8x5_CH450 {
    private:
        uint8_t sda;
        uint8_t scl;
        uint8_t delayms;
        void startComm();
        void stopComm();
        bool writeByte(uint8_t data);
        uint8_t readByte();
        const uint8_t keycodeMap[8][5] = { {131, 130, 194, 114, 18}, \
                                           {128, 135, 199, 119, 23}, \
                                           {129, 132, 196, 116, 20}, \
                                           {136, 132, 197, 117, 21}, \
                                           { 19,  51, 115, 195, 50}, \
                                           { 16,  48, 112, 192, 55}, \
                                           { 17,  49, 113, 193, 52}, \
                                           { 24,  56, 120, 200, 53} };  // customize it yourself depending on your wirings
    public:
        /*
            @param 'sda_' I2C SDA
            @param 'scl_' I2C SCL
            @param 'delayms_' (optional) Controls delays in I2C communication.
            Default 1, aggressively 0 (still burns a little time in function calls)
        */
        Kbd_8x5_CH450(uint8_t sda_, uint8_t scl_, uint8_t delayms_ = 1);
        bool init(bool ifSendConfig);
        bool sendConfig(bool enterSleep = false);
        uint8_t getKeyData();
        bool toState(uint8_t rawdata);
        uint8_t toKeycode(uint8_t rawdata);
};
