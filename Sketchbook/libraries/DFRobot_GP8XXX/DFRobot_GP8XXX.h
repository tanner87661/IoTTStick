/*!
  * @file DFRobot_GP8XXX.h
  * @brief GP8XXX series DAC driver library (GP8101, GP8101S, GP8211S, GP8413, GP8501, GP8503, GP8512, GP8403, GP8302 driver method is implemented)
  * @copyright   Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
  * @license     The MIT License (MIT)
  * @author      [fary](feng.yang@dfrobot.com)
  * @version  V1.0
  * @date  2023-05-10
  * @url https://github.com/DFRobot/DFRobot_GP8XXX
  */


#ifndef _DFRobot_GP8XXX_H_
#define _DFRobot_GP8XXX_H_

#include "Arduino.h"
#include "Wire.h"

class DFRobot_GP8XXX
{
  public:
    /**
     * @enum eOutPutRange_t
     * @brief Analog voltage output range select
     */
    typedef enum{
      eOutputRange2_5V  = 0,
      eOutputRange5V  = 1,
      eOutputRange10V = 2,
      eOutputRangeVCC   = 3
    }eOutPutRange_t;
    DFRobot_GP8XXX(){

    }

    /**
     * @fn begin
     * @brief initialization function
     */
    virtual int begin(void) =0;

    /**
     * @fn setDACOutVoltage
     * @brief Set different channel output DAC values
     * @param data requires the output voltage value
     * @param channel output channel 0: channel 0; 1: Channel 1; 2: All channels
     * @return NONE
     */
    virtual void setDACOutVoltage(uint16_t data, uint8_t channel) =0;   
  protected:
    uint16_t _voltage = 0;
};

class DFRobot_GP8XXX_IIC:public DFRobot_GP8XXX
{
  public:

    #define RESOLUTION_12_BIT 0x0FFF
    #define RESOLUTION_15_BIT 0x7FFF
    #define GP8XXX_CONFIG_CURRENT_REG                  uint8_t(0x02)
    #define DFGP8XXX_I2C_DEVICEADDR                    uint8_t(0x58)   //!< i2c address

    #define GP8XXX_STORE_TIMING_HEAD            0x02  ///< Store function timing start head
    #define GP8XXX_STORE_TIMING_ADDR            0x10  ///< The first address for entering store timing
    #define GP8XXX_STORE_TIMING_CMD1            0x03  ///< The command 1 to enter store timing
    #define GP8XXX_STORE_TIMING_CMD2            0x00  ///< The command 2 to enter store timing
    #define GP8XXX_STORE_TIMING_DELAY           10    ///< Store procedure interval delay time: 10ms, more than 7ms
    #define I2C_CYCLE_TOTAL                     5     ///< Total I2C communication cycle
    #define I2C_CYCLE_BEFORE                    1     ///< The first half cycle 2 of the total I2C communication cycle
    #define I2C_CYCLE_AFTER                     2     ///< The second half cycle 3 of the total I2C communication cycle

    /**
     * @brief DFRobot_GP8XXX constructor
     * @param resolution resolution
     * @param deviceAddr I2C address
     * @param pWire I2C object
     */
    DFRobot_GP8XXX_IIC(uint16_t resolution,uint8_t deviceAddr = DFGP8XXX_I2C_DEVICEADDR,TwoWire *pWire = &Wire)
    :_resolution(resolution),_deviceAddr(deviceAddr),_pWire(pWire){

    }

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
	

  protected:
	  /**
     * @fn sendData
     * @brief Set the IIC input value
     * @param data input value to be set (0-fff)
     * @param channel output channel
     * @n 0: Channel 0 (valid when PWM0 output is configured)
     * @n 1: Channel 1 (valid when PWM1 output is configured)
     * @n 2: All channels (valid when configuring dual channel output)
     * @return NONE
     */
	  void sendData(uint16_t data, uint8_t channel);

    /**
     * @fn writeRegister
     * @brief Write I2C register
     * @param reg I2C register address
     * @param pBuf Data storage space
     * @param size Read length
     * @return Write result
     * @retval 0 indicates success
     * @retval other values indicate setting failure
     */
    uint8_t writeRegister(uint8_t reg, void* pBuf, size_t size);

    /**
     * @fn startSignal
     * @brief I2C start signal
     */
    void startSignal(void);

    /**
     * @fn stopSignal
     * @brief I2C stop signal
     */	
    void stopSignal(void);

    /**
     * @fn recvAck
     * @brief Receive a reply
     * @param ack signal to be received by ack
     * @return Answer signal
     */	
    uint8_t recvAck(uint8_t ack);

    /**
     * @fn sendByte
     * @brief Software I2C sends data
     * @param data The data to be sent
     * @param ack acknowledgement signal
     * @param bits How many bits are sent
     * @param flag whether the flag receives a reply
     * @return Answer signal
     */
    uint8_t sendByte(uint8_t data, uint8_t ack = 0, uint8_t bits = 8, bool flag = true);
  
  protected:
    uint16_t _resolution=0;
    uint8_t _deviceAddr;
    TwoWire *_pWire;
    #if (defined ARDUINO_BBC_MICROBIT_V2)
      int _scl= 19;
      int _sda = 20;
    #else
      int _scl= SCL;
      int _sda = SDA;
    #endif

    
    
};

class DFRobot_GP8503: public DFRobot_GP8XXX_IIC
{
  public:
    DFRobot_GP8503(uint16_t resolution = RESOLUTION_12_BIT):DFRobot_GP8XXX_IIC(resolution){};
};

class DFRobot_GP8211S: public DFRobot_GP8XXX_IIC
{
  public:
    DFRobot_GP8211S(uint16_t resolution = RESOLUTION_15_BIT):DFRobot_GP8XXX_IIC(resolution){};
};

class DFRobot_GP8512: public DFRobot_GP8XXX_IIC
{
  public:
    DFRobot_GP8512(uint16_t resolution = RESOLUTION_15_BIT):DFRobot_GP8XXX_IIC(resolution){};
    void setDACOutVoltage(uint16_t voltage, uint8_t channel=0);
};

class DFRobot_GP8413: public DFRobot_GP8XXX_IIC
{
  public:
    DFRobot_GP8413(uint8_t deviceAddr = DFGP8XXX_I2C_DEVICEADDR,uint16_t resolution = RESOLUTION_15_BIT):DFRobot_GP8XXX_IIC(resolution,deviceAddr){};
};

class DFRobot_GP8302: public DFRobot_GP8XXX_IIC
{
  public:
    DFRobot_GP8302(uint8_t deviceAddr = DFGP8XXX_I2C_DEVICEADDR,uint16_t resolution = RESOLUTION_12_BIT):DFRobot_GP8XXX_IIC(resolution,deviceAddr){};
    void setDACOutElectricCurrent(uint16_t current){setDACOutVoltage(current);};
};

class DFRobot_GP8403: public DFRobot_GP8XXX_IIC
{
  public:
    DFRobot_GP8403(uint8_t deviceAddr = DFGP8XXX_I2C_DEVICEADDR,uint16_t resolution = RESOLUTION_12_BIT):DFRobot_GP8XXX_IIC(resolution,deviceAddr){};
};


/**************************************************************************
                       PWM to 2 analog voltage modules
 **************************************************************************/
class DFRobot_GP8XXX_PWM: public DFRobot_GP8XXX
{
  public:
    DFRobot_GP8XXX_PWM(int pin0 = -1,int pin1 = -1)
    :_pin0(pin0),_pin1(pin1){

    }
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
    
  private:	
	/**
     * @fn sendData
     * @brief Set PWM duty cycle
     * @param data PWM pulse width
     * @param channel output channel
     * @n 0: Channel 0 (valid when PWM0 output is configured)
     * @n 1: Channel 1 (valid when PWM1 output is configured)
     * @n 2: All channels (valid when configuring dual channel output)
     * @return NONE
     */
	void sendData(uint8_t data, uint8_t channel);
  
  protected:

	  int _pin0=-1;
	  int _pin1=-1;
  
};

class DFRobot_GP8501: public DFRobot_GP8XXX_PWM
{
  public:
    DFRobot_GP8501(int pin0 = -1,int pin1 = -1):DFRobot_GP8XXX_PWM(pin0,pin1){};
};

class DFRobot_GP8101: public DFRobot_GP8XXX_PWM
{
  public:
    DFRobot_GP8101(int pin0 = -1):DFRobot_GP8XXX_PWM(pin0){};
};
class DFRobot_GP8101S: public DFRobot_GP8XXX_PWM
{
  public:
    DFRobot_GP8101S(int pin0 = -1):DFRobot_GP8XXX_PWM(pin0){};
};
#endif