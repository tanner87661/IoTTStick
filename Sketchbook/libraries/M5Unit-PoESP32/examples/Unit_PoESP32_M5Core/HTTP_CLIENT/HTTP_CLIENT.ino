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
  Use UNIT PoESP32 to create HTTP request
  使用UNIT PoESP32来创建HTTP请求
*/

#include "Unit_PoESP32.h"

Unit_PoESP32 eth;
String readstr;

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

    readstr = eth.createHTTPClient(HEAD, APPLICATION_X_WWW_FORM_URLENCODED,
                                   "http://httpbin.org/get");
    Serial.println(readstr);

    readstr = eth.createHTTPClient(GET, APPLICATION_X_WWW_FORM_URLENCODED,
                                   "http://httpbin.org/get");
    Serial.println(readstr);

    readstr = eth.createHTTPClient(POST, APPLICATION_X_WWW_FORM_URLENCODED,
                                   "http://httpbin.org/post",
                                   "field1=value1&field2=value2");
    Serial.println(readstr);

    readstr = eth.createHTTPClient(PUT, APPLICATION_X_WWW_FORM_URLENCODED,
                                   "http://httpbin.org/put");
    Serial.println(readstr);

    readstr = eth.createHTTPClient(DELETE, APPLICATION_X_WWW_FORM_URLENCODED,
                                   "http://httpbin.org/delete");
    Serial.println(readstr);
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
