/*!
 * @brief  An 8-channel servo driver module that works From M5Stack
 * @copyright Copyright (c) 2022 by M5Stack[https://m5stack.com]
 *
 * @Links [8Servos HAT v1.1](https://docs.m5stack.com/en/hat/hat_8servos_1.1)
 * @version  V0.0.2
 * @date  2022-07-25
 */
#ifndef _HAT_8SERVOS_H_
#define _HAT_8SERVOS_H_

#include <Arduino.h>
#include <Wire.h>

#define SERVOS_DEVICE_ADDR 0x36
#define SERVOS_ANGLE_REG   0x00
#define SERVOS_PULSE_REG   0x10
#define SERVOS_MOS_CTL_REG 0x30

class Hat_8Servos {
   private:
    uint8_t _addr;
    TwoWire *_wire;
    uint8_t _sda;
    uint8_t _scl;
    bool writeBytes(uint8_t addr, uint8_t reg, uint8_t *buffer, uint8_t length);
    bool readBytes(uint8_t addr, uint8_t reg, uint8_t *buffer, uint8_t length);

   public:
    bool begin(TwoWire *wire = &Wire, uint8_t sda = 0, uint8_t scl = 26,
               uint8_t addr = SERVOS_DEVICE_ADDR);
    bool setServoAngle(uint8_t index, uint8_t deg);
    bool setAllServoAngle(uint8_t deg);
    bool setServoPulse(uint8_t index, uint16_t pulse);
    bool setAllServoPulse(uint16_t pulse);
    bool enableServoPower(uint8_t state);
    uint8_t getServoAngle(uint8_t index);
    uint16_t getServoPulse(uint8_t index);
};

#endif
