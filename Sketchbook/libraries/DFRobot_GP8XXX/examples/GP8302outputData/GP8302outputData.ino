/*!
  * @file GP8302outputData.ino
  * @brief  Set the output current value and convert the I2C signal into a single-channel analog current output ranging from 2-25mA.
  * @copyright   Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
  * @license     The MIT License (MIT)
  * @author      [fary](feng.yang@dfrobot.com)
  * @version  V1.0
  * @date  2023-05-24
  * @url https://github.com/DFRobot/DFRobot_GP8XXX
  */

#include <DFRobot_GP8XXX.h>

DFRobot_GP8302 GP8302;

void setup() {
  
  Serial.begin(9600);

  while(GP8302.begin()!=0){
    Serial.println("Communication with the device failed. Please check if the connection is correct or if the device address is set correctly.");
    delay(1000);
  }

  /**
   * @brief Set the channel output DAC value.
   * @param data Data values corresponding to voltage levels.
   * @n With this 12-bit precision DAC module, the data values ranging from 0 to 4095 correspond to current ranges of 0-25mA.
   */   
  GP8302.setDACOutElectricCurrent(2048);

  delay(1000);
  
  //Save the set voltage in the chip's internal memory for power loss recovery.
  //GP8302.store();
}

void loop() {

}
