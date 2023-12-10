#include "M5_ADS1115.h"
#include "Wire.h"

void ADS1115::i2cBegin() {
    // Wire.begin();
}

/*! @brief Read a certain length of data to the specified register address.
    @return True if the read was successful, otherwise false.. */
bool ADS1115::i2cReadBytes(uint8_t addr, uint8_t reg_addr, uint8_t* buff,
                           uint16_t len) {
    Wire.beginTransmission(addr);
    Wire.write(reg_addr);
    uint8_t i = 0;
    if (Wire.endTransmission(false) == 0 &&
        Wire.requestFrom(addr, (uint8_t)len)) {
        while (Wire.available()) {
            buff[i++] = Wire.read();
        }
        return true;
    }

    return false;
}

/*! @brief Write a certain length of data to the specified register address.
    @return True if the write was successful, otherwise false.. */
bool ADS1115::i2cWriteBytes(uint8_t addr, uint8_t reg_addr, uint8_t* buff,
                            uint16_t len) {
    bool function_result = false;

    Wire.beginTransmission(addr);
    Wire.write(reg_addr);
    for (int i = 0; i < len; i++) {
        Wire.write(*(buff + i));
    }
    function_result = (Wire.endTransmission() == 0);
    return function_result;
}

/*! @brief Read a certain length of 16-bit data to the specified register
   address.
    @return True if the read was successful, otherwise false.. */
bool ADS1115::i2cReadU16(uint8_t addr, uint8_t reg_addr, uint16_t* value) {
    uint8_t read_buf[2] = {0x00, 0x00};
    bool result         = i2cReadBytes(addr, reg_addr, read_buf, 2);
    *value              = (read_buf[0] << 8) | read_buf[1];
    return result;
}

/*! @brief Writes a certain length of 16-bit data to the specified register
   address.
    @return True if the read was successful, otherwise false.. */
bool ADS1115::i2cWriteU16(uint8_t addr, uint8_t reg_addr, uint16_t value) {
    uint8_t write_buf[2];
    write_buf[0] = value >> 8;
    write_buf[1] = value & 0xff;
    return i2cWriteBytes(addr, reg_addr, write_buf, 2);
}

/*! @brief Get the current resolution
    @return Read resolution.. */
float ADS1115::getResolution(bool device, ADS1115Gain_t gain) {
    float tem = (device) ? VOLTMETER_PRESSURE_COEFFICIENT
                         : AMMETER_PRESSURE_COEFFICIENT;
    switch (gain) {
        case PAG_6144:
            return ADS1115_MV_6144 / tem;
        case PAG_4096:
            return ADS1115_MV_4096 / tem;
        case PAG_2048:
            return ADS1115_MV_2048 / tem;
        case PAG_1024:
            return ADS1115_MV_1024 / tem;
        case PAG_512:
            return ADS1115_MV_512 / tem;
        case PAG_256:
            return ADS1115_MV_256 / tem;
        default:
            return ADS1115_MV_256 / tem;
    };
}

/*! @brief Get EEPROM address
    @return EEPROM address.. */
uint8_t ADS1115::getPGAEEEPROMAddr(ADS1115Gain_t gain) {
    switch (gain) {
        case PAG_6144:
            return ADS1115_PAG_6144_CAL_ADDR;
        case PAG_4096:
            return ADS1115_PAG_4096_CAL_ADDR;
        case PAG_2048:
            return ADS1115_PAG_2048_CAL_ADDR;
        case PAG_1024:
            return ADS1115_PAG_1024_CAL_ADDR;
        case PAG_512:
            return ADS1115_PAG_512_CAL_ADDR;
        case PAG_256:
            return ADS1115_PAG_256_CAL_ADDR;
        default:
            return 0x00;
    };
}

uint16_t ADS1115::getCoverTime(ADS1115Rate_t rate) {
    switch (rate) {
        case RATE_8:
            return 1000 / 8;
        case RATE_16:
            return 1000 / 16;
        case RATE_32:
            return 1000 / 32;
        case RATE_64:
            return 1000 / 64;
        case RATE_128:
            return 1000 / 128;
        case RATE_250:
            return 1000 / 250;
        case RATE_475:
            return 1000 / 475;
        case RATE_860:
            return 1000 / 860;
        default:
            return 1000 / 128;
    };
}

ADS1115::ADS1115(bool device, uint8_t ads1115_addr, uint8_t eeprom_addr) {
    _device            = device;
    _ads1115_addr      = ads1115_addr;
    _eeprom_addr       = eeprom_addr;
    _gain              = PAG_2048;
    _mode              = SINGLESHOT;
    _rate              = RATE_128;
    calibration_factor = 1;
    adc_raw            = 0;
    resolution         = getResolution(_device, _gain);
    cover_time         = getCoverTime(_rate);
}

/*! @brief Set gain factor */
void ADS1115::setGain(ADS1115Gain_t gain) {
    uint16_t reg_value = 0;
    bool result = i2cReadU16(_ads1115_addr, ADS1115_RA_CONFIG, &reg_value);

    if (result == false) {
        return;
    }

    reg_value &= ~(0b0111 << 9);
    reg_value |= gain << 9;

    result = i2cWriteU16(_ads1115_addr, ADS1115_RA_CONFIG, reg_value);

    if (result) {
        _gain          = gain;
        resolution     = getResolution(_device, gain);
        int16_t hope   = 1;
        int16_t actual = 1;
        if (readCalibrationFromEEPROM(gain, &hope, &actual)) {
            calibration_factor = fabs((double)hope / actual);
        }
    }
}

/*! @brief Set the read rate */
void ADS1115::setRate(ADS1115Rate_t rate) {
    uint16_t reg_value = 0;
    bool result = i2cReadU16(_ads1115_addr, ADS1115_RA_CONFIG, &reg_value);
    if (result == false) {
        return;
    }

    reg_value &= ~(0b0111 << 5);
    reg_value |= rate << 5;

    result = i2cWriteU16(_ads1115_addr, ADS1115_RA_CONFIG, reg_value);

    if (result) {
        _rate      = rate;
        cover_time = getCoverTime(_rate);
    }

    return;
}

/*! @brief Set read continuous read/single read data */
void ADS1115::setMode(ADS1115Mode_t mode) {
    uint16_t reg_value = 0;
    bool result = i2cReadU16(_ads1115_addr, ADS1115_RA_CONFIG, &reg_value);
    if (result == false) {
        return;
    }

    reg_value &= ~(0b0001 << 8);
    reg_value |= mode << 8;

    result = i2cWriteU16(_ads1115_addr, ADS1115_RA_CONFIG, reg_value);
    if (result) {
        _mode = mode;
    }

    return;
}

/*! @brief Determine if data is being converted
    @return Data being converted returns 1, otherwise 0.. */
bool ADS1115::isInConversion() {
    uint16_t value = 0x00;
    i2cReadU16(_ads1115_addr, ADS1115_RA_CONFIG, &value);

    return (value & (1 << 15)) ? false : true;
}

/*! @brief Turn on single data conversion */
void ADS1115::startSingleConversion() {
    uint16_t reg_value = 0;
    bool result = i2cReadU16(_ads1115_addr, ADS1115_RA_CONFIG, &reg_value);

    if (result == false) {
        return;
    }

    reg_value &= ~(0b0001 << 15);
    reg_value |= 0x01 << 15;

    i2cWriteU16(_ads1115_addr, ADS1115_RA_CONFIG, reg_value);
}

/*! @brief Determine if data is being converted
    @return Data being converted returns 1, otherwise 0.. */
float ADS1115::getValue(bool calibration) {
    if (calibration) {
        return resolution * calibration_factor * getConversion() *
               MEASURING_DIR;
    } else {
        return resolution * getConversion() * MEASURING_DIR;
    }
}

int16_t ADS1115::getAdcRaw() {
    uint16_t value = 0x00;
    i2cReadU16(_ads1115_addr, ADS1115_RA_CONVERSION, &value);
    adc_raw = value;
    return value;
}

int16_t ADS1115::getConversion(uint16_t timeout) {
    if (_mode == SINGLESHOT) {
        startSingleConversion();
        delay(cover_time);
        uint64_t time = millis() + timeout;
        while (time > millis() && isInConversion())
            ;
    }

    return getAdcRaw();
}

bool ADS1115::EEPORMWrite(uint8_t address, uint8_t* buff, uint8_t len) {
    return i2cWriteBytes(_eeprom_addr, address, buff, len);
}

bool ADS1115::EEPORMRead(uint8_t address, uint8_t* buff, uint8_t len) {
    return i2cReadBytes(_eeprom_addr, address, buff, len);
}

bool ADS1115::saveCalibration2EEPROM(ADS1115Gain_t gain, int16_t hope,
                                     int16_t actual) {
    if (hope == 0 || actual == 0) {
        return false;
    }

    uint8_t buff[8];
    memset(buff, 0, 8);
    buff[0] = gain;
    buff[1] = hope >> 8;
    buff[2] = hope & 0xFF;

    buff[3] = actual >> 8;
    buff[4] = actual & 0xFF;

    for (uint8_t i = 0; i < 5; i++) {
        buff[5] ^= buff[i];
    }

    uint8_t addr = getPGAEEEPROMAddr(gain);
    return EEPORMWrite(addr, buff, 8);
}

bool ADS1115::readCalibrationFromEEPROM(ADS1115Gain_t gain, int16_t* hope,
                                        int16_t* actual) {
    uint8_t addr = getPGAEEEPROMAddr(gain);
    uint8_t buff[8];
    memset(buff, 0, 8);

    *hope   = 1;
    *actual = 1;

    bool result = EEPORMRead(addr, buff, 8);

    if (result == false) {
        return false;
    }

    uint8_t xor_result = 0x00;
    for (uint8_t i = 0; i < 5; i++) {
        xor_result ^= buff[i];
    }

    if (xor_result != buff[5]) {
        return false;
    }

    *hope   = (buff[1] << 8) | buff[2];
    *actual = (buff[3] << 8) | buff[4];
    return true;
}