#include "UNIT_GLASS.h"

void UNIT_GLASS::writeBytes(uint8_t addr, uint8_t reg, uint8_t *buffer,
                            uint8_t length) {
    _wire->beginTransmission(addr);
    _wire->write(reg);
    for (int i = 0; i < length; i++) {
        _wire->write(*(buffer + i));
    }
    _wire->endTransmission();
}

void UNIT_GLASS::readBytes(uint8_t addr, uint8_t reg, uint8_t *buffer,
                           uint8_t length) {
    uint8_t index = 0;
    _wire->beginTransmission(addr);
    _wire->write(reg);
    _wire->endTransmission(false);
    _wire->requestFrom(addr, length);
    for (int i = 0; i < length; i++) {
        buffer[index++] = _wire->read();
    }
}

bool UNIT_GLASS::begin(TwoWire *wire, uint8_t addr, uint8_t sda, uint8_t scl,
                       uint32_t speed) {
    _wire = wire;
    _addr = addr;
    _sda  = sda;
    _scl  = scl;
    _wire->begin(_sda, _scl);
    delay(10);
    _wire->beginTransmission(_addr);
    uint8_t error = _wire->endTransmission();
    if (error == 0) {
        return true;
    } else {
        return false;
    }
}

void UNIT_GLASS::clear(void) {
    uint8_t data[4];

    data[0] = 1;
    writeBytes(_addr, GLASS_CLEAR_REG, data, 1);
}

void UNIT_GLASS::show(void) {
    uint8_t data[4];

    data[0] = 1;
    writeBytes(_addr, GLASS_SHOW_REG, data, 1);
}

void UNIT_GLASS::draw_string(uint8_t x, uint8_t y, uint8_t fonts,
                             uint8_t mode) {
    uint8_t data[4];

    data[0] = x;
    data[1] = y;
    data[2] = fonts;
    data[3] = mode;
    writeBytes(_addr, GLASS_DRAW_STRING_REG, data, 4);
    delay(1);
}

void UNIT_GLASS::draw_picture(uint8_t x, uint8_t y, uint8_t size_x,
                              uint8_t size_y, uint8_t mode) {
    uint8_t data[5];

    data[0] = x;
    data[1] = y;
    data[2] = size_x;
    data[3] = size_y;
    data[4] = mode;
    writeBytes(_addr, GLASS_DRAW_PICTURE_REG, data, 5);
}

void UNIT_GLASS::draw_point(uint8_t x, uint8_t y, uint8_t mode) {
    uint8_t data[3];

    data[0] = x;
    data[1] = y;
    data[2] = mode;
    writeBytes(_addr, GLASS_DRAW_POINT_REG, data, 3);
}

void UNIT_GLASS::draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2,
                           uint8_t mode) {
    uint8_t data[5];

    data[0] = x1;
    data[1] = y1;
    data[2] = x2;
    data[3] = y2;
    data[4] = mode;
    writeBytes(_addr, GLASS_DRAW_LINE_REG, data, 5);
}

void UNIT_GLASS::draw_circle(uint8_t x, uint8_t y, uint8_t r, uint8_t mode) {
    uint8_t data[4];

    data[0] = x;
    data[1] = y;
    data[2] = r;
    data[3] = mode;
    writeBytes(_addr, GLASS_DRAW_CIRCLE_REG, data, 4);
}

void UNIT_GLASS::invert(uint8_t inv) {
    uint8_t data[4];

    data[0] = inv;
    writeBytes(_addr, GLASS_INVERT_REG, data, 1);
    delay(1);
}

void UNIT_GLASS::color_invert(uint8_t inv) {
    uint8_t data[4];

    data[0] = inv;
    writeBytes(_addr, GLASS_COLOR_INVERT_REG, data, 1);
    delay(1);
}

void UNIT_GLASS::dis_on_off(uint8_t sw) {
    uint8_t data[4];

    data[0] = sw;
    writeBytes(_addr, GLASS_DISPLAY_ON_OFF_REG, data, 1);
    delay(1000);
}

void UNIT_GLASS::set_string_buffer(const char *buffer) {
    uint8_t data[4];
    uint16_t index = 0;

    while ((*buffer) != 0) {
        data[0] = (uint8_t)index;
        data[1] = (uint8_t)(index >> 8);
        data[2] = *buffer;

        writeBytes(_addr, GLASS_STRING_BUFFER_REG, data, 3);
        buffer++;
        index++;
    }
}

void UNIT_GLASS::set_picture_buffer(unsigned char *buffer, uint16_t size) {
    uint8_t data[4];
    uint16_t index = 0;

    for (int i = 0; i < size; i++) {
        data[0] = (uint8_t)index;
        data[1] = (uint8_t)(index >> 8);
        data[2] = *buffer;

        writeBytes(_addr, GLASS_PICTURE_BUFFER_REG, data, 3);
        buffer++;
        index++;
    }
}

uint8_t UNIT_GLASS::getFirmwareVersion(void) {
    _wire->beginTransmission(_addr);
    _wire->write(FIRMWARE_VERSION_REG);
    _wire->endTransmission();

    uint8_t RegValue;

    _wire->requestFrom(_addr, (uint8_t)1);
    RegValue = Wire.read();
    return RegValue;
}

void UNIT_GLASS::setBuzzer(uint16_t freq, uint8_t duty) {
    uint8_t data[4];
    data[0] = freq & 0xff;
    data[1] = (freq >> 8) & 0xff;
    data[2] = duty;
    writeBytes(_addr, GLASS_DRAW_BUZZ_REG, data, 3);
}

void UNIT_GLASS::enable_buzz(void) {
    uint8_t data[4];
    uint8_t reg = GLASS_DRAW_BUZZ_REG + 3;

    data[0] = 1;

    writeBytes(_addr, reg, data, 1);
}

void UNIT_GLASS::disable_buzz(void) {
    uint8_t data[4];
    uint8_t reg = GLASS_DRAW_BUZZ_REG + 3;

    data[0] = 0;

    writeBytes(_addr, reg, data, 1);
}

uint8_t UNIT_GLASS::getKeyA(void) {
    uint8_t data[4];

    readBytes(_addr, GLASS_DRAW_KEY_REG, data, 1);
    return data[0];
}

uint8_t UNIT_GLASS::getKeyB(void) {
    uint8_t data[4];
    uint8_t reg = GLASS_DRAW_KEY_REG + 1;

    readBytes(_addr, reg, data, 1);
    return data[0];
}
