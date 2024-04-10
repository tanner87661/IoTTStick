/*!
  * @file GP8501outputData.ino
  * @brief Set the output channels and voltage values to convert the PWM signal into two channels of analog voltage signals ranging from 0-2.5V or 0-VCC (2.7V-5.5V), respectively.
  * @copyright   Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
  * @license     The MIT License (MIT)
  * @author      [fary](feng.yang@dfrobot.com)
  * @version  V1.0
  * @date  2023-05-10
  * @url https://github.com/DFRobot/DFRobot_GP8XXX
  */

#include <DFRobot_GP8XXX.h>


//#define SINGLE_CHANNEL
#ifdef SINGLE_CHANNEL 
//Single-channel output, IO port configuration.
int pwmPin0 = 9;
DFRobot_GP8501 GP8501(pwmPin0);
#else
//Dual-channel output, IO port configuration.
int pwmPin0 = 9;
int pwmPin1 = 10;
DFRobot_GP8501 GP8501(pwmPin0,pwmPin1);
#endif

void setup() {

  GP8501.begin();

  /**
   * @brief Set different channel outputs for the DAC values
   * @param data PWM pulse width
   * @n Optional parameter (0-255) corresponding to (0-2.5V) or (0-VCC), the specific voltage range depends on the module's voltage selection switch
   * @param channel Output channel
   * @n 0: Channel 0 (valid when configuring PWM0 output)
   * @n 1: Channel 1 (valid when configuring PWM1 output)
   * @n 2: All channels (valid when configuring dual-channel output)
   */ 
  GP8501.setDACOutVoltage(255,2);

}

void loop() {

}
