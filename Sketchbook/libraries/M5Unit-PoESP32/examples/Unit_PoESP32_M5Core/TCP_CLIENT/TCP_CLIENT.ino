/*
*******************************************************************************
* Copyright (c) 2021 by M5Stack
*                  Equipped with M5Core sample source code
*                          配套  M5Core 示例源代码
* Visit more information: https://docs.m5stack.com/en/unit/poesp32
* 获取更多资料请访问: https://docs.m5stack.com/zh_CN/unit/poesp32
*
* Product: Unit PoESP32.
* Date: 2022/7/8
*******************************************************************************
  Use UNIT PoESP32 to connect TCP server
  使用UNIT PoESP32来连接TCP服务器
*/

#include "Unit_PoESP32.h"

Unit_PoESP32 eth;

uint8_t data[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
                  0x06, 0x07, 0x08, 0x09, 0x10};

void setup() {
    Serial.begin(115200);
    eth.Init(&Serial2, 9600, 32, 26);
    delay(10);
    Serial.println("wait device connect");
    while (!eth.checkDeviceConnect()) {
        delay(10);
    }

    Serial.println("device connected");
    Serial.println("wait ethernet connect");
    while (!eth.checkETHConnect()) {
        delay(10);
    }
    Serial.println("ethernet connected");
    Serial.println("Config TCP Client");

    // AT+CIPSTART="TCP","192.168.3.102",8080
    Serial.println("wait tcp connect");
    while (!eth.createTCPClient("120.77.157.90", 1883)) {
        delay(10);
    }

    // while (!eth.configTCPClient("192.168.1.5", 60000)) {
    //     delay(10);
    // }

    Serial.println("tcp connected");

    if (eth.sendTCPData(data, sizeof(data))) {
        Serial.println("send ok");
    } else {
        Serial.println("send fail");
    }
}

void loop() {
    if (Serial.available()) {
        char ch = Serial.read();
        Serial2.write(ch);
    }
    if (Serial2.available()) {
        char ch = Serial2.read();
        Serial.write(ch);
    }
}
