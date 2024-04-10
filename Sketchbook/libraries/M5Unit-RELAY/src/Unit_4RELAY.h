/*!
 * @brief An integrated 4-way relay module From M5Stack
 * @copyright Copyright (c) 2022 by M5Stack[https://m5stack.com]
 *
 * @Links [4-Relay](https://docs.m5stack.com/en/unit/4relay)
 * @version  V0.0.1
 * @date  2022-07-22
 */
#ifndef _UNIT_4RELAY_H_
#define _UNIT_4RELAY_H_

#include <Wire.h>
#include "pins_arduino.h"

#define UNIT_4RELAY_ADDR      0X26
#define UNIT_4RELAY_REG       0X10
#define UNIT_4RELAY_RELAY_REG 0X11

class UNIT_4RELAY {
   private:
    TwoWire *_wire;
    uint8_t _sda;
    uint8_t _scl;
    void write1Byte(uint8_t address, uint8_t register_address, uint8_t data);
    uint8_t read1Byte(uint8_t address, uint8_t register_address);

   public:
    bool begin(TwoWire *wire = &Wire, uint8_t sda = SDA, uint8_t scl = SCL);
    void Init(bool mode);

    void relayWrite(uint8_t number, bool state);
    void relayAll(bool state);

    void ledWrite(uint8_t number, bool state);
    void ledAll(bool state);

    void switchMode(bool mode);
};

#endif
