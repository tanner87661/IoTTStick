# DFRobot_GP8XXX

* [中文版](./README_CN.md)

GP8XXX series driver libraries on the Arduino IDE (currently compatible with GP8101, GP8211S, GP8413, GP8501, GP8503, GP8512, GP8403, GP8302)

## Product Link([www.dfrobot.com](www.dfrobot.com))

SUK：

1. DFR1034 --- GP8503
2. DFR1035 --- GP8512
3. DFR1036 --- GP8101S
4. DFR1037 --- GP8501
5. DFR1071 --- GP8211S
6. DFR1073 --- GP8413
7. DFR0971 --- GP8403
8. DFR0972 --- GP8302

## Table of Contents

  - [Summary](#summary)
  - [Installation](#installation)
  - [Methods](#methods)
  - [Compatibility](#compatibility)
  - [History](#history)
  - [Credits](#credits)

## Summary
    This library has already provided sample demos for each of the 8 currently compatible products, and users can use them according to their names
## Installation

There two methods: 
1. To use this library, first download the library file, paste it into the \Arduino\libraries directory, then open the examples folder and run the demo in the folder.
2. Search the DFRobot_GP8XXX library from the Arduino Software Library Manager and download it.

## Methods

```C++

/**************************************************************************
                                  PWM 系列
 **************************************************************************/

	  /**
     * @fn begin
     * @brief Initialize the function
     * @return 0
     */
    int begin();

    /**
     * @fn setDACOutVoltage
     * @brief Set different channel output DAC values
     * @param data PWM pulse width
     * @param channel output channel
     * @n 0: Channel 0 (valid when PWM0 output is configured)
     * @n 1: Channel 1 (valid when PWM1 output is configured)
     * @n 2: All channels (valid when configuring dual channel output)
     * @return NONE
     */
    void setDACOutVoltage(uint16_t data, uint8_t channel=0);

/**************************************************************************
                                  I2C 系列
 **************************************************************************/
    /**
     * @fn begin
     * @brief initialization function
     * @return returns 0 for success, and other values for failure 
     */
    int begin(void);

    /**
     * @fn setDACOutRange
     * @brief Set the DAC output range
     * @param range DAC output range
     * @n     eOutputRange0_5V(0-5V)
     * @n     eOutputRange0_10V(0-10V)
     * @return NONE
     */
    void setDACOutRange(eOutPutRange_t range);

    /**
     * @fn setDACOutVoltage
     * @brief Set different channel output DAC values
     * @param data value corresponding to the voltage value
     * @param channel output channel
     * @n 0: Channel 0 (valid when PWM0 output is configured)
     * @n 1: Channel 1 (valid when PWM1 output is configured)
     * @n 2: All channels (valid when configuring dual channel output)
     * @return NONE
     */
    void setDACOutVoltage(uint16_t data, uint8_t channel=0);
	
    /**
     * @fn store
     * @brief Save the set voltage inside the chip
     * @return NONE
     */
    void store(void);

```

## Compatibility

MCU                |  Work Well    | Work Wrong   | Untested    | Remarks
------------------ | :----------: | :----------: | :---------: | -----
Arduino Uno        |       √       |              |             | 
Mega2560           |      √       |              |             | 
Leonardo           |      √       |              |             | 
ESP32              |      √       |              |             | 
ESP8266            |      √       |              |             | 
micro:bit          |      √       |              |             | 
FireBeetle M0      |      √       |              |             | 

## History

- 2023/05/10 - Version 1.0.0 released.

## Credits

- Written by fary( feng.yang@dfrobot.com), 2023. (Welcome to our [website](https://www.dfrobot.com/))