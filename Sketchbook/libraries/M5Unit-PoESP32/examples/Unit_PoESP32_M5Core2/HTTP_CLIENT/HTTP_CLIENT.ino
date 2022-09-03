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
  Use UNIT PoESP32 to create HTTP request
  使用UNIT PoESP32来创建HTTP请求
*/

#include "Unit_PoESP32.h"
#include "M5Core2.h"
#include "M5GFX.h"

Unit_PoESP32 eth;
String readstr;

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
    canvas.pushSprite(0, 0);

    readstr = eth.createHTTPClient(HEAD, APPLICATION_X_WWW_FORM_URLENCODED,
                                   "http://httpbin.org/get");
    Serial.println(readstr);
    canvas.println(readstr);
    canvas.pushSprite(0, 0);

    readstr = eth.createHTTPClient(GET, APPLICATION_X_WWW_FORM_URLENCODED,
                                   "http://httpbin.org/get");
    Serial.println(readstr);
    canvas.println(readstr);
    canvas.pushSprite(0, 0);

    readstr = eth.createHTTPClient(POST, APPLICATION_X_WWW_FORM_URLENCODED,
                                   "http://httpbin.org/post",
                                   "field1=value1&field2=value2");
    Serial.println(readstr);
    canvas.println(readstr);
    canvas.pushSprite(0, 0);

    readstr = eth.createHTTPClient(PUT, APPLICATION_X_WWW_FORM_URLENCODED,
                                   "http://httpbin.org/put");
    Serial.println(readstr);
    canvas.println(readstr);
    canvas.pushSprite(0, 0);

    readstr = eth.createHTTPClient(DELETE, APPLICATION_X_WWW_FORM_URLENCODED,
                                   "http://httpbin.org/delete");
    Serial.println(readstr);
    canvas.println(readstr);
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