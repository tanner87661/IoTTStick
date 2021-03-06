//display size C 80 x 160
//display size C Plus 135 * 240
#include "iottlogo2020.c"
#include "iottlogo8080.c"
#include "iottlogo3030.c"
#include "Nickel4020.c"

uint8_t screenDef = 1; //0 for Stick; 1 for Stick C Plus

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

#define dccStrLen 45

bool dccOneTimeDisp = false;
uint32_t pwrOffTimer = millis();
uint32_t pwrDispTimer = millis();
uint32_t dccDispTimer = millis();

bool darkScreen = false;

uint8_t wifiResetCtr = 0;
uint32_t wifiResetLastClick = 0;
#define wifiResetMaxDelay 2000
#define wifiResetReqCount 3

OneDimKalman batCurrent;

char dispBuffer[oneShotBufferSize][dccStrLen];

bool isOneTime()
{
  return dccOneTimeDisp;
}

void getRTCTime()
{
  RTC_TimeTypeDef TimeStruct;
  RTC_DateTypeDef DateStruct;
  M5.Rtc.GetTime(&TimeStruct);
  M5.Rtc.GetData(&DateStruct);
  timeinfo.tm_hour = TimeStruct.Hours;
  timeinfo.tm_min =  TimeStruct.Minutes;
  timeinfo.tm_sec = TimeStruct.Seconds;
  timeinfo.tm_wday = DateStruct.WeekDay;
  timeinfo.tm_mon = DateStruct.Month - 1;
  timeinfo.tm_mday = DateStruct.Date;
  timeinfo.tm_year = DateStruct.Year - 1900;
  now = mktime(&timeinfo);
  struct timeval thisTimeVal = { .tv_sec = now };
  settimeofday(&thisTimeVal, NULL);  
}

void processDisplay()
{
  if (!(batCurrent.getEstimate(M5.Axp.GetBatCurrent()) < (-5))) //check for Power Status, but not for BlackHat
  {
//  if !((M5.Axp.GetVinCurrent() > 0) || (M5.Axp.GetVBusCurrent() > 0)) //check for Power Status
//  if ((M5.Axp.GetVinVoltage() > 4.8) || (M5.Axp.GetVBusVoltage() > 4.6)) //check for Power Status
    pwrOffTimer = millis();
    if (darkScreen)
    {
      darkScreen = false;
      M5.Axp.ScreenBreath(15);//7-15
    } 
  }
  else
    if ((pwrOffTimer + pwrOffTimeout) < millis()) //power off after timeout without power. Device will come on automatically when power is back
    {
      if (useHat.devId == 5) //BlackHat, do not power down
      {
        if (!darkScreen)
        {
          M5.Axp.ScreenBreath(7);//7-15
          darkScreen = true;
        }
      }
      else
      {
        saveToFile(bufferFileName);
        M5.Axp.PowerOff();
      }
    }
  if (m5CurrentPage == 3) //in case status changes while displayed, we update the page
  {
    uint16_t currStatus = (WiFi.getMode() << 8) + WiFi.status();
    if (currStatus != oldWifiStatus)
    {
      oldWifiStatus = currStatus;
      switch (WiFi.getMode())
      {
        case 0: setNoWifiPage(); break; //Wifi shut off
        case 1: // STA
          if (WiFi.status() == WL_CONNECTED)
            setWifiPage(wifiCfgMode);
          else
            setNoWifiPage();
          break;
        case 2: //AP
          if (!wifiCancelled)
            setWifiPage(wifiCfgMode);
          else
            setNoWifiPage();
          break;
      }
    }
  }
  if ((useInterface.devCommMode == 0))// && (useHat.devCommMode == 0)) //DCC/Decoder
    if (millis() > dccDispTimer)
    {
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
  if(M5.BtnA.wasPressed()) //the big one
  {
//    Serial.println("Button A");
  
    pwrOffTimer = millis();
    if (darkScreen)
    {
      M5.Axp.ScreenBreath(15);//7-15
      darkScreen = false;
    }
    else
      {
      if (m5CurrentPage != 5)
        useM5Viewer = 0;
      if (m5CurrentPage == 6)
        m5CurrentPage = 3;
      else
        m5CurrentPage++;
      }
    switch (m5CurrentPage)
    {
      case 0:
        setOpeningPage();
        break;
      case 1:
        setWifiConnectPage();
        break;
      case 2:
        m5CurrentPage++;
        useM5Viewer = 0;
      case 3:
//        Serial.print(oldWifiStatus);
        oldWifiStatus = 0xFFFF; //impossible mode, requires update
//        Serial.println(" Set Wifi page");
        break;
      case 4:
        setStatusPage();
        break;
      case 5:
        pwrDispTimer = millis() + pwrDispInterval;
        setPwrStatusPage();
        break;
      case 6:
//        Serial.println(useInterface.devId);
        switch (useInterface.devId)
        {
          case 1: dccViewerPage(); break;
          case 2:;
          case 3:;
          case 4: lnViewerPage(); break;
          case 5:;
          case 6:
//          case 7: olcbViewerPage(); break;
          case 8: mqttViewerPage(); break;
          case 9: dccViewerPage(); break;
        }
        clearDisplay();
        m5DispLine = 0;
        break;
    }
  }
  if(M5.BtnB.wasPressed()) //the one below
  {
//    Serial.printf("Button B Page %i Viewer %i\n", m5CurrentPage, useM5Viewer);
    pwrOffTimer = millis();
    switch (m5CurrentPage)
    {
      case 6: //DCC Viewer active, toggle status
        if (useM5Viewer == 2) //DCC, so toggle mode
        {
//          Serial.println("Switch Mode");
          dccOneTimeDisp = !dccOneTimeDisp;
          dccViewerPage();
        }
        if (useM5Viewer == 4) //MQTT, so toggle mode
        {
//          Serial.println("Switch Mode");
          dccOneTimeDisp = !dccOneTimeDisp;
          clearDisplay();
          mqttViewerPage();
        }
        break;
      case 3: //Wifi Status page active
        if (WiFi.getMode() == 0)
        {
          Serial.println("Connect WiFi");  
          establishWifiConnection(myWebServer,dnsServer);
        }
        break;
    }
  }
  uint8_t pwrBtn = M5.BtnC.getPwrPin();
  if(pwrBtn) 
  {
    pwrOffTimer = millis();
    if (pwrBtn == 1) //long press
      saveToFile(bufferFileName); //could be on the way to power off, so we save the data to SPIFFS
    switch (m5CurrentPage)
    {
      case 3: //Wifi Status page active
        if (pwrBtn == 2) //short press
        {
          if (wifiCfgMode == 1)  //STA mode is used, so reset credentials
          {
            if ((wifiResetCtr == 0 ) || millis() < (wifiResetLastClick + wifiResetMaxDelay))
            {
              wifiResetCtr++;
              wifiResetLastClick = millis();
//              Serial.println("up");
              if (wifiResetCtr == wifiResetReqCount)
              {
                
                if (WiFi.status() == WL_CONNECTED)
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
            else
              wifiResetCtr = 0;
            
          }
        }
        else
          wifiResetCtr = 0;
        break;
      default:
        wifiResetCtr = 0;
        break;
    }
  }
}

void initDisplay()
{
  M5.Lcd.setRotation(3);
  if (M5.Lcd.width() == 160)
    screenDef = 0;
  else
    screenDef = 1;
  setOpeningPage();
}

void drawLogo(int x, int y, int logoSize)
{
  switch (screenDef)
  {
    case 0:  //Stick C
      switch (logoSize)
      {
        case 0 : M5.Lcd.pushImage( x,y,20,20, (uint16_t *)image_data_iottlogo2020); break;
        case 1 : M5.Lcd.pushImage( x,y,30,30, (uint16_t *)image_data_iottlogo3030); break;
        case 2 : M5.Lcd.pushImage( x,y,80,80, (uint16_t *)image_data_iottlogo8080); break;
      }
      break;
    case 1:  //Stick C plus
      switch (logoSize)
      {
        case 0 : M5.Lcd.pushImage( x,y,30,30, (uint16_t *)image_data_iottlogo3030); break;
        case 1 : M5.Lcd.pushImage( x,y,80,80, (uint16_t *)image_data_iottlogo8080); break;
        case 2 : M5.Lcd.pushImage( x,y,80,80, (uint16_t *)image_data_iottlogo8080); break;
      }
      break;
  }
}

void drawText(char * payload, int x, int y, int what)
{
  M5.Lcd.drawString(payload,getXCoord(x),getYCoord(y),what);
}

void drawBackground()
{
  switch (screenDef)
  {
    case 0: 
      for (int x = 0; x < 4; x++)
        for (int y = 0; y < 4; y++)
          M5.Lcd.pushImage(40 * x, 20 *y,40,20, (uint16_t *)image_data_Nickel4020); 
      break;
    case 1: 
      for (int x = 0; x < 6; x++)
        for (int y = 0; y < 7; y++)
          M5.Lcd.pushImage( 40 * x,20 * y,40,20, (uint16_t *)image_data_Nickel4020); 
      break;
  }
}

void setOpeningPage()
{
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
}

void setWifiConnectPage()
{
  char outText[50];
  M5.Lcd.fillScreen(TFT_LIGHTGREY);
  M5.Lcd.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
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
}

void setWifiPage(int wifiCfgMode)
{
  char outText[50];
  M5.Lcd.fillScreen(TFT_LIGHTGREY);
  M5.Lcd.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  switch (screenDef)
  {
    case 0: 
      drawLogo(140, 60, 0);
      break;
    case 1: 
      drawLogo(210, 105, 0);
      break;
  }

  switch (wifiCfgMode)
  {
    case 1:
    {
      drawText("Wifi LAN connected", 5, 3, 2);
      sprintf(outText, "IP: %s", WiFi.localIP().toString().c_str());
      drawText(outText, 5, 18, 2);
      break;
    }
    case 2:
    {
      drawText("Local AP active", 5, 3, 2);
      sprintf(outText, "AP: %s", deviceName.c_str());
      drawText(outText, 5, 18, 2);
      sprintf(outText, "Password: %s", apPassword.c_str());
      drawText(outText, 5, 33, 2);
      sprintf(outText, "IP: %s", WiFi.softAPIP().toString().c_str());
      drawText(outText, 5, 48, 2);
      break;
    }
  }
}

void setNoWifiPage()
{
  M5.Lcd.fillScreen(TFT_LIGHTGREY);
  M5.Lcd.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  switch (screenDef)
  {
    case 0: 
      drawLogo(65, 25, 1); //30x30
      break;
    case 1: 
      drawLogo(80, 25, 1); //80x80
      break;
  }
  drawText("Wifi LAN disconnected", 5, 3, 2);
  drawText("Click here to connect", 5, 63, 2);
}

//commMode: 0: DCC, 1: LocoNet, 2: MQTT, 3: Gateway
//workMode: 0: Decoder, 1: ALM

void setStatusPage()
{
  M5.Lcd.fillScreen(TFT_LIGHTGREY);
  M5.Lcd.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
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
    case 0: drawText("Hat: None", 5, 3, 2); break;
    case 1: drawText("Hat: BlueHat", 5, 3, 2); break;
    case 2: drawText("Hat: USB Serial", 5, 3, 2); break;
    case 3: drawText("Hat: YellowHat", 5, 3, 2); break;
    case 4: drawText("Hat: GreenHat", 5, 3, 2); break;
    case 5: drawText("Hat: BlackHat", 5, 3, 2); break;
//    case 6: drawText("Hat: Switch Decoder", 5, 3, 2); break;
    default: drawText("Hat: unknown", 5, 3, 2); break;
  }
  switch (useInterface.devId)
  {
    case 0: drawText("No Interface", 5, 18, 2); break;
    case 1: drawText("using DCC", 5, 18, 2); break;
    case 2: drawText("using LocoNet", 5, 18, 2); break;
    case 3: drawText("using LN over MQTT", 5, 18, 2); break;
    case 4: drawText("using LocoNet", 5, 18, 2); break; //LN Gateway
    case 5: drawText("using OpenLCB", 5, 18, 2); break; 
    case 6: drawText("using OpenLCB over MQTT", 5, 18, 2); break; //OpenLCB 
    case 7: drawText("using OpenLCB", 5, 18, 2); break; //OpenLCB 
    case 8: drawText("using MQTT", 5, 18, 2); break; //native MQTT 
    case 9: drawText("using DCC to MQTT", 5, 18, 2); break;
    default: drawText("unknown", 5, 18, 2); break;
  }
  if ((useInterface.devId == 4) || (useInterface.devId == 7))
    drawText("Gateway: active", 5, 33, 2);
  else
    drawText("Gateway: not used", 5, 33, 2);
  drawText("Modules:", 5, 48, 2);
  String modList = "[";
  if (eventHandler)
    modList += "Evnt Hdlr";
  modList += "]";
  drawText(&modList[0], 5, 63, 2); 
}

void setPwrStatusPage()
{
  char outText[50];
  M5.Lcd.fillScreen(TFT_LIGHTGREY);
  M5.Lcd.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  switch (screenDef)
  {
    case 0: 
      drawLogo(140, 0, 0);
      break;
    case 1: 
      drawLogo(210, 0, 0);
      break;
  }
  sprintf(outText, "IoTT Stick V. %c.%c.%c", BBVersion[0], BBVersion[1], BBVersion[2]);
  drawText(outText, 5, 3, 2);

  sprintf(outText, "Stick Temp: %.1f C \n", M5.Axp.GetTempInAXP192());
  drawText(outText, 5, 20, 1);
  sprintf(outText, "Bat:  V: %.1fV I: %.1fmA\n", M5.Axp.GetBatVoltage(), M5.Axp.GetBatCurrent());
  drawText(outText, 5, 30, 1);
  sprintf(outText, "USB:  V: %.1fV I: %.1fmA\n", M5.Axp.GetVBusVoltage(), M5.Axp.GetVBusCurrent());
  drawText(outText, 5, 40, 1);
  sprintf(outText, "5VIn: V: %.1fV I: %.1fmA\n", M5.Axp.GetVinVoltage(), M5.Axp.GetVinCurrent());
  drawText(outText, 5, 50, 1);
  sprintf(outText, "Bat Pwr: %.1fmW", M5.Axp.GetBatPower());
  drawText(outText, 5, 60, 1);

  unsigned long allSeconds=millis()/1000;
  int runHours= allSeconds/3600;
  int secsRemaining=allSeconds%3600;
  int runMinutes=secsRemaining/60;
  int runSeconds=secsRemaining%60;
  sprintf(outText,"Uptime: %02d:%02d:%02d",runHours,runMinutes,runSeconds);  
  drawText(outText, 5, 70, 1);
}

void lnViewerPage()
{
  M5.Lcd.fillScreen(TFT_LIGHTGREY);
  M5.Lcd.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
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
}
/*
void olcbViewerPage()
{
  M5.Lcd.fillScreen(TFT_LIGHTGREY);
  M5.Lcd.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
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
}
*/
void dccViewerPage()
{
  M5.Lcd.fillScreen(TFT_LIGHTGREY);
  M5.Lcd.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
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
}

void mqttViewerPage()
{
  M5.Lcd.fillScreen(TFT_LIGHTGREY);
  M5.Lcd.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
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
}

String getLNString(lnReceiveBuffer * newData)
{
  String outText;
  for (byte i=0; i < newData->lnMsgSize; i++)
  { 
    if (i > 0)
      outText += ",0x";
    else
      outText += "0x";
    if (newData->lnData[i] < 16)
      outText += "0";
    String thisData = String(newData->lnData[i],16);
    thisData.toUpperCase();
    outText += thisData;
  }
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
    Serial.println(dispText);
  }
}
