/******************************************************************************
SparkFun_TMAG5273_Arduino_Library.h
SparkFunTMAG5273 Library Source File
Madison Chodikov @ SparkFun Electronics
Original Creation Date: May 1st, 2023
https://github.com/sparkfun/SparkFun_TMAG5273_Arduino_Library

This header file implements all functions of the TMAG5273. Functions here range
from reading the X, Y,and Z Magnetic Fields, to reading and writing various
settings in the sensor.

Development environment specifics:
    IDE: Arduino 2.1.0
    Hardware Platform: Arduino Uno
    TMAG5273 Breakout Version: 1.0.1
Distributed as-is; no warranty is given.
******************************************************************************/

#ifndef __SparkFun_TMAG5273_Arduino_Library_H__
#define __SparkFun_TMAG5273_Arduino_Library_H__

#include "SparkFun_TMAG5273_Arduino_Library_Defs.h"
#include <Arduino.h>
#include <Wire.h>


class TMAG5273
{
  public:
    TMAG5273(); // Constructor

    int8_t begin(uint8_t sensorAddress = 0X22,
                 TwoWire &wirePort =
                     Wire); // Checks for ACK over I2C, and sets the device ID of the TMAG and chooses the wire port
    int8_t isConnected();   // Checks for I2C address verification along with the device address
    int8_t setupWakeUpAndSleep(); // Sets the device to be in Wake Up and Sleep Mode

    int8_t readWakeUpAndSleepData(float *xVal, float *yVal, float *zVal,
                                  float *temperature); // Returns the data for the X, Y, Z, and T values

    // Set Device Configuration Register Settings
    int8_t setCRCMode(uint8_t crcMode);     // Sets the I2C CRC byte to be sent
    int8_t setMagTemp(uint8_t magTempMode); // Selects the temp coefficient of the magnet
    int8_t setConvAvg(uint8_t avgMode);     // Sets additional sampling of the sensor data
    int8_t setReadMode(uint8_t readMode);   // Sets the I2C read mode of the device
    int8_t setIntThreshold(
        uint8_t threshold);               // Selects the threshold for the interrupt function (one direction, OR N/S)
    int8_t setLowPower(uint8_t lpLnMode); // Selects if device is operating in low power or low noise mode
    int8_t setGlitchFilter(uint8_t glitchMode);     // Selects I2C filter on or off
    int8_t setTriggerMode(uint8_t trigMode);        // Sets condition that initiates a single conversion command
    int8_t setOperatingMode(uint8_t opMode);        // Selects operating mode and updates values based on operating mode
    int8_t setMagneticChannel(uint8_t channelMode); // Sets the data acquisition from magnetic axis channels
    int8_t setSleeptime(uint8_t sleepTime);         // Sets the time spent in low power mode between conversions
    int8_t setMagDir(uint8_t threshDir);            // Sets the direction of threshold check
    int8_t setMagnitudeGain(uint8_t gainAdjust);    // Sets the axis for magnitude gain correction value
    int8_t setMagneticGain(float magneticGain);     // Sets the 8-bit gain value to adjust a Hall axis gain
    int8_t setMagneticOffset1(float offset1);       // Sets the offset value determined by a primary for the first axis
    int8_t setMagneticOffset2(float offset2);       // Sets the offset value determined by a primary for the second axis
    int8_t setAngleEn(uint8_t angleEnable);         // Sets the angle caluclation, mag gain, and offset corections
    int8_t setXYAxisRange(uint8_t xyAxisRange);     // Sets the X and Y axes magnetic range from ±40mT or ±80mT
    int8_t setZAxisRange(uint8_t zAxisRange);       // Sets the Z axis magnetic range from ±40mT or ±80mT
    int8_t setXThreshold(float xThreshold);         // Sets the X axis threshold code for limit check
    int8_t setYThreshold(float yThreshold);         // Sets the Y axis threshold code for limit check
    int8_t setZThreshold(float zThresh);            // Sets the Z axis threshold code for limit check
    int8_t setTemperatureThreshold(int8_t tempThresh); // Sets the Temperature threshold code for limit check
    int8_t setTemperatureEn(
        bool temperatureEnable); // Sets the enable bit that determines the data acquisition of the temp channel
    int8_t setInterruptResult(bool interruptEnable); // Sets the enable interrupt response bit on conversion complete
    int8_t setThresholdEn(
        bool enableInterruptResponse); // Configures bit to enable interrupt response on predefined thershold cross
    int8_t setIntPinState(bool interruptState);         // Emables the interrupt if latched or pulsed
    int8_t setInterruptMode(uint8_t configurationMode); // Configures the interrupt mode select
    int8_t setMaskInterrupt(bool interruptPinEnable);   // Configres the Mask !INT pin when !INT connected to GND
    int8_t setI2CAddress(uint8_t address);              // Change these bits to a new I2C address if required
    int8_t setI2CAddressEN(
        bool addressEnable); // Enables/disables bit to allow the user to change the I2C address of the devices
    int8_t setOscillatorError(bool oscError); // Clear the Oscillator Error pin accordingly

    // Get Device Configuration Register Settings
    uint8_t getCRCMode();                // Returns the I2C CRC byte to be sent
    uint8_t getMagTemp();                // Returns the temperature coefficient of the magnet
    uint8_t getConvAvg();                // Returns the average sampling rate of the sensor data
    uint8_t getReadMode();               // Returns the I2C read mode
    uint8_t getIntThreshold();           // Returns the threshold for the interrupt function
    uint8_t getLowPower();               // Returns if the device is operating in low power or noise mode
    uint8_t getGlitchFiler();            // Returns I2C glitch filter on or off
    uint8_t getTriggerMode();            // Returns if trigger is set to I2C command or INT pin
    uint8_t getOperatingMode();          // Returns the operating mode from 1 of the 4 modes.
    uint8_t getMagneticChannel();        // Returns data acquisiton from the list of mag axis channels
    uint8_t getSleeptime();              // Returns the time spent in low power mode
    uint8_t getMagDir();                 // Returns the direction of threshold check
    uint8_t getMagnitudeChannelSelect(); // Returns the axis for magnitude gain correct XYAxisRange section value
    uint8_t getMagneticGain();           // Returns the value determined to adjust a Hall axis gain
    int8_t getMagneticOffset1();         // Returns the offset value determined by a primary for the first axis
    int8_t getMagneticOffset2();         // Returns the offset value determined by a primary for the second axis
    uint8_t getAngleEn();                // Returns the angle calculation and associated channel order
    uint8_t getXYAxisRange();            // Returns the X and Y axes magnetic range
    uint8_t getZAxisRange();             // Returns the Z axis magnetic range
    float getXThreshold();               // Returns the X axis threshold code for limit check
    float getYThreshold();               // Returns the Y axis threshold code for limit check
    float getZThreshold();               // Returns the Z axis threshold code for limit check
    float getTemperatureThreshold();     // Returns the temperature threshold code entered by the user
    uint8_t getTemperatureEN();          // Returns the enable bit to determine if temp channel is on or off
    uint8_t getInterruptResult();        // Returns the enable interrupt response bit on conversion complete.
    uint8_t getThresholdEn();            // Returns the bit that enables the interrupt
    uint8_t getIntPinState();            // Returns the !INT interrupt if latched or pulsed
    uint8_t getInterruptMode();          // Returns the configuration for the interrupt mode select
    uint8_t getMaskInt();                // Returns the Mask !INT pin when !INT is connected to GND
    uint8_t getSetCount();               // Returns the rolling count of conversion data sets
    uint8_t getPOR();                    // Returns if the device is powered up or expereinced POR
    uint8_t getDiagStatus();             // Returns if there was a detection of any internal diagnostics fail
    uint8_t getResultStatus();           // Returns the conversion data buffer status (Data complete or not)
    uint8_t getI2CAddress();             // Returns the I2C address of the device
    uint8_t getDeviceID();               // Returns the device version indicator
    uint16_t getManufacturerID();        // Returns the manufacturer ID

    float getTemp();            // Uses T_MSB_RESULT and T_LSB_RESULT Registers
    float getXData();           // Uses X_MSB_RESULT and X_LSB_RESULT Registers
    float getYData();           // Uses Y_MSB_RESULT and Y_LSB_RESULT Registers
    float getZData();           // Uses Z_MSB_RESULT and Z_LSB_RESULT Registers
    float getAngleResult();     // Uses ANGLE_RESULT_LSB and ANGLE_RESULT_MSB Registers
    float getMagnitudeResult(); // Uses the MAGNITUDE_RESULT Register

    uint8_t getInterruptPinStatus(); // Returns the interrupt pin status

    uint8_t getDeviceStatus(); // Returns the error detected
    int8_t getError();         // Returns an error code (0 is success, negative is failure, positive is warning)

  private:
    // I2C Communication interface settings
    TwoWire *_i2cPort = NULL;
    uint8_t _deviceAddress;

    int8_t writeRegisters(uint8_t regAddress, uint8_t *dataBuffer, uint8_t numBytes);
    int8_t readRegisters(uint8_t regAddress, uint8_t *dataBuffer, uint8_t numBytes);
    uint8_t readRegister(uint8_t regAddress);
    uint8_t writeRegister(uint8_t regAddress, uint8_t data);
    bool ping(uint8_t i2c_address); // Checks for device presence
};

#endif