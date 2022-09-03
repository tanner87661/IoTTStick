#include "M5_KMeter.h"

/*! @brief Initialize the Unit KMeter.*/
void M5_KMeter::begin(TwoWire* wire, uint8_t addr) {
    _wire          = wire;
    _addr          = addr;
    _temperature   = 0.0f;
    _internal_temp = 0.0f;
}

/*! @brief Read raw data.
    @return True if the read was successful, otherwise false.. */
bool M5_KMeter::getRawData(uint8_t* result, size_t len) {
    if (!_wire->requestFrom((int)_addr, len)) {
        return false;
    }

    _wire->readBytes(result, len);

    return true;
}

/*! @brief Read temperature data.
    @return temperature data.. */
float M5_KMeter::getTemperature(void) {
    float result = _temperature;
    if (_wire->requestFrom((int)_addr, 2)) {
        int16_t t = (_wire->read() << 8);
        t |= _wire->read();
        if (0 == (t & 1)) {
            result       = 0.25f * (t >> 2);
            _temperature = result;
        }
    }
    return result;
}

/*! @brief Read internal temperature data.
    @return internal temperature data.. */
float M5_KMeter::getInternalTemp(void) {
    float result = _internal_temp;
    if (_wire->requestFrom((int)_addr, 4)) {
        int16_t t = (_wire->read() << 8);
        t |= _wire->read();
        if (0 == (t & 1)) {
            _temperature = 0.25f * (t >> 2);
        }

        t = (_wire->read() << 8);
        t |= _wire->read();
        if (0 == (t & 7)) {
            result         = 0.0625f * (t >> 4);
            _internal_temp = result;
        }
    }
    return result;
}
