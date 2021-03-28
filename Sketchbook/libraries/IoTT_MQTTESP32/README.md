# Esp32 LocoNet MQTT Interface

Implementation of the LocoNet MQTT Interface for ESP32

This is a wrapper for PubSubClient with interface functions for uploading and receiving LocoNet Messages via MQTT

Download the ZIP file and install it using the normal Arduino IDE installation procedure. 

Make sure to install the PubSubClient by Nick O'Leary library as well. The current version used is 2.7.0 If a newer version is available, you may try to use it instead.

Use in your own sketch

Load the MQTTViewer.ino example to see how the library is used in your own sketch.

In a nutshell:

- Add an include statement to your sketch: #include <IoTT_MQTTESP32.h>
- also add the WiFi library

- define the MQTT broker to be used
- define your Wifi Access Point credentials

- create a Wifi client and initialize the MQTT client with it:

WiFiClient wifiClient;
MQTTESP32 lnMQTT(wifiClient); //true is inverted signals

make sure to specify a callback function for the library to deliver incoming messages:
void onMQTTMessage(lnReceiveBuffer * newData)
{
  //decode the message here
}

If you use the above function name, it will be called by the library automatically. Alternatively, you can set a callback function with any name, but the same structure:

lnMQTT.setMQTTCallback(&myCallbackFunction)

if you specify your own callback function, the default function will no longer be called.

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

Add the following to your loop() function:

  if (WiFi.status() == WL_CONNECTED) 
    lnMQTT.processLoop();
  else
    WiFi.reconnect();

This will verify that WiFi is connected and process the MQTT processLoop. If WiFi is disconneted, it will try to reconnect



