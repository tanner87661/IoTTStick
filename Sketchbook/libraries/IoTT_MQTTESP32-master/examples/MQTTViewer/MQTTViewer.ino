#include <arduino.h>
#include <IoTT_MQTTESP32.h> 
#include <WiFi.h>

/////////USER CONFIGURATION//////////////////////////////////////////
#define showPerformance

const char* mqtt_server = "broker.hivemq.com"; //replace with the ip of your (local) MQTT broker or use this free one
uint16_t mqtt_port = 1883; // = Mosquitto port number, standard is 1883, 8883 for SSL connection (use WiFiSecureClient in this case);
char mqtt_user[50] = "";
char mqtt_password[50] = "";
char mqtt_NodeName[50] = "myMQTTViewer";

const char* ssid     = "your_access_point_name";
const char* password = "your_wifi_password";
const char* gateway ="your_gateway_ip"; //Example: "192.168.1.1";

/////////END OF USER CONFIGURATION//////////////////////////////////////////

WiFiClient wifiClient;
MQTTESP32 lnMQTT(wifiClient); //true is inverted signals

//used for performance analysis
#ifdef showPerformance
  uint16_t loopCtr = 0;
  uint32_t myTimer = millis() + 1000;
#endif

long lastReconnectAttempt = 0;

void onMQTTMessage(lnReceiveBuffer * newData)
{
   Serial.printf("LN Msg Error Flags %i ReqID %i with %i bytes requested %i response time %i echo time %i: ", newData->errorFlags, newData->reqID, newData->lnMsgSize, newData->reqRecTime, newData->reqRespTime, newData->echoTime); 
   for (int i=0; i<newData->lnMsgSize; i++)
   {
     Serial.print(newData->lnData[i],16);
     Serial.print(" ");
   }
   Serial.println();
}
 
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1000);

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

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  Serial.println("Init Done");
  randomSeed(2000);
}

void sendBDInput(uint16_t bdNum, bool bdStat)
{
  lnTransmitMsg myMsg;
  myMsg.lnMsgSize = 4;
  myMsg.reqID = random(2000);
  myMsg.lnData[0] = 0xB2; //B2 1A 50 7
  myMsg.lnData[1] = (bdNum & 0x7E) >> 1;
  myMsg.lnData[2] = ((uint8_t)bdStat<<5) | ((bdNum & 0x01)<<4);
  myMsg.lnData[3] = myMsg.lnData[0] ^ myMsg.lnData[1] ^ myMsg.lnData[2] ^ 0xFF;
  if (lnMQTT.connected())
  {
    int numBytes = lnMQTT.lnWriteMsg(myMsg);
    Serial.printf("Write %i bytes for ID %i Target %i\n", numBytes, myMsg.reqID, myMsg.lnMsgSize);
  }
  else
    Serial.println("LocoNet not connected");
}

void sendLocoReq(uint8_t LocoNum)
{
  lnTransmitMsg myMsg;
  myMsg.lnMsgSize = 4;
  myMsg.reqID = random(2000);
  myMsg.lnData[0] = 0xBF; //B2 1A 50 7
  myMsg.lnData[1] = 0;
  myMsg.lnData[2] = LocoNum;
  myMsg.lnData[3] = myMsg.lnData[0] ^ myMsg.lnData[1] ^ myMsg.lnData[2] ^ 0xFF;
  if (lnMQTT.connected())
  {
    int numBytes = lnMQTT.lnWriteMsg(myMsg);
    Serial.printf("Write %i bytes for ID %i Target %i\n", numBytes, myMsg.reqID, myMsg.lnMsgSize);
  }
  else
    Serial.println("LocoNet not connected");
}


void loop() {
  // put your main code here, to run repeatedly:

#ifdef showPerformance
  loopCtr++;
  if (millis() > myTimer)
  {
    Serial.printf("Timer Loop: %i\n", loopCtr);
    loopCtr = 0;
    myTimer += 1000;
  }
#endif

  if (WiFi.status() == WL_CONNECTED) 
    lnMQTT.processLoop();
  else
    WiFi.reconnect();

  if (Serial.available())
  {
    char c;
    while (Serial.available())
      c = Serial.read();
    Serial.println("Sending LN Messages:");
    for (int i = 0; i < 2; i++)
    {
      sendLocoReq(50+i);
      sendBDInput(i, false);
      sendBDInput(i, true);
    }
  }
}
