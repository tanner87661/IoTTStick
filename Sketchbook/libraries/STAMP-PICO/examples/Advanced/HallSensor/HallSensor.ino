/*
*******************************************************************************
* Copyright (c) 2021 by M5Stack
*                  Equipped with STAMP-PICO sample source code
*                          配套  STAMP-PICO 示例源代码
* Visit the website for more information：https://docs.m5stack.com/en/core/stamp_pico
* 获取更多资料请访问：https://docs.m5stack.com/zh_CN/core/stamp_pico
*
* describe：Hall sensor.  霍尔传感器
* date：2021/9/25
*******************************************************************************
  The ESP32 MCU has build in Hall sensor.  ESP32 MCU内置霍尔传感器
  It is not very suscescible but if you bring a magnet close to the
  upper screen you will see the influence on the measurement.
  In this example we use some low pass filters to get rid of the noise.
*/
#include <Arduino.h>

#define RANGE 320

float HallOffset = 0; //Store the initial value of magnetic force.  存储磁力的初值
float LowPassFilteredValue=0;

float ESP32_hallRead()  //ESP32 hall value read.  ESP32霍尔值读取
{
  float value = 0;
  int count = 400;
  // mean value filter.  数据低通滤波器
  for (int n = 0; n < count; n++) value += hallRead();
  return value / count * 10;
}

void setup()
{
  Serial.begin(115200);
  Serial.println("ESP32 Hall sensor:"); //Serial printout.  串口打印输出
  HallOffset = ESP32_hallRead(); // callibrate the output value to the magnetic field at start up.  在启动时将输出值校准到磁场
}

void showSignal()
{
  int n;
  for (n = 0; n < RANGE; n++) //Within the horizontal resolution, each pixel is drawn according to the calculated magnetic force
  {                          //在此范围内,根据计算得到的磁力大小
    float value = ESP32_hallRead()- HallOffset; //Reduce the influence of own magnetism.  减少自身磁力的影响
    LowPassFilteredValue+=(value-LowPassFilteredValue)*0.05;
    Serial.printf("Low Pass Filtered Value:%d\n",(int)LowPassFilteredValue);
    if (n > 0){
      Serial.printf("Magnetic value:%.2f\n\n",value);
    }
  }
}

void loop(void)
{
  showSignal();
  delay(200);
}
