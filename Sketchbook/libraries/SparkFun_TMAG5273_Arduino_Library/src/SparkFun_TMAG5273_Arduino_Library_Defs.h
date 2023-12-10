/******************************************************************************
SparkFun_TMAG5273_Arduino_Library_Defs.h
SparkFunTMAG5273 Library Definitions Header File
Madison Chodikov @ SparkFun Electronics
Original Creation Date: May 1st, 2023
https://github.com/sparkfun/SparkFun_TMAG5273_Arduino_Library

This file defines all of the constants used in the device library,
along with defining the registers used.

Development environment specifics:
    IDE: Arduino 2.1.0
    Hardware Platform: Arduino Uno
    TMAG5273 Breakout Version: 1.0.1
Distributed as-is; no warranty is given.
******************************************************************************/

/*
Features as per datasheet
    - ±40mT on the X, Y linear magnetic range
        - 820 LSB/mT Sensitivity
    - ±80mT on the Z linear magnetic range
        - 410 LSB/mT Sensitivity
    - ±300 µT Offset
        - ±3 µT Offset Drift
*/

#ifndef __SparkFun_TMAG5273_Arduino_Library_Defs_H__
#define __SparkFun_TMAG5273_Arduino_Library_Defs_H__


/********************************** Constant Variable Definitions **********************************/
#define TMAG5273_DEVICE_ID_VALUE 0x5449   // Value found in the device ID register
#define TMAG5273_I2C_ADDRESS_INITIAL 0X22 // Initial I2C address value - can be changed using functions as seen below
#define TMAG5273_TSENSE_T0 25             // Reference temperature for TADC_T0
#define TMAG5273_TADC_T0 17508            // Temp result in decimal value (from 16-buit format)
#define TMAG5273_TADC_RES 60.1            // Temperature sensing resolution (in 16-bit format)

#define TMAG5273_CRC_DISABLE 0X0 // Disables I2C CRC byte to be sent
#define TMAG5273_CRC_ENABLE 0X1  // Enable I2C CRC byte to be sent

#define TMAG5273_X1_CONVERSION 0X0  // 1X Average
#define TMAG5273_X2_CONVERSION 0X1  // 2X Average
#define TMAG5273_X4_CONVERSION 0X2  // 4X Average
#define TMAG5273_X8_CONVERSION 0X3  // 8X Average
#define TMAG5273_X16_CONVERSION 0X4 // 16X Average
#define TMAG5273_X32_CONVERSION 0X5 // 32X Average

#define TMAG5273_I2C_MODE_3BYTE 0X0       // Standard I2C 3-byte read command
#define TMAG5273_I2C_MODE_1BYTE_16BIT 0X1 // 1-byte I2C read command for 16bit sensor data and conversion status
#define TMAG5273_I2C_MODE_1BYTE_8BIT 0X2  // 1-byte I2C read command for 8 bit sensor MSB data and conversion status

#define TMAG5273_LOW_ACTIVE_CURRENT_MODE 0X0 // Low active current mode
#define TMAG5273_LOW_NOISE_MODE 0X1          // Low noise mode

#define TMAG5273_GLITCH_ON 0X0  // Glitch filter on
#define TMAG5273_GLITCH_OFF 0X1 // Glitch filter off

#define TMAG5273_STANDY_BY_MODE 0X0          // Stand-by mode
#define TMAG5273_SLEEP_MODE 0X1              // Sleep mode
#define TMAG5273_CONTINUOUS_MEASURE_MODE 0X2 // Continous measure mode
#define TMAG5273_WAKE_UP_AND_SLEEP_MODE 0X3  // Wake-up and sleep mode

#define TMAG5273_CHANNELS_OFF 0X0 // Turn all the magnetic channels off
#define TMAG5273_X_ENABLE 0X1     // X Channel enabled
#define TMAG5273_Y_ENABLE 0X2     // Y Channel enabled
#define TMAG5273_X_Y_ENABLE 0X3   // X, Y Channel enabled
#define TMAG5273_Z_ENABLE 0X4     // Z Channel enabled
#define TMAG5273_Z_X_ENABLE 0X5   // Z, X Channel enabled
#define TMAG5273_Y_Z_ENABLE 0X6   // Y, Z Channel enabled
#define TMAG5273_X_Y_Z_ENABLE 0X7 // X, Y, Z Channel enabled
#define TMAG5273_XYX_ENABLE 0X8   // XYX Channel enabled
#define TMAG5273_YXY_ENABLE 0X9   // YXY Channel enabled
#define TMAG5273_YZY_ENABLE 0XA   // YZY Channel enabled
#define TMAG5273_XZX_ENABLE 0XB   // XZX Channel enabled

#define TMAG5273_SLEEP_1MS 0X0     // 1ms
#define TMAG5273_SLEEP_5MS 0X1     // 5ms
#define TMAG5273_SLEEP_10MS 0X2    // 10 ms
#define TMAG5273_SLEEP_15MS 0X3    // 15ms
#define TMAG5273_SLEEP_20MS 0X4    // 20ms
#define TMAG5273_SLEEP_30MS 0X5    // 30ms
#define TMAG5273_SLEEP_50MS 0X6    // 50ms
#define TMAG5273_SLEEP_100MS 0X7   // 100ms
#define TMAG5273_SLEEP_500MS 0X8   // 500ms
#define TMAG5273_SLEEP_1000MS 0X9  // 1000ms
#define TMAG5273_SLEEP_2000MS 0XA  // 2000ms
#define TMAG5273_SLEEP_5000MS 0XB  // 5000ms
#define TMAG5273_SLEEP_20000MS 0XC // 20000ms

#define TMAG5273_THRESHOLD_1 0X0 // 1 Threshold crossing
#define TMAG5273_THRESHOLD_4 0X1 // 4 Threshold crossing

#define TMAG5273_THRESHOLD_INT_ABOVE 0X0 // Sets interrupt for field above the threshold
#define TMAG5273_THRESHOLD_INT_BELOW 0X1 // Sets interrupt for field below the threshold

#define TMAG5273_GAIN_ADJUST_CHANNEL_1 0X0 // 1st channel is selected for gain adjustment
#define TMAG5273_GAIN_ADJUST_CHANNEL_2 0X1 // 2nd channel is selected for gain adjustment

#define TMAG5273_NO_ANGLE_CALCULATION 0X0 // No angle calculation, magnetic gain, and offset correction enabled
#define TMAG5273_XY_ANGLE_CALCULATION 0X1 // X 1st, Y 2nd
#define TMAG5273_YZ_ANGLE_CALCULATION 0X2 // Y 1st, Z 2nd
#define TMAG5273_XZ_ANGLE_CALCULATION 0X3 // X 1st, Z 2nd

#define TMAG5273_RANGE_40MT 0X0 // +/-40mT, DEFAULT
#define TMAG5273_RANGE_80MT 0X1 // +/-80mT, DEFAULT

#define TMAG5273_TEMPERATURE_DISABLE 0X0 // Temperature channel disabled
#define TMAG5273_TEMPERATURE_ENABLE 0X1  // Temperature channel enabled

#define TMAG5273_INTERRUPT_NOT_ASSERTED 0X0 // Interrupt is not asserved when set
#define TMAG5273_INTERRUPT_ASSERTED 0X1     // Interrupt is asserted

#define TMAG5273_NO_INTERRUPT 0X0              // No interrupt
#define TMAG5273_INTERRUPT_THROUGH_INT 0X1     // Interrupt thru INT
#define TMAG5273_INTERRUPT_THROUGH_INT_I2C 0X2 // Interrupt thru INT except when I2C bus is busy
#define TMAG5273_INTERRUPT_THROUGH_SCL 0X3     // Interrupt thru SCL
#define TMAG5273_INTERRUPT_THROUGH_SCL_I2C 0X4 // Interrupt thru SCL except when I2C bus is busy

#define TMAG5273_INTERRUPT_ENABLED 0X0  // Interrupt pin is enabled
#define TMAG5273_INTERRUPT_DISABLED 0X1 // Interrupt pin is disabled

#define TMAG5273_INT_PIN_ENABLE 0X0  // INT pin is enabled
#define TMAG5273_INT_PIN_DISABLE 0X1 // INT pin is disabled (for wake-up and trigger functions)

#define TMAG5273_I2C_ADDRESS_CHANGE_DISABLE 0X0 // Disable update of I2C address
#define TMAG5273_I2C_ADDRESS_CHANGE_ENABLE 0X1  // Enable update of I2C address

#define TMAG5273_NO_POR 0X0  // Conversion data not complete
#define TMAG5273_YES_POR 0X1 // Conversion data complete

#define TMAG5273_OSCILLATOR_ERROR_UNDETECTED 0X0 // No Oscillator error detected
#define TMAG5273_OSCILLATOR_ERROR_DETECTED 0X1   // Oscillator error detected

#define TMAG5273_INT_ERROR_UNDETECTED 0X0 // No INT error detected
#define TMAG5273_INT_ERROR_DETECTED 0X1   // INT error detected


/********************************** Register Definitions **********************************/
enum TMAG5273_Register
{
    TMAG5273_REG_DEVICE_CONFIG_1 = 0X00,
    TMAG5273_REG_DEVICE_CONFIG_2 = 0X01,
    TMAG5273_REG_SENSOR_CONFIG_1 = 0X02,
    TMAG5273_REG_SENSOR_CONFIG_2 = 0X03,
    TMAG5273_REG_X_THR_CONFIG = 0X04,
    TMAG5273_REG_Y_THR_CONFIG = 0X05,
    TMAG5273_REG_Z_THR_CONFIG = 0X06,
    TMAG5273_REG_T_CONFIG = 0X07,
    TMAG5273_REG_INT_CONFIG_1 = 0X08,
    TMAG5273_REG_MAG_GAIN_CONFIG = 0X09,
    TMAG5273_REG_MAG_OFFSET_CONFIG_1 = 0X0A,
    TMAG5273_REG_MAG_OFFSET_CONFIG_2 = 0X0B,
    TMAG5273_REG_I2C_ADDRESS = 0X0C,
    TMAG5273_REG_DEVICE_ID = 0X0D,
    TMAG5273_REG_MANUFACTURER_ID_LSB = 0X0E,
    TMAG5273_REG_MANUFACTURER_ID_MSB = 0X0F,
    TMAG5273_REG_T_MSB_RESULT = 0X10,
    TMAG5273_REG_T_LSB_RESULT = 0X11,
    TMAG5273_REG_X_MSB_RESULT = 0X12,
    TMAG5273_REG_X_LSB_RESULT = 0X13,
    TMAG5273_REG_Y_MSB_RESULT = 0X14,
    TMAG5273_REG_Y_LSB_RESULT = 0X15,
    TMAG5273_REG_Z_MSB_RESULT = 0X16,
    TMAG5273_REG_Z_LSB_RESULT = 0X17,
    TMAG5273_REG_CONV_STATUS = 0X18,
    TMAG5273_REG_ANGLE_RESULT_MSB = 0X19,
    TMAG5273_REG_ANGLE_RESULT_LSB = 0X1A,
    TMAG5273_REG_MAGNITUDE_RESULT = 0X1B,
    TMAG5273_REG_DEVICE_STATUS = 0X1C
};

#endif
