uint8_t BBVersion[] = {1,3,0};

//Arduino published libraries. Install using the Arduino IDE or download from Github and install manually
#include <arduino.h>
#include <Math.h>
#include <M5StickC.h>
#include <ArduinoUniqueID.h>
#include <time.h>
#include <FS.h>
#include <SPIFFS.h>
#define FORMAT_SPIFFS_IF_FAILED true
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <ESPAsyncWiFiManager.h>         //https://github.com/alanswx/ESPAsyncWiFiManager
#include <ArduinoJson.h> //standard JSON library, can be installed in the Arduino IDE. Make sure to use version 6.x

//following libraries can be downloaded from https://github.com/tanner87661?tab=repositories
#include <IoTT_DigitraxBuffers.h> //as introduced in video # 30
#include <IoTT_LocoNetHBESP32.h> //this is a hybrid library introduced in video #29
#include <IoTT_MQTTESP32.h> //as introduced in video # 29
#include <IoTT_Gateway.h> //LocoNet Gateway as introduced in video # 29
#include <IoTT_RemoteButtons.h> //as introduced in video # 29, special version for Wire connection via 328P
#include <IoTT_LocoNetButtons.h> //as introduced in video # 29
#include <IoTT_LEDChain.h> //as introduced in video # 30
#include <NmraDcc.h> //install via Arduino IDE
#include <OneDimKalman.h>

//library object pointers. Libraries will be dynamically initialized as needed during the setup() function
AsyncWebServer * myWebServer = NULL; //(80)
DNSServer * dnsServer = NULL;
//WiFiClientSecure * wifiClientSec = NULL;
WiFiClient * wifiClient = NULL;
AsyncWebSocket * ws = NULL; //("/ws");
AsyncWebSocketClient * globalClient = NULL;
uint16_t wsReadPtr = 0;
char * wsBuffer; //[32768]; //should this by dynamic?

//global variables
bool useStaticIP = false;
IPAddress static_ip;
IPAddress static_gw;
IPAddress static_nm;
IPAddress static_dns;
IPAddress ap_ip;
IPAddress ap_nm(255,255,255,0);

uint8_t wifiCfgMode = 0x00; //1: STA, 2: AP, 3: STA+AP
String deviceName; //device name used for AP, will load from node.cfg
String apPassword; //AP password, will load from node.cfg

bool wifiAlwaysOn = false; //set to false to shut Wifi after some time of inactivity. Gateway and MQTT should be disabled, though
bool wifiCancelled = false; //true if Wifi was switched off due to no use
uint32_t lastWifiUse = millis();
#define wifiShutTimeout 120000 //after 2 Mins of not using, Wifi is closed
#define apShutTimeout 180000 //after 3 Mins of not using, AP is shut. 3 Min to give enough time to connect to the AP from a PC

String bufferFileName = "/buffers.dat";

//more library object pointers. Libraries will be dynamically initialized as needed during the setup() function
IoTT_Mux64Buttons * myButtons = NULL;
IoTT_LocoNetButtonList * buttonHandler = NULL; 
LocoNetESPSerial * lnSerial = NULL;
ln_mqttGateway * commGateway = NULL;
IoTT_ledChain * myChain = NULL;
MQTTESP32 * lnMQTT = NULL;
NmraDcc  * myDcc = NULL;

//Adafruit_MCP23017 mcp;

//some variables used for performance measurement
#ifdef measurePerformance
uint16_t loopCtr = 0;
uint32_t myTimer = millis() + 1000;
#endif

//********************************************************Hardware Configuration******************************************************
#define groveRxD 33 //pin 1 yellow RxD for LocoNet, DCC
#define groveTxD 32 //pin 2 white TxD for LocoNet, DCC
constexpr gpio_num_t groveRxCAN = GPIO_NUM_33; //for OpenLCB CAN
constexpr gpio_num_t groveTxCAN = GPIO_NUM_32; //for OpenLCB CAN
 
#define stickLED 10 //red LED on stick
#define hatSDA 26 //changes between input and output by I2C master
#define hatSCL 0 //pull SCL low while sending non I2C data over SDA
#define hatRxD 0
#define hatTxD 26
#define hatInputPin 36
#define hatDataPin 26 //used to send LED data from FastLED. First, set to output low, then call show
#define hatWireAddr 0x18 //I2C Address used on hats with I2C slave 328P
uint8_t hatData[] = {1, hatDataPin}; //pin definition for LED library
uint8_t analogPins[] = {hatInputPin, 2}; //analog pin used for button reading, number of button MUX (connected via I2C)

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
              commGateway->lnWriteMsg(txData); 
            break;
  }
}

void resetPin(uint8_t pinNr)
{
  pinMode(pinNr, OUTPUT);
  digitalWrite(pinNr,0);
  pinMode(pinNr, INPUT);
}

void setup() {
  wsBuffer = (char*) malloc(16384); 
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
  //verify some library settings
  if (MQTT_MAX_PACKET_SIZE < 480)
  {
    Serial.printf("PubSubClient.h MQTT_MAX_PACKET_SIZE only %i. Should be >= 480\n", MQTT_MAX_PACKET_SIZE);
    delay(5000);
    ESP.restart(); //configuration update requires restart to be sure dynamic allocation of objects is not messed up
  }
  DynamicJsonDocument * jsonConfigObj = NULL;
  DynamicJsonDocument * jsonDataObj = NULL;
  jsonConfigObj = getDocPtr("/configdata/node.cfg"); //read and decode the master config file. See ConfigLoader tab
  if (jsonConfigObj != NULL)
  {
    //first, read all Wifi Paramters
    if (jsonConfigObj->containsKey("wifiMode"))
      wifiCfgMode = (*jsonConfigObj)["wifiMode"];
    Serial.printf("Wifi Config Mode %i\n",wifiCfgMode);
    if (jsonConfigObj->containsKey("devName"))
    {
      String thisData = (*jsonConfigObj)["devName"];
      deviceName = thisData;
    }
    if ((*jsonConfigObj)["inclMAC"])
    {
      deviceName += '_';
      deviceName += String((uint32_t)ESP.getEfuseMac());
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
      strcpy(useHat.devName, devTypes[listIndex]["Name"]);
      useHat.devId = devTypes[listIndex]["HatId"];
      JsonArray IntfList = devTypes[listIndex]["InterfaceList"];
      if (IntfList)
      {
        useHat.validInterfaces = (uint8_t*) realloc (useHat.validInterfaces, IntfList.size() * sizeof(uint8_t));
        useHat.numValid = IntfList.size();
        for (int i = 0; i < IntfList.size(); i++)
          useHat.validInterfaces[i] = IntfList[i];
      }
      useHat.devCommMode = devTypes[listIndex]["Type"];
    }
    else
    {
      Serial.println("No HAT defined, go with none");
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
      strcpy(useInterface.devName, commTypes[listIndex]["Name"]);
      useInterface.devCommMode = commTypes[listIndex]["Type"];
      useInterface.devId = commTypes[listIndex]["IntfId"];
    }
    else
    {
      Serial.println("No Interface defined, use LocoNet");
      strcpy(useHat.devName, "LocoNet");
      useInterface.devCommMode = 1;
      useInterface.devId = 2;
    }
    //verify that hat and interface go together
    bool interfaceOK = false;
    for (int i = 0; i < useHat.numValid; i++) // is array of uint8_t
    {
      if (useInterface.devId == useHat.validInterfaces[i])
      {
        interfaceOK = true;
        break;
      }
    }

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
      lnSerial = new LocoNetESPSerial(); //UART2 by default
      lnSerial->begin(groveRxD, groveTxD, true, true); //true is inverted signals on Rx, Tx
      lnSerial->setBusyLED(stickLED, false);
      lnSerial->setLNCallback(callbackLocoNetMessage);
    } 
    else 
      Serial.println("LocoNet not activated");

    if ((useInterface.devId == 3) || (useInterface.devId == 4) || (useInterface.devId == 6) || (useInterface.devId == 7))//MQTT or LN Gateway/ALM or OpenLCB with Gateway
    {
      Serial.println("Init MQTT");  
      jsonDataObj = getDocPtr("/configdata/mqtt.cfg");
      if (jsonDataObj != NULL)
      {
        lnMQTT = new MQTTESP32(*wifiClient); 
        lnMQTT->loadMQTTCfgJSON(*jsonDataObj);
        switch (useInterface.devId)
        {
          case 3:;
          case 4: lnMQTT->setMQTTCallback(callbackLocoNetMessage); break;
        }
        wifiAlwaysOn = true;
        delete(jsonDataObj);
      }
    }
    else 
      Serial.println("MQTT not activated");

    if ((useInterface.devId == 4) || (useInterface.devId == 7)) // && (modMode == 1))) //LocoNet or OIpenLCB Gateway/ALM
    {
      Serial.println("Load Gateway");  
      switch (useInterface.devId)
      {
        case 4: commGateway = new ln_mqttGateway(lnSerial, lnMQTT, &callbackLocoNetMessage); break;
      }
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
        
    if (useHat.devId == 1) //BlueHat or CTC Hat
    {
      {
        Serial.println("Load BlueHat Data"); 
        jsonDataObj = getDocPtr("/configdata/led.cfg");
        if (jsonDataObj != NULL)
        {
          myChain = new IoTT_ledChain(); // ... construct now, and call setup later
          myChain->loadLEDChainJSON(*jsonDataObj);
          delete(jsonDataObj);
          uint16_t subFileCtr = 1;
          while (SPIFFS.exists("/configdata/led" + String(subFileCtr) + ".cfg"))
          {
            jsonDataObj = getDocPtr("/configdata/led" + String(subFileCtr) + ".cfg");
            if (jsonDataObj)
            {
              myChain->loadLEDChainJSON(*jsonDataObj, false);
              delete(jsonDataObj);
            }
            subFileCtr++;
          }
          Serial.printf("Init LED Chain on Pin %i, %i LEDs long\n", hatDataPin, myChain->getChainLength());
            if (myChain->colTypeNum == 0x66)
            {
              FastLED.addLeds<WS2811, hatDataPin, GRB>(myChain->getChain(), myChain->getChainLength()); 
            }
            if (myChain->colTypeNum == 0x0C)
            {
              FastLED.addLeds<WS2811, hatDataPin, RGB>(myChain->getChain(), myChain->getChainLength()); 
            }
        }
      }
    }
    else 
      Serial.println("BlueHat not activated");
    if ((useHat.devId == 3) || (useHat.devId == 4)) //YellowHat or GreenHat
    {
        Serial.println("Init YellowHat");  
        Wire.begin(hatSDA, hatSCL, 400000); //initialize the I2C interface
        jsonDataObj = getDocPtr("/configdata/led.cfg");
        if (jsonDataObj != NULL)
        {
          Serial.println("Load Yellow Hat LED Chain Data");  
          myChain = new IoTT_ledChain(&Wire, hatWireAddr); // set for using I2C Bus address 0x18
          myChain->loadLEDChainJSON(*jsonDataObj);
          delete(jsonDataObj);
          uint16_t subFileCtr = 1;
          while (SPIFFS.exists("/configdata/led" + String(subFileCtr) + ".cfg"))
          {
            jsonDataObj = getDocPtr("/configdata/led" + String(subFileCtr) + ".cfg");
            if (jsonDataObj)
            {
              myChain->loadLEDChainJSON(*jsonDataObj, false);
              delete(jsonDataObj);
            }
            subFileCtr++;
          }
          //myChain->initI2CLED();
        }
        else
          Serial.println("Yellow Hat no led chain defined");

        jsonDataObj = getDocPtr("/configdata/btn.cfg");
        if (jsonDataObj != NULL)
        {
          Serial.println("Load Yellow Hat Button Data");  
          myButtons = new IoTT_Mux64Buttons();
          myButtons->initButtons(&Wire, hatWireAddr, &analogPins[0], true); //use WiFi with buttons (always). ok for pin 36
          myButtons->loadButtonCfgJSON(*jsonDataObj);
          delete(jsonDataObj);
        }
        else
          Serial.println("Yellow Hat no buttons defined");

    }
    else 
      Serial.println("YellowHat not activated");
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
        uint16_t subFileCtr = 1;
        while (SPIFFS.exists("/configdata/btnevt" + String(subFileCtr) + ".cfg"))
        {
          jsonDataObj = getDocPtr("/configdata/btnevt" + String(subFileCtr) + ".cfg");
          if (jsonDataObj)
          {
            buttonHandler->loadButtonCfgJSON(*jsonDataObj, false);
            delete(jsonDataObj);
          }
          subFileCtr++;
        }
      }
    }
    else 
      Serial.println("Button Handler not activated");
    Serial.println("Connect WiFi");  
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
      if ((useNTP) && (wifiCfgMode == 1))
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
  if (myDcc) myDcc->process(); //receives and decodes track signals
  if (buttonHandler) buttonHandler->processButtonHandler(); //drives the outgoing buffer and time delayed commands
  if (lnSerial) lnSerial->processLoop(); //handling all LocoNet communication
  if (myButtons) myButtons->processButtons(); //checks if a button was pressed and sends button messages
  if (myChain) myChain->processChain(); //updates all LED's based on received status information for switches, inputs, buttons, etc.

  checkWifiTimeout(); //checks if wifi has been inactive and disables it after timeout
  if ((wifiCfgMode == 1))// || (wifiCfgMode == 3)) //STA active, Internet connection
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
