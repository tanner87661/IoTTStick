#ifndef _HAT_JOYC_H_
#define _HAT_JOYC_H_

#include <Arduino.h>
#include <Wire.h>

#define JOYC_ADDRESS 0x38

class JoyC {
   private:
    uint8_t _addr;
    TwoWire *_wire;
    uint8_t _sda;
    uint8_t _scl;
    void writeBytes(uint8_t addr, uint8_t reg, uint8_t *buffer, uint8_t length);
    void readBytes(uint8_t addr, uint8_t reg, uint8_t *buffer, uint8_t length);

   public:
    bool begin(TwoWire *wire = &Wire, uint8_t sda = 0, uint8_t scl = 26,
               uint8_t addr = JOYC_ADDRESS);
    uint8_t getX(uint8_t pos);
    uint8_t getY(uint8_t pos);
    uint16_t getAngle(uint8_t pos);
    uint16_t getDistance(uint8_t pos);
    uint8_t getPress(uint8_t pos);
    void setLEDColor(uint32_t color);
    void update();

   public:
    uint8_t x0, x1, y0, y1;
    uint16_t distance0, distance1, angle0, angle1;
    bool btn0, btn1;
};

#endif
