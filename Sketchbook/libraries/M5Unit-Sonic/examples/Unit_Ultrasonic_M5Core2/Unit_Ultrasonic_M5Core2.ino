/*
*******************************************************************************
* Copyright (c) 2021 by M5Stack
*                  Equipped with M5Core2 sample source code
*                          配套  M5Core2 示例源代码
* Visit for more information: https://docs.m5stack.com/en/unit/sonic
* 获取更多资料请访问: https://docs.m5stack.com/zh_CN/unit/sonic
*
* Product:  Ultrasonic.  超声波测距传感器
* Date: 2022/7/21
*******************************************************************************
  Please connect to Port A,Display the distance measured by ultrasonic
  请连接端口A,显示超声波测量的距离
*/

#include <M5Core2.h>
#include <Unit_Sonic.h>

SONIC_I2C sensor;

void setup() {
    M5.begin();  // Init M5Core2.  初始化M5Core2
    sensor.begin();
    M5.Lcd.setCursor(
        105, 0, 4);  // Set the cursor at (105,0) and set the font to a 4 point
                     // font.  将光标设置在(105,0)处,且设置字体为4号字体
    M5.Lcd.print("Ultrasonic\nDistance:");
}

void loop() {
    static float newvalue = 0;
    newvalue              = sensor.getDistance();
    if ((newvalue < 4000) && (newvalue > 20)) {
        M5.Lcd.setCursor(105, 27);
        M5.Lcd.printf("%.2fmm", newvalue);
    }
    delay(100);
}
