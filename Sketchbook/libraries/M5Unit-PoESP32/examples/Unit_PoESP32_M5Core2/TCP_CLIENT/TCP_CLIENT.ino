/*
*******************************************************************************
* Copyright (c) 2021 by M5Stack
*                  Equipped with M5Core2 sample source code
*                          配套  M5Core2 示例源代码
* Visit more information: https://docs.m5stack.com/en/unit/poesp32
* 获取更多资料请访问: https://docs.m5stack.com/zh_CN/unit/poesp32
*
* Product: Unit PoESP32.
* Date: 2022/7/8
*******************************************************************************
  UNIT PoESP32 Connect to M5Core2 PORT-C (G13/14)
  UNIT PoESP32 连接到 M5Core2 端口-C（G13/14）。
  Use UNIT PoESP32 to connect TCP server
  使用UNIT PoESP32来连接TCP服务器
*/

#include "Unit_PoESP32.h"
#include "M5Core2.h"
#include "M5GFX.h"

Unit_PoESP32 eth;
uint8_t data[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
                  0x06, 0x07, 0x08, 0x09, 0x10};

M5GFX display;
M5Canvas canvas(&display);

void setup() {
    M5.begin();
    display.begin();
    canvas.setColorDepth(1);  // mono color
    canvas.setFont(&fonts::efontCN_14);
    canvas.setTextSize(2);
    canvas.setPaletteColor(1, GREEN);
    canvas.createSprite(display.width(), display.height());
    canvas.setTextScroll(true);
    eth.Init(&Serial2, 9600, 13, 14);
    delay(10);
    canvas.println("wait device connect");
    canvas.pushSprite(0, 0);
    while (!eth.checkDeviceConnect()) {
        delay(10);
    }

    canvas.println("device connected");
    canvas.println("wait ethernet connect");
    canvas.pushSprite(0, 0);
    while (!eth.checkETHConnect()) {
        delay(10);
    }
    canvas.println("ethernet connected");
    canvas.println("Config TCP Client");
    canvas.println("wait tcp connect");
    canvas.pushSprite(0, 0);
    // AT+CIPSTART="TCP","192.168.3.102",8080
    while (!eth.createTCPClient("120.77.157.90", 1883)) {
        delay(10);
    }
    // while (!eth.configTCPClient("192.168.1.5", 60000)) {
    //     delay(10);
    // }
    canvas.println("tcp connected");
    canvas.pushSprite(0, 0);

    if (eth.sendTCPData(data, sizeof(data))) {
        canvas.println("send ok");
    } else {
        canvas.println("send fail");
    }
    canvas.pushSprite(0, 0);
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
