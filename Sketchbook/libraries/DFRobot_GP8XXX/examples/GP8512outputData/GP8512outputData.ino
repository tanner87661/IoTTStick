/*!
  * @file GP8512outputData.ino
  * @brief Set the output voltage value to convert the I2C signal into a single-channel analog voltage signal ranging from 0-2.5V or 0-VCC (2.7V-5.5V).
  * @copyright   Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
  * @license     The MIT License (MIT)
  * @author      [fary](feng.yang@dfrobot.com)
  * @version  V1.0
  * @date  2023-05-10
  * @url https://github.com/DFRobot/DFRobot_GP8XXX
  */

#include <DFRobot_GP8XXX.h>

DFRobot_GP8512 GP8512;

void setup() {
  
  
  Serial.begin(9600);

  while(GP8512.begin()!=0){
    Serial.println("Communication with the device failed. Please check if the connection is correct or if the device address is set correctly.");
    delay(1000);
  }

  /**
   * @brief Set the output DAC value
   * @param data Data value corresponding to voltage level
   * @n With a 15-bit precision DAC module, the data values ranging from 0 to 32767 correspond to voltage ranges of 0-2.5V or 0-VCC, respectively. The specific voltage range depends on the module's voltage selection switch.
   */
  GP8512.setDACOutVoltage(2000);

  delay(1000);
  // Save the set voltage in the chip's internal memory for power loss recovery.
  // GP8512.store();
}

void loop() {
 
}
