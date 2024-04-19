//jpeg functions: https://github.com/pix3lize/M5Stick-ImageViewerSPIFFS/blob/master/src/main.cpp

//display size C 80 x 160
//display size C Plus 135 * 240
//display size C Plus2 135 * 240
//tft color model: R/G/B  5/6/5 bits

#define oneShotBufferSize 4
#define refreshBufferSize 100

uint32_t lastTouchAnglePos; //Angle (Hi word) and Radius (Lo word) of last touch position

uint16_t colLocoSel = M5.Display.color24to16(0xFF0000); 
uint16_t colLocoUnsel = M5.Display.color24to16(0xFF7F7F); 
uint16_t colSwiSel = M5.Display.color24to16(0x007F00); 
uint16_t colSwiUnsel = M5.Display.color24to16(0x8ED973); 
uint16_t colRteSel = M5.Display.color24to16(0x0000FF); 
uint16_t colRteUnsel = M5.Display.color24to16(0xA6CAEC); //A6CAEC
uint16_t colSensSel = M5.Display.color24to16(0xFFFF00); 
uint16_t colSensUnsel = M5.Display.color24to16(0xFFFF99); 
uint16_t colFNOn = M5.Display.color24to16(0x457AE3); //457AE3
uint16_t colFNOff = M5.Display.color24to16(0xA6CAEC); //A6CAEC

uint8_t screenDef = 1; //0 for Stick; 1 for Stick C Plus
uint8_t lineY_0[] = {3,18,33,48,63};
uint8_t lineY_1[] = {3,15,27,39,51,63};

long curEncoderPos = 0;
static m5::touch_state_t prev_state;
int prev_x = -1;
int prev_y = -1;
uint32_t prev_rotPos = 0;

uint32_t getRotPosOfCoord(int16_t x, int16_t y)
{
  int16_t effX = x-120;
  int16_t effY = y-120;
  float Radius = sqrt(sq(effX) + sq(effY));
  float Alpha = asin((effX)/Radius);
  if (effY >=0)
  {
    Alpha = PI - Alpha;
  }
  else
    if (effX < 0)
    {
      Alpha = 2*PI + Alpha;
    }
  effX = round(Alpha * 180/PI); 
  effY = round(Radius);
  return ((effY<<16) + effX); //Radius - Alpha in degrees
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
  if (m5CurrentPage != 5)
    useM5Viewer = 0;
  if (m5CurrentPage == 7)
    m5CurrentPage = 3;
  else
    m5CurrentPage++;
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
        setThrottlePage();
        break;
    case 7:
      switch (useInterface.devId)
      {
        case 17:
        case 18:
        case 0: useM5Viewer = 0; m5CurrentPage = 3; oldWifiStatus = 0xFFFF; break;
//        case 1:;
//        case 10: dccViewerPage(); break;
        
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
//        case 7: olcbViewerPage(); break;
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

void processEncoder(long currPos)
{
  M5Dial.Speaker.tone(8000, 20);
  Serial.println(currPos);
}

void processTouch(uint16_t x, uint16_t y, uint8_t state)
{
  if (m5CurrentPage == 6)
  {
    uint32_t rotPos = getRotPosOfCoord(x, y); //Radius - Angle
    uint16_t Radius = rotPos>>16;
    uint16_t Angle = rotPos & 0x0000FFFF;
    if ((state == 1) && (Radius > 75) &&((Angle >= 270) || (Angle < 90)))
    {
      uint16_t throttleSel = round((Angle / 45)-2);
      throttleSel %= 4;
      if (m5ThrottleMenu != throttleSel)
      {
        Serial.println(throttleSel);
        m5ThrottleMenu = throttleSel;
        updateMainMenuRing(m5ThrottleMenu);
      }
    }
/*
    Serial.print(" ");
    Serial.println(state);
    Serial.print(rotPos>>16);
    Serial.print(" ");
    Serial.println(rotPos & 0x0000FFFF); 
*/    
  }
}

void processDisplay()
{
  long newPosition = M5Dial.Encoder.read();
  if (newPosition != curEncoderPos) 
  {
        curEncoderPos = newPosition;
        processEncoder(newPosition);
  }

  auto t = M5Dial.Touch.getDetail();
  if (prev_state != t.state || prev_x != t.x || prev_y != t.y)  
  {
    prev_state = t.state;
    prev_x = t.x;
    prev_y = t.y;
    static constexpr const char* state_name[16] = {
          "none", "touch", "touch_end", "touch_begin",
          "___",  "hold",  "hold_end",  "hold_begin",
          "___",  "flick", "flick_end", "flick_begin",
          "___",  "drag",  "drag_end",  "drag_begin"};
//    M5_LOGI("%s", state_name[t.state]);
//    Serial.println(state_name[t.state]);
    processTouch(t.x, t.y, t.state);
  }

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

/*
  switch (state)
  {
    case 1: btnCOnHold(0); break;
//    case 3: btnCClick(); btnCClickCount++;  break;
    case 5: btnCDblClick(M5.BtnPWR.getClickCount());
//            btnCClickCount = 0;
            break;
  }
*/
  if (pwrRefreshTimer < millis())
  {
    switch (pwrRefreshCtr)
    {
//      case 1: startmeas = millis();
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
      case 24: wifiMode = WiFi.getMode(); break; 
      case 26: wifiStatus = WiFi.status(); yield(); break;
      case 30:
          axpBattCurr = axpBattChargeCurrent == 0? - axpBattDischargeCurrent : axpBattChargeCurrent;
          hatPresent = axpACInVoltage > 0.5;
            pwrUSB = axpVBUSVoltage > 4.5;
            pwrDC = axpACInVoltage > 4.7;
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
/*
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
*/
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
/*
  if ((useInterface.devCommMode == 0))// && (useHat.devCommMode == 0)) //DCC/Decoder
    if (millis() > dccDispTimer)
    {
//      Serial.println("Refresh");
      sendRefreshBuffer();
      dccDispTimer = millis() + pwrDispInterval; //exact interval not important, we start with millis as base
    }
*/
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
      updateThrottlePage(); 
      pwrDispTimer = millis() + speedDispInterval;
    }
  }
}

void initDisplay()
{
  uint16_t devType = M5.getBoard(); //4: stick c 5: stick c plus
  M5.Display.setRotation(0);
  screenDef = 1;
  // The jpeg image can be scaled by a factor of 1, 2, 4, or 8
  TJpgDec.setJpgScale(1);

  // The byte order can be swapped (set true for TFT_eSPI)
  TJpgDec.setSwapBytes(true);

  // The decoder must be given the exact name of the rendering function above
  TJpgDec.setCallback(tft_output);
  setOpeningPage();
}


void setOpeningPage()
{
  drawBackground();
  drawLogo(60, 60, 1); //120x120 in the middle of 240 x 240
}

void setWifiConnectPage()
{
  char outText[50];
  drawBackground();
  drawLogo(90, 30, 2);
//  M5.Display.fillScreen(TFT_LIGHTGREY);
  M5.Display.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  drawCenterText(NULL, "Connect to AP", 120, 100, 2);
  String hlpStr = "IoTT_Dial_M5_" + String((uint32_t)ESP.getEfuseMac());
  sprintf(outText, hlpStr.c_str());
  drawCenterText(NULL, outText, 120, 120, 2);
  drawCenterText(NULL, "to set Wifi credentials", 120, 140, 2);
}

void setWifiPage(int wifiCfgModePrm)
{
//  Serial.println("setWifiPage");
  uint8_t * lineY = screenDef == 0 ? &lineY_0[0] : &lineY_1[0];
  char outText[50];
//  M5.Display.fillScreen(TFT_LIGHTGREY);
  drawBackground();
  M5.Display.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  drawLogo(170, 90, 2);
  switch (wifiCfgModePrm)
  {
    case 1:
    {
      sprintf(outText, "IoTT Dial: %s", deviceName.c_str());
      drawText(NULL, outText, 10, 80, 2);
      drawText(NULL, "Wifi LAN connected", 10, 100, 2);
      sprintf(outText, "AP: %s", mySSID.c_str());
      drawText(NULL, outText, 10, 120, 2);
      sprintf(outText, "IP: %s", dyn_ip.toString().c_str());
      drawText(NULL, outText, 10, 140, 2);
      break;
    }
    case 2:
    {
      drawText(NULL, "Local AP active", 10, 80, 2);
      sprintf(outText, "AP: %s", deviceName.c_str());
      drawText(NULL, outText, 10, 100, 2);
      sprintf(outText, "Password: %s", apPassword.c_str());
      drawText(NULL, outText, 10, 120, 2);
      sprintf(outText, "IP: %s", ap_ip.toString().c_str());
      drawText(NULL, outText, 10, 140, 2);
      break;
    }
  }
//  Serial.println("done");
}

void setNoWifiPage()
{
//  Serial.println("setNoWifiPage");
  drawBackground();
  M5.Display.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  drawLogo(170, 90, 2);
  drawCenterText(NULL, "Wifi LAN disconnected", 120, 180, 2);
  drawCenterText(NULL, "Click here to connect", 120, 220, 2);
//  Serial.println("done");
}

//commMode: 0: DCC, 1: LocoNet, 2: MQTT, 3: Gateway
//workMode: 0: Decoder, 1: ALM

void setStatusPage()
{
//  Serial.println("setStatusPage");
  drawBackground();
  M5.Display.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
//  M5.Display.setTextSize(1);
  drawLogo(170, 90, 2);
  drawText(NULL, "Hat: Dial Throttle", 10, 80, 2);
  switch (useInterface.devId)
  {
    case 0: drawText(NULL, "No Interface", 10, 100, 2); break;
//    case 1: drawText("using DCC", 5, lineY[1], 2); break;
//    case 2: drawText("using LocoNet", 5, lineY[1], 2); break;
    case 3: drawText(NULL, "using LN over MQTT", 10, 100, 2); break;
//    case 4: drawText("using LocoNet", 5, lineY[1], 2); break; //LN Gateway
//    case 5: drawText("using OpenLCB", 5, lineY[1], 2); break; 
//    case 6: drawText("using OpenLCB over MQTT", 5, lineY[1], 2); break; //OpenLCB 
//    case 7: drawText("using OpenLCB", 5, lineY[1], 2); break; //OpenLCB 
//    case 8: drawText("using MQTT", 5, lineY[1], 2); break; //native MQTT 
//    case 9: drawText("using DCC to MQTT", 5, lineY[1], 2); break;
//    case 10: drawText("using DCC from MQTT", 5, lineY[1], 2); break;
//    case 11: drawText("using LN/TCP(Server)", 5, lineY[1], 2); break;
    case 12: 
    {
      char outText[50];
//      String outStr = lbServer->getServerIP();
//      Serial.println(outStr);
      sprintf(outText, "using LN/TCP(Client)");// %s", outStr[);
      drawText(NULL, outText, 10,100, 2); break;
    }
    case 16: drawText(NULL, "using LN Loopback", 10, 100, 2); break;
    case 17: drawText(NULL, "using WiThrottle", 10, 100, 2); break;
    default: drawText(NULL, "unknown", 10, 100, 2); break;
  }
/*
  if ((useInterface.devId == 4) || (useInterface.devId == 7))
    drawText("Gateway: active", 10, 72, 2);
  else
    drawText("Gateway: not used", 10, 72, 2);
*/
  drawText(NULL, "Servers:", 10, 120, 2);
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
  modList += "]";
  drawText(NULL, &modList[0], 55, 120, 2); 

  drawText(NULL, "Modules:", 10, 140, 2);
  modList = "[";
  if (eventHandler)
    modList += "Evnt Hdlr";
  modList += "]";
  drawText(NULL, &modList[0], 55, 140, 2); 
//  Serial.println("done");
}

void setPwrStatusPage()
{
//  Serial.println("setPwrStatusPage");
  char outText[50];
  drawBackground();
  M5.Display.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  drawLogo(170, 90, 2);
  sprintf(outText, "IoTT Stick V. %s", BBVersion.c_str());
  drawText(NULL, outText, 10, 40, 2);
  sprintf(outText, "Stick Temp: %.1f C \n", axpIntTemp);
  drawText(NULL, outText, 10, 60, 2);
  sprintf(outText, "Bat:  V: %.1fV I: %.1fmA\n", axpBattVoltage, axpBattCurr);
  drawText(NULL, outText, 10, 80, 2);
  M5.Display.setTextColor(pwrUSB ? TFT_BLACK : TFT_RED, TFT_LIGHTGREY);
  sprintf(outText, "USB:  V: %.1fV I: %.1fmA\n", axpVBUSVoltage, axpVBUSCurrent);
  drawText(NULL, outText, 10, 100, 2);
  M5.Display.setTextColor(pwrDC ? TFT_BLACK : hatPresent ? TFT_BLUE : TFT_RED, TFT_LIGHTGREY);
  sprintf(outText, "5VIn: V: %.1fV I: %.1fmA\n", axpACInVoltage, axpACInCurrent);                                                                                                                                                               
  drawText(NULL, outText, 10, 120, 2);
  M5.Display.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  sprintf(outText, "Bat Pwr: %.1fmW", axpBattPower);
  drawText(NULL, outText, 10, 140, 2);
  unsigned long allSeconds = millis()/1000;
  int runHours = allSeconds/3600;
  int secsRemaining = allSeconds%3600;
  int runMinutes = secsRemaining/60;
  int runSeconds=secsRemaining%60;
  sprintf(outText,"Uptime: %02d:%02d:%02d",runHours,runMinutes,runSeconds);  
  drawText(NULL, outText, 10, 160, 2);
//  Serial.println("done");
}

void lnViewerPage()
{
//  Serial.println("lnViewerPage");
  drawBackground();
  M5.Display.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  drawLogo(90, 20, 2);
  drawCenterText(NULL, "LocoNet Viewer", 120, 90, 2);
  m5DispLine = 0;
  useM5Viewer = 1;
//  Serial.println("done");
}
/*
void olcbViewerPage()
{
//  Serial.println("olcbViewerPage");
  drawBackground();
  M5.Display.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  drawLogo(170, 90, 2);
  drawText("OpenLCB Viewer", 5, 3, 2);
  m5DispLine = 0;
  useM5Viewer = 3;
//  Serial.println("done");
}
/*
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
*/
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
    drawText(NULL, "Published", 5, 3, 2);
  else
    drawText(NULL, "Received", 5, 3, 2);
  drawText(NULL, "Click here to toggle", 5, 63, 2);
  m5DispLine = 0;
  useM5Viewer = 4;
//  Serial.println("done");
}

void updateThrottlePage()
{
  
}

void drawLocoScreen()
{
  M5.Display.startWrite();
  drawBackground();
  drawMainMenuSegment(0,true);
  drawMainMenuSegment(1,false);
  drawMainMenuSegment(2,false);
  drawMainMenuSegment(3,false);
  
  drawFunctionSegment(0,1,false);
  drawFunctionSegment(1,2,true);
  drawFunctionSegment(2,3,false);
  drawFunctionSegment(3,4,true);
  
  M5.Display.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  M5.Display.endWrite();
}

void drawSwitchScreen()
{
  M5.Display.startWrite();
  drawBackground();
  drawMainMenuSegment(0,false);
  drawMainMenuSegment(1,true);
  drawMainMenuSegment(2,false);
  drawMainMenuSegment(3,false);
  M5.Display.endWrite();
}

void drawSignalScreen()
{
  M5.Display.startWrite();
  drawBackground();
  drawMainMenuSegment(0,false);
  drawMainMenuSegment(1,false);
  drawMainMenuSegment(2,true);
  drawMainMenuSegment(3,false);
  M5.Display.endWrite();
}

void drawRouteScreen()
{
  M5.Display.startWrite();
  drawBackground();
  drawMainMenuSegment(0,false);
  drawMainMenuSegment(1,false);
  drawMainMenuSegment(2,false);
  drawMainMenuSegment(3,true);
  M5.Display.endWrite();
}


void setThrottlePage()
{
  switch (m5ThrottleMenu)
  {
    case 0: //loco
      drawLocoScreen();
    break;
    case 1: //switch
      drawSwitchScreen();
    break;
    case 2: //signal
      drawSignalScreen();
    break;
    case 3: //routes
      drawRouteScreen();
    break;
  }
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
    dispY = (20 * (i+1)) + 100;
    drawText(NULL, &emptyLine[0], 10, dispY, 2);
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
    dispY = (20 * (i+1)) + 100;
    drawText(NULL, &emptyLine[0], 10, dispY, 2);
    drawText(NULL, &dispBuffer[(m5DispLine+oneShotBufferSize-i) % oneShotBufferSize][0], 15, dispY, 2);
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
    drawText(NULL, &emptyLine[0], 5, dispY, 1);
    drawText(NULL, &dispBuffer[(m5DispLine+oneShotBufferSize-i) % oneShotBufferSize][0], 5, dispY, 1);
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
      drawText(NULL, &emptyLine[0], 5, dispY, 1);
      drawText(NULL, &dispBuffer[(m5DispLine+oneShotBufferSize-i) % oneShotBufferSize][0], 5, dispY, 1);
    }
    m5DispLine = (m5DispLine + 1) % oneShotBufferSize; //line
  }
}
/*
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
*/
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
