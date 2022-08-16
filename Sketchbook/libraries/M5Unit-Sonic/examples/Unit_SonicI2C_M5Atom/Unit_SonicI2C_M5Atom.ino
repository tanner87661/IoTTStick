/*
*******************************************************************************
* Copyright (c) 2021 by M5Stack
*                  Equipped with Atom-Lite/Matrix sample source code
*                          配套  Atom-Lite/Matrix 示例源代码
* Visit for more information：https://docs.m5stack.com/en/unit/sonic.i2c
* 获取更多资料请访问：https://docs.m5stack.com/zh_CN/unit/sonic.i2c
*
* Product: Ultrasonic.  超声波测距传感器 I2C版本
* Date：2022/7/21
*******************************************************************************
  Please connect to Port A,Display the distance measured by ultrasonic
  请连接端口A,显示超声波测量的距离
*/

#include <M5Atom.h>
#include <Unit_Sonic.h>

SONIC_I2C sensor;

void setup() {
    M5.begin(true, true, fasle);  // Init M5Atom.  初始化M5Atom
    sensor.begin();
}

void loop() {
    static float Distance = 0;
    Distance              = sensor.getDistance();
    if ((Distance < 4000) && (Distance > 20)) {
        Serial.printf("Distance: %.2fmm\r\n", Distance);
    }
    delay(100);
}
