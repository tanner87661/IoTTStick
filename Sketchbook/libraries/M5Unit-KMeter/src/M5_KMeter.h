/*!
 * @brief An thermocouple sensor with I2C communication From M5Stack
 * @copyright Copyright (c) 2022 by M5Stack[https://m5stack.com]
 *
 * @Links [Unit KMeter](https://docs.m5stack.com/en/unit/kmeter)
 * @version  V0.0.2
 * @date  2022-07-07
 */
#ifndef _M5_KMETER_H_
#define _M5_KMETER_H_

#include <Arduino.h>
#include <Wire.h>

class M5_KMeter {
   private:
    TwoWire* _wire;
    uint8_t _addr;
    float _temperature;
    float _internal_temp;

   public:
    void begin(TwoWire* wire = &Wire, uint8_t addr = 0x66);

    bool getRawData(uint8_t* result, size_t len = 4);

    float getTemperature(void);
    float getInternalTemp(void);
};

#endif
