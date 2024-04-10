/*
*******************************************************************************
* Copyright (c) 2021 by M5Stack
*                  Equipped with STAMP-PICO sample source code
*                          配套  STAMP-PICO 示例源代码
* Visit the website for more information：https://docs.m5stack.com/en/core/stamp_pico
* 获取更多资料请访问：https://docs.m5stack.com/zh_CN/core/stamp_pico
*
* describe：I2C Scanner.  I2C探测
* date：2021/9/25
*******************************************************************************
  This program scans the addresses 1-127 continuosly and shows the devices found on the TFT.
  该程序连续扫描地址 1-127 并显示在外部(内部)I2C发现的设备。
*/
#include <Arduino.h>
#include <Wire.h>

void setup()
{
  Serial.begin(115200);
  Wire.begin(21, 22);
  Serial.println("M5Atom I2C Tester"); //Print a string on the screen.  在屏幕上打印字符串
  delay(3000);
}

void loop()
{
  int address;
  int error;
  Serial.printf("\nscanning Address [HEX]\n");
  for(address = 1; address < 127; address++ )
  {
    Wire.beginTransmission(address);  //Data transmission to the specified device address starts.   开始向指定的设备地址进行传输数据
    error = Wire.endTransmission(); /*Stop data transmission with the slave.  停止与从机的数据传输
                                         0: success.  成功
                                         1: The amount of data exceeds the transmission buffer capacity limit.  数据量超过传送缓存容纳限制
              return value:              2: Received NACK when sending address.  传送地址时收到 NACK
                                         3: Received NACK when transmitting data.  传送数据时收到 NACK
                                         4: Other errors.  其它错误 */
    if(error==0)
    {
      Serial.print(address,HEX);
      Serial.print(" ");
    }
    else Serial.print(".");

    delay(10);
  }
}
