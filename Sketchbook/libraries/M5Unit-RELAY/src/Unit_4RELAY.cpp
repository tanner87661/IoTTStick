#include "Unit_4RELAY.h"

/*! @brief Initialize the 4RELAY.*/
bool UNIT_4RELAY::begin(TwoWire *wire, uint8_t sda, uint8_t scl) {
    _wire = wire;
    _sda  = sda;
    _scl  = scl;
    return _wire->begin((int)_sda, (int)_scl, (uint32_t)0U);
}

/*! @brief Write a certain length of data to the specified register address. */
void UNIT_4RELAY::write1Byte(uint8_t address, uint8_t Register_address,
                             uint8_t data) {
    _wire->beginTransmission(address);
    _wire->write(Register_address);
    _wire->write(data);
    _wire->endTransmission();
}

/*! @brief Setting the mode of the device, and turn off all relays.
 *  @param mode Async = 0, Sync = 1. */
void UNIT_4RELAY::Init(bool mode) {
    write1Byte(UNIT_4RELAY_ADDR, UNIT_4RELAY_REG, mode);
    write1Byte(UNIT_4RELAY_ADDR, UNIT_4RELAY_RELAY_REG, 0);
}

/*! @brief Setting the mode of the device.
 *  @param mode Async = 0, Sync = 1. */
void UNIT_4RELAY::switchMode(bool mode) {
    write1Byte(UNIT_4RELAY_ADDR, UNIT_4RELAY_REG, mode);
}

/*! @brief Read a certain length of data to the specified register address. */
uint8_t UNIT_4RELAY::read1Byte(uint8_t address, uint8_t Register_address) {
    _wire->beginTransmission(address);  // Initialize the Tx buffer
    _wire->write(Register_address);  // Put slave register address in Tx buffer
    _wire->endTransmission();
    _wire->requestFrom(address, uint8_t(1));
    uint8_t data = _wire->read();
    return data;
}

/*! @brief Set the mode of all relays at the same time.
 *  @param state OFF = 0, ON = 1. */
void UNIT_4RELAY::relayAll(bool state) {
    write1Byte(UNIT_4RELAY_ADDR, UNIT_4RELAY_RELAY_REG, state * (0x0f));
}

/*! @brief Set the mode of all leds at the same time.
 *  @param state OFF = 0, ON = 1. */
void UNIT_4RELAY::ledAll(bool state) {
    write1Byte(UNIT_4RELAY_ADDR, UNIT_4RELAY_RELAY_REG, state * (0xf0));
}

/*! @brief Control the on/off of the specified relay.
 *  @param number Bit number of relay (0~3).
    @param state OFF = 0, ON = 1 . */
void UNIT_4RELAY::relayWrite(uint8_t number, bool state) {
    uint8_t StateFromDevice =
        read1Byte(UNIT_4RELAY_ADDR, UNIT_4RELAY_RELAY_REG);
    if (state == 0) {
        StateFromDevice &= ~(0x01 << number);
    } else {
        StateFromDevice |= (0x01 << number);
    }
    write1Byte(UNIT_4RELAY_ADDR, UNIT_4RELAY_RELAY_REG, StateFromDevice);
}

/*! @brief Control the on/off of the specified led.
 *  @param number Bit number of led (0~3).
    @param state OFF = 0, ON = 1 . */
void UNIT_4RELAY::ledWrite(uint8_t number, bool state) {
    uint8_t StateFromDevice =
        read1Byte(UNIT_4RELAY_ADDR, UNIT_4RELAY_RELAY_REG);
    if (state == 0) {
        StateFromDevice &= ~(UNIT_4RELAY_REG << number);
    } else {
        StateFromDevice |= (UNIT_4RELAY_REG << number);
    }
    write1Byte(UNIT_4RELAY_ADDR, UNIT_4RELAY_RELAY_REG, StateFromDevice);
}
