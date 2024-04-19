//jpeg functions: https://github.com/pix3lize/M5Stick-ImageViewerSPIFFS/blob/master/src/main.cpp

//display size C 80 x 160
//display size C Plus 135 * 240
//display size C Plus2 135 * 240

uint8_t screenDef = 1; //0 for Stick; 1 for Stick C Plus
uint8_t lineY_0[] = {3,18,33,48,63};
uint8_t lineY_1[] = {3,15,27,39,51,63};


uint16_t getXCoord(uint16_t ofValue)
{
  switch (screenDef)
  {
    case 1: return round(1.5 * ofValue);
    default: return ofValue;
  }
}

uint16_t getYCoord(uint16_t ofValue)
{
  switch (screenDef)
  {
    case 1: return round(1.6875 * ofValue);
    default: return ofValue;
  }
}

#define pwrOffTimeout 30000
#define pwrDispInterval 2000
#define speedDispInterval 500
#define pwrRefreshInterval 500
#define dccStrLen 45

bool dccOneTimeDisp = false;
uint32_t pwrOffTimer = millis();
uint32_t pwrDispTimer = millis();
uint32_t dccDispTimer = millis();
uint32_t pwrRefreshTimer = millis();

float axpVBUSVoltage = 0;
//float axpInVoltage = 0;


float axpIntTemp = 0;
float axpBattChargeCurrent = 0;
float axpBattDischargeCurrent = 0;

float axpBattVoltage = 0;
float axpVBUSCurrent = 0;
float axpACInCurrent = 0;
float axpACInVoltage = 0;
float axpBattPower = 0;
uint8_t axpBattLevel = 0;
float axpBattCurr = 0;
uint8_t pwrRefreshCtr = 0;
//uint32_t startmeas;

uint8_t wifiResetCtr = 0;
uint32_t wifiResetLastClick = 0;
#define wifiResetMaxDelay 2000
#define wifiResetReqCount 2

//uint8_t btnBClickCount = 0;
//uint8_t btnCClickCount = 0;

char dispBuffer[oneShotBufferSize][dccStrLen];

bool isOneTime()
{
  return dccOneTimeDisp;
}

void getRTCTime()
{
  m5::rtc_datetime_t dt;
  M5.Rtc.getDateTime(&dt);
  {
    timeinfo.tm_hour = dt.time.hours;
    timeinfo.tm_min =  dt.time.minutes;
    timeinfo.tm_sec = dt.time.seconds;
    timeinfo.tm_wday = dt.date.weekDay & 7;
    timeinfo.tm_mon = dt.date.month - 1;
    timeinfo.tm_mday = dt.date.date;
    timeinfo.tm_year = dt.date.year - 1900;
    now = mktime(&timeinfo);
    struct timeval thisTimeVal = { .tv_sec = now };
    settimeofday(&thisTimeVal, NULL);  
  }
}

void btnADown()
{
//  Serial.println("Button A Down");
}

void btnAUp()
{
//  Serial.println("Button A Up");
}

void btnAClick()
{
//  Serial.println("Button A Clicked");
//  Serial.println(m5CurrentPage);
  pwrOffTimer = millis();
  if (darkScreen)
  {
    M5.Display.setBrightness(255);//0-255
    darkScreen = false;
  }
  else
  {
    if (m5CurrentPage != 5)
      useM5Viewer = 0;
    if (m5CurrentPage == 7)
      m5CurrentPage = 3;
    else
      m5CurrentPage++;
  }
  switch (m5CurrentPage)
  {
    case 0:
//      if (millis() > scrDispTimer)      
//      { 
        setOpeningPage();
//        scrDispTimer = millis() + scrRefreshInterval;
//      }
      break;
    case 1:
//      if (millis() > scrDispTimer)      
//      { 
        setWifiConnectPage();
//        scrDispTimer = millis() + scrRefreshInterval;
//      }
      break;
    case 2:
      m5CurrentPage++;
      useM5Viewer = 0;
    case 3:
      oldWifiStatus = 0xFFFF; //impossible mode, requires update
      break;
    case 4:
//      if (millis() > scrDispTimer)      
//      { 
        setStatusPage();
//        scrDispTimer = millis() + scrRefreshInterval;
//      }
      break;
    case 5:
      if (millis() > pwrDispTimer)
      {
        setPwrStatusPage(); 
        pwrDispTimer = millis() + pwrDispInterval;
      }
//      pwrDispTimer = millis() + pwrDispInterval;
//      setPwrStatusPage();
      break;
    case 6:
      if (useHat.devId == 7) //PurpleHat
      {
        pwrDispTimer = millis() + speedDispInterval;
        sensorViewerPage();
        break;
      } //else go to 7
      else
        m5CurrentPage = 7; //keep going, no break
    case 7:
      switch (useInterface.devId)
      {
        case 17:
        case 18:
        case 0: useM5Viewer = 0; m5CurrentPage = 3; oldWifiStatus = 0xFFFF; break;
        case 1:;
        case 10: dccViewerPage(); break;
        
        case 2:;
        case 3:;
        case 4:;
        case 11:;
        case 12:;
        case 13:;
        case 14:;
        case 15:;
        case 16: lnViewerPage(); break;
         
        case 5:;
        case 6:;
        case 7: olcbViewerPage(); break;
        case 8: mqttViewerPage(); break;
      }
      clearDisplay();
      m5DispLine = 0;
      break;
  }
}

void btnADblClick(uint8_t evtCtr)
{
//  Serial.printf("Button A %i Double Clicked\n", evtCtr);
}

void btnAOnHold(uint8_t evtCtr)
{
//  Serial.printf("Button A %i Hold\n", evtCtr);
}

void btnBDown()
{
//  Serial.println("Button B Down");
}

void btnBUp()
{
//  Serial.println("Button B Up");
}

void btnBClick()
{
//  Serial.printf("Button B Clicked Page %i Wifi\n", m5CurrentPage);
  pwrOffTimer = millis();
  switch (useHat.devId)
  {
    case 6://RedHat
    {
      if (m5CurrentPage == 3)
        if (wifiStatus != WL_CONNECTED)
        {
          Serial.println("Connect WiFi");  
          establishWifiConnection(myWebServer,dnsServer);
          return;
        }
//      Serial.println("Set Power Mode");
      uint8_t currPwr = digitraxBuffer->getPowerStatus();
      switch (currPwr)
      {
        case 0: //off
          currPwr = 0x83;
          break;
        case 1: //on
          currPwr = 0x85;
          break;
        case 2: //idle
          currPwr = 0x83;
          break;
        default: // Serial.println("Invalid Power Status. Set to Idle.");
          currPwr = 0x83;
          break;
      }
      digitraxBuffer->localPowerStatusChange(currPwr);
      return;
    }
    case 7: //PurpleHat
      if (trainSensor) trainSensor->resetDistance();
      return;
    case 8: //SilverHat
    {
//          Serial.println(wifiStatus);  
      if (m5CurrentPage == 3)
        if (wifiStatus != WL_CONNECTED)
        {
          Serial.println("Connect WiFi");  
          establishWifiConnection(myWebServer,dnsServer);
          return;
        }
      if (usbSerial)
        usbSerial->toggleBoosterOutput();
      return;
    }
  }
    
  switch (m5CurrentPage)
  {
    case 7: //DCC Viewer active, toggle status
      if (useM5Viewer == 2) //DCC, so toggle mode
      {
        dccOneTimeDisp = !dccOneTimeDisp;
        dccViewerPage();
      }
      if (useM5Viewer == 4) //MQTT, so toggle mode
      {
        dccOneTimeDisp = !dccOneTimeDisp;
        clearDisplay();
        mqttViewerPage();
      }
      break;
    case 3: //Wifi Status page active
      if (wifiMode == 0)
      {
        Serial.println("Connect WiFi");  
        establishWifiConnection(myWebServer,dnsServer);
      }
      break;
  }
}

void btnBDblClick(uint8_t evtCtr)
{
//  Serial.printf("Button B %i Double Clicked\n", evtCtr);
  if (useHat.devId == 6) //RedHat Shield
    if(evtCtr == 2) 
        if (digitraxBuffer->getPowerStatus() != 0)
          digitraxBuffer->localPowerStatusChange(0x82);
}

void btnBOnHold(uint8_t evtCtr)
{
//  Serial.printf("Button B %i Hold\n", evtCtr);
}

void btnCClick()
{
//  Serial.println("Button C Clicked");
}

void btnCDblClick(uint8_t evtCtr)
{
//  Serial.printf("Button C %i Double Clicked\n", evtCtr);
  pwrOffTimer = millis();
  if (evtCtr > wifiResetReqCount)
  {
    Serial.println("Reset Wifi");
    if (wifiStatus == WL_CONNECTED)
      WiFi.disconnect();
    WiFi.begin("0","0");
    delay(100);
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    Serial.println("Wifi Credentials deleted, Restart IoTT Stick");  
    delay(100);
    ESP.restart();
  }
}

void btnCOnHold(uint8_t evtCtr)
{
  Serial.println("Button C Hold");
  pwrOffTimer = millis();
  prepareShutDown(); //could be on the way to power off, so we save the data to SPIFFS
}

void processDisplay()
{
  int state = M5.BtnA.wasHold() ? 1
        : M5.BtnA.wasClicked() ? 2
        : M5.BtnA.wasPressed() ? 3
        : M5.BtnA.wasReleased() ? 4
        : M5.BtnA.wasDecideClickCount() ? 5
        : 0;
  switch (state)
  {
    case 3: btnAClick(); break;
  }

  state = M5.BtnB.wasHold() ? 1
        : M5.BtnB.wasClicked() ? 2
        : M5.BtnB.wasPressed() ? 3
        : M5.BtnB.wasReleased() ? 4
        : M5.BtnB.wasDecideClickCount() ? 5
        : 0;
  switch (state)
  {
    case 3: btnBClick(); break;
    case 5: btnBDblClick(M5.BtnB.getClickCount());
            break;
  }

  state = M5.BtnPWR.wasHold() ? 1
        : M5.BtnPWR.wasClicked() ? 2
        : M5.BtnPWR.wasPressed() ? 3
        : M5.BtnPWR.wasReleased() ? 4
        : M5.BtnPWR.wasDecideClickCount() ? 5
        : 0;

  switch (state)
  {
    case 1: btnCOnHold(0); break;
//    case 3: btnCClick(); btnCClickCount++;  break;
    case 5: btnCDblClick(M5.BtnPWR.getClickCount());
//            btnCClickCount = 0;
            break;
  }

  if (pwrRefreshTimer < millis())
  {
    switch (pwrRefreshCtr)
    {
//      case 1: startmeas = millis();
      #if defined StickPlus
        case 2: axpVBUSVoltage = PowerSys.getVBUSVoltage(); break;
        case 4: axpACInVoltage = PowerSys.getACINVoltage(); break;
        case 6: axpIntTemp = PowerSys.getInternalTemperature(); break;
        case 8: axpBattChargeCurrent = PowerSys.getBatteryChargeCurrent(); break;
        case 10: axpBattDischargeCurrent = PowerSys.getBatteryDischargeCurrent(); break;
        case 12: axpBattChargeCurrent = PowerSys.getBatteryChargeCurrent(); break;
        case 14: axpBattVoltage = PowerSys.getBatteryVoltage(); break;
        case 16: axpVBUSCurrent = PowerSys.getVBUSCurrent(); break;
        case 18: axpACInCurrent = PowerSys.getACINCurrent(); break;
        case 20: axpBattPower = PowerSys.getBatteryPower(); break;
        case 22: axpBattLevel = PowerSys.getBatteryLevel(); break;
      #endif
      #if defined StickPlus2
        case 6: 
        {        auto imu_update = M5.Imu.update();
                 M5.Imu.getTemp(&axpIntTemp); 
                 break;
        }
        case 14: axpBattVoltage = (float)M5.Power.getBatteryVoltage()/1000; break;
        case 22: axpBattLevel = M5.Power.getBatteryLevel(); break;
      #endif
      case 24: wifiMode = WiFi.getMode(); break; 
      case 26: wifiStatus = WiFi.status(); break;
      case 30:
          axpBattCurr = axpBattChargeCurrent == 0? - axpBattDischargeCurrent : axpBattChargeCurrent;
          hatPresent = axpACInVoltage > 0.5;
          #if defined StickPlus
            pwrUSB = axpVBUSVoltage > 4.5;
            pwrDC = axpACInVoltage > 4.7;
          #endif
          #if defined StickPlus2
            pwrUSB = axpBattVoltage > 3.5; //M5.Power.isCharging();
            pwrDC = axpBattVoltage > 3.5; //M5.Power.isCharging();
          #endif
          pwrRefreshTimer += pwrRefreshInterval;
          pwrRefreshCtr = 0;
          dataValid = true;
//          Serial.printf("Duration: %i\n", millis() - startmeas);
          break;
      default: break;
    }
    pwrRefreshCtr++;
  }

//  if (!dataValid)
  if ((!dataValid) || (pwrRefreshCtr > 1))
    return;
    
  if (pwrUSB || pwrDC) //check for Power Status, but not for BlackHat
  {
    pwrOffTimer = millis();
    if (darkScreen)
    {
      darkScreen = false;
//      Serial.println("Set bright");
      M5.Display.setBrightness(255); //0-255
    } 
  }
  else
    if ((pwrOffTimer + pwrOffTimeout) < millis()) //power off after timeout without power. Device will come on automatically when power is back
    {
      if ((useHat.devId == 5) || (useHat.devId == 7))//BlackHat or PurpleHat, do not power down
      {
        if ((useHat.devId == 5) && (!darkScreen)) //darkScreen only for BlackHat
        {
          Serial.println("Set dark");
          M5.Display.setBrightness(0);
          darkScreen = true;
        }
      }
      else
      {
        Serial.println("Shut down");
        prepareShutDown();
        M5.Power.powerOff();
      }
    }

  if (m5CurrentPage == 3) //in case status changes while displayed, we update the page
  {
    uint16_t currStatus = (wifiMode << 8) + wifiStatus;
    if (currStatus != oldWifiStatus)
    {
//      Serial.printf("Status change %i %i %i %i\n", oldWifiStatus, currStatus, wifiMode, wifiStatus);
      oldWifiStatus = currStatus;
      switch (wifiMode)
      {
        case 0: setNoWifiPage(); break; //Wifi shut off
        case 1: // STA
          if (wifiStatus == WL_CONNECTED)
            setWifiPage(wifiMode);
          else
            setNoWifiPage();
          break;
        case 2: //AP
          if (!wifiCancelled)
            setWifiPage(wifiMode);
          else
            setNoWifiPage();
          break;
      }
//    Serial.println("Status change complete");
    }
  }

  if ((useInterface.devCommMode == 0))// && (useHat.devCommMode == 0)) //DCC/Decoder
    if (millis() > dccDispTimer)
    {
//      Serial.println("Refresh");
      sendRefreshBuffer();
      dccDispTimer = millis() + pwrDispInterval; //exact interval not important, we start with millis as base
    }

  if (m5CurrentPage == 5)
  {
    if (millis() > pwrDispTimer)
    {
      setPwrStatusPage(); 
      pwrDispTimer = millis() + pwrDispInterval;
    }
  }

  if ((m5CurrentPage == 6) && (useHat.devId == 7))
  {
    if (millis() > pwrDispTimer)
    {
      sensorViewerPage(); 
      pwrDispTimer = millis() + speedDispInterval;
    }
  }
}

void initDisplay()
{
  uint16_t devType = M5.getBoard(); //4: stick c 5: stick c plus
  M5.Display.setRotation(3);
  if (M5.Display.width() == 160)
    screenDef = 0;
  else
    screenDef = 1;
  // The jpeg image can be scaled by a factor of 1, 2, 4, or 8
  TJpgDec.setJpgScale(1);

  // The byte order can be swapped (set true for TFT_eSPI)
  TJpgDec.setSwapBytes(true);

  // The decoder must be given the exact name of the rendering function above
  TJpgDec.setCallback(tft_output);
  setOpeningPage();
}

void drawLogo(int x, int y, int logoSize)
{
  String fileName;
//  Serial.println("drawLogo");
  switch (screenDef)
  {
    case 0:  //Stick C
      switch (logoSize)
      {
        case 0 : fileName = "/www/iottlogo2020.jpg"; break;
        case 1 : fileName = "/www/iottlogo3030.jpg"; break;
        case 2 : fileName = "/www/iottlogo8080.jpg"; break;
      }
      break;
    case 1:  //Stick C plus
      switch (logoSize)
      {
        case 0 : fileName = "/www/iottlogo3030.jpg"; break;
        case 1 : fileName = "/www/iottlogo8080.jpg"; break;
        case 2 : fileName = "/www/iottlogo8080.jpg"; break;
      }
      break;
  }
  loadJpgFile(x,y,1, fileName.c_str());
}

void drawText(const char * payload, int x, int y, int what)
{
//  Serial.println("drawText");
  M5.Display.drawString(payload,getXCoord(x),getYCoord(y),what);
//  Serial.println("done");
}

void drawBackground()
{

  String fileName;
  switch (screenDef)
  {
    case 0: //Stick
      fileName = "/www/Nickel16080.jpg";
      break;
    case 1: //Stick Plus
      fileName = "/www/Nickel240135.jpg";
      break;
  }
  M5.Display.drawJpgFile(SPIFFS, fileName.c_str());//,0,0,240,240,0,0);
  loadJpgFile(0,0,1, fileName.c_str());
}

void setOpeningPage()
{
//  Serial.println("setOpeningPage");
  switch (screenDef)
  {
    case 0: 
      drawBackground();
      drawLogo(40, 00, 2); 
      break;
    case 1: 
      drawBackground();
      drawLogo(80, 25, 2); //80x80
      break;
  }
//  Serial.println("done");
}

void setWifiConnectPage()
{
//  Serial.println("setWifiConnectPage");
  char outText[50];
  M5.Display.fillScreen(TFT_LIGHTGREY);
  M5.Display.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  switch (screenDef)
  {
    case 0: 
      drawLogo(140, 60, 0);
      break;
    case 1: 
      drawLogo(210, 105, 0);
      break;
  }
  drawText("Connect to AP", 5, 3, 2);
  String hlpStr = "IoTT_Stick_M5_" + String((uint32_t)ESP.getEfuseMac());
  sprintf(outText, hlpStr.c_str());
  drawText(outText, 5, 18, 2);
  drawText("to set Wifi credentials", 5, 33, 2);
//  Serial.println("done");
}

void setWifiPage(int wifiCfgModePrm)
{
//  Serial.println("setWifiPage");
  uint8_t * lineY = screenDef == 0 ? &lineY_0[0] : &lineY_1[0];
  char outText[50];
  M5.Display.fillScreen(TFT_LIGHTGREY);
  M5.Display.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  switch (screenDef)
  {
    case 0: 
      drawLogo(140, 60, 0);
      break;
    case 1: 
      drawLogo(210, 105, 0);
      break;
  }
  switch (wifiCfgModePrm)
  {
    case 1:
    {
      sprintf(outText, "IoTT Stick: %s", deviceName.c_str());
      drawText(outText, 5, lineY[0], 2);
      drawText("Wifi LAN connected", 5, lineY[1], 2);
      sprintf(outText, "AP: %s", mySSID.c_str());
      drawText(outText, 5, lineY[2], 2);
      sprintf(outText, "IP: %s", dyn_ip.toString().c_str());
      drawText(outText, 5, lineY[3], 2);
      break;
    }
    case 2:
    {
      drawText("Local AP active", 5, lineY[0], 2);
      sprintf(outText, "AP: %s", deviceName.c_str());
      drawText(outText, 5, lineY[1], 2);
      sprintf(outText, "Password: %s", apPassword.c_str());
      drawText(outText, 5, lineY[2], 2);
      sprintf(outText, "IP: %s", ap_ip.toString().c_str());
      drawText(outText, 5, lineY[3], 2);
      break;
    }
  }
//  Serial.println("done");
}

void setNoWifiPage()
{
//  Serial.println("setNoWifiPage");
  uint8_t * lineY = screenDef == 0 ? &lineY_0[0] : &lineY_1[0];
  M5.Display.fillScreen(TFT_LIGHTGREY);
  M5.Display.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  switch (screenDef)
  {
    case 0: 
      drawLogo(65, 25, 1); //30x30
      break;
    case 1: 
      drawLogo(80, 25, 1); //80x80
      break;
  }
  drawText("Wifi LAN disconnected", 5, lineY[0], 2);
  drawText("Click here to connect", 5, 63, 2);
//  Serial.println("done");
}

//commMode: 0: DCC, 1: LocoNet, 2: MQTT, 3: Gateway
//workMode: 0: Decoder, 1: ALM

void setStatusPage()
{
//  Serial.println("setStatusPage");
  uint8_t * lineY = screenDef == 0 ? &lineY_0[0] : &lineY_1[0];
  M5.Display.fillScreen(TFT_LIGHTGREY);
  M5.Display.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
//  M5.Display.setTextSize(1);
  switch (screenDef)
  {
    case 0: 
      drawLogo(140, 60, 0);
      break;
    case 1: 
      drawLogo(210, 105, 0);
      break;
  }
  switch (useHat.devId)
  {
    case 0: drawText("Hat: None", 5, lineY[0], 2); break;
    case 1: drawText("Hat: BlueHat", 5, lineY[0], 2); break;
    case 2: drawText("Hat: USB Serial", 5, lineY[0], 2); break;
    case 3: drawText("Hat: YellowHat", 5, lineY[0], 2); break;
    case 4: drawText("Hat: GreenHat", 5, lineY[0], 2); break;
    case 5: drawText("Hat: BlackHat", 5, lineY[0], 2); break;
    case 6: drawText("Hat: RedHat++", 5, lineY[0], 2); break;
    case 7: drawText("Hat: PurpleHat", 5, lineY[0], 2); break;
    case 8: drawText("Hat: SilverHat", 5, lineY[0], 2); break;
    default: drawText("Hat: unknown", 5, lineY[0], 2); break;
  }
  switch (useInterface.devId)
  {
    case 0: drawText("No Interface", 5, lineY[1], 2); break;
    case 1: drawText("using DCC", 5, lineY[1], 2); break;
    case 2: drawText("using LocoNet", 5, lineY[1], 2); break;
    case 3: drawText("using LN over MQTT", 5, lineY[1], 2); break;
//    case 4: drawText("using LocoNet", 5, lineY[1], 2); break; //LN Gateway
//    case 5: drawText("using OpenLCB", 5, lineY[1], 2); break; 
//    case 6: drawText("using OpenLCB over MQTT", 5, lineY[1], 2); break; //OpenLCB 
//    case 7: drawText("using OpenLCB", 5, lineY[1], 2); break; //OpenLCB 
    case 8: drawText("using MQTT", 5, lineY[1], 2); break; //native MQTT 
//    case 9: drawText("using DCC to MQTT", 5, lineY[1], 2); break;
    case 10: drawText("using DCC from MQTT", 5, lineY[1], 2); break;
//    case 11: drawText("using LN/TCP(Server)", 5, lineY[1], 2); break;
    case 12: 
    {
      char outText[50];
//      String outStr = lbServer->getServerIP();
//      Serial.println(outStr);
      sprintf(outText, "using LN/TCP(Client)");// %s", outStr[);
      drawText(outText, 5, lineY[1], 2); break;
    }
    case 16: drawText("using LN Loopback", 5, lineY[1], 2); break;
    case 17: drawText("using WiThrottle", 5, lineY[1], 2); break;
    default: drawText("unknown", 5, lineY[1], 2); break;
  }
//  if ((useInterface.devId == 4) || (useInterface.devId == 7))
//    drawText("Gateway: active", 5, lineY[2], 2);
//  else
//    drawText("Gateway: not used", 5, lineY[2], 2);

  drawText("Servers:", 5, lineY[2], 2);
  String modList = "[";

  if (lnMQTTServer)
    modList += "MQTT";
  if (lbServer)
  {
    if (modList.length() > 1)
      modList += ", ";
    modList += "TCP";
  }
  if (wiThServer)
  {
    if (modList.length() > 1)
      modList += ", ";
    modList += "WiTh";
  }
  if (lnSubnet)
  {
    if (modList.length() > 1)
      modList += ", ";
    modList += "SbNet";
  }
  
  modList += "]";
  drawText(&modList[0], 50, lineY[2], 2); 

  drawText("Modules:", 5, lineY[3], 2);
  modList = "[";
  if (eventHandler)
    modList += "Evnt Hdlr";
#ifdef useAI
  if (voiceWatcher)
  {
    if (modList.length() > 1)
      modList += ", ";
    modList += "Voice";
  }
#endif  
/*
if (secElHandlerList)
  {
    if (modList.length() > 1)
      modList += ", ";
    modList += "Sec El";
  }
*/  
//  if (sensorIntegrHandlerList)
//  {
//    if (modList.length > 1)
//      modList += ", ";
//    modList += "Sensor Chain";
//  }
  modList += "]";
  drawText(&modList[0], 50, lineY[3], 2); 
//  Serial.println("done");
}

void setPwrStatusPage()
{
//  Serial.println("setPwrStatusPage");
  char outText[50];
  M5.Display.fillScreen(TFT_LIGHTGREY);
  M5.Display.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  switch (screenDef)
  {
    case 0: 
      drawLogo(140, 0, 0);
      break;
    case 1: 
      drawLogo(210, 0, 0);
      break;
  }
  sprintf(outText, "IoTT Stick V. %s", BBVersion.c_str());
  drawText(outText, 5, 3, 2);
  sprintf(outText, "Stick Temp: %.1f C \n", axpIntTemp);
  drawText(outText, 5, 20, 1);
  #ifdef StickPlus
  sprintf(outText, "Bat:  V: %.1fV I: %.1fmA\n", axpBattVoltage, axpBattCurr);
  drawText(outText, 5, 30, 1);
  M5.Display.setTextColor(pwrUSB ? TFT_BLACK : TFT_RED, TFT_LIGHTGREY);
  sprintf(outText, "USB:  V: %.1fV I: %.1fmA\n", axpVBUSVoltage, axpVBUSCurrent);
  drawText(outText, 5, 40, 1);
  M5.Display.setTextColor(pwrDC ? TFT_BLACK : hatPresent ? TFT_BLUE : TFT_RED, TFT_LIGHTGREY);
  sprintf(outText, "5VIn: V: %.1fV I: %.1fmA\n", axpACInVoltage, axpACInCurrent);                                                                                                                                                               
  drawText(outText, 5, 50, 1);
  M5.Display.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  sprintf(outText, "Bat Pwr: %.1fmW", axpBattPower);
  drawText(outText, 5, 60, 1);
  #else
  sprintf(outText, "Bat:  V: %.1fV Level: %i %% \n", axpBattVoltage, axpBattLevel);
  drawText(outText, 5, 30, 1);
  #endif
  unsigned long allSeconds = millis()/1000;
  int runHours = allSeconds/3600;
  int secsRemaining = allSeconds%3600;
  int runMinutes = secsRemaining/60;
  int runSeconds=secsRemaining%60;
  sprintf(outText,"Uptime: %02d:%02d:%02d",runHours,runMinutes,runSeconds);  
  drawText(outText, 5, 70, 1);
//  Serial.println("done");
}

void lnViewerPage()
{
//  Serial.println("lnViewerPage");
  M5.Display.fillScreen(TFT_LIGHTGREY);
  M5.Display.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  switch (screenDef)
  {
    case 0: 
      drawLogo(140, 0, 0);
      break;
    case 1: 
      drawLogo(210, 0, 0);
      break;
  } 
  drawText("LocoNet Viewer", 5, 3, 2);
  m5DispLine = 0;
  useM5Viewer = 1;
//  Serial.println("done");
}

void olcbViewerPage()
{
//  Serial.println("olcbViewerPage");
  M5.Display.fillScreen(TFT_LIGHTGREY);
  M5.Display.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  switch (screenDef)
  {
    case 0: 
      drawLogo(140, 0, 0);
      break;
    case 1: 
      drawLogo(210, 0, 0);
      break;
  }
  drawText("OpenLCB Viewer", 5, 3, 2);
  m5DispLine = 0;
  useM5Viewer = 3;
//  Serial.println("done");
}

void dccViewerPage()
{
//  Serial.println("dccViewerPage");
  M5.Display.fillScreen(TFT_LIGHTGREY);
  M5.Display.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  switch (screenDef)
  {
    case 0: 
      drawLogo(140, 0, 0);
      break;
    case 1: 
      drawLogo(210, 0, 0);
      break;
  }
  if (dccOneTimeDisp)
    drawText("DCC Single Cmds", 5, 3, 2);
  else
    drawText("DCC Circular Cmds", 5, 3, 2);
  drawText("Click here to toggle", 5, 63, 2);
  m5DispLine = 0;
  useM5Viewer = 2;
//  Serial.println("done");
}

void mqttViewerPage()
{
//  Serial.println("mqttViewerPage");
  M5.Display.fillScreen(TFT_LIGHTGREY);
  M5.Display.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  switch (screenDef)
  {
    case 0: 
      drawLogo(140, 0, 0);
      break;
    case 1: 
      drawLogo(210, 0, 0);
      break;
  }
  if (dccOneTimeDisp)
    drawText("Published", 5, 3, 2);
  else
    drawText("Received", 5, 3, 2);
  drawText("Click here to toggle", 5, 63, 2);
  m5DispLine = 0;
  useM5Viewer = 4;
//  Serial.println("done");
}

void sensorViewerPage()
{
//  Serial.println("sensorViewerPage");
  uint8_t * lineY = screenDef == 0 ? &lineY_0[0] : &lineY_1[0];
  if (trainSensor)
  {
    sensorData currData = trainSensor->getSensorData();
    char outText[75];
    M5.Display.fillScreen(TFT_LIGHTGREY);
//    drawBackground();
    M5.Display.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
    switch (screenDef)
    {
      case 0: //Stick C
        drawLogo(140, 0, 0);
        break;
      case 1: //Stick C Plus
        drawLogo(210, 0, 0);
        break;
    }
    sprintf(outText, "%s Speed %.2f [mm/s]", currData.currDirFwd ? "Forward" : "Backward", abs(currData.currSpeedTech));
    drawText(outText, 5, lineY[0], 2);
    float_t scaleSpeed = (abs(currData.currSpeedTech)  * 36 * currData.modScale) / 10000; //[km/h]
    if (currData.dispDim == 1)
      scaleSpeed /= 1.6; //mph
    sprintf(outText, "%s Scale Speed %.2f [%s]", currData.scaleName, scaleSpeed, currData.dispDim == 1 ? "mph" : "km/h");
    M5.Display.setTextColor(TFT_BLUE, TFT_LIGHTGREY);
    drawText(outText, 5, lineY[1], 2);
    sprintf(outText, "Track Radius %.2f %s %s ", currData.dispDim == 1 ? abs(currData.currRadiusTech/25.4) : abs(currData.currRadiusTech), currData.dispDim == 1 ? "in" : "mm", currData.currRadiusTech == 0 ? "straight" : currData.currRadiusTech > 0 ? "right" : "left");
    M5.Display.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
    drawText(outText, 5, lineY[2], 2);
    sprintf(outText, "Dist [%s] Abs. %.2f Rel. %.2f", currData.dispDim == 1 ? "in" : "cm", currData.dispDim == 1 ? currData.absIntegrator/25.4 : currData.absIntegrator/10, currData.dispDim == 1 ? currData.relIntegrator/25.4 : currData.relIntegrator/10);
    drawText(outText, 5, lineY[3], 2);
    int8_t dirInd = currData.currDirFwd ? 1 : -1;
    sprintf(outText, "Hd.: %.0f Gr.[%s] %.1f S.El. [%s] %.1f", 180*currData.eulerVectorRad[0]/PI, "%", dirInd * trainSensor->getPercOfAngle(currData.imuVal[1]), "%", dirInd * trainSensor->getPercOfAngle(currData.imuVal[0]));
//    sprintf(outText, "Hd.: %.0f Gr.[%s] %.1f S.El. [%s] %.1f", 180*currData.eulerVectorRad[0]/PI, "%", 90*currData.imuVal[1], "%", 90*currData.imuVal[0]);
    drawText(outText, 5, lineY[4], 2);

    if (screenDef == 1)
    {
      sprintf(outText, "Wheel Angle %.2f [deg]", currData.axisAngle);
      drawText(outText, 5, lineY[5], 2);
    }
  }
  else
  {
    drawText("Sensor not initialized", 5, lineY[0], 2);
    drawText("Check IoTT Stick configuration", 5, lineY[1], 2);
  }
//  Serial.println("done");
}

String getLNString(lnReceiveBuffer * newData)
{
  String outText;
  char hexbuf[4];
  for (byte i=0; i < newData->lnMsgSize; i++)
  { 
    sprintf(hexbuf, "0x%02X", newData->lnData[i]);
    if (i==0)
      outText += String(hexbuf);
    else
      outText += ',' + String(hexbuf);
  }
  return outText;  
}

String getOLCBString(lnReceiveBuffer * newData)
{
  String outText = "";
/*
  olcbMsg thisMsg;
  if (gc_format_parse_olcb(&thisMsg, newData) >= 0)
  {
    String mtiData = String(thisMsg.MTI,16) + " ";
    mtiData.toUpperCase();
    outText += String(thisMsg.canFrameType) + " ";
    outText += "M:0x" + mtiData;
    for (byte i=0; i < thisMsg.dlc; i++)
    { 
      if (i > 0)
        outText += ",0x";
      else
        outText += "0x";
      if (thisMsg.olcbData.u8[i] < 16)
        outText += "0";
      String thisData = String(thisMsg.olcbData.u8[i],16);
      thisData.toUpperCase();
      outText += thisData;
    }
  }
*/
  return outText;  
}

String getMQTTString(char * topic, byte * payload)
{
  String outText = "sample message";
  return outText;  
}

void clearDisplay()
{
  String emptyLine = "                                        ";
  uint8_t dispY = 0;
  for (int i = 0; i < oneShotBufferSize; i++)
  {
    strncpy(dispBuffer[i], emptyLine.c_str(), dccStrLen);
    dispY = (15 * (i+1)) + 5;
    drawText(&emptyLine[0], 5, dispY, 1);
  }
}

void processLNtoM5(lnReceiveBuffer * newData)
{
  String emptyLine = "                                                                      ";
  String outText = getLNString(newData);
  strncpy(dispBuffer[m5DispLine], outText.c_str(), dccStrLen);
  uint8_t dispY = 0;
  for (int i = 0; i < oneShotBufferSize; i++)
  {
    dispY = (15 * (i+1)) + 5;
    drawText(&emptyLine[0], 5, dispY, 1);
    drawText(&dispBuffer[(m5DispLine+oneShotBufferSize-i) % oneShotBufferSize][0], 5, dispY, 1);
  }
  m5DispLine = (m5DispLine + 1) % oneShotBufferSize; //line
}

void processOLCBtoM5(lnReceiveBuffer * newData)
{
  String emptyLine = "                                                                      ";
  String outText = getOLCBString(newData);
  strncpy(dispBuffer[m5DispLine], outText.c_str(), dccStrLen);
  uint8_t dispY = 0;
  for (int i = 0; i < oneShotBufferSize; i++)
  {
    dispY = (15 * (i+1)) + 5;
    drawText(&emptyLine[0], 5, dispY, 1);
    drawText(&dispBuffer[(m5DispLine+oneShotBufferSize-i) % oneShotBufferSize][0], 5, dispY, 1);
  }
  m5DispLine = (m5DispLine + 1) % oneShotBufferSize; //line
}

void processMQTTtoM5(bool published, char * topic, char * payload)
{
  String emptyLine = "                                                                      ";
  if (((published) && isOneTime()) || ((!published) && (!isOneTime())))
  {
    strncpy(dispBuffer[m5DispLine], topic, dccStrLen);
    strncat(dispBuffer[m5DispLine], " ", dccStrLen - strlen(dispBuffer[m5DispLine]));
    strncat(dispBuffer[m5DispLine], payload, dccStrLen - strlen(dispBuffer[m5DispLine]));
    uint8_t dispY = 0;
//    Serial.println(dispBuffer[m5DispLine]);
    for (int i = 0; i < oneShotBufferSize; i++)
    {
      dispY = (15 * (i+1)) + 5;
      drawText(&emptyLine[0], 5, dispY, 1);
      drawText(&dispBuffer[(m5DispLine+oneShotBufferSize-i) % oneShotBufferSize][0], 5, dispY, 1);
    }
    m5DispLine = (m5DispLine + 1) % oneShotBufferSize; //line
  }
}

void processDCCtoM5(bool oneTime, String dispText)
{
  String emptyLine = "                                                                      ";
  if (oneTime)
  {
    strncpy(dispBuffer[m5DispLine], dispText.c_str(), dccStrLen);
    uint8_t dispY = 0;
    for (int i = 0; i < oneShotBufferSize; i++)
    {
      dispY = (15 * (i+1)) + 5;
      drawText(&emptyLine[0], 5, dispY, 1);
      drawText(&dispBuffer[(m5DispLine+oneShotBufferSize-i) % oneShotBufferSize][0], 5, dispY, 1);
    }
    m5DispLine = (m5DispLine + 1) % oneShotBufferSize; //line
  }
  else
  {
    dccViewerPage();
    drawText(&dispText[0], 5, 20, 1);
//    Serial.println(dispText);
  }
}

/*
void hard_restart() 
{
  esp_task_wdt_init(1,true);
  esp_task_wdt_add(NULL);
  while(true);
}
*/

//====================================================================================
//                                    tft_output
//====================================================================================
// This next function will be called during decoding of the jpeg file to
// render each block to the TFT.  If you use a different TFT library
// you will need to adapt this function to suit.
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
  // Stop further decoding as image is running off bottom of screen
  if ( y >= M5.Display.height() ) return 0;

  // This function will clip the image block rendering automatically at the TFT boundaries
  M5.Display.pushImage(x, y, w, h, bitmap);

  // This might work instead if you adapt the sketch to use the Adafruit_GFX library
  // M5.Lcd.drawRGBBitmap(x, y, bitmap, w, h);

  // Return 1 to decode next block
  return 1;
}

void loadJpgFile(uint16_t x, uint16_t y, uint8_t shrinkby, const char *name) //shrinkby is 1,2,4,8
{
  // Time recorded for test purposes
//  uint32_t t = millis();

  // Get the width and height in pixels of the jpeg if you wish
  uint16_t w = 0, h = 0, scale;
  TJpgDec.getFsJpgSize(&w, &h, name); // Note name preceded with "/"
  TJpgDec.setJpgScale(shrinkby);

  // Draw the image, top left at 0,0
  TJpgDec.drawFsJpg(x, y, name);
  // How much time did rendering take
//  t = millis() - t;

//  char buf[80];
//  sprintf(buf, "%s %dx%d 1:%d %u ms", name, w, h, scale, t);
//  Serial.println(buf);
}
