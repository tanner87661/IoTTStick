/*
*******************************************************************************
* Copyright (c) 2021 by M5Stack
*                  Equipped with M5StickCPlus sample source code
*                          配套  M5StickCPlus 示例源代码
* Visit for more information: https://docs.m5stack.com/en/unit/2relay
* 获取更多资料请访问: https://docs.m5stack.com/zh_CN/unit/2relay
*
* Product: 2Relay.  两路继电器
* Date: 2022/8/16
*******************************************************************************
  Please connect to Port ,Use RELAY to switch on and off the circuit.
  请连接端口,使用继电器开关电路。
*/

#include <M5StickCPlus.h>

void setup() {
    M5.begin();             // Init M5StickCPlus.  初始化 M5StickCPlus
    M5.Lcd.setRotation(3);  // Rotate the screen.  旋转屏幕
    M5.Lcd.setTextSize(2);  // Set text size.  设置文字大小
    M5.Lcd.setCursor(50, 0);
    M5.Lcd.println(("Relay Example"));
    pinMode(32, OUTPUT);  // Set pin 32 to output mode.  设置32号引脚为输出模式
    pinMode(33, OUTPUT);
}

void loop(void) {
    M5.Lcd.setCursor(100, 40);
    M5.Lcd.print("ON");
    digitalWrite(32, HIGH);
    digitalWrite(33, HIGH);
    delay(1000);
    M5.Lcd.fillRect(100, 40, 60, 50, BLACK);
    M5.Lcd.print("OFF");
    digitalWrite(32, LOW);
    digitalWrite(33, LOW);
    delay(1000);
    M5.Lcd.fillRect(100, 40, 60, 50, BLACK);
}
