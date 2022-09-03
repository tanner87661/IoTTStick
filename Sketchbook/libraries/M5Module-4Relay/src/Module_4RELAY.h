/*!
 * @brief A 4-Channel Relay module From M5Stack
 * @copyright Copyright (c) 2022 by M5Stack[https://m5stack.com]
 *
 * @Links [Moudle 4-Relay](https://docs.m5stack.com/en/module/4relay)
 * @version  V0.0.1
 * @date  2022-07-22
 */
#ifndef _MODULE_4RELAY_H
#define _MODULE_4RELAY_H

#include "Arduino.h"
#include "Wire.h"

#define MODULE_4RELAY_ADDR 0x26
#define MODULE_4RELAY_REG  0x10

class MODULE_4RELAY {
   private:
    uint8_t _addr;
    TwoWire* _wire;
    uint8_t _scl;
    uint8_t _sda;
    uint32_t _speed;
    bool writeReg(uint8_t data);
    uint8_t readReg();

   public:
    bool begin(TwoWire* wire = &Wire, uint8_t addr = MODULE_4RELAY_ADDR,
               uint8_t sda = 21, uint8_t scl = 22, uint32_t speed = 200000L);
    bool setRelay(uint8_t index, bool state);
    bool setAllRelay(bool state);
    bool reverseRelay(bool state);
    uint8_t getAllRelayState();
    bool getRelayState(uint8_t index);
};

#endif
