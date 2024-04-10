
/*
*******************************************************************************
* Copyright (c) 2022 by M5Stack
*                  Equipped with ATOM DTU CAT1 MQTT Client sample source code
* Visit the website for more
information：https://docs.m5stack.com/en/atom/atom_dtu_cat1
* describe: ATOM DTU CAT1 MQTT Client Example.
* Libraries:
 - [TinyGSM](https://github.com/vshymanskyy/TinyGSM)
 - [PubSubClient](https://github.com/knolleary/pubsubclient.git)
* date：2022/1/25
*******************************************************************************
*/

#include <M5Atom.h>
#include "ATOM_DTU_CAT1.h"
#include <PubSubClient.h>
#include <TinyGsmClient.h>
#include <time.h>
#include <sys/time.h>

#define MQTT_BROKER   "mqtt.m5stack.com"
#define MQTT_PORT     1883
#define MQTT_USERNAME "ATOM_DTU_CAT1"
#define MQTT_PASSWORD "ATOM_DTU_CAT1_PWD"
#define MQTT_D_TOPIC  "ATOM_DTU_CAT1/D"
#define MQTT_U_TOPIC  "ATOM_DTU_CAT1/U"

#define UPLOAD_INTERVAL 10000
uint32_t lastReconnectAttempt = 0;

TinyGsm modem(SerialAT);

TinyGsmClient tcpClient(modem);
PubSubClient mqttClient(MQTT_BROKER, MQTT_PORT, tcpClient);

void mqttCallback(char *topic, byte *payload, unsigned int len);
bool mqttConnect(void);
void InitNetwork(void);

// Your GPRS credentials, if any
const char apn[]      = "YourAPN";
const char gprsUser[] = "";
const char gprsPass[] = "";

struct tm now;
char s_time[50];

void setup() {
    M5.begin();
    SerialAT.begin(SIM7680_BAUDRATE, SERIAL_8N1, ATOM_DTU_SIM7680_RX,
                   ATOM_DTU_SIM7680_TX);
    InitNetwork();
    mqttClient.setCallback(mqttCallback);
}

void loop() {
    static unsigned long timer = 0;

    if (!mqttClient.connected()) {
        SerialMon.println(">>MQTT NOT CONNECTED");
        // Reconnect every 10 seconds
        uint32_t t = millis();
        if (t - lastReconnectAttempt > 3000L) {
            lastReconnectAttempt = t;
            if (mqttConnect()) {
                lastReconnectAttempt = 0;
            }
        }
        delay(100);
    }

    if (millis() >= timer) {
        timer = millis() + UPLOAD_INTERVAL;
        mqttClient.publish(MQTT_U_TOPIC, "hello");  // 发送数据
    }

    mqttClient.loop();
}

void mqttCallback(char *topic, byte *payload, unsigned int len) {
    char info[len];
    memcpy(info, payload, len);
    SerialMon.println("Message arrived [" + String(topic) + "]: ");
    SerialMon.println(info);
}

bool mqttConnect(void) {
    SerialMon.println("Connecting to ");
    SerialMon.println(MQTT_BROKER);
    // Connect to MQTT Broker
    String mqttid = ("MQTTID_" + String(random(65536)));
    bool status =
        mqttClient.connect(mqttid.c_str(), MQTT_USERNAME, MQTT_PASSWORD);
    if (status == false) {
        SerialMon.println(" fail");
        return false;
    }
    SerialMon.println("MQTT CONNECTED!");
    mqttClient.publish(MQTT_U_TOPIC, "CATM MQTT CLIENT ONLINE");
    mqttClient.subscribe(MQTT_D_TOPIC);
    return mqttClient.connected();
}

void InitNetwork(void) {
    unsigned long start = millis();
    SerialMon.println("Initializing modem...");
    while (!modem.init()) {
        SerialMon.println("waiting...." + String((millis() - start) / 1000) +
                          "s");
    };

    start = millis();
    SerialMon.println("Waiting for network...");
    while (!modem.waitForNetwork()) {
        SerialMon.println("waiting...." + String((millis() - start) / 1000) +
                          "s");
    }

    SerialMon.println("Waiting for GPRS connect...");
    if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
        SerialMon.println("waiting...." + String((millis() - start) / 1000) +
                          "s");
    }
    SerialMon.println("success");
}
