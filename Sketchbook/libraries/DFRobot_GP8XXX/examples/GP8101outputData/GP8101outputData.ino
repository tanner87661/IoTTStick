/*!
  * @file GP8101outputData.ino
  * @brief To set the output voltage, convert the PWM signal to an analog voltage signal ranging from 0-5V or 0-10V.
  * @copyright   Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
  * @license     The MIT License (MIT)
  * @author      [fary](feng.yang@dfrobot.com)
  * @version  V1.0
  * @date  2023-05-10
  * @url https://github.com/DFRobot/DFRobot_GP8XXX
  */

#include <DFRobot_GP8XXX.h>

//Output the IO port.
int pwmPin = 9;
DFRobot_GP8101 GP8101(pwmPin);
void setup() {

  GP8101.begin();
  
  /**
   * @brief Set the output DAC value.
   * @param data pwm Pulse width.
   * @n The optional parameter (0-255) corresponds to a voltage range of (0-5V) or (0-10V), depending on the voltage selection switch of the module.
   */
  GP8101.setDACOutVoltage(255);
}

void loop() {

}
