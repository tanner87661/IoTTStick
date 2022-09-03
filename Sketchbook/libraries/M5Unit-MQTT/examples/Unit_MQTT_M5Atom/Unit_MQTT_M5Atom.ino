/*
*******************************************************************************
* Copyright (c) 2021 by M5Stack
*                  Equipped with M5Atom sample source code
*                          配套  M5Atom 示例源代码
* Visit for more information: https://docs.m5stack.com/en/unit/mqtt
* 获取更多资料请访问: https://docs.m5stack.com/zh_CN/unit/mqtt
*
* Product: Unit MQTT.
* Date: 2022/7/7
*******************************************************************************
  Connect to M5 MQTT server and subscribe to publish topics
  连接至M5 MQTT服务器，并订阅发布话题
*/
#include "M5Atom.h"
#include "M5_MQTT.h"

M5_MQTT unit;
uint32_t chipId     = 0;
unsigned long start = 0;

void setup() {
    M5.begin(true, false, true);
    // INIT UNIT MQTT
    unit.Init(&Serial2, 9600, 32, 26);
    Serial.println("Waiting LAN Connect");
    M5.dis.fillpix(0x0000ff);
    while (!unit.isConnectedLAN()) Serial.print('.');

    Serial.println("LAN Connected");
    Serial.println("Config MQTT");
    for (int i = 0; i < 17; i = i + 8) {
        chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }

    String id = String(chipId, HEX);
    Serial.println("ID: " + id);

    unit.configMQTT("mqtt.m5stack.com",  // host
                    "1883",              // port
                    id,                  // client id
                    "user" + id,         // user name
                    "pwd",               // password
                    "60"                 // keepalive
    );

    Serial.println("Subcribe Topic");

    SubscribeTopic topic1 = {
        "1",          // No 1~-4
        "UNIT_MQTT",  // Topic
        "0"           // QoS
    };

    unit.subscribe(topic1);
    Serial.println("Save config and reset");
    M5.dis.fillpix(0x5400ff);
    unit.configSave();
    unit.startMQTT();
    Serial.println("Start MQTT Connect");
    while (!unit.isConnectedMQTT())
        ;
    Serial.println("MQTT Server Connected");
}

void loop() {
    if (unit.isConnectedMQTT()) {
        M5.dis.fillpix(0x00ff00);
        if (unit.receiveMessage()) {
            Serial.println(unit.payload.Topic);
            Serial.println(unit.payload.Len);
            Serial.println(unit.payload.Data);
        }
        if (millis() - start > 5000) {
            unit.publish({
                "UNIT_MQTT_TOPIC_1",                     // Topic
                "Hello UNIT MQTT: " + String(millis()),  // Data
                "2"                                      // QoS
            });
            start = millis();
        }
    } else {
        M5.dis.fillpix(0x00ff00);
        Serial.print("MQTT disconnect");
    }
}
