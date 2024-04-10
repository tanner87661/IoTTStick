/*!
  * @file GP8503outputData.ino
  * @brief Set the output channels and voltage values to convert the I2C signal into two channels of analog voltage signals ranging from 0-2.5V or 0-VCC (2.7V-5.5V).
  * @copyright   Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
  * @license     The MIT License (MIT)
  * @author      [fary](feng.yang@dfrobot.com)
  * @version  V1.0
  * @date  2023-05-10
  * @url https://github.com/DFRobot/DFRobot_GP8XXX
  */

#include <DFRobot_GP8XXX.h>

DFRobot_GP8503 GP8503;

void setup() {
  
  Serial.begin(9600);

  while(GP8503.begin()!=0){
    Serial.println("Communication with the device failed. Please check if the connection is correct or if the device address is set correctly.");
    delay(1000);
  }

  /**
   * @brief Set different channel outputs for the DAC values
   * @param data Data values corresponding to voltage levels
   * @n With a 12-bit precision DAC module, the data values ranging from 0 to 4095 correspond to voltage ranges of 0-2.5V or 0-VCC, respectively. The specific voltage range depends on the module's voltage selection switch.
   * @param channel Output channel
   * @n 0: Channel 0
   * @n 1: Channel 1
   * @n 2: All channels
   */
  GP8503.setDACOutVoltage(4095,2);

  delay(1000);
  
  //Save the set voltage in the chip's internal memory for power loss recovery.
  //GP8503.store();
}

void loop() {

}
