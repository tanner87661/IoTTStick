#include "Module_4RELAY.h"

/*! @brief Initialize the 4RELAY.*/
bool MODULE_4RELAY::begin(TwoWire* wire, uint8_t addr, uint8_t sda, uint8_t scl,
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

/*! @brief Write data to the specified register address. */
bool MODULE_4RELAY::writeReg(uint8_t data) {
    _wire->beginTransmission(_addr);
    _wire->write(MODULE_4RELAY_REG);
    _wire->write(data);
    if (_wire->endTransmission() == 0) return true;
    return false;
}

/*! @brief Read data to the specified register address. */
uint8_t MODULE_4RELAY::readReg() {
    _wire->beginTransmission(_addr);
    _wire->write(MODULE_4RELAY_REG);
    _wire->endTransmission(false);
    _wire->requestFrom(_addr, (uint8_t)1);
    return _wire->read();
}

/*! @brief Read all relay status. */
uint8_t MODULE_4RELAY::getAllRelayState() {
    return readReg();
}

/*! @brief Read the status of the specified relay. */
bool MODULE_4RELAY::getRelayState(uint8_t index) {
    uint8_t stateByte = getAllRelayState();
    return (stateByte >> index) & 0x01;
}

/*! @brief Set the state of the specified relay. */
bool MODULE_4RELAY::setRelay(uint8_t index, bool state) {
    uint8_t stateByte = getAllRelayState();
    if (state) {
        stateByte |= (0x01 << index);
    } else {
        stateByte &= ~(0x01 << index);
    }
    return writeReg(stateByte);
}

/*! @brief Set all relay states. */
bool MODULE_4RELAY::setAllRelay(bool state) {
    return writeReg(state ? 0xff : 0x00);
}
