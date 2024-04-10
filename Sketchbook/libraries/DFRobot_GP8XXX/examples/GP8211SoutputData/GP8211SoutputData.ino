/*!
  * @file GP8211SoutputData.ino
  * @brief To convert the I2C signal into a single-channel analog voltage signal of (0-5V) or (0-10V), you need to set the output channel and voltage value.
  * @copyright   Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
  * @license     The MIT License (MIT)
  * @author      [fary](feng.yang@dfrobot.com)
  * @version  V1.0
  * @date  2023-05-10
  * @url https://github.com/DFRobot/DFRobot_GP8XXX
  */

#include <DFRobot_GP8XXX.h>

DFRobot_GP8211S GP8211S;

void setup() {

  Serial.begin(9600);

  while(GP8211S.begin()!=0){
    Serial.println("与设备通信失败，请检查连接是否正常或者设备地址是否设置正确");
    delay(1000);
  }
 
  /**
   * @brief Set the DAC output range.
   * @param range DAC output range.
   * @n     eOutputRange5V(0-5V)
   * @n     eOutputRange10V(0-10V)
   */	
  GP8211S.setDACOutRange(GP8211S.eOutputRange5V);
 
  /**
   * @brief Set the channel output DAC value.
   * @param data Data values corresponding to voltage levels.
   * @n In this module, with a 15-bit precision DAC, the data values ranging from 0 to 32767 correspond to voltage ranges of 0-5V or 0-10V, respectively.
   */   
  GP8211S.setDACOutVoltage(32767);

  delay(1000);

  //Save the set voltage in the chip's internal memory for power loss recovery.
  //GP8211S.store();
}

void loop() {

}
