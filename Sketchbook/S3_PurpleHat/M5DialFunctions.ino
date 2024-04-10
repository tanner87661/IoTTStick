//jpeg functions: https://github.com/pix3lize/M5Stick-ImageViewerSPIFFS/blob/master/src/main.cpp

//display size Dial 240 x 240 px

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

long curEncoderPos = 0;
static m5::touch_state_t prev_state;
int prev_x = -1;
int prev_y = -1;

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
  // The jpeg image can be scaled by a factor of 1, 2, 4, or 8
  TJpgDec.setJpgScale(1);

  // The byte order can be swapped (set true for TFT_eSPI)
  TJpgDec.setSwapBytes(true);

  // The decoder must be given the exact name of the rendering function above
  TJpgDec.setCallback(tft_output);

  setOpeningPage();
}

void processDisplay()
{

  long newPosition = M5Dial.Encoder.read();
  if (newPosition != curEncoderPos) 
  {
        M5Dial.Speaker.tone(8000, 20);
        curEncoderPos = newPosition;
        USBSerial.println(newPosition);
  }

  auto t = M5Dial.Touch.getDetail();
  if (prev_state != t.state) 
  {
    prev_state = t.state;
    static constexpr const char* state_name[16] = {
          "none", "touch", "touch_end", "touch_begin",
          "___",  "hold",  "hold_end",  "hold_begin",
          "___",  "flick", "flick_end", "flick_begin",
          "___",  "drag",  "drag_end",  "drag_begin"};
    M5_LOGI("%s", state_name[t.state]);
    USBSerial.println(state_name[t.state]);
  }
  if (prev_x != t.x || prev_y != t.y) 
  {
    prev_x = t.x;
    prev_y = t.y;
//    M5Dial.Display.drawPixel(prev_x, prev_y);
    USBSerial.print(prev_x);
    USBSerial.print(" ");
    USBSerial.println(prev_y);
  }

  int state = M5.BtnA.wasHold() ? 1
        : M5.BtnA.wasClicked() ? 2
        : M5.BtnA.wasPressed() ? 3
        : M5.BtnA.wasReleased() ? 4
        : M5.BtnA.wasDecideClickCount() ? 5
        : 0;
  switch (state)
  {
    case 2: USBSerial.println("Btn A Clicked"); break;
  }


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

void drawLogo(int x, int y, int logoScale)
{
  String fileName = "/www/iottlogo120120.jpg";
  loadJpgFile(x,y,logoScale, fileName.c_str());
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
  String fileName = "/www/Nickel240240.jpg";
  loadJpgFile(0,0,1, fileName.c_str());
}

void setOpeningPage()
{
  drawBackground();
  drawLogo(60, 60, 1); //120x120 in the middle of 240 x 240
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
  drawBackground();
  drawLogo(90, 30, 2);
//  M5.Display.fillScreen(TFT_LIGHTGREY);
  M5.Display.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  drawText("Connect to AP", 50, 60, 2);
  String hlpStr = "IoTT_Dial_M5_" + String((uint32_t)ESP.getEfuseMac());
  sprintf(outText, hlpStr.c_str());
  drawText(outText, 20, 75, 2);
  drawText("to set Wifi credentials", 30, 90, 2);
}

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
//  uint16_t w = 0, h = 0, scale;
//  TJpgDec.getFsJpgSize(&w, &h, name); // Note name preceded with "/"
  TJpgDec.setJpgScale(shrinkby);

  // Draw the image, top left at 0,0
  TJpgDec.drawFsJpg(x, y, name);
  // How much time did rendering take
//  t = millis() - t;

//  char buf[80];
//  sprintf(buf, "%s %dx%d 1:%d %u ms", name, w, h, scale, t);
//  Serial.println(buf);
}
