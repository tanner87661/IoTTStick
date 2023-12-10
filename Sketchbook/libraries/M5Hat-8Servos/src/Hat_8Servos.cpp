#include "Hat_8Servos.h"

/*! @brief Initialize the hardware.
    @return True if the init was successful, otherwise false.. */
bool Hat_8Servos::begin(TwoWire *wire, uint8_t sda, uint8_t scl, uint8_t addr) {
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

/*! @brief Write a certain length of data to the specified register address.
    @return True if the write was successful, otherwise false.. */
bool Hat_8Servos::writeBytes(uint8_t addr, uint8_t reg, uint8_t *buffer,
                             uint8_t length) {
    _wire->beginTransmission(addr);
    _wire->write(reg);
    _wire->write(buffer, length);
    if (_wire->endTransmission() == 0) return true;
    return false;
}

/*! @brief Read a certain length of data to the specified register address.
    @return True if the read was successful, otherwise false.. */
bool Hat_8Servos::readBytes(uint8_t addr, uint8_t reg, uint8_t *buffer,
                            uint8_t length) {
    uint8_t index = 0;
    _wire->beginTransmission(addr);
    _wire->write(reg);
    _wire->endTransmission();
    if (_wire->requestFrom(addr, length)) {
        for (uint8_t i = 0; i < length; i++) {
            buffer[index++] = _wire->read();
        }
        return true;
    }
    return false;
}

/*! @brief Set the angle of a certain servo.
    @return True if the set was successful, otherwise false.. */
bool Hat_8Servos::setServoAngle(uint8_t index, uint8_t deg) {
    uint8_t data = deg;
    uint8_t reg  = index;
    return writeBytes(_addr, reg, &data, 1);
}

/*! @brief Set the angle of all servos.
    @return True if the set was successful, otherwise false.. */
bool Hat_8Servos::setAllServoAngle(uint8_t deg) {
    uint8_t data[8] = {0};
    memset(data, deg, 8);
    return writeBytes(_addr, SERVOS_ANGLE_REG, data, 8);
}

/*! @brief Set the pulse of a certain servo.
    @return True if the set was successful, otherwise false.. */
bool Hat_8Servos::setServoPulse(uint8_t index, uint16_t pulse) {
    uint8_t data[2] = {0};
    data[1]         = pulse & 0xff;
    data[0]         = (pulse >> 8) & 0xff;
    uint8_t reg     = SERVOS_PULSE_REG + (2 * index);
    return writeBytes(_addr, reg, data, 2);
    ;
}

/*! @brief Set the pulse of all servos.
    @return True if the set was successful, otherwise false.. */
bool Hat_8Servos::setAllServoPulse(uint16_t pulse) {
    for (uint8_t i = 0; i < 8; i++) {
        if (!setServoPulse(i, pulse)) {
            return false;
        }
    }
    return true;
}

/*! @brief Turn on power to all servos.
    @return True if the set was successful, otherwise false.. */
bool Hat_8Servos::enableServoPower(uint8_t state) {
    uint8_t data = state;
    return writeBytes(_addr, SERVOS_MOS_CTL_REG, &data, 1);
}

/*! @brief Get the angle of a particular servo.
    @return Angle of a certain servo.. */
uint8_t Hat_8Servos::getServoAngle(uint8_t index) {
    uint8_t data = 0;
    uint8_t reg  = SERVOS_ANGLE_REG + index;
    if (readBytes(_addr, reg, &data, 1)) {
        return data;
    } else {
        return 0;
    };
}

/*! @brief Get the angle of a particular pulse.
    @return Pulse of a certain servo.. */
uint16_t Hat_8Servos::getServoPulse(uint8_t index) {
    uint8_t data[2] = {0};
    uint8_t reg     = SERVOS_PULSE_REG + index;
    if (readBytes(_addr, reg, data, 2)) {
        return (data[1] << 8) | data[0];
    } else {
        return 0;
    };
}
