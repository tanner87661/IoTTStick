#include "iottlogo8080.c"
#include "Nickel4020.c"

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
float axpBattCurr = 0;
uint8_t pwrRefreshCtr = 0;
//uint32_t startmeas;

uint8_t screenDef = 1; //0 for Stick; 1 for Stick C Plus


void initDisplay()
{
  uint16_t devType = M5.getBoard(); //4: stick c 5: stick c plus 12: Stamp-S3
  USBSerial.printf("Board: %i\n", devType);
  USBSerial.println(devType);
  M5.Display.setRotation(0);
  screenDef = 1;
  setOpeningPage();
}

void processDisplay()
{
  int state = M5.BtnA.wasHold() ? 1
        : M5.BtnA.wasClicked() ? 2
        : M5.BtnA.wasPressed() ? 3
        : M5.BtnA.wasReleased() ? 4
        : M5.BtnA.wasDeciedClickCount() ? 5
        : 0;
//  switch (state)
//  {
//    case 3: btnAClick(); break;
//  }

  state = M5.BtnB.wasHold() ? 1
        : M5.BtnB.wasClicked() ? 2
        : M5.BtnB.wasPressed() ? 3
        : M5.BtnB.wasReleased() ? 4
        : M5.BtnB.wasDeciedClickCount() ? 5
        : 0;
//  switch (state)
//  {
//    case 3: btnBClick(); break;
//    case 5: btnBDblClick(M5.BtnB.getClickCount());
//            break;
//  }

  state = M5.BtnPWR.wasHold() ? 1
        : M5.BtnPWR.wasClicked() ? 2
        : M5.BtnPWR.wasPressed() ? 3
        : M5.BtnPWR.wasReleased() ? 4
        : M5.BtnPWR.wasDeciedClickCount() ? 5
        : 0;

//  switch (state)
//  {
//    case 1: btnCOnHold(0); break;
//    case 3: btnCClick(); btnCClickCount++;  break;
//    case 5: btnCDblClick(M5.BtnPWR.getClickCount());
//            btnCClickCount = 0;
//            break;
//  }

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
      case 22: wifiMode = WiFi.getMode();
      case 24: wifiStatus = WiFi.status();
      case 26:
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

  if (!dataValid)
    return;
/*    
  if (pwrUSB || pwrDC) //check for Power Status, but not for BlackHat
  {
    pwrOffTimer = millis();
    if (darkScreen)
    {
      darkScreen = false;
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
          M5.Display.setBrightness(0);
          darkScreen = true;
        }
      }
      else
      {
        prepareShutDown();
        M5.Power.powerOff();
      }
    }

  if (m5CurrentPage == 3) //in case status changes while displayed, we update the page
  {
    uint16_t currStatus = (wifiMode << 8) + wifiStatus;
    if (currStatus != oldWifiStatus)
    {
      oldWifiStatus = currStatus;
      switch (wifiMode)
      {
        case 0: setNoWifiPage(); break; //Wifi shut off
        case 1: // STA
          if (wifiStatus == WL_CONNECTED)
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
//      setPwrStatusPage(); 
      pwrDispTimer = millis() + pwrDispInterval;
    }
  }

  if ((m5CurrentPage == 6) && (useHat.devId == 7))
  {
    if (millis() > pwrDispTimer)
    {
//      sensorViewerPage(); 
      pwrDispTimer = millis() + speedDispInterval;
    }
  }
*/
}

void drawLogo(int x, int y, int logoSize)
{
  switch (logoSize)
  {
    case 0 : M5.Display.pushImage( x,y,20,20, (uint16_t *)image_data_iottlogo8080); break;
    case 1 : M5.Display.pushImage( x,y,30,30, (uint16_t *)image_data_iottlogo8080); break;
    case 2 : M5.Display.pushImage( x,y,80,80, (uint16_t *)image_data_iottlogo8080); break;
  }
}

void drawText(const char * payload, int x, int y, int what)
{
  M5.Display.drawString(payload,getXCoord(x),getYCoord(y),what);
//  M5.Display.startWrite();
//  M5.Display.setCursor(getXCoord(x), getYCoord(y));
//  M5.Display.setTextSize(what); // = what;
//  M5.Display.print(payload);
//  M5.Display.endWrite();
}

void drawBackground()
{
  for (int x = 0; x < 6; x++)
    for (int y = 0; y < 12; y++)
      M5.Display.pushImage( 40 * x,20 * y,40,20, (uint16_t *)image_data_Nickel4020); 
}

void setOpeningPage()
{
  drawBackground();
  drawLogo(80, 80, 2); //80x80
}

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

void setWifiConnectPage()
{
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
  String hlpStr = "IoTT_Dial_M5_" + String((uint32_t)ESP.getEfuseMac());
  sprintf(outText, hlpStr.c_str());
  drawText(outText, 5, 18, 2);
  drawText("to set Wifi credentials", 5, 33, 2);
}
