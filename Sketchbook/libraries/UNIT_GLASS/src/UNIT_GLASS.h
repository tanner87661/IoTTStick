#ifndef __UNIT_GLASS_H
#define __UNIT_GLASS_H

#include "Arduino.h"
#include "Wire.h"

#define GLASS_ADDR               0x3D
#define GLASS_CLEAR_REG          0x00
#define GLASS_SHOW_REG           0x10
#define GLASS_DRAW_STRING_REG    0x20
#define GLASS_DRAW_POINT_REG     0x30
#define GLASS_DRAW_LINE_REG      0x40
#define GLASS_DRAW_CIRCLE_REG    0x50
#define GLASS_INVERT_REG         0x60
#define GLASS_DISPLAY_ON_OFF_REG 0x70
#define GLASS_STRING_BUFFER_REG  0x80
#define GLASS_PICTURE_BUFFER_REG 0x90
#define GLASS_COLOR_INVERT_REG   0xA0
#define GLASS_DRAW_PICTURE_REG   0xB0
#define GLASS_DRAW_BUZZ_REG      0xC0
#define GLASS_DRAW_KEY_REG       0xD0
#define FIRMWARE_VERSION_REG     0xFE

class UNIT_GLASS {
   private:
    uint8_t _addr;
    TwoWire* _wire;
    uint8_t _scl;
    uint8_t _sda;
    void writeBytes(uint8_t addr, uint8_t reg, uint8_t* buffer, uint8_t length);
    void readBytes(uint8_t addr, uint8_t reg, uint8_t* buffer, uint8_t length);

   public:
    bool begin(TwoWire* wire = &Wire, uint8_t addr = GLASS_ADDR,
               uint8_t sda = 21, uint8_t scl = 22, uint32_t speed = 100000L);
    void clear(void);
    void show(void);
    void draw_string(uint8_t x, uint8_t y, uint8_t fonts, uint8_t mode);
    void draw_picture(uint8_t x, uint8_t y, uint8_t size_x, uint8_t size_y,
                      uint8_t mode);
    void draw_point(uint8_t x, uint8_t y, uint8_t mode);
    void draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2,
                   uint8_t mode);
    void draw_circle(uint8_t x, uint8_t y, uint8_t r, uint8_t mode);
    void invert(uint8_t inv);
    void color_invert(uint8_t inv);
    void dis_on_off(uint8_t sw);
    void set_string_buffer(const char* buffer);
    void set_picture_buffer(unsigned char* buffer, uint16_t size);
    uint8_t getFirmwareVersion(void);
    void setBuzzer(uint16_t freq, uint8_t duty);
    void enable_buzz(void);
    void disable_buzz(void);
    uint8_t getKeyB(void);
    uint8_t getKeyA(void);
};

#endif
