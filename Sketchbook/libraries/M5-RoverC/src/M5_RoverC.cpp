#include "M5_RoverC.h"

/*! @brief Sets up the Hardware.*/
bool M5_RoverC::begin(TwoWire *wire, uint8_t sda, uint8_t scl, uint8_t addr) {
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

/*! @brief Write a certain length of data to the specified register address. */
void M5_RoverC::writeBytes(uint8_t addr, uint8_t reg, uint8_t *buffer,
                           uint8_t length) {
    _wire->beginTransmission(addr);
    _wire->write(reg);
    for (int i = 0; i < length; i++) {
        _wire->write(*(buffer + i));
    }
    _wire->endTransmission();
}

/*! @brief Read a certain length of data to the specified register address. */
void M5_RoverC::readBytes(uint8_t addr, uint8_t reg, uint8_t *buffer,
                          uint8_t length) {
    uint8_t index = 0;
    _wire->beginTransmission(addr);
    _wire->write(reg);
    _wire->endTransmission();
    _wire->requestFrom(addr, length);
    for (int i = 0; i < length; i++) {
        buffer[index++] = _wire->read();
    }
}

void M5_RoverC::setPulse(uint8_t pos, int8_t width) {
    writeBytes(_addr, pos, (uint8_t *)&width, 1);
}

void M5_RoverC::setAllPulse(int8_t width0, int8_t width1, int8_t width2,
                            int8_t width3) {
    setPulse(0, width0);
    setPulse(1, width1);
    setPulse(2, width2);
    setPulse(3, width3);
}

/*! @brief Set the speed of the motor. */
void M5_RoverC::setSpeed(int8_t x, int8_t y, int8_t z) {
    int8_t buffer[4];
    if (z != 0) {
        x = int(x * (100 - abs(z)) / 100);
        y = int(y * (100 - abs(z)) / 100);
    }
    buffer[0] = max(-100, min(100, y + x - z));
    buffer[1] = max(-100, min(100, y - x + z));
    buffer[3] = max(-100, min(100, y + x + z));
    buffer[2] = max(-100, min(100, y - x - z));
    writeBytes(_addr, 0x00, (uint8_t *)buffer, 4);
}

/*! @brief Set the angle of the servo. */
void M5_RoverC::setServoAngle(uint8_t pos, uint8_t angle) {
    uint8_t reg = 0x10 + pos;
    writeBytes(_addr, reg, &angle, 1);
}

/*! @brief Set the pulse of the servo. */
void M5_RoverC::setServoPulse(uint8_t pos, uint16_t width) {
    uint8_t reg = 0x20 + pos;
    writeBytes(_addr, reg, (uint8_t *)&width, 1);
}
