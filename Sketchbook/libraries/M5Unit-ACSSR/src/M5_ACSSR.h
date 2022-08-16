/*!
 * @brief An Single-phase solid state relay controller From M5Stack
 * @copyright Copyright (c) 2022 by M5Stack[https://m5stack.com]
 *
 * @Links [ACSSR](https://docs.m5stack.com/en/unit/acssr)
 * @version  V0.0.2
 * @date  2022-07-05
 */
#ifndef _M5_ACSSR_H_
#define _M5_ACSSR_H_

#include <Arduino.h>
#include <Wire.h>
#include "pins_arduino.h"

// I2C
#define ACSSR_DEFAULT_ADDR    0x50
#define ACSSR_I2C_RELAY_REG   0x00
#define ACSSR_I2C_LED_REG     0x10
#define ACSSR_I2C_ADDR_REG    0x20
#define ACSSR_I2C_VERSION_REG 0xFE

// Modbus
#define ACSSR_DEFAULT_SLAVE_ID     0x04
#define ACSSR_RELAY_COIL_ADDR      0x0000
#define ACSSR_LED_HOLDING_ADDR     0x0000
#define ACSSR_VERSION_HOLDING_ADDR 0x0001
#define ACSSR_ID_HOLDING_ADDR      0x0002

class M5_ACSSR {
   private:
    uint8_t _addr;
    TwoWire *_wire;
    uint8_t _sda;
    uint8_t _scl;
    bool writeBytes(uint8_t addr, uint8_t reg, uint8_t *buffer, uint8_t length);
    bool readBytes(uint8_t addr, uint8_t reg, uint8_t *buffer, uint8_t length);

   public:
    bool begin(TwoWire *wire = &Wire, uint8_t sda = SDA, uint8_t scl = SCL,
               uint8_t addr = ACSSR_DEFAULT_ADDR);
    bool setDeviceAddr(uint8_t addr);
    bool setLEDColor(uint32_t color);
    uint8_t getVersion();
    uint32_t getLEDColor();
    bool on();
    bool off();
    bool status();
};

#endif