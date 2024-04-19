/******************************************************************************
SparkFunTMAG5273.cpp
SparkFunTMAG5273 Library Source File
Madison Chodikov @ SparkFun Electronics
Original Creation Date: May 1st, 2023
https://github.com/sparkfun/SparkFun_TMAG5273_Arduino_Library

This file implements all functions of the TMAG5273. Functions here range
from reading the X, Y,and Z Magnetic Fields, to reading and writing various
settings in the sensor.

Development environment specifics:
    IDE: Arduino 2.1.0
    Hardware Platform: Arduino Uno
    TMAG5273 Breakout Version: 1.0.1
Distributed as-is; no warranty is given.
******************************************************************************/

#include "SparkFun_TMAG5273_Arduino_Library.h"
#include "SparkFun_TMAG5273_Arduino_Library_Defs.h"
#include <Arduino.h>
#include <Wire.h>

TMAG5273::TMAG5273()
{
    /* Nothing to do */
}

/// @brief Begin communication with the TMAG over I2C, initialize it, and
/// and set the wire for the I2C communication
/// @param sensorAddress I2C address of the sensor
/// @param wirePort I2C port to use for communication, defaults to Wire
/// @return Error code (1 is success, 0 is failure, negative is warning)
int8_t TMAG5273::begin(uint8_t sensorAddress, TwoWire &wirePort)
{
    _i2cPort = &wirePort;           // Chooses the wire port of the device
    _deviceAddress = sensorAddress; // Sets the address of the device

    // Makes sure the TMAG will acknowledge over I2C along with matching Device ID's
    if (isConnected() != 0)
    {
        return 0;
    }

    // Following the Detailed Design Prodedure on page 42 of the datasheet
    setMagneticChannel(TMAG5273_X_Y_Z_ENABLE);
    setTemperatureEn(true);
    setOperatingMode(TMAG5273_CONTINUOUS_MEASURE_MODE);

    // Set the axis ranges for the device to be the largest
    setXYAxisRange(TMAG5273_RANGE_80MT);
    setZAxisRange(TMAG5273_RANGE_80MT);

    // Check if there is any issue with the device status register
    if (getError() != 0)
    {
        return 0;
    }

    // Check the low active current mode (0)
    if (getLowPower() != TMAG5273_LOW_ACTIVE_CURRENT_MODE)
    {
        return 0;
    }

    // Check the operating mode to make sure it is set to continuous measure (0X2)
    if (getOperatingMode() != TMAG5273_CONTINUOUS_MEASURE_MODE)
    {
        return 0;
    }

    // Check that all magnetic channels have been enables(0X7)
    if (getMagneticChannel() != TMAG5273_X_Y_Z_ENABLE)
    {
        return 0;
    }

    // Check that the temperature data acquisition has been enabled
    if (getTemperatureEN() != TMAG5273_TEMPERATURE_ENABLE)
    {
        return 0;
    }

    // Check that X and Y angle calculation is disabled
    if (getAngleEn() != TMAG5273_NO_ANGLE_CALCULATION)
    {
        return 0;
    }

    // returns true if all the checks pass
    return 1;
}

/// @brief This function will make sure the TMAG5273 acknowledges
///  over I2C, along with checking the Device ID to ensure proper
///  connection.
/// @return Error code (0 is success, negative is failure)
int8_t TMAG5273::isConnected()
{
    // make sure the TMAG will acknowledge over I2C
    _i2cPort->beginTransmission(_deviceAddress);
    if (_i2cPort->endTransmission() != 0)
    {
        return -1;
    }

    if (getManufacturerID() != TMAG5273_DEVICE_ID_VALUE)
    {
        return -1;
    }

    return 0;
}

/// @brief Reads the register bytes from the sensor when called upon. This reads
/// 2 bytes of information from the 16-bit register
/// @param regAddress Register's address to read from
/// @param dataBuffer Pointer to the data location being read to
/// @param numBytes Number of bytes to read
/// @return Error code (0 is success, negative is failure)
int8_t TMAG5273::readRegisters(uint8_t regAddress, uint8_t *dataBuffer, uint8_t numBytes)
{
    // uint8_t _deviceAddress = 0X22;
    //  Jump to desired register address
    Wire.beginTransmission(_deviceAddress);
    Wire.write(regAddress);
    if (Wire.endTransmission())
    {
        return -1;
    }

    // Read bytes from these registers
    Wire.requestFrom(_deviceAddress, numBytes);

    // Store all requested bytes
    for (uint8_t i = 0; i < numBytes && Wire.available(); i++)
    {
        dataBuffer[i] = Wire.read();
    }

    return 0;
}

/// @brief Reads a register region from a device.
/// @param regAddress I2C address of device
/// @param dataBuffer Pointer to byte to store read data
/// @param numBytes Number of bytes to write
/// @return Error code (0 is success, negative is failure, positive is warning)
int8_t TMAG5273::writeRegisters(uint8_t regAddress, uint8_t *dataBuffer, uint8_t numBytes)
{
    // uint8_t _deviceAddress = 0X22;
    //  Begin transmission
    Wire.beginTransmission(_deviceAddress);

    // Write the address
    Wire.write(regAddress);

    // Write all the data
    for (uint8_t i = 0; i < numBytes; i++)
    {
        Wire.write(dataBuffer[i]);
    }

    // End transmission
    if (Wire.endTransmission())
    {
        return -1;
    }

    return 0;
}

/// @brief Reads the register byte from the sensor when called upon.
/// @param regAddress Register's address to read from
/// @return Value of the register chosen to be read from
uint8_t TMAG5273::readRegister(uint8_t regAddress)
{
    uint8_t regVal = 0;
    readRegisters(regAddress, &regVal, 2);
    return regVal;
}

/// @brief Reads a register region from a device.
/// @param regAddress I2C address of device
/// @param data Value to fill register with
/// @return Error code (0 is success, negative is failure, positive is warning)
uint8_t TMAG5273::writeRegister(uint8_t regAddress, uint8_t data)
{
    // Write 1 byte to writeRegisters()
    writeRegisters(regAddress, &data, 1);
    return data;
}

/// @brief Write to the correct registers the correct values to
///  enter wake up and sleep mode. See page 42 of the datasheet for more
///  information on the values chosen.
///     TMAG5273_REG_INT_CONFIG_1
///     TMAG5273_REG_DEVICE_CONFIG_2
/// @return Error code (0 is success, negative is failure, positive is warning)
int8_t TMAG5273::setupWakeUpAndSleep()
{
    writeRegister(TMAG5273_REG_INT_CONFIG_1, 0X64);
    writeRegister(TMAG5273_REG_DEVICE_CONFIG_2, 0X23);

    return getError();
}

/// @brief Read the 16-bit magnetic reading for the X, Y, Z and temperature
///  data during the wakeup and sleep mode
/// @param xVal 16-bit value for the X magnetic reading
/// @param yVal 16-bit value for the Y magnetic reading
/// @param zVal 16-bit value for the Z magnetic reading
/// @param temperature 8-bit value for the temperature reading
/// @return Error code (0 is success, negative is failure, positive is warning)
int8_t TMAG5273::readWakeUpAndSleepData(float *xVal, float *yVal, float *zVal, float *temperature)
{
    uint8_t wakeupRegisterRead[8];
    // Read 4 bits of data
    readRegisters(TMAG5273_REG_T_MSB_RESULT, wakeupRegisterRead, 4);

    // Need to get the values to themselves (bitwise operation)
    *zVal = (wakeupRegisterRead[6] << 8) & wakeupRegisterRead[7];
    *yVal = (wakeupRegisterRead[4] << 8) & wakeupRegisterRead[5];
    *xVal = (wakeupRegisterRead[2] << 8) & wakeupRegisterRead[3];
    *temperature = (wakeupRegisterRead[0] << 8) & wakeupRegisterRead[1];

    // Reads to see if the range is set to 40mT or 80mT
    uint8_t rangeValXY = getXYAxisRange();
    uint8_t range = 0;
    if (rangeValXY == 0)
    {
        range = 40;
    }
    else if (rangeValXY == 1)
    {
        range = 80;
    }

    float div = 32768;

    // Return the values in the form that the equation will give
    *temperature = TMAG5273_TSENSE_T0 + (256 * (*temperature - (TMAG5273_TADC_T0 / 256)) / TMAG5273_TADC_RES);
    *xVal = -(range * (*xVal)) / div;
    *yVal = -(range * (*yVal)) / div;
    *zVal = -(range * (*zVal)) / div;

    return getError();
}

/************************************************************************************************/
/**************************        SET CONFIGURATION SETTINGS         ***************************/
/************************************************************************************************/

/// @brief Sets the I2C CRC byte to be sent
/// @param crcMode 0b0-0b1
/// 0X0 = CRC disabled
/// 0X1 = CRC enabled
/// TMAG5273_REG_DEVICE_CONFIG_1 - bit 7
/// @return Error code (0 is success, negative is failure, positive is warning)
int8_t TMAG5273::setCRCMode(uint8_t crcMode)
{
    uint8_t mode = 0;
    mode = readRegister(TMAG5273_REG_DEVICE_CONFIG_1);

    // If-Else statement for writing values to the register, bit by bit
    if (crcMode == 0)
    {
        // adds the crcMode (0 or 1) to bit 7 in the register
        bitWrite(mode, 7, 0);
        // Writes new register value to reg
        writeRegister(TMAG5273_REG_DEVICE_CONFIG_1, mode);
    }

    else if (crcMode == 1)
    {
        // adds the crc_mode (0 or 1) to bit 7 in the register
        bitWrite(mode, 7, 1);
        // Writes new register value to reg
        writeRegister(TMAG5273_REG_DEVICE_CONFIG_1, mode);
    }
    return getError();
}

/// @brief Sets the temperature coefficient of the magnet
/// @param magTempMode value to set the temp coefficient of the device
///     0X0 = 0% (No temperature compensation)
///     X1 = 0.12%/deg C (NdBFe)
///     0X2 = Reserved
///     0X3 = 0.2%/deg C (Ceramic)
///     TMAG5273_REG_DEVICE_CONFIG_1 - bit 6-5
/// @return Error code (0 is success, negative is failure, positive is warning)
int8_t TMAG5273::setMagTemp(uint8_t magTempMode)
{
    uint16_t mode = 0;
    mode = readRegister(TMAG5273_REG_DEVICE_CONFIG_1);

    // If-Else statement for writing values to the register, bit by bit
    if (magTempMode == 0) // 0b00
    {
        bitWrite(mode, 6, 0);
        bitWrite(mode, 5, 0);
        writeRegister(TMAG5273_REG_DEVICE_CONFIG_1, mode);
    }
    else if (magTempMode == 0x1) // 0b01
    {
        bitWrite(mode, 6, 0);
        bitWrite(mode, 5, 1);
        writeRegister(TMAG5273_REG_DEVICE_CONFIG_1, mode);
    }
    else if (magTempMode == 0x2) // 0b10
    {
        bitWrite(mode, 6, 1);
        bitWrite(mode, 5, 0);
        writeRegister(TMAG5273_REG_DEVICE_CONFIG_1, mode);
    }
    else if (magTempMode == 0x3) // 0b11
    {
        bitWrite(mode, 6, 1);
        bitWrite(mode, 5, 1);
        writeRegister(TMAG5273_REG_DEVICE_CONFIG_1, mode);
    }
    return getError();
}

/// @brief Sets the additional sampling of the sensor data to reduce the
/// noise effect (or to increase resolution)
/// @param avgMode value to set the conversion average
///     0X0 = 1x average, 10.0-kSPS (3-axes) or 20-kSPS (1 axis)
///     0X1 = 2x average, 5.7-kSPS (3-axes) or 13.3-kSPS (1 axis)
///     0X2 = 4x average, 3.1-kSPS (3-axes) or 8.0-kSPS (1 axis)
///     0X3 = 8x average, 1.6-kSPS (3-axes) or 4.4-kSPS (1 axis)
///     0X4 = 16x average, 0.8-kSPS (3-axes) or 2.4-kSPS (1 axis)
///     0X5 =  32x average, 0.4-kSPS (3-axes) or 1.2-kSPS (1 axis)
///     TMAG5273_REG_DEVICE_CONFIG_1 - bit 4-2
/// @return Error code (0 is success, negative is failure, positive is warning)
int8_t TMAG5273::setConvAvg(uint8_t avgMode)
{
    uint8_t mode = 0;
    mode = readRegister(TMAG5273_REG_DEVICE_CONFIG_1);

    // If-Else statement for writing values to the register, bit by bit
    if (avgMode == 0) // 0b000
    {
        bitWrite(mode, 2, 0);
        bitWrite(mode, 3, 0);
        bitWrite(mode, 4, 0);
        writeRegister(TMAG5273_REG_DEVICE_CONFIG_1, mode);
    }
    else if (avgMode == 0x1) // 0b001
    {
        bitWrite(mode, 2, 1);
        bitWrite(mode, 3, 0);
        bitWrite(mode, 4, 0);
        writeRegister(TMAG5273_REG_DEVICE_CONFIG_1, mode);
    }
    else if (avgMode == 0x2) // 0b010
    {
        bitWrite(mode, 2, 0);
        bitWrite(mode, 3, 1);
        bitWrite(mode, 4, 0);
        writeRegister(TMAG5273_REG_DEVICE_CONFIG_1, mode);
    }
    else if (avgMode == 0x3) // 0b011
    {
        bitWrite(mode, 2, 1);
        bitWrite(mode, 3, 1);
        bitWrite(mode, 4, 0);
        writeRegister(TMAG5273_REG_DEVICE_CONFIG_1, mode);
    }
    else if (avgMode == 0x4) // 0b100
    {
        bitWrite(mode, 2, 0);
        bitWrite(mode, 3, 0);
        bitWrite(mode, 4, 1);
        writeRegister(TMAG5273_REG_DEVICE_CONFIG_1, mode);
    }
    else if (avgMode == 0x5) // 0b101
    {
        bitWrite(mode, 2, 1);
        bitWrite(mode, 3, 0);
        bitWrite(mode, 4, 1);
        writeRegister(TMAG5273_REG_DEVICE_CONFIG_1, mode);
    }

    return getError();
}

/// @brief Defines the I2C read mode of the device
/// @param readMode Value to set the read mode
///     0X0 = Standard I2C 3-byte read command
///     0X1 = 1-byte I2C read command for 16bit sensor data and
///           conversion status
///     0X2 = 1-byte I2C read command for 8bit sernsor MSB data and
///           conversion status
///     TMAG5273_REG_DEVICE_CONFIG_1 Bits 0-1
/// @return Error code (0 is success, negative is failure, positive is warning)
int8_t TMAG5273::setReadMode(uint8_t readMode)
{
    uint8_t mode = 0;
    mode = readRegister(TMAG5273_REG_DEVICE_CONFIG_1);

    // Write values to the register, bit by bit
    if (readMode == 0)
    {
        bitWrite(mode, 0, 0);
        bitWrite(mode, 1, 0);
    }
    else if (readMode == 1)
    {
        bitWrite(mode, 0, 1);
        bitWrite(mode, 1, 0);
    }
    else if (readMode == 2) //
    {
        bitWrite(mode, 0, 0);
        bitWrite(mode, 1, 1);
    }

    return getError();
}

/// @brief Sets the threshold for the interrupt function
/// @param threshold Value to set the threshold
///     0X0 = Takes the 2's complement value of each x_THR_CONFIG
///       register to create a magnetic threshold of the corresponding axis
///     0X1 = Takes the 7LSB bits of the x_THR_CONFIG register to create
///       two opposite magnetic thresholds (one north, and another south)
///       of equal magnitude
///     The rest of the bits are reserved (0x2 - 0x7) - DO NOT USE
///     TMAG5273_REG_DEVICE_CONFIG_2 - bit 7-5
/// @return Error code (0 is success, negative is failure, positive is warning)
int8_t TMAG5273::setIntThreshold(uint8_t threshold)
{
    uint16_t mode = 0;
    mode = readRegister(TMAG5273_REG_DEVICE_CONFIG_2);

    // If-Else statement for writing values to the register, bit by bit
    if (threshold == 0) // 0b00
    {
        // Writes a 0 to bit 7
        bitWrite(mode, 7, 0);
        // Writes a 0 to bit 6
        bitWrite(mode, 6, 0);
        // Writes a 0 to bit 5
        bitWrite(mode, 5, 0);
        writeRegister(TMAG5273_REG_DEVICE_CONFIG_2, mode);
    }
    else if (threshold == 0x1) // 0b001
    {
        // Writes a 0 to bit 7
        bitWrite(mode, 7, 0);
        // Writes a 0 to bit 6
        bitWrite(mode, 6, 0);
        // Writes a 1 to bit 5
        bitWrite(mode, 5, 1);
        writeRegister(TMAG5273_REG_DEVICE_CONFIG_2, mode);
    }

    return getError();
}

/// @brief Sets the device to low power or low noise mode
/// @param lpLnMode Value to set the mode
///    0X0 = Low active current mode
///    0X1 = Low noise mode
///    TMAG5273_REG_DEVICE_CONFIG_2 - bit 4
/// @return Error code (0 is success, negative is failure, positive is warning)
int8_t TMAG5273::setLowPower(uint8_t lpLnMode)
{
    uint16_t mode = 0;
    mode = readRegister(TMAG5273_REG_DEVICE_CONFIG_2);

    // If-Else statement for writing values to the register, bit by bit
    if (lpLnMode == 0) // 0b0
    {
        // 0b0 - Low active current mode
        bitWrite(mode, 4, 0);
        // Writes new value to register
        writeRegister(TMAG5273_REG_DEVICE_CONFIG_2, mode);
    }
    else if (lpLnMode == 1) // 0b1 - Low noise mode
    {
        bitWrite(mode, 4, 1);
        writeRegister(TMAG5273_REG_DEVICE_CONFIG_2, mode);
    }

    return getError();
}

/// @brief Sets the I2C glitch filter on and off
/// @param glitchMode value to set the mode
///     0x0 = Glitch filter ON
///     0X0 = Glitch filter OFF
///     TMAG5273_REG_DEVICE_CONFIG_2 - bit 3
/// @return Error code (0 is success, negative is failure, positive is warning)
int8_t TMAG5273::setGlitchFilter(uint8_t glitchMode)
{
    uint16_t mode = 0;
    mode = readRegister(TMAG5273_REG_DEVICE_CONFIG_2);

    // If-Else statement for writing values to the register, bit by bit
    if (glitchMode == 0) // 0b0 - Glitch filter on
    {
        // Writes 0 to glitch filter bit
        bitWrite(mode, 3, 0);
        // Writes new register value to reg
        writeRegister(TMAG5273_REG_DEVICE_CONFIG_2, mode);
    }
    else if (glitchMode == 1) // 0b1 - Glitch filter off
    {
        bitWrite(mode, 3, 1);
        writeRegister(TMAG5273_REG_DEVICE_CONFIG_2, mode);
    }

    return getError();
}

/// @brief Sets a condition which initiates a single conversion based off
///  already configured registers. A running conversion completes before
///  execuring a trigger. Redundant triggers are ignored. TRIGGER_MODE
///  is available only during the mode 'Stand-by mode' mentioned in OPERATING_MODE.
/// @param trigMode value to set the trigger mode of the device
///     0X0 = Conversion Start at I2C Command Bits, DEFAULT
///     0X1 = Conversion starts through trigger signal at !INT pin
///     TMAG5273_REG_DEVICE_CONFIG_2 - bit 2
/// @return Error code (0 is success, negative is failure, positive is warning)
int8_t TMAG5273::setTriggerMode(uint8_t trigMode)
{
    uint16_t mode = 0;
    mode = readRegister(TMAG5273_REG_DEVICE_CONFIG_2);

    // If-Else statement for writing values to the register, bit by bit
    if (trigMode == 0) // 0b0
    {
        bitWrite(mode, 2, 0);
        writeRegister(TMAG5273_REG_DEVICE_CONFIG_2, mode);
    }
    else if (trigMode == 0X1) // 0b1
    {
        bitWrite(mode, 2, 1);
        writeRegister(TMAG5273_REG_DEVICE_CONFIG_2, mode);
    }

    return getError();
}

/// @brief Sets the operating mode from one of the 4 modes:
///  stand-by mode, sleed mode, continuous measure mode, and
///  wake-up and sleep mode.
/// @param opMode value to set the operating mode of the device
///     0X0 = Stand-by mode (starts new conversion at trigger event)
///     0X1 = Sleep mode
///     0X2 = Continuous measure mode
///     0X3 = Wake-up and sleep mode (W&S Mode)
///     TMAG5273_REG_DEVICE_CONFIG_2 - bit 1-0
/// @return Error code (0 is success, negative is failure, positive is warning)
int8_t TMAG5273::setOperatingMode(uint8_t opMode)
{
    uint16_t mode = 0;
    mode = readRegister(TMAG5273_REG_DEVICE_CONFIG_2);

    // If-Else statement for writing values to the register, bit by bit
    if (opMode == 0) // 0b00
    {
        // Write 0 to bit 0
        bitWrite(mode, 0, 0);
        // Write 0 to bit 1
        bitWrite(mode, 1, 0);
        writeRegister(TMAG5273_REG_DEVICE_CONFIG_2, mode);
    }
    else if (opMode == 0X1) // 0b01
    {
        // Write 1 to bit 0
        bitWrite(mode, 0, 1);
        // Write 0 to bit 1
        bitWrite(mode, 1, 0);
        writeRegister(TMAG5273_REG_DEVICE_CONFIG_2, mode);
    }
    else if (opMode == 0X2) // 0b10
    {
        bitWrite(mode, 0, 0);
        bitWrite(mode, 1, 1);
        writeRegister(TMAG5273_REG_DEVICE_CONFIG_2, mode);
    }
    else if (opMode == 0X3) // 0b11
    {
        bitWrite(mode, 0, 1);
        bitWrite(mode, 1, 1);
        writeRegister(TMAG5273_REG_DEVICE_CONFIG_2, mode);
    }

    return getError();
}

/// @brief Sets the data aquisition from the following magnetic
///  axis channels listed below
/// @param channelMode Value that sets the channel for data aquisition
///     0X0 = All magnetic channels off, DEFAULT
///     0X1 = X Channel Enabled
///     0X2 = Y Channel Enabled
///     0X3 = X, Y Channel Enabled
///     0X4 = Z Channel Enabled
///     0X5 = Z, X Channel Enabled
///     0X6 = Y, Z Channel Enabled
///     0X7 = X, Y, Z Channel Enabled
///     0X8 = XYX Channel Enabled
///     0X9 = YXY Channel Enabled
///     0XA = YZY Channel Enabled
///     0XB = XZX Channel Enabled
///     TMAG5273_REG_SENSOR_CONFIG_1 - bits 7-4
/// @return Error code (0 is success, negative is failure, positive is warning)
int8_t TMAG5273::setMagneticChannel(uint8_t channelMode)
{
    uint16_t mode = 0;
    mode = readRegister(TMAG5273_REG_SENSOR_CONFIG_1);

    // If-Else statement for writing values to the register, bit by bit
    if (channelMode == 0X0) // 0b0000
    {
        // Writes 0 to bit 4 of the register
        bitWrite(mode, 4, 0);
        // Writes 0 to bit 5 of the register
        bitWrite(mode, 5, 0);
        // Writes 0 to bit 6 of the register
        bitWrite(mode, 6, 0);
        // Writes 0 to bit 7 of the register
        bitWrite(mode, 7, 0);
        // Writes the new register value back into TMAG5273_REG_SENSOR_CONFIG_1
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_1, mode);
        // writeRegisterRegion(TMAG5273_REG_SENSOR_CONFIG_1, mode, 0);
    }
    else if (channelMode == 0X1) // 0x0001
    {
        bitWrite(mode, 4, 1);
        bitWrite(mode, 5, 0);
        bitWrite(mode, 6, 0);
        bitWrite(mode, 7, 0);
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_1, mode);
    }
    else if (channelMode == 0X2) // 0x0010
    {
        bitWrite(mode, 4, 0);
        bitWrite(mode, 5, 1);
        bitWrite(mode, 6, 0);
        bitWrite(mode, 7, 0);
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_1, mode);
    }
    else if (channelMode == 0X3) // 0x0011
    {
        bitWrite(mode, 4, 1);
        bitWrite(mode, 5, 1);
        bitWrite(mode, 6, 0);
        bitWrite(mode, 7, 0);
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_1, mode);
    }
    else if (channelMode == 0X4) // 0x0100
    {
        bitWrite(mode, 4, 0);
        bitWrite(mode, 5, 0);
        bitWrite(mode, 6, 1);
        bitWrite(mode, 7, 0);
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_1, mode);
    }
    else if (channelMode == 0X5) // 0x0101
    {
        bitWrite(mode, 4, 1);
        bitWrite(mode, 5, 0);
        bitWrite(mode, 6, 1);
        bitWrite(mode, 7, 0);
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_1, mode);
    }
    else if (channelMode == 0X6) // 0x0110
    {
        bitWrite(mode, 4, 0);
        bitWrite(mode, 5, 1);
        bitWrite(mode, 6, 1);
        bitWrite(mode, 7, 0);
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_1, mode);
    }
    else if (channelMode == 0X7) // 0x0111
    {
        bitWrite(mode, 4, 1);
        bitWrite(mode, 5, 1);
        bitWrite(mode, 6, 1);
        bitWrite(mode, 7, 0);
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_1, mode);
    }
    else if (channelMode == 0X8) // 0x1000
    {
        bitWrite(mode, 4, 0);
        bitWrite(mode, 5, 0);
        bitWrite(mode, 6, 0);
        bitWrite(mode, 7, 1);
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_1, mode);
    }
    else if (channelMode == 0X9) // 0x1001
    {
        bitWrite(mode, 4, 1);
        bitWrite(mode, 5, 0);
        bitWrite(mode, 6, 0);
        bitWrite(mode, 7, 1);
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_1, mode);
    }
    else if (channelMode == 0XA) // 0x1010
    {
        bitWrite(mode, 4, 0);
        bitWrite(mode, 5, 1);
        bitWrite(mode, 6, 0);
        bitWrite(mode, 7, 1);
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_1, mode);
    }
    else if (channelMode == 0XB) // 0x1011
    {
        bitWrite(mode, 4, 1);
        bitWrite(mode, 5, 1);
        bitWrite(mode, 6, 0);
        bitWrite(mode, 7, 1);
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_1, mode);
    }

    return getError();
}

/// @brief Sets the time spent in low power mode between conversions
///  when OPERATING_MODE = 11b.
/// @param sleepTime Value to set the time desired
///     0X0 = 1ms
///     0X1 = 5ms
///     0X2 = 10ms
///     0X3 = 15ms
///     0X4 = 20ms
///     0X5 = 30ms
///     0X6 = 50ms
///     0X7 = 100ms
///     0X8 = 500ms
///     0X9 = 1000ms
///     0XA = 2000ms
///     0XB = 5000ms
///     0XC = 20000ms
///     TMAG5273_REG_SENSOR_CONFIG_1 - bit 3-0
/// @return Error code (0 is success, negative is failure, positive is warning)
int8_t TMAG5273::setSleeptime(uint8_t sleepTime)
{
    uint16_t mode = 0;
    mode = readRegister(TMAG5273_REG_SENSOR_CONFIG_1);

    if (sleepTime == 0X0) // 0b0000
    {
        bitWrite(mode, 0, 0);
        bitWrite(mode, 1, 0);
        bitWrite(mode, 2, 0);
        bitWrite(mode, 3, 0);
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_1, mode);
    }
    else if (sleepTime == 0X1) // 0b0001
    {
        bitWrite(mode, 0, 1);
        bitWrite(mode, 1, 0);
        bitWrite(mode, 2, 0);
        bitWrite(mode, 3, 0);
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_1, mode);
    }
    else if (sleepTime == 0X2) // 0b0010
    {
        bitWrite(mode, 0, 0);
        bitWrite(mode, 1, 1);
        bitWrite(mode, 2, 0);
        bitWrite(mode, 3, 0);
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_1, mode);
    }
    else if (sleepTime == 0X3) // 0b0011
    {
        bitWrite(mode, 0, 1);
        bitWrite(mode, 1, 1);
        bitWrite(mode, 2, 0);
        bitWrite(mode, 3, 0);
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_1, mode);
    }
    else if (sleepTime == 0X4) // 0b0100
    {
        bitWrite(mode, 0, 0);
        bitWrite(mode, 1, 0);
        bitWrite(mode, 2, 1);
        bitWrite(mode, 3, 0);
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_1, mode);
    }
    else if (sleepTime == 0X5) // 0b0101
    {
        bitWrite(mode, 0, 1);
        bitWrite(mode, 1, 0);
        bitWrite(mode, 2, 1);
        bitWrite(mode, 3, 0);
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_1, mode);
    }
    else if (sleepTime == 0X6) // 0b0110
    {
        bitWrite(mode, 0, 0);
        bitWrite(mode, 1, 1);
        bitWrite(mode, 2, 1);
        bitWrite(mode, 3, 0);
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_1, mode);
    }
    else if (sleepTime == 0X7) // 0b0111
    {
        bitWrite(mode, 0, 1);
        bitWrite(mode, 1, 1);
        bitWrite(mode, 2, 1);
        bitWrite(mode, 3, 0);
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_1, mode);
    }
    else if (sleepTime == 0X8) // 0b1000
    {
        bitWrite(mode, 0, 0);
        bitWrite(mode, 1, 0);
        bitWrite(mode, 2, 0);
        bitWrite(mode, 3, 1);
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_1, mode);
    }
    else if (sleepTime == 0X9) // 0b1001
    {
        bitWrite(mode, 0, 1);
        bitWrite(mode, 1, 0);
        bitWrite(mode, 2, 0);
        bitWrite(mode, 3, 1);
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_1, mode);
    }
    else if (sleepTime == 0XA) // 0b1010
    {
        bitWrite(mode, 0, 0);
        bitWrite(mode, 1, 1);
        bitWrite(mode, 2, 0);
        bitWrite(mode, 3, 1);
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_1, mode);
    }
    else if (sleepTime == 0XB) // 0b1011
    {
        bitWrite(mode, 0, 1);
        bitWrite(mode, 1, 1);
        bitWrite(mode, 2, 0);
        bitWrite(mode, 3, 1);
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_1, mode);
    }
    else if (sleepTime == 0XC) // 0b1100
    {
        bitWrite(mode, 0, 0);
        bitWrite(mode, 1, 0);
        bitWrite(mode, 2, 1);
        bitWrite(mode, 3, 1);
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_1, mode);
    }

    return getError();
}

/// @brief Sets the direction of threshold check. This bit
///  is ignored when THR_HYST > 001b
/// @param threshDir value to set the direction of threshold
///     0X0 = sets interrupt for field above the threshold
///     0X1 = sets interrupt for field below the threshold
///     TMAG5273_REG_SENSOR_CONFIG_2 - bit 5
/// @return Error code (0 is success, negative is failure, positive is warning)
int8_t TMAG5273::setMagDir(uint8_t threshDir)
{
    uint16_t mode = 0;
    mode = readRegister(TMAG5273_REG_SENSOR_CONFIG_2);

    // If-Else statement for writing values to the register, bit by bit
    if (threshDir == 0X0) // 0b0
    {
        // Write 0 to bit 5 of the register value
        bitWrite(mode, 5, 0);
        // Writes mode to the CONFIG_2 register
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_2, mode);
    }
    else if (threshDir == 0X1) // 0b1
    {
        bitWrite(mode, 5, 1);
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_2, mode);
    }

    return getError();
}

/// @brief Sets the axis for magnitude gain correction valued
/// entered in MAG_GAIN_CONFIG register
/// @param gainAdjust Value to set the gain correction value
///     0X0 = 1st channel is selected for gain adjustment
///     0X1 = 2nd channel is selected for gain adjustment
///     TMAG5273_REG_SENSOR_CONFIG_2 - bit 4
/// @return Error code (0 is success, negative is failure, positive is warning)
int8_t TMAG5273::setMagnitudeGain(uint8_t gainAdjust)
{
    uint16_t mode = 0;
    mode = readRegister(TMAG5273_REG_SENSOR_CONFIG_2);

    // If-Else statement for writing values to the register, bit by bit
    if (gainAdjust == 0X0) // 0b0
    {
        // Write 0 to bit 4 of the register value
        bitWrite(mode, 4, 0);
        // Writes mode to the CONFIG_2 register
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_2, mode);
    }
    else if (gainAdjust == 0X1) // 0b1
    {
        bitWrite(mode, 4, 1);
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_2, mode);
    }

    return getError();
}

/// @brief This function sets an 8-bit gain value determined by a
///  primary to adjust a Hall axis gain. The particular axis is selected
///  based off the settings of MAG_GAIN_CH and ANGLE_EN register bits.
///  The binary 8-bit input is interpreted as a fracitonal value between
///  0 and 1 based off the formula 'user entered value in decimal/256.'
///  Gain value of 0 is interpreted by the device as 1.
///     TMAG5273_REG_MAG_GAIN_CONFIG
/// @param magneticGain
/// @return Error code (0 is success, negative is failure, positive is warning)
int8_t TMAG5273::setMagneticGain(float magneticGain)
{
    // Convert magneticGain to be the correct value to write to their register
    int8_t magneticGainReg = 0;
    magneticGainReg = magneticGain / 256;

    // Write value to register
    writeRegister(TMAG5273_REG_MAG_GAIN_CONFIG, magneticGainReg);

    return getError();
}

/// @brief This function will write an 8-bit, 2's complement offset value
///  determined by a primary to adjust the first axis offset value. The
///  range of possible offset valid entrees can be +/-128. The offset value
///  is calculated by multiplying bit resolution with the entered value.
///     TMAG5273_REG_MAG_OFFSET_CONFIG_1
/// @param offset1 Value within the range +/-128
/// @return Error code (0 is success, negative is failure, positive is warning)
int8_t TMAG5273::setMagneticOffset1(float offset1)
{
    uint8_t rangeValXY = getXYAxisRange();
    uint8_t range = 0;
    if (rangeValXY == 0)
    {
        range = 40;
    }
    else if (rangeValXY == 1)
    {
        range = 80;
    }

    // Multiply bit resolution with entered value from datasheet
    int8_t magOffset = (2048 * offset1) / range; // 2048 = 2^12
    writeRegister(TMAG5273_REG_MAG_OFFSET_CONFIG_1, magOffset);

    return getError();
}

/// @brief This function will write an 8-bit, 2's complement offset value
///  determined by a primary to adjust the first axis offset value. The
///  range of possible offset valid entrees can be +/-128. The offset value
///  is calculated by multiplying bit resolution with the entered value.
///     TMAG5273_REG_MAG_OFFSET_CONFIG_2
/// @param offset2 Value within the range +/-128
/// @return Error code (0 is success, negative is failure, positive is warning)
int8_t TMAG5273::setMagneticOffset2(float offset2)
{
    // Determine whether the magnetic offset channel is Y or Z for the range selection
    uint8_t channelSelect = getAngleEn(); // 1, 2, or 3

    uint8_t range = 0;

    // See which XY axis range is currently set
    uint8_t rangeValXY = getXYAxisRange();
    uint8_t rangeXY = 0;
    if (rangeValXY == 0)
    {
        range = 40;
    }
    else if (rangeValXY == 1)
    {
        range = 80;
    }

    // See which Z axis range is currently set
    uint8_t rangeValZ = getZAxisRange();
    uint8_t rangeZ = 0;
    if (rangeValZ == 0)
    {
        rangeZ = 40;
    }
    else if (rangeValZ == 1)
    {
        rangeZ = 80;
    }

    // Choose the channel
    if (channelSelect == 1) // Y
    {
        range = rangeXY; // 40 or 80
    }
    else if (channelSelect == 2) // Z
    {
        range = rangeZ; // 40 or 80
    }
    else if (channelSelect == 3) // Z
    {
        range = rangeZ; // 40 or 80
    }
    else
    {
        return -1; // Returns error
        // Error could also be the ranges being disabled
    }

    // Multiply bit resolution with entered value
    int8_t magOffset = (2048 * offset2) / range; // 2048 = 2^12
    // Write correct value to register
    writeRegister(TMAG5273_REG_MAG_OFFSET_CONFIG_2, magOffset);

    return getError();
}

/// @brief Sets the angle calculation, magnetic gain, and offset corrections
///  between two selected magnetic channels
/// @param angleEnable value to write to the register for which angle calculation enabled
///     0X0 = No angle calculation, magnitude gain, and offset
///            correction enabled
///     0X1 = X 1st, Y 2nd
///     0X2 = Y 1st, Z 2nd
///     0X3 = X 1st, Z 2nd
///     TMAG5273_REG_SENSOR_CONFIG_2 - bit 3-2
/// @return Error code (0 is success, negative is failure, positive is warning)
int8_t TMAG5273::setAngleEn(uint8_t angleEnable)
{
    uint16_t mode = 0;
    mode = readRegister(TMAG5273_REG_SENSOR_CONFIG_2);

    // If-Else statement for writing values to the register, bit by bit
    if (angleEnable == 0X0) // 0b00
    {
        // Write 0 to bit 2 of the register value
        bitWrite(mode, 2, 0);
        // Write 0 to bit 3 of the register value
        bitWrite(mode, 3, 0);
        // Writes mode to the CONFIG_2 register
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_2, mode);
    }
    else if (angleEnable == 0X1) // 0b01
    {
        bitWrite(mode, 2, 1);
        bitWrite(mode, 3, 0);
        // Writes mode to the CONFIG_2 register
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_2, mode);
    }
    else if (angleEnable == 0X2) // 0b10
    {
        bitWrite(mode, 2, 0);
        bitWrite(mode, 3, 1);
        // Writes mode to the CONFIG_2 register
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_2, mode);
    }
    else if (angleEnable == 0X3) // 0b11
    {
        bitWrite(mode, 2, 1);
        bitWrite(mode, 3, 1);
        // Writes mode to the CONFIG_2 register
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_2, mode);
    }

    return getError();
}

/// @brief Sets the X and Y axes magnetic range from 2 different options
/// @param xyAxisRange Value to choose the magnetic range
///     0X0 = ±40mT, DEFAULT
///     0X1 = ±80mT
///     TMAG5273_REG_SENSOR_CONFIG_2 - bit 1
/// @return Error code (0 is success, negative is failure, positive is warning)
int8_t TMAG5273::setXYAxisRange(uint8_t xyAxisRange)
{
    uint16_t mode = 0;
    mode = readRegister(TMAG5273_REG_SENSOR_CONFIG_2);

    // If-Else statement for writing values to the register, bit by bit
    if (xyAxisRange == 0X0) // 0b0
    {
        // Write 0 to bit 1 of the register value
        bitWrite(mode, 1, 0);
        // Writes mode to the CONFIG_2 register
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_2, mode);
    }
    else if (xyAxisRange == 0X1) // 0b1
    {
        bitWrite(mode, 1, 1);
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_2, mode);
    }

    return getError();
}

/// @brief Sets the Z magnetic range from 2 different options
/// @param zAxisRange Value to set the range from either 40mT or 80mT
///     0X0 = ±40mT, DEFAULT
///     0X1 = ±80mT
///     TMAG5273_REG_SENSOR_CONFIG_2 - bit 0
/// @return Error code (0 is success, negative is failure, positive is warning)
int8_t TMAG5273::setZAxisRange(uint8_t zAxisRange)
{
    uint16_t mode = 0;
    mode = readRegister(TMAG5273_REG_SENSOR_CONFIG_2);

    // If-Else statement for writing values to the register, bit by bit
    if (zAxisRange == 0X0) // 0b0
    {
        // Write 0 to bit 0 of the register value
        bitWrite(mode, 0, 0);
        // Writes mode to the CONFIG_2 register
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_2, mode);
    }
    else if (zAxisRange == 0X1) // 0b1
    {
        bitWrite(mode, 0, 1);
        writeRegister(TMAG5273_REG_SENSOR_CONFIG_2, mode);
    }

    return getError();
}

/// @brief Sets an 8-bit, 2's complement X axis threshold code for limit
///  check. The range of possible threshold entrees can be +/-128. The
///  threshold value in mT is calculated as (40(1+X_Y_RANGE)/128)*X_THR_CONFIG.
///  Default 0h means no threshold comparison.
/// @param xThreshold 8-bit value to set the threshold for the X limit
///     TMAG5273_REG_X_THR_CONFIG - bits 7-0
/// @return Error code (0 is success, negative is failure, positive is warning)
int8_t TMAG5273::setXThreshold(float xThreshold)
{
    uint8_t range = getXYAxisRange();
    // Mulyiply raw value by threshold equation in datasheet
    int8_t threshold = xThreshold * 128 / (40 * (1 + range));

    // Write the correct value to the register
    writeRegister(TMAG5273_REG_X_THR_CONFIG, threshold);

    return getError();
}

/// @brief Sets an 8-bit, 2's complement Y axis threshold code for limit
///  check. The range of possible threshold entrees can be +/-128. The
///  threshold value in mT is calculated as (40(1+X_Y_RANGE)/128)*Y_THR_CONFIG.
///  Default 0h means no threshold comparison.
/// @param yThreshold 8-bit value to set the threshold for the Y limit
///     TMAG5273_REG_Y_THR_CONFIG - bits 7-0
/// @return Error code (0 is success, negative is failure, positive is warning)
int8_t TMAG5273::setYThreshold(float yThreshold)
{
    uint8_t range = getXYAxisRange();
    // Mulyiply raw value by threshold equation in datasheet
    int8_t threshold = yThreshold * 128 / (40 * (1 + range));
    // Write the correct value to the register
    writeRegister(TMAG5273_REG_Y_THR_CONFIG, threshold);

    return getError();
}

/// @brief Sets an 8-bit, 2's complement Z axis threshold code for limit
///  check. The range of possible threshold entrees can be +/-128. The
///  threshold value in mT is calculated as (40(1+Z_RANGE)/128)*Z_THR_CONFIG.
///  Default 0h means no threshold comparison.
/// @param zThresh 8-bit value to set the threshold for the Y limit
///     TMAG5273_REG_Z_THR_CONFIG - bits 7-0
/// @return Error code (0 is success, negative is failure, positive is warning)
int8_t TMAG5273::setZThreshold(float zThreshold)
{
    // See which Z axis range is currently set
    uint8_t range = getZAxisRange();
    // Mulyiply raw value by threshold equation in datasheet
    int8_t threshold = zThreshold * 128 / (40 * (1 + range));
    // Write the correct value to the register
    writeRegister(TMAG5273_REG_Z_THR_CONFIG, threshold);

    return getError();
}

/// @brief Sets the temperature threshold code entered by the user. The
///  valid temperature threshold ranges are -41C to 170C.
///  Default 0x0 means no threshold comparison.
/// @param tempThresh 8-bit value to set the threshold for the temperature limit
///     TMAG5273_REG_T_CONFIG - bits 7-1
/// @return Error code (0 is success, negative is failure, positive is warning)
int8_t TMAG5273::setTemperatureThreshold(int8_t tempThresh)
{
    // Write the new address into the register and enable the temperature to be read 
    // NOTE: Temperature is set to be enabled here for temperatures readings. 
    writeRegister(TMAG5273_REG_T_CONFIG, (tempThresh << 1) | 0x01);

    return getError();
}

/// @brief Sets the enable bit that determines the data acquisition of the
///  temperature channel.
/// @param temperatureEnable Value to determine enable or disable
///     0x0 = Temp Channel Disabled
///     0x1 = Temp Channel Enabled
///     TMAG5273_REG_T_CONFIG - bit 0
/// @return Error code (0 is success, negative is failure, positive is warning)
int8_t TMAG5273::setTemperatureEn(bool temperatureEnable)
{
    uint16_t mode = 0;
    mode = readRegister(TMAG5273_REG_T_CONFIG);

    // If-Else statement for writing values to the register, bit by bit
    if (temperatureEnable == 0) // 0b0
    {
        // Write 0 to bit 7 of the register value
        bitWrite(mode, 0, 0);
        // Writes mode to the T_CONFIG register
        writeRegister(TMAG5273_REG_T_CONFIG, mode);
    }
    else if (temperatureEnable == 1) // 0b1
    {
        bitWrite(mode, 0, 1);
        writeRegister(TMAG5273_REG_T_CONFIG, mode);
    }

    return getError();
}

/// @brief Sets the enable interrupt response bit on conversion complete.
/// @param interruptEnable Value to determine if interrupt is or is not asserted
///     0X0 = Interrupt is NOT asserted when the configured set of
///           conversions are complete
///     0X1 = Interrupt is asserted when the configured set of
///           conversions are complete
///     TMAG5273_REG_INT_CONFIG_1 - bit 7
/// @return Error code (0 is success, negative is failure, positive is warning)
int8_t TMAG5273::setInterruptResult(bool interruptEnable)
{
    uint16_t mode = 0;
    mode = readRegister(TMAG5273_REG_INT_CONFIG_1);

    // If-Else statement for writing values to the register, bit by bit
    if (interruptEnable == 0) // 0b0
    {
        // Write 0 to bit 7 of the register value
        bitWrite(mode, 7, 0);
        // Writes mode to the CONFIG_1 register
        writeRegister(TMAG5273_REG_INT_CONFIG_1, mode);
    }
    else if (interruptEnable == 1) // 0b1
    {
        bitWrite(mode, 7, 1);
        writeRegister(TMAG5273_REG_INT_CONFIG_1, mode);
    }

    return getError();
}

/// @brief Configures the bit that enables interrupt response on a
///   predefined threshold cross.
/// @param enableInterruptResponse Value to determine if interrupt is or is not asserted
///     0X0 = Interrupt is NOT asserted when a threshold is crossed
///     0X1 = Interrupt is asserted when a threshold is crossed
///     TMAG5273_REG_INT_CONFIG_1 - bit 6
/// @return Error code (0 is success, negative is failure, positive is warning)
int8_t TMAG5273::setThresholdEn(bool enableInterruptResponse)
{
    uint16_t mode = 0;
    mode = readRegister(TMAG5273_REG_INT_CONFIG_1);

    // If-Else statement for writing values to the register, bit by bit
    if (enableInterruptResponse == 0) // 0b0
    {
        // Write 0 to bit 6 of the register value
        bitWrite(mode, 6, 0);
        // Writes mode to the CONFIG_2 register
        writeRegister(TMAG5273_REG_INT_CONFIG_1, mode);
    }
    else if (enableInterruptResponse == 1) // 0b1
    {
        bitWrite(mode, 6, 1);
        writeRegister(TMAG5273_REG_INT_CONFIG_1, mode);
    }

    return getError();
}

/// @brief Enables the !INT interrupt if it is latched or pulsed.
/// @param interruptState Value to determine the interrupt state
///     0X0 = !INT interrupt latched until clear by a primary
///           addressing the device
///     0X1 = !INT interrupt pulse for 10us
///     TMAG5273_REG_INT_CONFIG_1 - bit 5
/// @return Error code (0 is success, negative is failure, positive is warning)
int8_t TMAG5273::setIntPinState(bool interruptState)
{
    uint16_t mode = 0;
    mode = readRegister(TMAG5273_REG_INT_CONFIG_1);

    // If-Else statement for writing values to the register, bit by bit
    if (interruptState == 0) // 0b0
    {
        // Write 0 to bit 5 of the register value
        bitWrite(mode, 5, 0);
        // Writes mode to the CONFIG_2 register
        writeRegister(TMAG5273_REG_INT_CONFIG_1, mode);
    }
    else if (interruptState == 1) // 0b1
    {
        bitWrite(mode, 5, 1);
        writeRegister(TMAG5273_REG_INT_CONFIG_1, mode);
    }

    return getError();
}

/// @brief Configures the interrupt mode select
/// @param config_mode Value to determine the int select
///     0X0 = No interrupt
///     0X1 = Interrupt through !INT
///     0X2 = Interrupt through !INT except when I2C bus is busy
///     0X3 = Interrupt through SCL
///     0X4 = Interrupt through SCL except when I2C bus is busy
///     TMAG5273_REG_INT_CONFIG_1 - bit 4-2
/// @return Error code (0 is success, negative is failure, positive is warning)
int8_t TMAG5273::setInterruptMode(uint8_t configurationMode)
{
    uint16_t mode = 0;
    mode = readRegister(TMAG5273_REG_INT_CONFIG_1);

    // If-Else statement for writing values to the register, bit by bit
    if ((configurationMode == 0X0) || (configurationMode == 0X1) || (configurationMode == 0X2) ||
        (configurationMode == 0x3) || (configurationMode == 0x4))
    {
        // Shifts the bits over 4 to be concatenated into the new register
        uint8_t config = configurationMode << 2;
        // Make sure all of the INT mode bits are set to 0
        mode = mode & 0XE3;
        mode = mode | config;
        writeRegister(TMAG5273_REG_INT_CONFIG_1, mode);
    }

    return getError();
}

/// @brief Configures the Mask !INT pin when !INT is connected to GND
/// @param interruptPinEnable Value to choose the INT enable or disable
///     0X0 = !INT pin is enabled
///     0X1 = !INT pin is disabled (for wake-up and trigger functions)
///     TMAG5273_REG_INT_CONFIG_1 - bit 0
/// @return Error code (0 is success, negative is failure, positive is warning)
int8_t TMAG5273::setMaskInterrupt(bool interruptPinEnable)
{
    uint16_t mode = 0;
    mode = readRegister(TMAG5273_REG_INT_CONFIG_1);

    // If-Else statement for writing values to the register, bit by bit
    if (interruptPinEnable == 0) // 0b0
    {
        // Write 0 to bit 1 of the register value
        bitWrite(mode, 0, 0);
        // Writes mode to the CONFIG_2 register
        writeRegister(TMAG5273_REG_INT_CONFIG_1, mode);
    }
    else if (interruptPinEnable == 1) // 0b1
    {
        bitWrite(mode, 0, 1);
        writeRegister(TMAG5273_REG_INT_CONFIG_1, mode);
    }

    return getError();
}

/// @brief This register is loaded with the default I2C address
///  from OTP during first power up. Change these bits to a
///  new setting if a new I2C address is required (at each
///  power cycle these btis must be written again to avoid
///  going back to default factory address).
///  *NOTE*: To be able to use this function, make sure to
///  enable the I2C Address Bit (see function setI2CAddressEN
///  for more information).
/// @param address Address value to set device to
///     TMAG5273_REG_I2C_ADDRESS - bits 7-1
/// @return Error code (0 is success, negative is failure, positive is warning)
int8_t TMAG5273::setI2CAddress(uint8_t address)
{
    // Write the new address into the register and enable the register to be read
    writeRegister(TMAG5273_REG_I2C_ADDRESS, (address << 1) | 0x01);
    // Set the device address to the new address chosen
    _deviceAddress = address;

    return getError();
}

/// @brief Writes to the I2C_ADDRESS_UPDATE_EN bit to enable
///  a new user defined I2C address.
/// @param addressEnable Value to determine if the user can write a new address.
///     0X0 = Disable update of I2C address
///     0X1 = Enable update of I2C address with bits (7:1)
/// @return Error code (0 is success, negative is failure, positive is warning)
int8_t TMAG5273::setI2CAddressEN(bool addressEnable)
{
    uint16_t addReg = 0;
    addReg = readRegister(TMAG5273_REG_I2C_ADDRESS);

    // If-Else statement for writing values to the register, bit by bit
    if (addressEnable == 0)
    {
        // Write 0 to bit 0 of register
        bitWrite(addReg, 0, 0);
        writeRegister(TMAG5273_REG_I2C_ADDRESS, addReg);
    }
    else if (addressEnable == 1)
    {
        // Write 1 to bit 0 of register
        bitWrite(addReg, 0, 1);
        writeRegister(TMAG5273_REG_I2C_ADDRESS, addReg);
    }

    return getError();
}

/// @brief This function clears the oscillator error flag
///  when it is raised high by an error.
/// @param oscError Write '1' to clear the error flag
///     0X0 = No oscialltor error detected
///     0X1 = Oscillator error detected
///     TMAG5273_REG_DEVICE_STATUS - bit 3
/// @return Error code (0 is success, negative is failure, positive is warning)
int8_t TMAG5273::setOscillatorError(bool oscError)
{
    uint16_t deviceStatusReg = 0;
    deviceStatusReg = readRegister(TMAG5273_REG_DEVICE_STATUS);

    if (oscError == 1)
    {
        // Writes 1 to bit 4 of the register
        bitWrite(deviceStatusReg, 4, 1);
    }
    else
    {
        return getError();
    }

    return getError();
}

/************************************************************************************************/
/**************************        GET CONFIGURATION SETTINGS         ***************************/
/************************************************************************************************/

/// @brief Returns the I2C CRC byte to be sent
///     0X0 = CRC disabled
///     0X1 = CRC enabled
///     TMAG5273_REG_DEVICE_CONFIG_1 - bit 7
/// @return CRC disabled (0) or CRC enables (1)
uint8_t TMAG5273::getCRCMode()
{
    uint8_t getCRC = 0;
    getCRC = readRegister(TMAG5273_REG_DEVICE_CONFIG_1);

    uint8_t mode = bitRead(getCRC, 7);

    return mode;
}

/// @brief Returns the temp coefficient of the magnet
///     0X0 = 0% (No temperature compensation)
///     0X1 = 0.12%/deg C (NdBFe)
///     0X2 = Reserved
///     0X3 = 0.2%/deg C (Ceramic)
///     TMAG5273_REG_DEVICE_CONFIG_1 - bit 6-5
/// @return Temperature coefficient of the magnet
uint8_t TMAG5273::getMagTemp()
{
    uint8_t magTemp = 0;
    magTemp = readRegister(TMAG5273_REG_DEVICE_CONFIG_1);

    // Fills with bit 5
    uint8_t tempReg5 = bitRead(magTemp, 5);
    // Fills with bit 6
    uint8_t tempReg6 = bitRead(magTemp, 6);

    if ((tempReg5 == 0) && (tempReg6 == 0)) // 0b00
    {
        return 0; // 0%
    }
    else if ((tempReg5 == 1) && (tempReg6 == 0)) // 0b01
    {
        return 1; // 0.12%/deg C
    }
    else if ((tempReg5 == 0) && (tempReg6 == 1)) // 0b10
    {
        return 2; // Reserved
    }
    else if ((tempReg5 == 1) && (tempReg6 == 1)) // 0b11
    {
        return 3; // 0.2%/deg C
    }
    else
    {
        return 1; // default
    }
}

/// @brief Returns the sampling of the sensor data to reduce
///  the noise effect (or to increase resolution)
///     0X0 = 1x average, 10.0-kSPS (3-axes) or 20-kSPS (1 axis)
///     0X1 = 2x average, 5.7-kSPS (3-axes) or 13.3-kSPS (1 axis)
///     0X2 = 4x average, 3.1-kSPS (3-axes) or 8.0-kSPS (1 axis)
///     0X3 = 8x average, 1.6-kSPS (3-axes) or 4.4-kSPS (1 axis)
///     0X4 = 16x average, 0.8-kSPS (3-axes) or 2.4-kSPS (1 axis)
///     0X5 =  32x average, 0.4-kSPS (3-axes) or 1.2-kSPS (1 axis)
///     TMAG5273_REG_DEVICE_CONFIG_1 - bit 4-2
/// @return Value of the sampling average of the sensor data
uint8_t TMAG5273::getConvAvg()
{
    uint8_t convAv = 0;
    convAv = readRegister(TMAG5273_REG_DEVICE_CONFIG_1);

    uint8_t convBit2 = bitRead(convAv, 2);
    uint8_t convBit3 = bitRead(convAv, 3);
    uint8_t convBit4 = bitRead(convAv, 4);

    if ((convBit2 == 0) && (convBit3 == 0) && (convBit4 == 0)) // 0b000
    {
        return 0; // 1x
    }
    else if ((convBit2 == 1) && (convBit3 == 0) && (convBit4 == 0)) // 0b001
    {
        return 1; // 2x
    }
    else if ((convBit2 == 0) && (convBit3 == 1) && (convBit4 == 0)) // 0b010
    {
        return 2; // 4x
    }
    else if ((convBit2 == 1) && (convBit3 == 1) && (convBit4 == 0)) // 0b011
    {
        return 3; // 8x
    }
    else if ((convBit2 == 1) && (convBit3 == 0) && (convBit4 == 0)) // 0b100
    {
        return 4; // 16x
    }
    else if ((convBit2 == 1) && (convBit3 == 0) && (convBit4 == 1)) // 0b001
    {
        return 5; // 32x
    }
    else
    {
        return 1; // default
    }
}

/// @brief Returns the I2C read mode
///     0X0 = Standard I2C 3-byte read command
///     0X1 = 1-byte I2C read command for 16bit sensor data and
///           conversion status
///     0X2 = 1-byte I2C read command for 8bit sernsor MSB data and
///           conversion status
///     TMAG5273_REG_DEVICE_CONFIG_1 Bits 0-1
/// @return I2C read mode (0-2)
uint8_t TMAG5273::getReadMode()
{
    // Read the device config 1 register
    uint8_t readModeReg = readRegister(TMAG5273_REG_DEVICE_CONFIG_1);

    // Read the bits to determine the value to return
    uint8_t readMode0 = bitRead(readModeReg, 0);
    uint8_t readMode1 = bitRead(readModeReg, 1);

    if ((readMode0 == 0) && (readMode1 == 0)) // 0b00
    {
        return 0;
    }
    else if ((readMode0 == 1) && (readMode1 == 0)) // 0b01
    {
        return 1;
    }
    else if ((readMode0 == 0) && (readMode1 == 1)) // 0b10
    {
        return 2;
    }
    else
    {
        return 0; // default
    }
}

/// @brief Returns the threshold for the interrupt function.
///     0X0 = Takes the 2's complement value of each x_THR_CONFIG
///      register to create a magnetic threshold of the corresponding axis
///     0X1 = Takes the 7LSB bits of the x_THR_CONFIG register to create
///      two opposite magnetic thresholds (one north, and another south)
///      of equal magnitude
///     TMAG5273_REG_DEVICE_CONFIG_2 - bit 7-5
/// @return Threshold for the interrupt function (0X0 - 0X1)
uint8_t TMAG5273::getIntThreshold()
{
    uint8_t interruptThreshold = 0;
    interruptThreshold = readRegister(TMAG5273_REG_DEVICE_CONFIG_2);

    uint8_t interruptThreshold5 = bitRead(interruptThreshold, 5);
    uint8_t interruptThreshold6 = bitRead(interruptThreshold, 6);
    uint8_t interruptThreshold7 = bitRead(interruptThreshold, 7);

    if ((interruptThreshold5 == 0) && (interruptThreshold6 == 0) && (interruptThreshold7 == 0)) // 0b000
    {
        // 2's complement
        return 0;
    }
    else if ((interruptThreshold5 == 1) && (interruptThreshold6 == 0) && (interruptThreshold7 == 0)) // 0b001
    {
        // 7LSB for two opposite mag thresholds
        return 1;
    }
    else
    {
        return 0; // default
    }
}

/// @brief Returns if the device is operating in low power
///  or low noise mode.
///     0X0 = Low active current mode
///     0X1 = Low noise mode
///     TMAG5273_REG_DEVICE_CONFIG_2 - bit 4
/// @return Low power (0) or low noise (1) mode
uint8_t TMAG5273::getLowPower()
{
    uint8_t lowPowerMode = 0;
    lowPowerMode = readRegister(TMAG5273_REG_DEVICE_CONFIG_2);

    uint8_t lowPowerModeBit = bitRead(lowPowerMode, 4);

    return lowPowerModeBit;
}

/// @brief Returns if the I2C glitch filter is on or off
///     0x0 = Glitch filter ON
///     0X0 = Glitch filter OFF
///     TMAG5273_REG_DEVICE_CONFIG_2 - bit 3
/// @return I2C filter ON (0) or OFF (1)
uint8_t TMAG5273::getGlitchFiler()
{
    uint8_t glitchMode = 0;
    glitchMode = readRegister(TMAG5273_REG_DEVICE_CONFIG_2);

    uint8_t glitchModeBit = bitRead(glitchMode, 3);

    return glitchModeBit;
}

/// @brief Returns the condition which initiates a single conversion
///  based off already configured registers. A running conversion
///  completes before executing a trigger. Redundant triggers are
///  ignored. TRIGGER_MODE is available only during the mode 'Stand-
///  by Mode' mentioned in OPERATING_MODE.
///     0X0 = Conversion Start at I2C Command Bits, DEFAULT
///     0X1 = Conversion starts through trigger signal at !INT pin
///     TMAG5273_REG_DEVICE_CONFIG_2 - bit 2
/// @return Condition that initiates conversion thru I2C or INT
uint8_t TMAG5273::getTriggerMode()
{
    uint8_t triggerMode = 0;
    triggerMode = readRegister(TMAG5273_REG_DEVICE_CONFIG_2);

    uint8_t triggerModeBit = bitRead(triggerMode, 2);

    return triggerModeBit;
}

/// @brief Returns the operating mode from one of the 4 listed below:
///     0X0 = Stand-by mode (starts new conversion at trigger event)
///     0X1 = Sleep mode
///     0X2 = Continuous measure mode
///     0X3 = Wake-up and sleep mode (W&S Mode)
///     TMAG5273_REG_DEVICE_CONFIG_2 - bit 1-0
/// @return Operating mode: stand-by, sleep, continuous, or wake-up and sleep
uint8_t TMAG5273::getOperatingMode()
{
    uint8_t opMode = 0;
    opMode = readRegister(TMAG5273_REG_DEVICE_CONFIG_2);

    uint8_t opMode0 = bitRead(opMode, 0);
    uint8_t opMode1 = bitRead(opMode, 1);

    if ((opMode0 == 0) && (opMode1 == 0)) // 0b00
    {
        // Stand-by mode
        return 0;
    }
    else if ((opMode0 == 1) && (opMode1 == 0)) // 0b01
    {
        // Sleep mode
        return 1;
    }
    else if ((opMode0 == 0) && (opMode1 == 1)) // 0b10
    {
        // Continuous measure mode
        return 2;
    }
    else if ((opMode0 == 1) && (opMode1 == 1)) // 0b11
    {
        // Wake-up and sleep mode (W&S Mode)
        return 3;
    }

    return 0;
}

/// @brief Returns data acquisition from the following magnetic axis channels:
///     0X0 = All magnetic channels off, DEFAULT
///     0X1 = X Channel Enabled
///     0X2 = Y Channel Enabled
///     0X3 = X, Y Channel Enabled
///     0X4 = Z Channel Enabled
///     0X5 = Z, X Channel Enabled
///     0X6 = Y, Z Channel Enabled
///     0X7 = X, Y, Z Channel Enabled
///     0X8 = XYX Channel Enabled
///     0X9 = YXY Channel Enabled
///     0XA = YZY Channel Enabled
///     0XB = XZX Channel Enabled
///     TMAG5273_REG_SENSOR_CONFIG_1 - bit 7-4
/// @return Code for the magnetic channel axis being read
uint8_t TMAG5273::getMagneticChannel()
{
    uint8_t magChannel = 0;
    magChannel = readRegister(TMAG5273_REG_SENSOR_CONFIG_1);

    uint8_t magMode4 = bitRead(magChannel, 4);
    uint8_t magMode5 = bitRead(magChannel, 5);
    uint8_t magMode6 = bitRead(magChannel, 6);
    uint8_t magMode7 = bitRead(magChannel, 7);

    if ((magMode4 == 0) && (magMode5 == 0) && (magMode6 == 0) && (magMode7 == 0)) // 0b0000
    {
        return 0; // All mag channels off
    }
    else if ((magMode4 == 1) && (magMode5 == 0) && (magMode6 == 0) && (magMode7 == 0)) // 0b0001
    {
        return 1; // X channel enabled
    }
    else if ((magMode4 == 0) && (magMode5 == 1) && (magMode6 == 0) && (magMode7 == 0)) // 0b0010
    {
        return 2; // Y channel enabled
    }
    else if ((magMode4 == 1) && (magMode5 == 1) && (magMode6 == 0) && (magMode7 == 0)) // 0b0011
    {
        return 3; // X, Y channel enabled
    }
    else if ((magMode4 == 0) && (magMode5 == 0) && (magMode6 == 1) && (magMode7 == 0)) // 0b0100
    {
        return 4; // Z channel enabled
    }
    else if ((magMode4 == 1) && (magMode5 == 0) && (magMode6 == 1) && (magMode7 == 0)) // 0b0101
    {
        return 5; // Z, X channel enabled
    }
    else if ((magMode4 == 0) && (magMode5 == 1) && (magMode6 == 1) && (magMode7 == 0)) // 0b0110
    {
        return 6; // Y, Z channel enabled
    }
    else if ((magMode4 == 1) && (magMode5 == 1) && (magMode6 == 1) && (magMode7 == 0)) // 0b0111
    {
        return 7; // X, Y, Z channel enabled
    }
    else if ((magMode4 == 0) && (magMode5 == 0) && (magMode6 == 0) && (magMode7 == 1)) // 0b1000
    {
        return 8; // XYX channel enabled
    }
    else if ((magMode4 == 1) && (magMode5 == 0) && (magMode6 == 0) && (magMode7 == 1)) // 0b1001
    {
        return 9; // YXY channel enabled
    }
    else if ((magMode4 == 0) && (magMode5 == 1) && (magMode6 == 0) && (magMode7 == 1)) // 0b1010
    {
        return 10; // YZY channel enabled
    }
    else if ((magMode4 == 1) && (magMode5 == 1) && (magMode6 == 0) && (magMode7 == 1)) // 0b1011
    {
        return 11; // XZX channel enabled
    }
    else
    {
        return 0; // DEFAULT
    }
}

/// @brief Returns the time spent in low power mode between
///  conversions when OPERATING_MODE=11b.
///     0X0 = 1ms
///     0X1 = 5ms
///     0X2 = 10ms
///     0X3 = 15ms
///     0X4 = 20ms
///     0X5 = 30ms
///     0X6 = 50ms
///     0X7 = 100ms
///     0X8 = 500ms
///     0X9 = 1000ms
///     0XA = 2000ms
///     0XB = 5000ms
///     0XC = 20000ms
///     TMAG5273_REG_SENSOR_CONFIG_1 - bit 3-0
/// @return Code for the time spent in low power mode between conversions
uint8_t TMAG5273::getSleeptime()
{
    uint8_t sleepReg = 0;
    sleepReg = readRegister(TMAG5273_REG_SENSOR_CONFIG_1);

    uint8_t sleep0 = bitRead(sleepReg, 0);
    uint8_t sleep1 = bitRead(sleepReg, 1);
    uint8_t sleep2 = bitRead(sleepReg, 2);
    uint8_t sleep3 = bitRead(sleepReg, 3);

    if ((sleep0 == 0) && (sleep1 == 0) && (sleep2 == 0) && (sleep3 == 0)) // 0b0000
    {
        return 0; // 1ms
    }
    else if ((sleep0 == 1) && (sleep1 == 0) && (sleep2 == 0) && (sleep3 == 0)) // 0b0001
    {
        return 1; // 5ms
    }
    else if ((sleep0 == 0) && (sleep1 == 1) && (sleep2 == 0) && (sleep3 == 0)) // 0b0010
    {
        return 2; // 10ms
    }
    else if ((sleep0 == 1) && (sleep1 == 1) && (sleep2 == 0) && (sleep3 == 0)) // 0b0011
    {
        return 3; // 15ms
    }
    else if ((sleep0 == 0) && (sleep1 == 0) && (sleep2 == 1) && (sleep3 == 0)) // 0b0100
    {
        return 4; // 20ms
    }
    else if ((sleep0 == 1) && (sleep1 == 0) && (sleep2 == 1) && (sleep3 == 0)) // 0b0101
    {
        return 5; // 30ms
    }
    else if ((sleep0 == 0) && (sleep1 == 1) && (sleep2 == 1) && (sleep3 == 0)) // 0b0110
    {
        return 6; // 50ms
    }
    else if ((sleep0 == 1) && (sleep1 == 1) && (sleep2 == 1) && (sleep3 == 0)) // 0b0111
    {
        return 7; // 100ms
    }
    else if ((sleep0 == 0) && (sleep1 == 0) && (sleep2 == 0) && (sleep3 == 1)) // 0b1000
    {
        return 8; // 500ms
    }
    else if ((sleep0 == 1) && (sleep1 == 0) && (sleep2 == 0) && (sleep3 == 1)) // 0b1001
    {
        return 9; // 1000ms
    }
    else if ((sleep0 == 0) && (sleep1 == 1) && (sleep2 == 0) && (sleep3 == 1)) // 0b1010
    {
        return 10; // 2000ms
    }
    else if ((sleep0 == 1) && (sleep1 == 1) && (sleep2 == 0) && (sleep3 == 1)) // 0b1011
    {
        return 11; // 5000ms
    }
    else if ((sleep0 == 0) && (sleep1 == 0) && (sleep2 == 1) && (sleep3 == 1)) // 0b1011
    {
        return 12; // 20000ms
    }
    else
    {
        return 0; // DEFAULT
    }
}

/// @brief Returns the direction of threshold check. This bit is
///  ignored when THR_HYST > 001b.
///     0X0 = sets interrupt for field above the threshold
///     0X1 = sets interrupt for field below the threshold
///     TMAG5273_REG_SENSOR_CONFIG_2 - bit 5
/// @return Direction of threshold check - 0 or 1
uint8_t TMAG5273::getMagDir()
{
    uint8_t magDirectionReg = 0;
    magDirectionReg = readRegister(TMAG5273_REG_SENSOR_CONFIG_2);

    uint8_t magDirection5 = bitRead(magDirectionReg, 5);

    return magDirection5;
}

/// @brief Returns the axis for magnitude gain correction value
///  entered in MAG_GAIN_CONFIG register
///     0X0 = 1st channel is selected for gain adjustment
///     0X1 = 2nd channel is selected for gain adjustment
///     TMAG5273_REG_SENSOR_CONFIG_2 - bit 4
/// @return First (0) or Second (0) channel selected for gain adjustment
uint8_t TMAG5273::getMagnitudeChannelSelect()
{
    uint8_t magGainReg = 0;
    magGainReg = readRegister(TMAG5273_REG_SENSOR_CONFIG_2);

    uint8_t magGain4 = bitRead(magGainReg, 4);

    return magGain4;
}

/// @brief This function returns an 8-bit gain value determined by a
///  primary to adjust a Hall axis gain. The particular axis is selected
///  based off the settings of MAG_GAIN_CH and ANGLE_EN register bits.
///  The binary 8-bit input is interpreted as a fracitonal value between
///  0 and 1 based off the formula 'user entered value in decimal/256.'
///  Gain value of 0 is interpreted by the device as 1.
///     TMAG5273_REG_MAG_GAIN_CONFIG
/// @return 8-bit gain value
uint8_t TMAG5273::getMagneticGain()
{
    uint8_t magneticGainReg = 0;
    magneticGainReg = readRegister(TMAG5273_REG_MAG_GAIN_CONFIG);

    // Multiply by resolution
    float magneticGain = magneticGainReg * 256;

    return magneticGain;
}

/// @brief This function will return an 8-bit, 2's complement offset value
///  determined by a primary to adjust the first axis offset value. The
///  range of possible offset valid entrees can be +/-128. The offset value
///  is calculated by multiplying bit resolution with the entered value.
///     TMAG5273_REG_MAG_OFFSET_CONFIG_1
/// @return Magnetic offset value for the first axis
int8_t TMAG5273::getMagneticOffset1()
{
    int8_t magOffset1 = 0;
    magOffset1 = readRegister(TMAG5273_REG_MAG_OFFSET_CONFIG_1);

    uint8_t rangeVal = 0;
    uint8_t range = 0;
    if (rangeVal == 0)
    {
        range = 40;
    }
    else
    {
        range = 80;
    }

    // Divide by resoltuion (2^12 = 2048)
    float offsetVal = (magOffset1 * range) / 2048;

    return offsetVal;
}

/// @brief This function will return an 8-bit, 2's complement offset value
///  determined by a primary to adjust the first axis offset value. The
///  range of possible offset valid entrees can be +/-128. The offset value
///  is calculated by multiplying bit resolution with the entered value.
///     TMAG5273_REG_MAG_OFFSET_CONFIG_2
/// @return Magnetic offset value for the second axis
int8_t TMAG5273::getMagneticOffset2()
{
    int8_t magOffset2 = 0;
    magOffset2 = readRegister(TMAG5273_REG_MAG_OFFSET_CONFIG_2);

    // Choose the XY axis range
    uint8_t channelSelect = getAngleEn(); // 1, 2, or 3
    uint8_t rangeValXY = getXYAxisRange();
    uint8_t rangeXY = 0;
    if (rangeValXY == 0)
    {
        rangeXY = 40;
    }
    else if (rangeValXY == 1)
    {
        rangeXY = 80;
    }

    // Choose the Z axis range
    uint8_t rangeValZ = getZAxisRange();
    uint8_t rangeZ = 0;
    if (rangeValZ == 0)
    {
        rangeZ = 40;
    }
    else if (rangeValZ == 1)
    {
        rangeZ = 80;
    }

    // Range value for caluclation
    uint8_t range;

    // Select Channel
    if (channelSelect == 1) // Y
    {
        range = rangeXY; // 40 or 80
    }
    else if (channelSelect == 2) // Z
    {
        range = rangeZ; // 40 or 80
    }
    else if (channelSelect == 3) // Z
    {
        range = rangeZ; // 40 or 80
    }
    else
    {
        return -1; // Returns error
        // Error could also be the ranges being disabled
    }

    // Use resolution equation in datasheet
    float offsetVal = (magOffset2 * range) / 2048; // 2^11 = 2048

    return offsetVal;
}

/// @brief Returns angle calculation, magnetic gain, and offset
///  corrections between two selected magnetic channels.
///     0X0 = No angle calculation, magnitude gain, and offset
///           correction enabled
///     0X1 = X 1st, Y 2nd
///     0X2 = Y 1st, Z 2nd
///     0X3 = X 1st, Z 2nd
///     TMAG5273_REG_SENSOR_CONFIG_2 - bit 3-2
/// @return Angle calculation and associated channel order
uint8_t TMAG5273::getAngleEn()
{
    uint8_t angleReg = 0;
    angleReg = readRegister(TMAG5273_REG_SENSOR_CONFIG_2);

    uint8_t angleDir2 = bitRead(angleReg, 2);
    uint8_t angleDir3 = bitRead(angleReg, 3);

    if ((angleDir2 == 0) && (angleDir3 == 0)) // 0b00
    {
        // NO angle calculation
        return 0;
    }
    else if ((angleDir2 == 1) && (angleDir3 == 0)) // 0b01
    {
        // X 1st, Y 2nd
        return 1;
    }
    else if ((angleDir2 == 0) && (angleDir3 == 1)) // 0b10
    {
        // Y 1st, Z 2nd
        return 2;
    }
    else if ((angleDir2 == 1) && (angleDir3 == 1)) // 0b11
    {
        // X 1st, Z 2nd
        return 3;
    }

    return 0;
}

/// @brief Returns the X and Y axes magnetic range from the
///  two following options:
///     0X0 = ±40mT, DEFAULT
///     0X1 = ±80mT
///     TMAG5273_REG_SENSOR_CONFIG_2 - bit 1
/// @return X and Y axes magnetic range (0 or 1)
uint8_t TMAG5273::getXYAxisRange()
{
    uint8_t XYrangeReg = 0;
    XYrangeReg = readRegister(TMAG5273_REG_SENSOR_CONFIG_2);

    uint8_t axisRange = bitRead(XYrangeReg, 1);

    if (axisRange == 0)
    {
        return 0;
    }
    else if (axisRange == 1)
    {
        return 1;
    }

    return 0;
}

/// @brief Returns the Z axis magnetic range from the
///  two following options:
///     0X0 = ±40mT, DEFAULT
///     0X1 = ±80mT
///     TMAG5273_REG_SENSOR_CONFIG_2 - bit 0
/// @return Z axis magnetic range from ±40mT or ±80mT
uint8_t TMAG5273::getZAxisRange()
{
    uint8_t ZrangeReg = 0;
    ZrangeReg = readRegister(TMAG5273_REG_SENSOR_CONFIG_2);

    uint8_t ZaxisRange = bitRead(ZrangeReg, 0);

    if (ZaxisRange == 0)
    {
        return 0;
    }
    else if (ZaxisRange == 1)
    {
        return 1;
    }

    return 0;
}

/// @brief Returns an 8-bit, 2's complement X axis threshold code for
///  limit check. The range of possible threshold entrees can be +/-128.
///  The thershold value in mT is calculated as (40(1+X_Y_RANGE)/128)*X_THR_CONFIG.
///  Default 0h means no threshold comparison.
///     TMAG5273_REG_X_THR_CONFIG - bits 7-0
/// @return Returns the X threshold code for limit check
float TMAG5273::getXThreshold()
{
    int8_t xThresh = 0;
    xThresh = readRegister(TMAG5273_REG_X_THR_CONFIG);
    uint8_t range = getXYAxisRange();

    float thresh = (float)(40 * (1 + range)) / 128 * xThresh;

    return thresh; // mT
}

/// @brief Returns an 8-bit, 2's complement Y axis threshold code for
///  limit check. The range of possible threshold entrees can be +/-128.
///  The thershold value in mT is calculated as (40(1+X_Y_RANGE)/128)*Y_THR_CONFIG.
///  Default 0h means no threshold comparison.
///     TMAG5273_REG_Y_THR_CONFIG - bits 7-0
/// @return Returns the Y threshold code for limit check
float TMAG5273::getYThreshold()
{
    int8_t yThresh = 0;
    yThresh = readRegister(TMAG5273_REG_Y_THR_CONFIG);
    uint8_t range = getXYAxisRange();

    float thresh = (float)(40 * (1 + range)) / 128 * yThresh;

    return thresh; // mT
}

/// @brief Returns an 8-bit, 2's complement Z axis threshold code for
///  limit check. The range of possible threshold entrees can be +/-128.
///  The thershold value in mT is calculated as (40(1+Z_RANGE)/128)*Z_THR_CONFIG.
///  Default 0h means no threshold comparison.
///     TMAG5273_REG_Z_THR_CONFIG - bits 7-0
/// @return Returns the Z threshold code for limit check
float TMAG5273::getZThreshold()
{
    int8_t zThresh = 0;
    zThresh = readRegister(TMAG5273_REG_Z_THR_CONFIG);
    uint8_t range = getZAxisRange();

    float thresh = (float)(40 * (1 + range)) / 128 * zThresh;

    return thresh; // mT
}

/// @brief Returns the temperature threshold code entered by the user.
///  The valid temperature threshold values are -41C to 170C with the
///  threshold codes for -41C = 0x1A, and 170C = 0X34. Resolution is
///  8 degree C/LSB. Default 0x0 means no threshold comparison.
///     TMAG5273_REG_T_CONFIG - bits 7-1
/// @return Temperature threshold code entered by the user originally.
float TMAG5273::getTemperatureThreshold()
{
    int8_t tempThreshReg = 0;
    tempThreshReg = readRegister(TMAG5273_REG_T_CONFIG);

    // Value to hold
    int8_t tempThresh = 0;
    // Shifts the last bit off the value
    tempThresh = (tempThreshReg >> 1);

    return tempThresh; // degrees C
}

/// @brief Returns the enable bit that determines the data
///  acquisition of the temperature channel.
////    0x0 = Temp Channel Disabled
///     0x1 = Temp Channel Enabled
///     TMAG5273_REG_T_CONFIG - bit 0
/// @return Enable bit that determines if temp channel is enabled or disabled
uint8_t TMAG5273::getTemperatureEN()
{
    uint8_t tempENreg = 0;
    tempENreg = readRegister(TMAG5273_REG_T_CONFIG);

    uint8_t tempEN = bitRead(tempENreg, 0);

    return tempEN;
}

/// @brief Returns the enable interrupt response bit on conversion
///  complete.
///     0X0 = Interrupt is NOT asserted when the configured set of
///           conversions are complete
///     0X1 = Interrupt is asserted when the configured set of
///           conversions are complete
///     TMAG5273_REG_INT_CONFIG_1 - bit 7
/// @return Interrupt responce bit for conversion complete.
uint8_t TMAG5273::getInterruptResult()
{
    uint8_t intRsltReg = 0;
    intRsltReg = readRegister(TMAG5273_REG_INT_CONFIG_1);

    uint8_t intRslt = bitRead(intRsltReg, 7);

    return intRslt;
}

/// @brief Returns the bit that enables the interrupt response
///  on a predefined threshold cross.
///     0X0 = Interrupt is NOT asserted when a threshold is crossed
///     0X1 = Interrupt is asserted when a threshold is crossed
///     TMAG5273_REG_INT_CONFIG_1 - bit 6
/// @return Enable bit for if the interrupt is or is not asserted
uint8_t TMAG5273::getThresholdEn()
{
    uint8_t threshReg = 0;
    threshReg = readRegister(TMAG5273_REG_INT_CONFIG_1);

    uint8_t threshEnRslt = bitRead(threshReg, 6);

    return threshEnRslt;
}

/// @brief Returns the !INT interrupt if it is latched or pusled.
///     0X0 = !INT interrupt latched until clear by a primary
///           addressing the device
///     0X1 = !INT interrupt pulse for 10us
///     TMAG5273_REG_INT_CONFIG_1 - bit 5
/// @return Value if !INT interrupt is latched or pulsed
uint8_t TMAG5273::getIntPinState()
{
    uint8_t intStateReg = 0;
    intStateReg = readRegister(TMAG5273_REG_INT_CONFIG_1);

    uint8_t intStateRslt = bitRead(intStateReg, 5);

    return intStateRslt;
}

/// @brief Returns the configuration for the interrupt mode select
///     0X0 = No interrupt
///     0X1 = Interrupt through !INT
///     0X2 = Interrupt through !INT except when I2C bus is busy
///     0X3 = Interrupt through SCL
///     0X4 = Interrupt through SCL except when I2C bus is busy
///     TMAG5273_REG_INT_CONFIG_1 - bit 4-2
/// @return Configuration for the interrupt mode select
uint8_t TMAG5273::getInterruptMode()
{
    uint8_t intModeReg = 0;
    intModeReg = readRegister(TMAG5273_REG_INT_CONFIG_1);

    uint8_t intCon2 = bitRead(intModeReg, 2);
    uint8_t intCon3 = bitRead(intModeReg, 3);
    uint8_t intCon4 = bitRead(intModeReg, 4);

    if ((intCon2 == 0) && (intCon3 == 0) && (intCon4 == 0)) // 0b000
    {
        // no interrupt
        return 0;
    }
    else if ((intCon2 == 1) && (intCon3 == 0) && (intCon4 == 0)) // 0b001
    {
        // interrupt through !INT
        return 1;
    }
    else if ((intCon2 == 0) && (intCon3 == 1) && (intCon4 == 0)) // 0b010
    {
        // Interrupt through !INT except when I2C is busy
        return 2;
    }
    else if ((intCon2 == 1) && (intCon3 == 1) && (intCon4 == 0)) // 0b011
    {
        // Interrupt through SCL
        return 3;
    }
    else if ((intCon2 == 0) && (intCon3 == 0) && (intCon4 == 1)) // 0b100
    {
        // Interrupt through SCL except when I2C is busy
        return 4;
    }
    else
    {
        // default
        return 0;
    }
}

/// @brief Returns the Mask !INT pin when !INT is connected to GND
///     0X0 = !INT pin is enabled
///     0X1 = !INT pin is disabled (for wake-up and trigger functions)
///     TMAG5273_REG_INT_CONFIG_1 - bit 0
/// @return !INT pin is enabled or disabled
uint8_t TMAG5273::getMaskInt()
{
    uint8_t maskIntReg = 0;
    maskIntReg = readRegister(TMAG5273_REG_INT_CONFIG_1);

    uint8_t maskInt = bitRead(maskIntReg, 0);

    return maskInt;
}

/// @brief Returns the rolling count of conversion data sets
///      TMAG5273_REG_CONV_STATUS - bit 7-5
/// @return Rolling count of conversion data sets
uint8_t TMAG5273::getSetCount()
{
    uint8_t convReg = 0;
    convReg = readRegister(TMAG5273_REG_CONV_STATUS);

    // variable for count value to be held in
    uint8_t count = 0;
    // Shift off the last 5 bits of the register
    count = convReg >> 5;

    return count;
}

/// @brief Returns if the device is powered up, or experienced power-
///  on-reset. Bit is cleared when the host writes back '1'.
///     0X0 = No POR
///     0X1 = POR occured
///     TMAG5273_REG_CONV_STATUS - bit 4
/// @return Device is powered up or experienced POR.
uint8_t TMAG5273::getPOR()
{
    uint8_t convReg = 0;
    convReg = readRegister(TMAG5273_REG_CONV_STATUS);

    uint8_t PORBit = bitRead(convReg, 4);

    return PORBit;
}

/// @brief Returns if there was a detection of any internal
///  diagnostics fail which include VCC UV, internal memory CRC
///  error, !INT pin error and internal clock error. Ignore
///  this bit status if VCC < 2.3V.
///     0X0 = no diag fail
///     0X1 = diag fail detected
///     TMAG5273_REG_CONV_STATUS - bit 1
/// @return  Detection of any internal diagnostics failure
uint8_t TMAG5273::getDiagStatus()
{
    uint8_t convReg = 0;
    convReg = readRegister(TMAG5273_REG_CONV_STATUS);

    uint8_t diagBit = bitRead(convReg, 1);

    return diagBit;
}

/// @brief Returns if the conversion data buffer is ready
///  to be read.
///     0X0 = Conversion data not complete
///     0X1 = Converstion data complete
///     TMAG5273_REG_CONV_STATUS - bit 0
/// @return Conversion data buffer status
uint8_t TMAG5273::getResultStatus()
{
    uint8_t convReg = 0;
    convReg = readRegister(TMAG5273_REG_CONV_STATUS);

    uint8_t resultBit = bitRead(convReg, 0);

    return resultBit;
}

/// @brief Returns the I2C address of the device. There is a 7-bit
///  default factory address is loaded from OTP during first power
///  up. Change these bits to a new setting if a new I2C address is
///  required (at each power cycle these bits must be written again
///  to avoid going back to default factory address).
///     TMAG5273_REG_I2C_ADDRESS - bits 7-1
/// @return I2C address of the device
uint8_t TMAG5273::getI2CAddress()
{
    uint8_t addressReg = 0;
    uint8_t address = readRegister(TMAG5273_REG_I2C_ADDRESS);

    addressReg = address >> 1; // Shift off the last bit to return the first 7

    return addressReg; // returns the address of the I2C device currently set
}

/// @brief Returns the device version indicator. The reset value of the
///  DEVICE_ID depends on the orderable part number
///     0X0 = Reserved
///     0X1 = ±40-mT and ±80-mT range
///     0X2 = ±133-mT and ±266-mT range
///     0X3 = Reserved
///     DEVICE_ID
/// @return Version number code of the device
uint8_t TMAG5273::getDeviceID()
{
    uint8_t deviceReg = 0;
    deviceReg = readRegister(TMAG5273_REG_DEVICE_ID);

    uint8_t reg1 = bitRead(deviceReg, 0);
    uint8_t reg2 = bitRead(deviceReg, 1);

    if ((reg1 == 1) && (reg2 == 0))
    {
        return 0;
    }
    else if ((reg1 == 0) && (reg2 == 1))
    {
        return 1;
    }
    else
    {
        // returns an error
        return -1;
    }
}

/// @brief Returns the 8-Bit Manufacturer ID. There are two
///  registers, LSB and MSB.
///     MANUFACTURER_ID_LSB
///     MANUFACTURER_ID_MSB
/// @return 16-Bit Manufacturer ID
uint16_t TMAG5273::getManufacturerID()
{
    uint16_t deviceIDReg = 0;
    uint8_t databuffer[2];

    readRegisters(TMAG5273_REG_MANUFACTURER_ID_LSB, databuffer, 2);

    deviceIDReg = (databuffer[1] << 8) | (databuffer[0]);

    return deviceIDReg;
}

/// @brief This function iundicates the level that the device is
///  reading back from the !INT pin. The reset value of DEVICE_STATUS
/// depends on the status of the !INT pin at power-up.
/// @return Returns the following:
///     0X0 = !INT pin driven low
///     0X1 = !INT pin status high
uint8_t TMAG5273::getInterruptPinStatus()
{
    uint8_t deviceStatusReg = 0;
    deviceStatusReg = readRegister(TMAG5273_REG_DEVICE_STATUS);
    // Reads back the bit we want to investigate
    uint8_t intPinStatus = bitRead(deviceStatusReg, 4);

    return intPinStatus; // Returns a 0 or 1 if low or high, respectively
}

/// @brief This function returns the device status register as its
///  raw hex value. This value can be taken and compared to the main
///  register as seen in the datasheet.
///  The errors include an oscillator error, INT pin error detected,
///  OTP CRC errors, or undervoltage resistors.
///     TMAG5273_REG_DEVICE_STATUS
/// @return Device Status Regsiter as a raw value.
uint8_t TMAG5273::getDeviceStatus()
{
    // Check for undervoltage, OTP CRC, Int Pin, and Oscillator errors
    uint8_t deviceStatusReg = readRegister(TMAG5273_REG_DEVICE_STATUS);

    return deviceStatusReg;
}

/// @brief This function will return the error code, if there is any
///  at the time when the function is called.
///  If any of the Error pins are raised to 1 then there is an error.
///  For more information on the specific error, checkout the
///  getDeviceStatus() function and compare to the datasheet.
/// @return Error code (0 is success, negative is failure, positive is warning)
int8_t TMAG5273::getError()
{
    // Pull in the device status register to compare to the error codes
    uint8_t statusReg = getDeviceStatus();
    uint8_t undervoltageError = bitRead(statusReg, 0);
    uint8_t otpCrcError = bitRead(statusReg, 1);
    uint8_t intPinError = bitRead(statusReg, 2);
    uint8_t oscillatorError = bitRead(statusReg, 3);

    // If there is any error with the error codes, return -1. Otherwise, success and return 0
    if ((undervoltageError != 0) && (otpCrcError != 0) && (intPinError != 0) && (oscillatorError != 0))
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

/************************************************************************************************/
/************************         MAGNETIC/TEMPERATURE FUNCTIONS         ************************/
/************************************************************************************************/

/// @brief Reads back the T-Channel data conversion results,
///  combining the MSB and LSB registers.
///     T_MSB_RESULT and T_LSB_RESULT
/// @return T-Channel data conversion results
float TMAG5273::getTemp()
{
    // Variable to store full temperature value
    int16_t temp = 0;
    uint8_t databuffer[2];
    // Read in the MSB and LSB registers
    readRegisters(TMAG5273_REG_T_MSB_RESULT, databuffer, 2);
    // Combines the two in one register where the MSB is shifted to the correct location
    temp = (databuffer[0] << 8) | (databuffer[1]);
    // Formula for correct output value
    float tempOut = TMAG5273_TSENSE_T0 + (((float)temp - TMAG5273_TADC_T0) / (TMAG5273_TADC_RES));

    return tempOut;
}

/// @brief Readcs back the X-Channel data conversion results, the
/// MSB 8-Bit and LSB 8-Bits. This reads from the following registers:
///     X_MSB_RESULT and X_LSB_RESULT
/// @return X-Channel data conversion results
float TMAG5273::getXData()
{
    int8_t xLSB = readRegister(TMAG5273_REG_X_LSB_RESULT);
    int8_t xMSB = readRegister(TMAG5273_REG_X_MSB_RESULT);

    // Variable to store full X data
    int16_t xData = 0;
    // Combines the two in one register where the MSB is shifted to the correct location
    xData = xLSB + (xMSB << 8);

    // Reads to see if the range is set to 40mT or 80mT
    uint8_t rangeValXY = getXYAxisRange();
    uint8_t range = 0;
    if (rangeValXY == 0)
    {
        range = 40;
    }
    else if (rangeValXY == 1)
    {
        range = 80;
    }

    // 16-bit data format equation
    float div = 32768;
    float xOut = -(range * xData) / div;

    return xOut;
}

/// @brief Reads back the Y-Channel data conversion results, the
///  MSB 8-Bits and LSB 8-Bits. This reads from the following registers:
///     Y_MSB_RESULT and Y_LSB_RESULT
/// @return Y-Channel data conversion results
float TMAG5273::getYData()
{
    int8_t yLSB = 0;
    int8_t yMSB = 0;

    yLSB = readRegister(TMAG5273_REG_Y_LSB_RESULT);
    yMSB = readRegister(TMAG5273_REG_Y_MSB_RESULT);

    // Variable to store full Y data
    int16_t yData = 0;
    // Combines the two in one register where the MSB is shifted to the correct location
    yData = yLSB + (yMSB << 8);

    // Reads to see if the range is set to 40mT or 80mT
    uint8_t rangeValXY = getXYAxisRange();
    uint8_t range = 0;
    if (rangeValXY == 0)
    {
        range = 40;
    }
    else if (rangeValXY == 1)
    {
        range = 80;
    }

    // 16-bit data format equation
    float div = 32768;
    float yOut = (range * yData) / div;

    return yOut;
}

/// @brief Reads back the Z-Channel data conversion results, the
///  MSB 8-Bits and LSB 8-Bits. This reads from the following registers:
///     Z_MSB_RESULT and Z_LSB_RESULT
/// @return Z-Channel data conversion results.
float TMAG5273::getZData()
{
    int8_t zLSB = 0;
    int8_t zMSB = 0;

    zLSB = readRegister(TMAG5273_REG_Z_LSB_RESULT);
    zMSB = readRegister(TMAG5273_REG_Z_MSB_RESULT);

    // Variable to store full X data
    int16_t zData = 0;
    // Combines the two in one register where the MSB is shifted to the correct location
    zData = zLSB + (zMSB << 8); 

    // Reads to see if the range is set to 40mT or 80mT
    uint8_t rangeValZ = getZAxisRange();
    uint8_t range = 0;
    if (rangeValZ == 0)
    {
        range = 40;
    }
    else if (rangeValZ == 1)
    {
        range = 80;
    }

    // div = (2^16) / 2    (as per the datasheet equation 10)
    // 16-bit data format equation
    float div = 32768;
    float zOut = (range * zData) / div;

    return zOut;
}

/// @brief Returns the angle measurement result in degree. The data
///  displayed from 0 to 360 degree in 13 LSB bits after combining the
///  MSB and LSB bits. The 4 LSB bits allocated for fraction of an angle
///  in the format (xxx/16).
///     TMAG5273_REG_ANGLE_RESULT_MSB
///     TMAG5273_REG_ANGLE_RESULT_LSB
/// @return Angle measurement result in degrees (float value)
float TMAG5273::getAngleResult()
{
    uint8_t angleLSB = 0;
    uint8_t angleMSB = 0;

    angleLSB = readRegister(TMAG5273_REG_ANGLE_RESULT_LSB) & 0b11111111;
    angleMSB = readRegister(TMAG5273_REG_ANGLE_RESULT_MSB);

    // Variable to hold the full angle MSB and LSB registers
    int16_t angleReg = 0;
    // Variable to hold the last 4 bits as the fraction of an angle
    float decValue = 0;
    // Variable to hold the angle value without the fraction value
    int16_t angleVal = 0;
    // Variable to hold the full final value to return
    float finalVal = 0;

    // Combining the register value
    angleReg = angleLSB + (angleMSB << 8); 

    // Removing the uneeded bits for the fraction value
    decValue = float(angleLSB & 0b1111) / 16;

    // Shift off the decimal value (last 4 bits)
    angleVal = angleReg >> 4;

    // Add the two values together now
    finalVal = angleVal + decValue;

    return finalVal;
}

/// @brief Returns the resultant vector magnitude (during angle
///  measurement) result. This value should be constant during 360
///  degree measurements.
/// @return Vector magnitude during angle measurement
float TMAG5273::getMagnitudeResult()
{
    // Creates a variable for the magnitude register
    uint16_t magReg = 0;

    // Reads in the magnitude result register
    magReg = readRegister(TMAG5273_REG_MAGNITUDE_RESULT);

    return magReg;
}
