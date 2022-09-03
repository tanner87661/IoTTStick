#ifndef ADXL345_h
#define ADXL345_h

#include "Arduino.h"

// I2C Address
#define ADXL345_STD     0x1D    // Standard address if SDO/ALT ADDRESS is HIGH.
#define ADXL345_ALT     0x53    // Alternate address if SDO/ALT ADDRESS is LOW.

// Data Rate
#define ADXL345_RATE_3200HZ   0x0F    // 3200 Hz
#define ADXL345_RATE_1600HZ   0x0E    // 1600 Hz
#define ADXL345_RATE_800HZ    0x0D    // 800 Hz
#define ADXL345_RATE_400HZ    0x0C    // 400 Hz
#define ADXL345_RATE_200HZ    0x0B    // 200 Hz
#define ADXL345_RATE_100HZ    0x0A    // 100 Hz
#define ADXL345_RATE_50HZ     0x09    // 50 Hz
#define ADXL345_RATE_25HZ     0x08    // 25 Hz
#define ADXL345_RATE_12_5HZ   0x07    // 12.5 Hz
#define ADXL345_RATE_6_25HZ   0x06    // 6.25 Hz
#define ADXL345_RATE_3_13HZ   0x05    // 3.13 Hz
#define ADXL345_RATE_1_56HZ   0x04    // 1.56 Hz
#define ADXL345_RATE_0_78HZ   0x03    // 0.78 Hz
#define ADXL345_RATE_0_39HZ   0x02    // 0.39 Hz
#define ADXL345_RATE_0_20HZ   0x01    // 0.20 Hz
#define ADXL345_RATE_0_10HZ   0x00    // 0.10 Hz

// Range
#define ADXL345_RANGE_2G      0x00    // +-2 g
#define ADXL345_RANGE_4G      0x01    // +-4 g
#define ADXL345_RANGE_8G      0x02    // +-8 g
#define ADXL345_RANGE_16G     0x03    // +-16 g


class ADXL345 {
  private:
    class PowerCtlBits {
      public:
        uint8_t link;       // D5
        uint8_t autoSleep;  // D4
        uint8_t measure;    // D3
        uint8_t sleep;      // D2
        uint8_t wakeup;     // D1 - D0

        PowerCtlBits();

        uint8_t toByte();
    };

    class DataFormatBits {
      public:
        uint8_t selfTest;   // D7
        uint8_t spi;        // D6
        uint8_t intInvert;  // D5
        uint8_t fullRes;    // D3
        uint8_t justify;    // D2
        uint8_t range;      // D1 - D0

        DataFormatBits();

        uint8_t toByte();
    };

    class BwRateBits {
      public:
        uint8_t lowPower;   // D4
        uint8_t rate;       // D3 - D0

        BwRateBits();

        uint8_t toByte();
    };

    static const float kRatio2g;
    static const float kRatio4g;
    static const float kRatio8g;
    static const float kRatio16g;

    TwoWire *_wire;
    int8_t _i2cAddress;
    int16_t _xyz[3];
    PowerCtlBits _powerCtlBits;
    DataFormatBits _dataFormatBits;
    BwRateBits _bwRateBits;

    float convertToSI(int16_t rawValue);

    bool write(uint8_t value);
    bool write(uint8_t *values, size_t size);
    bool read(uint8_t *values, int size);
    bool readRegister(uint8_t address, uint8_t *value);
    bool readRegisters(uint8_t address, uint8_t *values, uint8_t size);
    bool writeRegister(uint8_t address, uint8_t value);

  public:
    ADXL345(uint8_t i2cAddress, TwoWire *wire=&Wire);
    bool start();
    bool stop();
    uint8_t readDeviceID();
    bool update();
    float getX();
    float getY();
    float getZ();
    int16_t getRawX();
    int16_t getRawY();
    int16_t getRawZ();

    bool writeRate(uint8_t rate);
    bool writeRateWithLowPower(uint8_t rate);
    bool writeRange(uint8_t range);
};

#endif
