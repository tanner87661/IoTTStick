/*
*******************************************************************************
* Copyright (c) 2021 by M5Stack
*                  Equipped with Atom-Lite/Matrix sample source code
*                          配套  Atom-Lite/Matrix 示例源代码
* Visit for more information: https://docs.m5stack.com/en/unit/relay
* 获取更多资料请访问: https://docs.m5stack.com/zh_CN/unit/relay
*
* Product: Relay.  继电器
* Date: 2022/8/16
*******************************************************************************
  Please connect to Port ,Use RELAY to switch on and off the circuit.
  请连接端口,使用继电器开关电路。
*/

#include <M5Atom.h>

void setup() {
    M5.begin(true, false, true);  // Init M5Atom.  初始化 M5Atom
    Serial.println(("Relay Example"));
    pinMode(26, OUTPUT);  // Set pin 26 to output mode.  设置26号引脚为输出模式
    M5.dis.fillpix(0x00ff00);
}

void loop(void) {
    Serial.println("ON");
    digitalWrite(26, HIGH);
    delay(1000);
    Serial.println("OFF");
    digitalWrite(26, LOW);
    delay(1000);
}
