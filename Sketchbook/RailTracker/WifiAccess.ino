TaskHandle_t taskHandleBlink = NULL;

void burstLED(uint8_t outData, uint8_t burstLen)
{
    digitalWrite(LED_BUILTIN, 0);
    delay(burstLen);
    digitalWrite(LED_BUILTIN, 1);
}

void blinkTask(void * thisParam)
{
  pinMode(LED_BUILTIN, OUTPUT);
  while (true)
  {
    bool posLog = (*((uint8_t *) thisParam)) == 0x01;
    burstLED(0x00, 10);
    if (((!posLog) && (globalClient != NULL)) || (posLog && (WiFi.status() == WL_CONNECTED)))
    {
      delay(250);
      burstLED(0x00, 10);
    }
    delay(posLog ? 2000 : 500);
  }
}

void establishWifiConnection(AsyncWebServer * webServer,DNSServer * dnsServer)
{
  if (taskHandleBlink == NULL)
    xTaskCreate(    blinkTask,        /* Task function. */
                    "BlinkTask",      /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    &wifiCfgMode,     /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    &taskHandleBlink);            /* Task handle. */
 

    AsyncWiFiManager wifiManager(webServer,dnsServer);
    //reset settings - for testing
//    wifiManager.resetSettings();

//    setWifiConnectPage();
    
    if (useStaticIP) //set static IP information, if DHCP is not used for STA connection
    {
      Serial.println("Set Static IP Mode");
      wifiManager.setSTAStaticIPConfig(static_ip, static_gw, static_nm, static_dns);
    }
    if (wifiCfgMode == 1)  //STA mode is used, so wifiManager can handle AP selection and password
    {
        //sets timeout until configuration portal gets turned off
        //useful to make it all retry or go to sleep
        //in seconds
        Serial.println("Set STA Mode");
        wifiManager.setTimeout(120); 
        String hlpStr = "CMMME_" + String((uint32_t)ESP.getEfuseMac());
        if (!wifiManager.autoConnect(hlpStr.c_str()))
        {
          Serial.println("failed to connect and hit timeout, setting up AP, if configured, otherwise restart ESP32");
          wifiCfgMode  = 2;        //set AP Mode
        }
        else
          Serial.println(WiFi.localIP());
    }
    //AP Mode is fallback position 
    if (wifiCfgMode == 2) //if AP is needed, define the AP settings
    {
      WiFi.mode(WIFI_AP);
      WiFi.softAP(deviceName.c_str(), apPassword.c_str());
      delay(300); //wait for AP to get started
      WiFi.softAPConfig(ap_ip, ap_ip, ap_nm);
      Serial.print("Local Access Point at ");
      Serial.println(WiFi.softAPIP());
    }
    
    wifiCancelled = false; //reset cancellation mode
    lastWifiUse = millis(); //reset timeout timer
    
//    m5CurrentPage = 3;
    oldWifiStatus = 0xFFFF;
}

void checkWifiTimeout() //check if wifi can be switched off
{
  if (!wifiCancelled)
    if (!wifiAlwaysOn)
    {
      uint32_t shutTimeOut = wifiCfgMode == 1 ? wifiShutTimeout : apShutTimeout;
      if (millis() > (lastWifiUse + shutTimeOut))
      {
        wifiCancelled = true;
        oldWifiStatus = 0xFFFF; //impossible mode, requires update
        WiFi.disconnect(); 
        WiFi.mode(WIFI_OFF);
        delay(100); 
        if (taskHandleBlink)
        {
          vTaskDelete(taskHandleBlink);
          taskHandleBlink = NULL;
        }
        Serial.println("Wifi disabled. Restart device to re-enable");
      }
    }
}

/*
void getInternetTime() //periodically connect to an NTP server and get the current time
{
  
  int thisIntervall = ntpIntervallDefault;
  if (!ntpOK)
    thisIntervall = ntpIntervallShort;
  if (millis() > (ntpTimer + thisIntervall))
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      time(&now);
      localtime_r(&now, &timeinfo);
      if (timeinfo.tm_year <= (2016 - 1900)) // the NTP call was not successful
      {
        ntpOK = false;
        return;
      }
      else //success, so update RTC clock
      {      
        RTC_TimeTypeDef TimeStruct;
        TimeStruct.Hours   = timeinfo.tm_hour;
        TimeStruct.Minutes = timeinfo.tm_min;
        TimeStruct.Seconds = timeinfo.tm_sec;
        M5.Rtc.SetTime(&TimeStruct);
        RTC_DateTypeDef DateStruct;
        DateStruct.WeekDay = timeinfo.tm_wday;
        DateStruct.Month = timeinfo.tm_mon + 1;
        DateStruct.Date = timeinfo.tm_mday;
        DateStruct.Year = timeinfo.tm_year + 1900;
        M5.Rtc.SetData(&DateStruct);
      }
      ntpTimer = millis();
      char time_output[30];
      strftime(time_output, 30, "%a  %d-%m-%y %T", localtime(&now));
      Serial.println(time_output);
      ntpOK = true;
    }
  }
}
*/
