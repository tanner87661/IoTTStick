#include <Arduino.h>
#include <SPIFFS.h>
#include <M5Unified.h>
#include <M5Dial.h>
#include <math.h>
#include <Wire.h>
#include <WiFi.h>
#include <ArduinoUniqueID.h>
#include <time.h>
#include <FS.h>
#define FORMAT_SPIFFS_IF_FAILED true
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <ESPAsyncWiFiManager.h>         //https://github.com/alanswx/ESPAsyncWiFiManager
#include <ArduinoJson.h> //standard JSON library, can be installed in the Arduino IDE. Make sure to use version 6.x
#include <TJpg_Decoder.h>

#include <IoTT_CommDef.h>

#define PowerSys M5.Power.Axp2101
#define USBSerial Serial
String BBVersion = "0.1";

AsyncWebServer * myWebServer = NULL; //(80)
DNSServer * dnsServer = NULL;
//WiFiClientSecure * wifiClientSec = NULL;
WiFiClient * wifiClient = NULL;
AsyncWebSocket * ws = NULL; //("/ws");
//AsyncWebSocketClient * globalClient = NULL;
std::vector<wsClientInfo> globalClients; // a list to hold all clients when in server mode

String configDir = "/configdata";
String configExt = "cfg";
String configDotExt = ".cfg";

uint32_t wsBufferSize = 16384;
uint32_t wsRxReadPtr = 0;
uint32_t wsRxWritePtr = 0;
char * wsRxBuffer; //[16384, 32768]; //should this be dynamic?
uint32_t wsTxReadPtr = 0;
uint32_t wsTxWritePtr = 0;
char * wsTxBuffer; //[32768]; //should this be dynamic?

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

//global variables for the NTP module
int ntpTimeout = 5000; //ms timeout for NTP update request
char ntpServer[50] = "us.pool.ntp.org"; //default server for US. Change this to the best time server for your region, or set in node.cfg
char ntpTimeZone[100] = "EST5EDT";  // default for Eastern Time Zone. Enter your time zone from here: (https://remotemonitoringsystems.ca/time-zone-abbreviations.php) into node.cfg
bool ntpOK = false;
bool useNTP = false;
bool hatVerified = false;
tm timeinfo;
time_t now;
const uint32_t ntpIntervallDefault = 86400000; //1 day in milliseconds
const uint32_t ntpIntervallShort = 10000; //10 Seconds in case something went wrong
uint32_t ntpTimer = millis();
int millisRollOver = 0;
unsigned long lastMillis = 0;
//strDateime dateTime;

uint8_t m5CurrentPage = 0;
uint16_t oldWifiStatus = 0;
uint8_t useM5Viewer = 0;
uint8_t m5DispLine = 0;

bool darkScreen = false;
bool hatPresent = false;
bool pwrUSB = false;
bool pwrDC = false;

uint8_t wifiMode = 0;
uint8_t wifiStatus = 0;
bool dataValid = false;

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap);

void setup() 
{
  wsRxBuffer = (char*) malloc(wsBufferSize); 
  wsTxBuffer = (char*) malloc(wsBufferSize); 
  auto cfg = M5.config();
  cfg.serial_baudrate = 115200;   // default=115200. if "Serial" is not needed, set it to 0.
  cfg.clear_display = true;  // default=true. clear the screen when begin.
  cfg.output_power  = true;  // default=true. use external port 5V output.
  cfg.internal_imu  = true;  // default=true. use internal IMU.
  cfg.internal_rtc  = true;  // default=true. use internal RTC.
  cfg.internal_spk  = false;  // default=true. use internal speaker.
  cfg.internal_mic  = true;  // default=true. use internal microphone.
  cfg.external_imu  = false;  // default=false. use Unit Accel & Gyro.
  cfg.external_rtc  = false;  // default=false. use Unit RTC.
  cfg.external_spk  = false; // default=false. use SPK_HAT / ATOMIC_SPK
  cfg.led_brightness = 0;   // default= 0. system LED brightness (0=off / 255=max) (※ not NeoPixel)
//  M5Dial.begin(cfg);
  M5Dial.begin(cfg, true, false);
  USBSerial.begin(115200);
  delay(100);
  SPIFFS.begin(); //File System. Size is set to 1 MB during compile time and loaded with configuration data and web pages
  delay(100);
  initDisplay();

  UniqueIDdump(USBSerial);
  myWebServer = new AsyncWebServer(80);
  dnsServer = new DNSServer();
  wifiClient = new WiFiClient();
  uint16_t wsRequest = 0;
  delay(500);
  ws = new AsyncWebSocket("/ws");
  DynamicJsonDocument * jsonConfigObj = NULL;
  DynamicJsonDocument * jsonDataObj = NULL;
  jsonConfigObj = getDocPtr("/configdata/node.cfg", true); //read and decode the master config file. See ConfigLoader tab. Make data copy
  if (jsonConfigObj != NULL)
  {
    //first, read all Wifi Paramters
    if (jsonConfigObj->containsKey("wifiMode"))
      wifiCfgMode = (*jsonConfigObj)["wifiMode"];

    USBSerial.printf("Wifi Config Mode %i\n",wifiCfgMode);
//    if (jsonConfigObj->containsKey("useWifiTimeout"))
//      wifiAlwaysOn =  (!(bool)(*jsonConfigObj)["useWifiTimeout"]);
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

    //load the interface type

  }
  USBSerial.println("Connect WiFi");  
  establishWifiConnection(myWebServer,dnsServer);
  delay(1000);    
  startWebServer();
}

void loop() 
{
  M5Dial.update();
  processDisplay();
//  if (M5Dial.BtnA.wasPressed()) 
//    USBSerial.println("Button A");
    if (dataValid)
    {
      checkWifiTimeout(); //checks if wifi has been inactive and disables it after timeout
      if ((wifiCfgMode == 1))// || (wifiCfgMode == 3)) //STA active, Internet connection
      {
        if (!wifiCancelled) //handles keep alive updates as long connection is valid
        {
          if (wifiStatus == WL_CONNECTED)
          { 
            sendKeepAlive();
            if (useNTP)
              getInternetTime(); //gets periodic updates of date and time from NTP server
          }
          else
          {
            Serial.println("Reconnect WiFi");
            establishWifiConnection(myWebServer,dnsServer);
            startWebServer();
          }
        }
      }
      else
        sendKeepAlive();
    }
}
