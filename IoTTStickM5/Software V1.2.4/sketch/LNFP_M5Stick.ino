uint8_t BBVersion[] = {1,2,4}; //release date 7/6/2020

//#define measurePerformance //uncomment this to display the number of loop cycles per second


//Arduino published libraries. Install using the Arduino IDE or download from Github and install manually
#include <arduino.h>
#include <WiFi.h>
#include <time.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <ESPAsyncWiFiManager.h>         //https://github.com/alanswx/ESPAsyncWiFiManager
#define FORMAT_SPIFFS_IF_FAILED true
#include <ArduinoJson.h> //standard JSON library, can be installed in the Arduino IDE. Make sure to use version 6.x
#include <NmraDcc.h> //install via Arduino IDE
#include <M5StickC.h>
#include <ArduinoUniqueID.h>


//following libraries can be downloaded from https://github.com/tanner87661?tab=repositories
#include <IoTT_DigitraxBuffers.h> //as introduced in video # 30
//#include <IoTT_Mux64Buttons.h> //as introduced in video # 29
#include <IoTT_LocoNetButtons.h> //as introduced in video # 29
#include <IoTT_LEDChain.h> //as introduced in video # 30
#include <IoTT_LocoNetHBESP32.h> //this is a hybrid library introduced in video #29
#include <IoTT_MQTTESP32.h> //as introduced in video # 29
#include <IoTT_Gateway.h> //LocoNet Gateway as introduced in video # 29
//#include <IoTT_SecurityElements.h> //not ready yet. This is the support for ABS/APB as described in Videos #20, 21, 23, 24

//library object pointers. Libraries will be dynamically initialized as needed during the setup() function
AsyncWebServer * myWebServer = NULL; //(80)
DNSServer * dnsServer = NULL;
//WiFiClientSecure * wifiClientSec = NULL;
WiFiClient * wifiClient = NULL;
AsyncWebSocket * ws = NULL; //("/ws");
AsyncWebSocketClient * globalClient = NULL;
uint16_t wsReadPtr = 0;
char wsBuffer[16384]; //should this by dynamic?

//global variables
bool useStaticIP = false;
IPAddress static_ip;
IPAddress static_gw;
IPAddress static_nm;
IPAddress static_dns;
IPAddress ap_ip;
IPAddress ap_nm(255,255,255,0);

uint8_t wifiCfgMode = 0x00; //1: STA, 2: AP, 3: STA+AP
String devName; //device name used for AP, will load from node.cfg
String apPassword; //AP password, will load from node.cfg

bool wifiAlwaysOn = false; //set to false to shut Wifi after some time of inactivity. Gateway and MQTT should be disabled, though
bool wifiCancelled = false; //true if Wifi was switched off due to no use
uint32_t lastWifiUse = millis();
#define wifiShutTimeout 120000 //after 2 Mins of not using, Wifi is closed
#define apShutTimeout 180000 //after 10 Mins of not using, Wifi is closed
#define keepAliveInterval 30000 //send message every 30 secs to keep connection alive
uint32_t keepAlive = millis(); //timer used for periodic message sent over wifi to keep alive while browser is connected. Sent over websocket connection

String bufferFileName = "/buffers.dat";

//more library object pointers. Libraries will be dynamically initialized as needed during the setup() function
//IoTT_Mux64Buttons * myButtons = NULL;
IoTT_LocoNetButtonList * buttonHandler = NULL; 
//IoTT_SecurityElementList * secElHandlerList = NULL;
LocoNetESPSerial * lnSerial = NULL;
ln_mqttGateway * commGateway = NULL;
IoTT_ledChain * myChain = NULL;
MQTTESP32 * lnMQTT = NULL;
NmraDcc  * myDcc = NULL;

//some variables used for performance measurement
#ifdef measurePerformance
uint16_t loopCtr = 0;
uint32_t myTimer = millis() + 1000;
#endif

//********************************************************Hardware Configuration******************************************************
//#define LED_DATA_PIN 26 //9 //this is used to initialize the FastLED template
//#define LED_CLOCK_PIN 13 //10 //this is used to initialize the FastLED template
#define groveRxD 33 //pin 1 yellow SCL, RxD
#define groveTxD 32 //pin 2 white SDA, TxD
#define hatSDA 0
#define hatSCL 26
#define hatDataPin 26
#define stickLED 10 //red LED on stick

uint8_t hatData[] = {1, hatDataPin}; //pin definition for LED library
//***********************************************************************************************************************************

//global variables for the NTP module
int ntpTimeout = 5000; //ms timeout for NTP update request
char ntpServer[50] = "us.pool.ntp.org"; //default server for US. Change this to the best time server for your region, or set in node.cfg
char ntpTimeZone[100] = "EST5EDT";  // default for Eastern Time Zone. Enter your time zone from here: (https://remotemonitoringsystems.ca/time-zone-abbreviations.php) into node.cfg
bool ntpOK = false;
bool useNTP = false;
tm timeinfo;
time_t now;
const uint32_t ntpIntervallDefault = 86400000; //1 day in milliseconds
const uint32_t ntpIntervallShort = 10000; //10 Seconds in case something went wrong
uint32_t ntpTimer = millis();
//strDateTime dateTime;

typedef struct
{
  char devName[60];
  uint8_t devId;
  uint8_t devCommMode;
  uint8_t * validInterfaces = NULL;
  uint8_t numValid = 0;
} configType;

configType useHat;
configType useInterface;
uint32_t useALM = 0; //bit mask configuration list

uint8_t m5CurrentPage = 0;
uint16_t oldWifiStatus = 0;
uint8_t useM5Viewer = 0;
uint8_t m5DispLine = 0;

File uploadFile; //used for web server to upload files

//this is the outgoing communication function for IoTT_DigitraxBuffers.h, routing the outgoing messages to the correct interface
uint16_t sendMsg(lnTransmitMsg txData)
{
  switch (useInterface.devId)
  {
    case 0: break; //none
    case 1: break; //DCC
    case 2: if (lnSerial) lnSerial->lnWriteMsg(txData); break;
    case 3: if (lnMQTT) lnMQTT->lnWriteMsg(txData); break;
    case 4: if (commGateway) //LocoNet w/ Gateway
            { 
//              Serial.println("Calling Gateway"); 
              commGateway->lnWriteMsg(txData); 
            }
            break;
  }
}

void setup() {
  // put your setup code here, to run once:
  M5.begin();
  M5.Axp.EnableCoulombcounter();
  initDisplay();
  getRTCTime();
  char time_output[30];
  strftime(time_output, 30, "%a  %d-%m-%y %T", localtime(&now));
  Serial.println(time_output);
  Serial.println("Init SPIFFS");
  SPIFFS.begin(); //File System. Size is set to 1 MB during compile time and loaded with configuration data and web pages
  UniqueIDdump(Serial);
  loadFromFile(bufferFileName);
  myWebServer = new AsyncWebServer(80);
  dnsServer = new DNSServer();
  wifiClient = new WiFiClient();
  uint16_t wsRequest = 0;
  ws = new AsyncWebSocket("/ws");

  DynamicJsonDocument * jsonConfigObj = NULL;
  DynamicJsonDocument * jsonDataObj = NULL;
  jsonConfigObj = getDocPtr("/configdata/node.cfg"); //read and decode the master config file. See ConfigLoader tab
  if (jsonConfigObj != NULL)
  {
    //first, read all Wifi Paramters
    if (jsonConfigObj->containsKey("wifiMode"))
      wifiCfgMode = (*jsonConfigObj)["wifiMode"];
    Serial.printf("Wifi Config Mode %i\n",wifiCfgMode);
//    if (jsonConfigObj->containsKey("useWifiTimeout"))
//      wifiAlwaysOn =  (!(bool)(*jsonConfigObj)["useWifiTimeout"]);
    if (jsonConfigObj->containsKey("devName"))
    {
      String thisData = (*jsonConfigObj)["devName"];
      devName = thisData;
    }
    if ((*jsonConfigObj)["inclMAC"])
    {
      devName += '_';
      devName += String((uint32_t)ESP.getEfuseMac());
    }
    if ((*jsonConfigObj)["useStaticIP"])
      useStaticIP = (bool)(*jsonConfigObj)["useStaticIP"];
    if (useStaticIP && jsonConfigObj->containsKey("staticConfig"))
    {
      String thisIP = (*jsonConfigObj)["staticConfig"]["staticIP"];
      static_ip.fromString(thisIP);
      String thisGW = (*jsonConfigObj)["staticConfig"]["staticGateway"];
      static_gw.fromString(thisGW);
      String thisNM = (*jsonConfigObj)["staticConfig"]["staticNetmask"];
      static_nm.fromString(thisNM);
      String thisDNS = (*jsonConfigObj)["staticConfig"]["staticDNS"];
      static_dns.fromString(thisDNS);
    }

    if (((wifiCfgMode && 0x02) > 0) && jsonConfigObj->containsKey("apConfig"))
    {
      String thisIP = (*jsonConfigObj)["apConfig"]["apGateway"];
      ap_ip.fromString(thisIP);
      String thisPW = (*jsonConfigObj)["apConfig"]["apPassword"];
      apPassword = thisPW;
    }

    //load the hat type
    if (jsonConfigObj->containsKey("HatTypeList") && jsonConfigObj->containsKey("HatIndex"))
    {
      int listIndex = (*jsonConfigObj)["HatIndex"];
      JsonArray devTypes = (*jsonConfigObj)["HatTypeList"];
//      String thisName = devTypes[listIndex]["Name"];
//      useHat.devName = thisName;
      strcpy(useHat.devName, devTypes[listIndex]["Name"]);
      useHat.devId = devTypes[listIndex]["HatId"];
      JsonArray IntfList = devTypes[listIndex]["InterfaceList"];
      if (IntfList)
      {
        useHat.validInterfaces = (uint8_t*) realloc (useHat.validInterfaces, IntfList.size() * sizeof(uint8_t));
        useHat.numValid = IntfList.size();
        for (int i = 0; i < IntfList.size(); i++)
        {
          useHat.validInterfaces[i] = IntfList[i];
//          Serial.println(useHat.validInterfaces[i]);
        }
      }
      useHat.devCommMode = devTypes[listIndex]["Type"];
    }
    else
    {
      Serial.println("No HAT defined, go with none");
//      useHat.devName = "none";
      strcpy(useHat.devName, "none");
      useHat.devCommMode = 0;
      useHat.devId = 0;
      useHat.validInterfaces = (uint8_t*) realloc (useHat.validInterfaces, sizeof(uint8_t));
      useHat.validInterfaces[0] = 1;
      useHat.numValid = 1;
    }

    //load the interface type
    if (jsonConfigObj->containsKey("InterfaceTypeList") && jsonConfigObj->containsKey("InterfaceIndex"))
    {
      int listIndex = (*jsonConfigObj)["InterfaceIndex"];
      JsonArray commTypes = (*jsonConfigObj)["InterfaceTypeList"];
//      String thisName = commTypes[listIndex]["Name"];
//      useInterface.devName = thisName;
      strcpy(useInterface.devName, commTypes[listIndex]["Name"]);
      useInterface.devCommMode = commTypes[listIndex]["Type"];
      useInterface.devId = commTypes[listIndex]["IntfId"];
    }
    else
    {
      Serial.println("No Interface defined, go with LocoNet");
//      useInterface.devName = "LocoNet";
      strcpy(useHat.devName, "LocoNet");
      useInterface.devCommMode = 1;
      useInterface.devId = 2;
    }
    //verify that hat and interface go together
    bool interfaceOK = false;
//    Serial.println(useHat.numValid);
    for (int i = 0; i < useHat.numValid; i++) // is array of uint8_t
      if (useInterface.devId == useHat.validInterfaces[i])
      {
        interfaceOK = true;
        break;
      }

//    Serial.printf("Initialize with %s %s %i %i %i \n", useHat.devName, useInterface.devName, useHat.devCommMode, useInterface.devCommMode, interfaceOK);
    if ((!interfaceOK) || (useHat.devCommMode > useInterface.devCommMode))
    {
      Serial.println("Invalid Hat/CommNode combination. Reload Node Config File");
      delay(5000);      
      ESP.restart();
    }
    Serial.printf("Initialize with %s %s \n", useHat.devName, useInterface.devName);

    if ((useInterface.devId == 2) || (useInterface.devId == 4)) //LocoNet or Gateway with LocoNet
    {
      Serial.println("Init LocoNet");  
      pinMode(stickLED, OUTPUT);
      digitalWrite(stickLED, true);
      lnSerial = new LocoNetESPSerial(groveRxD, groveTxD, true); //true is inverted signals
      lnSerial->setBusyLED(stickLED, false);
      lnSerial->setLNCallback(callbackLocoNetMessage);
    } 
    else 
      Serial.println("LocoNet not activated");

    if ((useInterface.devId == 3) || (useInterface.devId == 4))//MQTT or LN Gateway/ALM
    {
      Serial.println("Load MQTT Data");  
      jsonDataObj = getDocPtr("/configdata/mqtt.cfg");
      if (jsonDataObj != NULL)
      {
        lnMQTT = new MQTTESP32(*wifiClient); 
        lnMQTT->loadMQTTCfgJSON(*jsonDataObj);
        lnMQTT->setMQTTCallback(callbackLocoNetMessage);
        wifiAlwaysOn = true;
        delete(jsonDataObj);
      }
    }
    else 
      Serial.println("MQTT not activated");

    if (useInterface.devId == 4) // && (modMode == 1))) //Gateway/ALM
    {
      Serial.println("Load Gateway");  
      commGateway = new ln_mqttGateway(lnSerial, lnMQTT, &callbackLocoNetMessage);
//      commGateway->setCommMode(GW); //set to Gateway Mode. Could also be LN or MQTT if used as interface
    }
    else 
      Serial.println("Gateway not activated");

    if (useInterface.devId == 1) //DCC Interface
    {
      Serial.println("Load DCC Interface");  
      myDcc = new NmraDcc();
      pinMode(groveTxD, OUTPUT);
      // Setup which External Interrupt, the Pin it's associated with that we're using and enable the Pull-Up 
      myDcc->pin(groveRxD, 1);
      // Call the main DCC Init function to enable the DCC Receiver
      myDcc->init(MAN_ID_DIY, 10, FLAGS_CV29_BITS, 0 );
      Serial.println("DCC Init Done");
    }
    else 
      Serial.println("DCC Interface not activated");

    setTxFunction(&sendMsg); //defined in IoTT_DigitraxBuffers.h
    if (jsonConfigObj->containsKey("useBushby"))
      if ((bool)(*jsonConfigObj)["useBushby"])
        enableBushbyWatch(true); //defined in IoTT_DigitraxBuffers.h
      else
        enableBushbyWatch(false);
        
//Initialize special hardware
    if ((useHat.devId == 1)) //BlueHat or CTC Hat
    {
      {
        Serial.println("Load BlueHat Data"); 
        jsonDataObj = getDocPtr("/configdata/led.cfg");
        if (jsonDataObj != NULL)
        {
          myChain = new IoTT_ledChain(); // ... construct now, and call setup later
          myChain->loadLEDChainJSON(*jsonDataObj);
//          char chainType[10];
          char colType[10];
          JsonObject * myParams = NULL;
          if (jsonDataObj->containsKey("ChainParams"))
          {
            Serial.println("Loading ChainParams");
            myParams = new JsonObject((*jsonDataObj)["ChainParams"]);
          }
          else
            Serial.println("No ChainParams");
          if (myParams != NULL)
          {
            Serial.println("Interpreting ChainParams");
//            if (myParams->containsKey("ChainType"))
//              strcpy(chainType, (*myParams)["ChainType"]);
//            else
//            {
//              Serial.println("No Key ChainType");
//              strcpy(chainType, "WS2812");
//            }
            if (myParams->containsKey("ColorSeq"))
              strcpy(colType, (*myParams)["ColorSeq"]);
            else
            {
              Serial.println("No Key ColorSeq");
              strcpy(colType, "GRB");
            }
            delete myParams;
          }
          else
          {
            Serial.println("Default ChainParams");
//            strcpy(chainType, "WS2812");
            strcpy(colType, "GRB");
          }
          delete(jsonDataObj);
//          if (strcmp(chainType, "WS2812") == 0)
//          {
            Serial.printf("Init LED Chain on Pin %i, %i LEDs long\n", hatDataPin, myChain->getChainLength());  
            if (strcmp(colType, "GRB") == 0)
            {
              Serial.println(colType);
//              Serial.println(chainType);
              FastLED.addLeds<WS2811, hatDataPin, GRB>(myChain->getChain(), myChain->getChainLength()); 
            }
            if (strcmp(colType, "RGB") == 0)
            {
              Serial.println(colType);
//              Serial.println(chainType);
              FastLED.addLeds<WS2811, hatDataPin, RGB>(myChain->getChain(), myChain->getChainLength()); 
            }
//          } 
/*
          if (strcmp(chainType, "WS2801") == 0)
          {
            Serial.printf("Init LED Chain on Pins %i %i, %i LEDs long\n", LED_DATA_PIN, LED_CLOCK_PIN, myChain->getChainLength());  
            if (strcmp(colType, "GRB") == 0)
            {
              Serial.println(colType);
              Serial.println(chainType);
              FastLED.addLeds<WS2801, LED_DATA_PIN, LED_CLOCK_PIN, GRB>(myChain->getChain(), myChain->getChainLength()); 
            }
            if (strcmp(colType, "RGB") == 0)
            {
              Serial.println(colType);
              Serial.println(chainType);
              FastLED.addLeds<WS2801, LED_DATA_PIN, LED_CLOCK_PIN, RGB>(myChain->getChain(), myChain->getChainLength()); 
            }
          } 
*/          
        }
      }
    }
    else 
      Serial.println("LED Module not activated");

/*
    if (useHat.devIndex == 2) //CTC Hat
    {
      Serial.println("Init Hardware Buttons");  
      {
        jsonDataObj = getDocPtr("/configdata/btn.cfg");
        if (jsonDataObj != NULL)
        {
          myButtons = new IoTT_Mux64Buttons();
          Serial.println("Load Button Data");  
          myButtons->loadButtonCfgJSON(*jsonDataObj);
          delete(jsonDataObj);
        }
      }
    }
    else 
      Serial.println("HW Button Module not activated");
*/

//Load ALM list
    if (jsonConfigObj->containsKey("ALMTypeList") && jsonConfigObj->containsKey("ALMIndex"))
    {
      JsonArray almIndex = (*jsonConfigObj)["ALMIndex"];
      for (int i = 0; i < almIndex.size(); i++)
      {
        byte shiftBy = almIndex[i];
        useALM |= (0x01 << shiftBy);
      }
    }

//Initialize ALMs
    if (useALM & 0x01) //Button Handler
    {
      Serial.println("Load Button Handler Data");  
      jsonDataObj = getDocPtr("/configdata/btnevt.cfg");
      if (jsonDataObj != NULL)
      {
        buttonHandler = new(IoTT_LocoNetButtonList);
        buttonHandler->loadButtonCfgJSON(*jsonDataObj);
        delete(jsonDataObj);
      }
    }
    else 
      Serial.println("Button Handler not activated");

/*    
    if (useALM & 0x02)
    if (jsonConfigObj->containsKey("useSecurityElements"))
    {
      if (((bool)(*jsonConfigObj)["useSecurityElements"]) && (modMode == 1)) //must be ALM
      {
        Serial.println("Load Security Element Data");  
        jsonDataObj = getDocPtr("/configdata/secel.cfg");
        if (jsonDataObj != NULL)
        {
          secElHandlerList = new(IoTT_SecurityElementList);
          secElHandlerList->loadSecElCfgJSON(*jsonDataObj);
          delete(jsonDataObj);
        }
      }
      else 
        Serial.println("Security Elements not activated");
    }    
*/
    Serial.println("Connect WiFi");  
//    delay(4000);
    establishWifiConnection(myWebServer,dnsServer);
    if (lnMQTT)
    {
      Serial.println("Connect MQTT");  
      establishMQTTConnection();
      Serial.println("Connect MQTT done");  
    }
    if (jsonConfigObj->containsKey("useNTP"))
    {
      useNTP = (bool)(*jsonConfigObj)["useNTP"];
      if (useNTP)
      {
        Serial.println("Create NTP Time Access");  
        JsonObject ntpConfig = (*jsonConfigObj)["ntpConfig"];
        if (ntpConfig.containsKey("NTPServer"))
          strcpy(ntpServer, ntpConfig["NTPServer"]);
        if (ntpConfig.containsKey("ntpTimeZone"))
          if (ntpConfig["ntpTimeZone"].is<const char*>())
            strcpy(ntpTimeZone, ntpConfig["ntpTimeZone"]);
          else
            Serial.println("ntpTimeZone is wrong data type");
        configTime(0, 0, ntpServer);
        setenv("TZ", ntpTimeZone, 1);
      }
      else 
        Serial.println("NTP Module not activated");
    }
    delete(jsonConfigObj);
    if (useNTP) getInternetTime();
    startWebServer();
    Serial.println(String(ESP.getFreeHeap()));
  }
  randomSeed((uint32_t)ESP.getEfuseMac()); //initialize random generator with MAC
}

void loop() {
  // put your main code here, to run repeatedly:
#ifdef measurePerformance
  loopCtr++;
  if (millis() > myTimer)
  {
    Serial.printf("Timer Loop: %i\n", loopCtr);
    loopCtr = 0;
    myTimer += 1000;
  }
#endif  

//  if (myButtons) myButtons->processButtons(); //checks if a button was pressed and sends button messages
  if (myChain) myChain->processChain(); //updates all LED's based on received status information for switches, inputs, buttons, etc.
//  if (secElHandlerList) secElHandlerList->processLoop(); //calculates speeds in all blocks and sets signals accordingly
  if (myDcc) myDcc->process(); //receives and decodes track signals
  if (buttonHandler) buttonHandler->processButtonHandler(); //drives the outgoing buffer and time delayed commands
  if (lnSerial) lnSerial->processLoop(); //handling all LocoNet communication

  checkWifiTimeout(); //checks if wifi has been inactive and disables it after timeout
  if (wifiCfgMode == 1) //STA active, Internet connection
  {
    if (!wifiCancelled) //handles keep alive updates as long connection is valid
    {
      if (WiFi.status() == WL_CONNECTED)
      { 
        sendKeepAlive();
        if (useNTP)
          getInternetTime(); //gets periodic updates of date and time from NTP server
      }
      else
      {
        Serial.println("Reconnect WiFi");
        establishWifiConnection(myWebServer,dnsServer);
      }
    }
    if ((!wifiCancelled) && ((commGateway) || (lnMQTT))) //handles all wifi communication for MQTT
      if (WiFi.status() == WL_CONNECTED)
      { 
        if (commGateway) 
          commGateway->processLoop();
        else
          if (lnMQTT) 
            lnMQTT->processLoop();
      }
      else
      {
        Serial.println("Reconnect WiFi");
        establishWifiConnection(myWebServer,dnsServer);
      }
  }
  else
    sendKeepAlive();

  M5.update();
  processDisplay();

  processBufferUpdates(); //updating DigitraxBuffers by querying information from LocoNet, e.g. slot statuses
}
