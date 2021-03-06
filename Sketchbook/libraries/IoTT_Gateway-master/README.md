# Esp32 LocoNet to MQTT Gatway

Implementation of the LocoNet to MQTT Gateway for ESP32

This library makes use of the LocoNet and MQTT libraries and provides routing functions for Loconet messages crossing the network bounderies. It also provides a 
message interface for the local ESP32 running the gateway.

Download the ZIP file and install it using the normal Arduino IDE installation procedure. 

Use in your own sketch

Load the LocoNetGateway.ino example to see how the library is used in your own sketch.

In a nutshell:

- Add an include statements to your sketch: 

#include <IoTT_LocoNetHBESP32.h> //use hybrid LocoNet library as Wifi is active
#include <IoTT_MQTTESP32.h> //this is the MQTT access library to send and receive LocoNet messages to/from MQTT broker
#include <IoTT_Gateway.h>

- also add the WiFi library

- define the MQTT broker to be used

const char* mqtt_server = "broker.hivemq.com"; //replace with the ip of your (local) MQTT broker or use this free one
uint16_t mqtt_port = 1883; // = Mosquitto port number, standard is 1883, 8883 for SSL connection (use WiFiSecureClient in this case);
char mqtt_user[50] = "";
char mqtt_password[50] = "";
char mqtt_NodeName[50] = "myGateway";

- define the LocoNet Serial port to be used

#define pinRx    22  //pin used to receive LocoNet signals
#define pinTx    23  //pin used to transmit LocoNet signals
#define InverseLogic true

- define your Wifi Access Point credentials or use WiFiManager instead

- create a Wifi client and initialize the MQTT client with it, initialize LocoNet Interface and the gateway itself:

make sure to specify a callback function for the library to deliver incoming messages:
void callbackLocoNetMessage(lnReceiveBuffer * newData)
{
  //decode the message here
}

WiFiClient wifiClient;
LocoNetESPSerial lnSerial(pinRx, pinTx, InverseLogic); //true is inverted signals
MQTTESP32 lnMQTT(wifiClient); //true is inverted signals
ln_mqttGateway commGateway(&lnSerial, &lnMQTT, &callbackLocoNetMessage);

Alternatively, you can define the gateway without parameters:
ln_mqttGateway commGateway();

and assign ports and callback function later:

commGateway.setSerialPort(&lnSerial);
commGateway.setMQTTPort(&lnMQTT);
commGateway.setAppCallback(&callbackLocoNetMessage);

In your setup() function, set the topic names to be used, specify server and port as well as node name to be used in your MQTT messages (should be unique in the network).
Then connect to your WiFi access point. That's it.

  lnMQTT.setServer(mqtt_server, 1883);
  lnMQTT.setNodeName(mqtt_NodeName);
  lnMQTT.setBCTopicName("lnIn");
  lnMQTT.setEchoTopicName("lnEcho");
  lnMQTT.setPingTopicName("lnPing");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

or with Wifi Manager:
  wifiManager.autoConnect();

Also, set the gatway work mode. It can operate as gateway (GW), LocoNet Interface (LN), or MQTT interface (MQTT)
commGateway.setCommMode(GW); //set to Gateway Mode


Add the following to your loop() function:

  if (WiFi.status() == WL_CONNECTED)
  { 
    commGateway.processLoop();
  }
  else
    WiFi.reconnect();

This will verify that WiFi is connected and process the Gateway processLoop. If WiFi is disconneted, it will try to reconnect



