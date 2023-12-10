/*!
 * @brief A 16-bit ADS1115 ADC converter library From M5Stack
 * @copyright Copyright (c) 2022 by M5Stack[https://m5stack.com]
 *
 * @Links [Unit Ameter](https://docs.m5stack.com/en/unit/ameter)
 * @Links [Unit Vmeter](https://docs.m5stack.com/en/unit/vmeter)
 * @version  V0.0.2
 * @date  2022-08-10
 */
#ifndef _M5_ADS1115_H_
#define _M5_ADS1115_H_
#pragma once

#include "Arduino.h"

#define AMETER 0
#define VMETER 1

#define AMETER_ADDR        0x48
#define VMETER_ADDR        0x49
#define AMETER_EEPROM_ADDR 0x51
#define VMETER_EEPROM_ADDR 0x53

#define ADS1115_RA_CONVERSION 0x00
#define ADS1115_RA_CONFIG     0x01

#define ADS1115_PGA_6144 0x00
#define ADS1115_PGA_4096 0x01
#define ADS1115_PGA_2048 0x02  // default
#define ADS1115_PGA_1024 0x03
#define ADS1115_PGA_512  0x04
#define ADS1115_PGA_256  0x05

#define ADS1115_MV_6144 0.187500F
#define ADS1115_MV_4096 0.125000F
#define ADS1115_MV_2048 0.062500F  // default
#define ADS1115_MV_1024 0.031250F
#define ADS1115_MV_512  0.015625F
#define ADS1115_MV_256  0.007813F

#define ADS1115_RATE_8   0x00
#define ADS1115_RATE_16  0x01
#define ADS1115_RATE_32  0x02
#define ADS1115_RATE_64  0x03
#define ADS1115_RATE_128 0x04  // default
#define ADS1115_RATE_250 0x05
#define ADS1115_RATE_475 0x06
#define ADS1115_RATE_860 0x07

#define MEASURING_DIR -1
#define MEASURING_DIR -1

#define ADS1115_MUX_P0N1 0x00  // ammeter & voltmeter  support

#define ADS1115_COMP_MODE_HYSTERESIS 0x00  // default
#define ADS1115_COMP_MODE_WINDOW     0x01

#define ADS1115_MODE_CONTINUOUS 0x00
#define ADS1115_MODE_SINGLESHOT 0x01  // default

#define AMMETER_PRESSURE_COEFFICIENT   0.05
#define VOLTMETER_PRESSURE_COEFFICIENT 0.015918958F

#define ADS1115_PAG_6144_CAL_ADDR 208
#define ADS1115_PAG_4096_CAL_ADDR 216
#define ADS1115_PAG_2048_CAL_ADDR 224
#define ADS1115_PAG_1024_CAL_ADDR 232
#define ADS1115_PAG_512_CAL_ADDR  240
#define ADS1115_PAG_256_CAL_ADDR  248

#define ADS1115_MODE_CONTINUOUS 0x00
#define ADS1115_MODE_SINGLESHOT 0x01  // default

#define VOLTMETER_FILTER_NUMBER 10

typedef enum {
    PAG_6144 = ADS1115_PGA_6144,
    PAG_4096 = ADS1115_PGA_4096,
    PAG_2048 = ADS1115_PGA_2048,  // default
    PAG_1024 = ADS1115_PGA_1024,
    PAG_512  = ADS1115_PGA_512,
    PAG_256  = ADS1115_PGA_256,
} ADS1115Gain_t;

typedef enum {
    RATE_8   = ADS1115_RATE_8,
    RATE_16  = ADS1115_RATE_16,
    RATE_32  = ADS1115_RATE_32,
    RATE_64  = ADS1115_RATE_64,
    RATE_128 = ADS1115_RATE_128,  // default
    RATE_250 = ADS1115_RATE_250,
    RATE_475 = ADS1115_RATE_475,
    RATE_860 = ADS1115_RATE_860,
} ADS1115Rate_t;

typedef enum {
    SINGLESHOT = ADS1115_MODE_SINGLESHOT,
    CONTINUOUS = ADS1115_MODE_CONTINUOUS,
} ADS1115Mode_t;

class ADS1115 {
   private:
    void i2cBegin();
    bool i2cReadBytes(uint8_t addr, uint8_t reg_addr, uint8_t* buff,
                      uint16_t len);
    bool i2cWriteBytes(uint8_t addr, uint8_t reg_addr, uint8_t* buff,
                       uint16_t len);
    bool i2cReadU16(uint8_t addr, uint8_t reg_addr, uint16_t* value);
    bool i2cWriteU16(uint8_t addr, uint8_t reg_addr, uint16_t value);
    float getResolution(bool devices, ADS1115Gain_t gain);
    uint16_t getCoverTime(ADS1115Rate_t rate);
    uint8_t getPGAEEEPROMAddr(ADS1115Gain_t gain);

    bool _device;
    uint8_t _ads1115_addr;
    uint8_t _eeprom_addr;

   public:
    ADS1115Gain_t _gain;
    ADS1115Rate_t _rate;
    ADS1115Mode_t _mode;

    float resolution;
    uint16_t cover_time;
    int16_t adc_raw;
    float calibration_factor;

   public:
    ADS1115(bool devices = VMETER, uint8_t ads1115_addr = VMETER_ADDR,
            uint8_t eeprom_addr = VMETER_EEPROM_ADDR);

    void setGain(ADS1115Gain_t gain);
    void setRate(ADS1115Rate_t rate);
    void setMode(ADS1115Mode_t mode);

    float getValue(bool calibration = true);

    int16_t getConversion(uint16_t timeout = 125);
    int16_t getAdcRaw();

    bool isInConversion();
    void startSingleConversion();

    bool EEPORMWrite(uint8_t address, uint8_t* buff, uint8_t len);
    bool EEPORMRead(uint8_t address, uint8_t* buff, uint8_t len);

    void setCalibration(int8_t voltage, uint16_t actual);
    bool saveCalibration2EEPROM(ADS1115Gain_t gain, int16_t hope,
                                int16_t actual);
    bool readCalibrationFromEEPROM(ADS1115Gain_t gain, int16_t* hope,
                                   int16_t* actual);
};

#endif