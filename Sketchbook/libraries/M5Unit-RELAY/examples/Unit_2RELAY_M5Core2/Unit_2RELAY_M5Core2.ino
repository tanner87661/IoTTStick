/*
*******************************************************************************
* Copyright (c) 2021 by M5Stack
*                  Equipped with M5Core2 sample source code
*                          配套  M5Core2 示例源代码
* Visit for more information: https://docs.m5stack.com/en/unit/2relay
* 获取更多资料请访问: https://docs.m5stack.com/zh_CN/unit/2relay
*
* Product: 2Relay.  两路继电器
* Date: 2022/8/16
*******************************************************************************
  Please connect to Port B,Use RELAY to switch on and off the circuit.
  请连接端口B,使用继电器开关电路。
*/

#include <M5Core2.h>

void setup() {
    M5.begin();             // Init M5Core2.  初始化 M5Core2
    M5.Lcd.setTextSize(2);  // Set the text size to 2.  设置文字大小为2
    M5.Lcd.setCursor(90, 0);
    M5.Lcd.println(("Relay Example"));
    dacWrite(25, 0);  // disable the speak noise
    pinMode(32, OUTPUT);  // Set pin 32 to output mode.  设置32号引脚为输出模式
    pinMode(33, OUTPUT);  // Set pin 33 to output mode.  设置33号引脚为输出模式
}

void loop(void) {
    M5.Lcd.setCursor(130, 40);
    M5.Lcd.print("ON");
    digitalWrite(32, HIGH);
    digitalWrite(33, HIGH);
    delay(1000);
    M5.Lcd.fillRect(130, 40, 60, 50, BLACK);
    M5.Lcd.print("OFF");
    digitalWrite(32, LOW);
    digitalWrite(33, LOW);
    delay(1000);
    M5.Lcd.fillRect(130, 40, 60, 50, BLACK);
}
