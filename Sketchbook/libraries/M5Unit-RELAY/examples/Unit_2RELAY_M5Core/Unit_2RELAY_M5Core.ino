/*
*******************************************************************************
* Copyright (c) 2022 by M5Stack
*                  Equipped with M5Core sample source code
*                          配套  M5Core 示例源代码
* Visit for more information: https://docs.m5stack.com/en/unit/2relay
* 获取更多资料请访问: https://docs.m5stack.com/zh_CN/unit/2relay
*
* Product: 2Relay.  两路继电器
* Date: 2022/8/16
*******************************************************************************
  Please connect to Port A(21,22),Use RELAY to switch on and off the circuit.
  请连接端口A(21,22),使用继电器开关电路。
*/

#include <M5Stack.h>

void setup() {
    M5.begin();             // Init M5Stack.  初始化M5Stack
    M5.Power.begin();       // Init power  初始化电源模块
    M5.Lcd.setTextSize(2);  // Set the text size to 2.  设置文字大小为2
    M5.Lcd.setCursor(85, 0);
    M5.Lcd.println(("Relay Example"));
    dacWrite(25, 0);  // disable the speak noise.  禁用喇叭
    pinMode(21, OUTPUT);  // Set pin 21 to output mode.  设置21号引脚为输出模式
    pinMode(22, OUTPUT);  // Set pin 22 to output mode.  设置22号引脚为输出模式
}

void loop(void) {
    M5.Lcd.setCursor(135, 40);
    M5.Lcd.print("ON");
    digitalWrite(21, HIGH);
    digitalWrite(22, HIGH);
    delay(1000);
    M5.Lcd.fillRect(135, 40, 60, 50, BLACK);
    M5.Lcd.print("OFF");
    digitalWrite(21, LOW);
    digitalWrite(22, LOW);
    delay(1000);
    M5.Lcd.fillRect(135, 40, 60, 50, BLACK);
}