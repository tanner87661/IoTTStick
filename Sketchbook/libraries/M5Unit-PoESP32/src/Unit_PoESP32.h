/*!
 * @brief An an ESP32 Ethernet Unit supports PoE From M5Stack
 * @copyright Copyright (c) 2022 by M5Stack[https://m5stack.com]
 *
 * @Links [Unit_PoESP32](https://docs.m5stack.com/en/unit/poesp32)
 * @version  V0.0.1
 * @date  2022-07-08
 */
#ifndef _UNIT_POESP32_H_
#define _UNIT_POESP32_H_
#include <Arduino.h>
#include "pins_arduino.h"

typedef enum { HEAD = 1, GET, POST, PUT, DELETE } http_method_t;
typedef enum {
    APPLICATION_X_WWW_FORM_URLENCODED = 0,
    APPLICATION_JSON,
    MULTIPART_FORM_DATA,
    TEXT_XML
} http_content_t;

class Unit_PoESP32 {
   private:
    HardwareSerial *_serial;
    String _readstr;

   public:
    void Init(HardwareSerial *serial = &Serial2, unsigned long baud = 9600,
              uint8_t RX = 16, uint8_t TX = 17);
    String waitMsg(unsigned long time);
    void sendCMD(String command);
    bool checkDeviceConnect();
    bool checkETHConnect();
    bool checkMQTTConnect();

    bool createTCPClient(String ip, int port);
    bool sendTCPData(uint8_t *buffer, size_t size);

    bool createMQTTClient(String host = "host", String port = "port",
                          String clientId  = "client id",
                          String user_name = "user", String user_pwd = "pwd");
    bool publicMQTTMsg(String topic, String payload, String qos = "0");
    bool subscribeMQTTMsg(String topic, String qos = "0");

    String createHTTPClient(
        http_method_t method        = GET,
        http_content_t content_type = APPLICATION_X_WWW_FORM_URLENCODED,
        String url = "", String payload = "");
};

#endif