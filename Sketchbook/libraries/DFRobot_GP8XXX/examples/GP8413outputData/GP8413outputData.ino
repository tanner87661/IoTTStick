/*!
  * @file GP8413outputData.ino
  * @brief Set the output channels, voltage values, and range to convert the I2C signal into two channels of analog voltage signals ranging from 0-5V or 0-10V.
  * @copyright   Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
  * @license     The MIT License (MIT)
  * @author      [fary](feng.yang@dfrobot.com)
  * @version  V1.0
  * @date  2023-05-10
  * @url https://github.com/DFRobot/DFRobot_GP8XXX
  */

#include <DFRobot_GP8XXX.h>
/**************************
----------------------------
| A2 |  A1 | A0 | i2c_addr |
----------------------------
| 0  |  0  | 0  |   0x58   |
----------------------------
| 0  |  0  | 1  |   0x59   |
----------------------------
| 0  |  1  | 0  |   0x5A   |
----------------------------
| 0  |  1  | 1  |   0x5B   |
----------------------------
| 1  |  0  | 0  |   0x5C   |
----------------------------
| 1  |  0  | 1  |   0x5D   |
----------------------------
| 1  |  1  | 0  |   0x5E   |
----------------------------
| 1  |  1  | 1  |   0x5F   |
----------------------------
***************************/
DFRobot_GP8413 GP8413(/*deviceAddr=*/0x58);

void setup() {

  
  Serial.begin(9600);

  while(GP8413.begin()!=0){
    Serial.println(" Communication with the device failed. Please check if the connection is correct or if the device address is set correctly.");
    delay(1000);
  }

  /**
   * @brief Set the DAC output range.
   * @param range DAC output range.
   * @n     eOutputRange5V(0-5V)
   * @n     eOutputRange10V(0-10V)
   */	
  GP8413.setDACOutRange(GP8413.eOutputRange5V);

  /**
   * @brief Set different channel outputs for the DAC values.
   * @param data Data values corresponding to voltage levels.
   * @n With a 15-bit precision DAC module, the data values ranging from 0 to 32767 correspond to voltage ranges of 0-5V or 0-10V, respectively.
   * @param channel Output channels.
   * @n  0:Channel 0.
   * @n  1:Channel 1.
   * @n  2:All channels.
   */  
  GP8413.setDACOutVoltage(2000,2);

  delay(1000);

  //Save the set voltage in the chip's internal memory for power loss recovery.
  //GP8413.store();
}

void loop() {

}
