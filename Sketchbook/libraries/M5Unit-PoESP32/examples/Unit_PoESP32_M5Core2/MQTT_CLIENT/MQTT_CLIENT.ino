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
  Use UNIT PoESP32 to connect to the MQTT server, and implement subscription
  and publishing messages. Check the status through Serial.
  使用UNIT PoESP32连接到MQTT服务器，并实现订阅和发布信息。通过Serial检查状态。
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
    canvas.println("wait mqtt connect");
    canvas.pushSprite(0, 0);

    while (!eth.createMQTTClient("120.77.157.90", "1883", "client_id",
                                 "user_name", "password")) {
        delay(10);
    }
    canvas.println("mqtt connected");
    canvas.pushSprite(0, 0);
    while (!eth.subscribeMQTTMsg("PoESP32_MQTT_D", "2")) {
        delay(10);
    }
}

void loop() {
    if (Serial2.available()) {
        readstr = Serial2.readString();
        Serial.println(readstr);
        canvas.println(readstr);
        canvas.pushSprite(0, 0);
    }
    eth.publicMQTTMsg("PoESP32_MQTT_U", "Hello From PoESP32", "2");
    delay(2000);
}
