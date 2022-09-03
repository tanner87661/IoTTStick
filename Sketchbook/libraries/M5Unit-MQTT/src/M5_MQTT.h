/*!
 * @brief An Ethernet MQTT communication module From M5Stack
 * @copyright Copyright (c) 2022 by M5Stack[https://m5stack.com]
 *
 * @Links [Unit MQTT](https://docs.m5stack.com/en/unit/mqtt)
 * @version  V0.0.1
 * @date  2022-07-07
 */
#ifndef _M5_MQTT_H_
#define _M5_MQTT_H_

#include <Arduino.h>

#include "pins_arduino.h"

struct SubscribeTopic {
    String No;
    String Topic;
    String QoS;
};

struct PublishTopic {
    String Topic;
    String Data;
    String QoS;
};

struct PayloadTopic {
    String Topic;
    int Len;
    String Data;
};

class M5_MQTT {
   private:
    HardwareSerial *_serial;

   public:
    void Init(HardwareSerial *serial = &Serial2, int baud = 9600,
              uint8_t RX = 16, uint8_t TX = 17);
    bool isConnectedLAN();
    void configMQTT(String host = "host", String port = "port",
                    String clientId = "client id", String user = "user",
                    String pwd = "pwd", String keepalive = "60");
    void subscribe(SubscribeTopic Topic);
    void publish(PublishTopic Topic);
    void configSave();
    bool isConnectedMQTT();
    bool receiveMessage();
    void startMQTT();
    String waitMsg(unsigned long time);
    void sendMsg(String command);

   public:
    PayloadTopic payload;
};

#endif