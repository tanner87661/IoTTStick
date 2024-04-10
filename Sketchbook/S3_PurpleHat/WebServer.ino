#define keepAliveInterval 30000 //send message every 30 secs to keep connection alive
uint32_t keepAlive = millis(); //timer used for periodic message sent over wifi to keep alive while browser is connected. Sent over websocket connection

#define sendDataInterval 250 //send data message every 50 msecs to browser
uint32_t sendDataTimer = millis(); //timer used for periodic message sent over wifi to keep alive while browser is connected. Sent over websocket connection

#define strBufLen 40
typedef struct
{
  AsyncWebSocketClient * targetClient;
  char fileName[strBufLen];
  char fileNameType[strBufLen];
  char cmdType[strBufLen];
  int  fileIndex;
  uint8_t multiFileMode;
} fileListEntry;

#define outListLen 50

fileListEntry outFileList[outListLen];
int fileListRdPtr = 0;
int fileListWrPtr = 0;

//------------------------------------------------------------------------------------------------------------------------------------------------

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void startWebServer()
{
  if (!myWebServer) return;
  myWebServer->on("/delete_led", HTTP_GET, [](AsyncWebServerRequest * request)
  {
//    deleteAllFiles("led", configDir, configExt, true); //delete the last one only to save as much as possible
    request->send(200, "text/plain", "Last set of LED Definitions deleted. Please reload after reboot");
//    prepareShutDown();
    delay(1000);
    ESP.restart();
  });
  myWebServer->on("/delete_event", HTTP_GET, [](AsyncWebServerRequest * request)
  {
//    deleteAllFiles("btnevt", configDir, configExt, true); //delete the last one only to save as much as possible
    request->send(200, "text/plain", "Last set of event definitions deleted. Please reload after reboot");
//    prepareShutDown();
    delay(1000);
    ESP.restart();
  });

//  myWebServer->on("/update", HTTP_POST, [](AsyncWebServerRequest * request) {
//  }, handleUpload);
  //      }, myWebServer->onFileUpload);

  // Send a POST request to <IP>/post with a form field message set to <message>
  ws->onEvent(onWsEvent);
  myWebServer->addHandler(ws);

  myWebServer->onNotFound(notFound);
  myWebServer->serveStatic("/", SPIFFS, "/www/").setDefaultFile("index.htm");
  myWebServer->begin();
  Serial.println("Web Server initialized");
}

void processStatustoWebClient()
{
//  Serial.println("Keep alive");
  DynamicJsonDocument doc(640);
  char myStatusMsg[600];
  doc["Cmd"] = "STATS";
  JsonObject Data = doc.createNestedObject("Data");
  float float1 = (millisRollOver * 4294967296) + millis(); //calculate millis including rollovers
  Data["uptime"] = round(float1);
//  Data["uptime"] = millis();
  if (useNTP)// && ntpOK) no longer needed in stick as we have RTC
  {
    now = time(0);
//    char buff[40]; //39 digits plus the null char
    strftime(myStatusMsg, 40, "%m-%d-%Y %H:%M:%S", localtime(&now));
    Data["systime"] = myStatusMsg;
  }
  Data["freemem"] = String(ESP.getFreeHeap());
  Data["totaldisk"] = String(SPIFFS.totalBytes());
  Data["useddisk"] = String(SPIFFS.usedBytes());
  Data["freedisk"] = String(SPIFFS.totalBytes() - SPIFFS.usedBytes());
  sprintf(myStatusMsg, "%s", BBVersion.c_str());
  Data["version"] = myStatusMsg;
  Data["ipaddress"] = WiFi.localIP().toString();
  Data["sigstrength"] = WiFi.RSSI();
  Data["apname"] = WiFi.SSID();

  Data["temp"] = axpIntTemp;
  Data["ubat"] = axpBattVoltage;
  Data["ibat"] =  axpBattCurr;
  Data["pwrbat"] = axpBattPower;
  Data["ubus"] = axpVBUSVoltage;
  Data["ibus"] = axpVBUSCurrent;
  Data["uin"] = axpACInVoltage;
  Data["iin"] = axpACInCurrent;

  serializeJson(doc, myStatusMsg);
//  Serial.println(myStatusMsg);

  for (int i = 0; i < globalClients.size(); i++)
    globalClients[i].wsClient->text(myStatusMsg);
  lastWifiUse = millis();
//  Serial.println("Keep alive done");
}

void sendKeepAlive()
{
  if (fileListRdPtr != fileListWrPtr) //this has priority over keepAlive
  {
    if (millis() > sendDataTimer)
    {
//    Serial.println("Send File");
      int hlpRxPtr = (fileListRdPtr + 1) % outListLen;
      sendJSONFile(hlpRxPtr);
      fileListRdPtr = hlpRxPtr;
      sendDataTimer = millis() + sendDataInterval;
//    Serial.println("Send File done");
    }
  } 
  else
    if (millis() > keepAlive)
    {
      if (globalClients.size() > 0)
        processStatustoWebClient();
      keepAlive += keepAliveInterval;
    }
}

void sendCTS(AsyncWebSocketClient * client)
{
  client->text("{\"Cmd\":\"CTS\"}");
}

/*
void sendEndConnection(AsyncWebSocketClient * client)
{
  client->text("{\"Cmd\":\"EOT\"}");
}
*/

bool addFileToTx(AsyncWebSocketClient * toClient, String fileName, int fileIndex, String cmdType, uint8_t multiModeStatus) //0: single file, 1: add multi file 2: reset multifile list 3: write multifile to disk
{

  int tmpWrPtr = (fileListWrPtr + 1) % outListLen;
  if (tmpWrPtr != fileListRdPtr) //protect override
  {
    String thisFileName = fileName;
//    String thisFileName = configDir + "/" + fileName;
    if (fileIndex > 0)
      thisFileName += String(fileIndex);
    thisFileName += configDotExt;
    outFileList[tmpWrPtr].multiFileMode = multiModeStatus;
    outFileList[tmpWrPtr].targetClient = toClient;
    cmdType.toCharArray(outFileList[tmpWrPtr].cmdType, strBufLen);
    fileName.toCharArray(outFileList[tmpWrPtr].fileNameType, strBufLen);
//    Serial.println(thisFileName);
    if (multiModeStatus > 1)
    {
      fileListWrPtr = tmpWrPtr;
//      Serial.println("Adding File command");
      return true;
    }
    else
      if (SPIFFS.exists(configDir + "/" + thisFileName))
      {
        thisFileName.toCharArray(outFileList[tmpWrPtr].fileName, strBufLen);
        outFileList[tmpWrPtr].fileIndex = fileIndex;
//        Serial.printf("Adding File %s Type %s as Index %i\n", &outFileList[tmpWrPtr].fileName[0], &outFileList[tmpWrPtr].cmdType[0], outFileList[tmpWrPtr].fileIndex);
        fileListWrPtr = tmpWrPtr;
        return true;
      }
  }
//  Serial.println("failed");
  return false;

}

void sendJSONFile(int thisFileIndex)
{

//  Serial.printf("Try to send Mode %i File %s Type %s as Index %i\n", outFileList[thisFileIndex].multiFileMode, &outFileList[thisFileIndex].fileName[0], &outFileList[thisFileIndex].cmdType[0], outFileList[thisFileIndex].fileIndex);
  String fileStr;
  uint32_t retStr = 0;//"";
  wsTxWritePtr = 0;
  wsTxReadPtr = 0;
  switch (outFileList[thisFileIndex].multiFileMode)
  {
    case 0: //single file
      if (outFileList[thisFileIndex].fileIndex == 0)
        strcpy(wsTxBuffer, "{\"Cmd\":\"CfgData\", \"ResetData\":true, ");
      else
        strcpy(wsTxBuffer, "{\"Cmd\":\"CfgData\", \"ResetData\":false, ");
      retStr = strlen(wsTxBuffer);
      retStr = createCfgEntryByName(&outFileList[thisFileIndex].fileName[0], &outFileList[thisFileIndex].cmdType[0], &wsTxBuffer[strlen(wsTxBuffer)]);
      break;
    case 1: //multi file adding
      strcpy(wsTxBuffer, "{\"Cmd\":\"CfgFiles\", \"FileMode\":1, \"FileNameType\":\"");
      strcat(wsTxBuffer, outFileList[thisFileIndex].fileNameType);
      strcat(wsTxBuffer, "\", \"FileName\":\"");
      strcat(wsTxBuffer, outFileList[thisFileIndex].fileName);
      strcat(wsTxBuffer, "\",");
      retStr = strlen(wsTxBuffer);
      retStr = createCfgEntryByName(&outFileList[thisFileIndex].fileName[0], &outFileList[thisFileIndex].cmdType[0], &wsTxBuffer[strlen(wsTxBuffer)]);
      break;
    case 2: //start new multifile
      strcpy(wsTxBuffer, "{\"Cmd\":\"CfgFiles\", \"FileMode\":2");
      break;
    case 3: //write multifile to disk
      strcpy(wsTxBuffer, "{\"Cmd\":\"CfgFiles\", \"FileMode\":3");
      break;
  }
  strcat(wsTxBuffer, "}");
//  Serial.println(wsTxBuffer);
    outFileList[thisFileIndex].targetClient->text(wsTxBuffer);
//    for (int8_t i = 0; i < globalClients.size(); i++)
//      globalClients[i].wsClient->text(wsTxBuffer);

}

uint32_t createCfgEntryByName(String fileName, String cmdType, char * toBuffer)
{

  strcat(toBuffer, "\"Type\":\"");
  strcat(toBuffer, cmdType.c_str());
  strcat(toBuffer, "\",\"Data\":");
//  uint32_t retStr = readFileToBuffer(configDir + "/" + fileName, &wsTxBuffer[strlen(wsTxBuffer)], wsBufferSize - strlen(wsTxBuffer));
  return readFileToBuffer(configDir + "/" + fileName, &wsTxBuffer[strlen(wsTxBuffer)], wsBufferSize - strlen(wsTxBuffer));

}

/*
String createCfgEntry(String cmdType)
{
  String fileStr = "\"Type\":\"" + cmdType + "\",\"Data\":";
  String fileNameStr;
  if (cmdType == "pgNodeCfg")
    fileNameStr = configDir + "/node" + configDotExt;
  if (cmdType == "pgMQTTCfg")
    fileNameStr = configDir + "/mqtt" + configDotExt;
  if (cmdType == "pgHWBtnCfg")
    fileNameStr = configDir + "/btn" + configDotExt;
  if (cmdType == "pgBtnHdlrCfg")
    fileNameStr = configDir + "/btnevt" + configDotExt;
  if (cmdType == "pgLEDCfg")
    fileNameStr = configDir + "/usb" + configDotExt;
  if (cmdType == "pgUSBCfg")
    fileNameStr = configDir + "/usb" + configDotExt;
  if (cmdType == "pgLBSCfg")
    fileNameStr = configDir + "/lbserver" + configDotExt;
  if (cmdType == "pgWiCfg")
    fileNameStr = configDir + "/wiclient" + configDotExt;
  if (cmdType == "pgThrottleCfg")
    fileNameStr = configDir + "/throttle" + configDotExt;
  fileStr += readFile(fileNameStr);
  return fileStr;
}
*/

void freeObjects() //free up RAM as ESP will restart at end of updating anyway
{
}

void processWsMessage(char * newMsg, int msgLen, AsyncWebSocketClient * client)
{
//  Serial.println(newMsg);
//  Serial.println(String(ESP.getFreeHeap()));
//  Serial.println(String(ESP.getMaxAllocHeap()));
  int docSize = 4096;
  DynamicJsonDocument doc(docSize);
//  Serial.println(String(ESP.getFreeHeap()));
  char duplMsg[strlen(newMsg) + 1];
  strcpy(duplMsg, newMsg);
//  Serial.println(String(ESP.getFreeHeap()));
  DeserializationError error = deserializeJson(doc, duplMsg);//, msgLen);
//  Serial.println(String(ESP.getFreeHeap()));

  if (!error)
  {
    if (doc.containsKey("Cmd"))
    {
      String thisCmd = doc["Cmd"];
      USBSerial.println(thisCmd);
/*
      if (thisCmd == "SetLED") //Request to switch on LED for identification purposes
      {
        JsonArray ledList = doc["LedNr"];
        for (int i = 0; i < ledList.size(); i++)
        {
//          Serial.printf("Setting Test LED %i\n", (uint16_t)ledList[i]);
          if (myChain) myChain->identifyLED(ledList[i]);
          if (mySwitchList) mySwitchList->identifyLED(ledList[i]);
        }
      }
      if (thisCmd == "SetServo") //Request to move Servo for position verification purposes
      {
        JsonArray servoList = doc["ServoNr"];
        for (int i = 0; i < servoList.size(); i++)
        {
//          Serial.printf("Setting Test LED %i\n", (uint16_t)ledList[i]);
          if (mySwitchList) mySwitchList->moveServo(servoList[i], doc["ServoPos"]);
        }
      }
*/
      if (thisCmd == "CfgFiles") //Config Request Format: {"Cmd":"CfgFiles", "Type":"pgxxxxCfg"}
      {
        keepAlive = millis() + keepAliveInterval;
        uint16_t fileSelector = 0xFFFF;
        if (doc.containsKey("Type"))
          fileSelector = doc["Type"];
        addFileToTx(client, "", 0, "pgStartFile", 2); //reset file list
        if (fileSelector & 0x0001)  
        if (fileSelector & 0x0002)  
          addFileToTx(client, "mqtt", 0, "pgMQTTCfg", 1); //add file
        if (fileSelector & 0x0004)  
          addFileToTx(client, "usb", 0, "pgUSBCfg", 1);
        if (fileSelector & 0x0010)  
          addFileToTx(client, "btn", 0, "pgHWBtnCfg", 1);
        if (fileSelector & 0x0100)  
          addFileToTx(client, "btn", 0, "pgThrottleCfg", 1);
        int fileCtr = 0;
        if (fileSelector & 0x0200)  
        {
          if (addFileToTx(client, "greenhat", 0, "pgGreenHatCfg", 1))
          {
            uint8_t modNr = 0;
            if (doc.containsKey("ModuleNr"))
              modNr = doc["ModuleNr"];
            String fileNameStr = "gh/" + String(modNr);
//            addFileToTx(client, fileNameStr + "/switches", 0, "pgSwitchCfg", 1);
//            addFileToTx(client, fileNameStr + "/btn", 0, "pgHWBtnCfg", 1);

            fileCtr = 0;
            while (addFileToTx(client, fileNameStr + "/switches", fileCtr, "pgSwitchCfg", 1))
              fileCtr++;
            fileCtr = 0;
            while (addFileToTx(client, fileNameStr + "/btn", fileCtr, "pgHWBtnCfg", 1))
              fileCtr++;
            fileCtr = 0;
            while (addFileToTx(client, fileNameStr + "/btnevt", fileCtr, "pgBtnHdlrCfg", 1))
              fileCtr++;
//            addFileToTx(client, fileNameStr + "/btnevt", 0, "pgBtnHdlrCfg", 1);

            fileCtr = 0;
            while (addFileToTx(client, fileNameStr + "/led", fileCtr, "pgLEDCfg", 1))
              fileCtr++;
//            addFileToTx(client, fileNameStr + "/led", 0, "pgLEDCfg", 1);
          }
        }
        if (fileSelector & 0x0400)  
          addFileToTx(client, "lbserver", 0, "pgLBSCfg", 1);
        if (fileSelector & 0x0800)  
          addFileToTx(client, "vwcfg", 0, "pgVoiceWCfg", 1);
        if (fileSelector & 0x1000)  
          addFileToTx(client, "rhcfg", 0, "pgRedHatCfg", 1);
        if (fileSelector & 0x2000)  
          addFileToTx(client, "phcfg", 0, "pgPrplHatCfg", 1);
        if (fileSelector & 0x4000)  
          addFileToTx(client, "wiclient", 0, "pgWiCfg", 1);
        fileCtr = 0;
        if (fileSelector & 0x0020)  
          while (addFileToTx(client, "led", fileCtr, "pgLEDCfg", 1))
            fileCtr++;
        fileCtr = 0;
        if (fileSelector & 0x0040)  
          while (addFileToTx(client, "btnevt", fileCtr, "pgBtnHdlrCfg", 1))
            fileCtr++;
        fileCtr = 0;
        if (fileSelector & 0x0080)  
          while (addFileToTx(client, "secel", fileCtr, "pgSecElCfg", 1))
            fileCtr++;
        addFileToTx(client, "", 0, "pgWriteFile", 3); //Write file to disk
      }

      if (thisCmd == "ReqStats") //Request Technical data
        keepAlive = millis();      

      if (thisCmd == "CfgData") //Config Request Format: {"Cmd":"CfgData", "Type":"pgxxxxCfg", "FileName":"name"}
      {
//        Serial.println(espconn_tcp_get_max_con_allow());
//        Serial.println(espconn_tcp_get_max_con());
        String cmdType = doc["Type"];
        int8_t thisClient = getWSClient(client->id()); 
        if (thisClient >= 0)
          strcpy(globalClients[thisClient].pageName, cmdType.c_str());
//        Serial.printf("%i %s \n", thisClient, cmdType);
        if (cmdType == "pgLNViewer")
          return;
        if (cmdType == "pgDCCViewer")
          return;
        if (cmdType == "pgOLCBViewer")
          return;

        String fileName = doc["FileName"];
        int fileCtr = 0;
        while (addFileToTx(client, fileName, fileCtr, cmdType, 0))
          fileCtr++;
      }
      if (thisCmd == "CfgUpdate") //Config Request Format: {"Cmd":"CfgData", "Type":"pgxxxxCfg", "FileType":"xxxx", "FileName":"nnnnx.cfg", "Data":{}}
      {
//        execLoop = false;
        const char *  cmdType = doc["Type"];
        const char * fileStr = doc["Data"];
        const char *  fileName = doc["FileName"];
        const char *  fileNameType = doc["Fi//leNameType"];
        int fileIndex = doc["Index"];
        if (strcmp(cmdType, "pgDelete") == 0)
        {
          deleteAllFiles(fileNameType, configDir, configExt, false);
          freeObjects();
          sendCTS(client);
          return;
        }
        writeJSONFile(configDir + "/" + fileName, fileStr);
        if (!doc.containsKey("Restart")) //if old format (before multi file), then restart
        {
          prepareShutDown();
          delay(500);
          Serial.println("Restart ESP");
          sendCTS(client);
          delay(100);
          ESP.restart(); //configuration update requires restart to be sure dynamic allocation of objects is not messed up
        }
        else
          if (doc["Restart"])
          {
            prepareShutDown();
            delay(500);
            Serial.println("Reboot ESP");
            sendCTS(client);
            delay(100);
            ESP.restart(); //configuration update requires restart to be sure dynamic allocation of objects is not messed up
          }
//          else
//            Serial.println("No Reboot needed");
        sendCTS(client);
      }
/*
      if (thisCmd == "SetSensor")  
      {
//        Serial.println(thisCmd);
        if (doc.containsKey("SubCmd"))
        {
          String subCmd = doc["SubCmd"];
          if (subCmd == "ClearDist")
            if (trainSensor) 
              trainSensor->resetDistance();
          if (subCmd == "ClearHeading")
            if (trainSensor) 
              trainSensor->resetHeading();
          if (subCmd == "RepRate")
            if (trainSensor) 
            {
              uint16_t repRate = doc["Val"];
              trainSensor->setRepRate(repRate);
            }
          if (subCmd == "SetDCC")
            if (trainSensor)
            {
              int16_t dccAddr = doc["Addr"];
              trainSensor->reqDCCAddrWatch(dccAddr, (useInterface.devId == 17) || (useInterface.devId == 18)); //17: WiThrottle Client 18: WiThrottle DCC EX format
            }
          if (subCmd == "RunTest")
            if (trainSensor)
            {
              float tLen = doc["TrackLen"];
              float vMax = doc["VMax"];
              JsonArray speedPOI = doc["POI"];
              std::vector<float> poiMode;
              if (speedPOI.size() > 0)
              {
                for (uint8_t i = 0; i < speedPOI.size(); i++)
                {
                  float thisVal = speedPOI[i];
                  poiMode.push_back(thisVal);
                }
              }
              trainSensor->startTest(tLen, vMax, poiMode);
            }
          if (subCmd == "ReadCV")
          {
            uint16_t dccAddr = doc["Addr"]; 
            uint8_t progMode = doc["ProgMode"]; //read SM direct, 
            uint8_t progMethod = doc["ProgMethod"];
            uint8_t cvNr = doc["CV"];
            uint8_t cmdCode = 0x01; //read direct mode byte
            switch (progMode)
            {
              case 0: //prog track
                switch (progMethod)
                {
                  case 0: break;//direct mode is preset
                  case 1: cmdCode = 0; break;//page mode
                }
                break;
              case 1: cmdCode = 0x04; break;//main line read, requires transponding
            }
            digitraxBuffer->progCVProc(dccAddr, cmdCode, cvNr, 0);
          }
          if (subCmd == "WriteCV")
          {
            uint16_t dccAddr = doc["Addr"]; 
            uint8_t progMode = doc["ProgMode"];
            uint8_t progMethod = doc["ProgMethod"];
            uint8_t cvNr = doc["CV"];
            uint8_t cvVal = doc["CVVal"];
            uint8_t cmdCode = 0x09; //write direct mode byte
            switch (progMode)
            {
              case 0: //prog track
                switch (progMethod)
                {
                  case 0: break;//direct mode is preset
                  case 1: cmdCode = 0x08; break;//page mode
                }
                break;
              case 1: cmdCode = 0x0C; break;//main line write
            }
            digitraxBuffer->progCVProc(dccAddr, cmdCode, cvNr, cvVal);
          }
          if (subCmd == "StopTest")
            if (trainSensor)
              trainSensor->stopTest();
        }
      }

      if (thisCmd == "ResetSlots")
      {
        digitraxBuffer->clearSlotBuffer(true);
        if (lnSerial)
          lnSerial->sendLineBreak(50);
      }
      if (thisCmd == "SetFC")
      {
        if (doc.containsKey("FCTime"))
        {
          uint32_t newTime = doc["FCTime"];
          digitraxBuffer->setFCTime(newTime, true);
        }
        if (doc.containsKey("FCRate"));
        {
          uint8_t newRate = doc["FCRate"];
          digitraxBuffer->setFCRate(newRate, true);
        }
      }
*/
/*
      if (thisCmd == "GetClients")
        if (doc.containsKey("SubCmd"))
        {
          String subCmd = doc["SubCmd"];
          if (subCmd == "WI")
            if (wiThServer)
              wiThServer->updateClientList();
          if (subCmd == "LN")
            if (lbServer)
              lbServer->updateClientList();
        }
*/
/*
      if (thisCmd == "GetFC")
        digitraxBuffer->sendFCCmdToWeb();
      if (thisCmd == "SetDCCPP")  
      {
        if (doc.containsKey("SubCmd"))
        {
          String subCmd = doc["SubCmd"];
          if (subCmd == "SendCmd")
          {
              String cmdStr = doc["OpCode"];
              digitraxBuffer->sendRedHatCmd(&cmdStr[0]);
          }
          if (subCmd == "GetConfig")
            if (digitraxBuffer) 
            {
              uint16_t cFlags = doc["Filter"];
              digitraxBuffer->getRedHatConfig(cFlags);
            }
          if (subCmd == "GetCurrent")
          {
            uint8_t trackId = doc["OpCode"];
            digitraxBuffer->sendTrackCurrent(trackId);
          }

        }
      }
      if (thisCmd == "LNOut")  
      {
//        Serial.println("Send LN");
        switch (useInterface.devId)
        {
          case 2:;
          case 3:;
          case 12:;
          case 16:
          {
            JsonArray lnData = doc["Data"];
            lnTransmitMsg txData;
            uint8_t msgSize = lnData.size();
            for (int i = 0; i < msgSize; i++)
              txData.lnData[i] = lnData[i];
            txData.lnMsgSize = ((txData.lnData[0] & 0x60) >> 4) + ((txData.lnData[0] & 0x80) >> 7); //-1 for indexing
            if (txData.lnMsgSize == 7)
              txData.lnMsgSize = txData.lnData[1];
            else
              txData.lnMsgSize++;
            setXORByte(&txData.lnData[0]);

//  Serial.printf("Msg Len %i\n", txData.lnMsgSize);
//  for (uint8_t i = 0; i < txData.lnMsgSize; i++)
//    Serial.printf("%2X ", txData.lnData[i]);
//  Serial.println();
            
            sendMsg(txData);
          }
          break;
        }
      }
*/
/*
      if (thisCmd == "BST")  
      {
        if (usbSerial)
        {
          if (doc.containsKey("SV"))
          {
            JsonArray lnData = doc["SV"];
            lnTransmitMsg txData;
            uint8_t msgSize = lnData.size();
            for (int i = 0; i < msgSize; i++)
              txData.lnData[i] = lnData[i];
            txData.lnMsgSize = ((txData.lnData[0] & 0x60) >> 4) + ((txData.lnData[0] & 0x80) >> 7); //-1 for indexing
            if (txData.lnMsgSize == 7)
              txData.lnMsgSize = txData.lnData[1];
            else
              txData.lnMsgSize++;
            setXORByte(&txData.lnData[0]);
//  Serial.printf("Msg Len %i\n", txData.lnMsgSize);
//  for (uint8_t i = 0; i < txData.lnMsgSize; i++)
//    Serial.printf("%2X ", txData.lnData[i]);
//  Serial.println();
            
            usbSerial->lnWriteMsg(txData);
          }
        }
      }
*/
    } 
  } 
  else
    Serial.printf("processWsMessage deserializeJson() wsProcessing failed: %s\n", error.c_str());
  yield();
}

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{
  int8_t currClient = -1;  
  lastWifiUse = millis();
//  Serial.printf("WS Event %i \n", globalClient);
  switch (type)
  {
    case WS_EVT_CONNECT:
      {
        wsClientInfo thisClient;
        thisClient.wsClient = client;
        thisClient.pageName[0] = '\0'; 
        globalClients.push_back(thisClient);
        keepAlive = millis() + 500;
        Serial.printf("Websocket client connection received from %u\n", client->id());
        break;
      }
    case WS_EVT_DISCONNECT:
      {
        currClient = getWSClient(client->id());
        if (currClient >= 0)
        {
          Serial.printf("Websocket client disconnected from %u %i\n", client->id(), currClient);
          globalClients.erase(globalClients.begin() + currClient);
        }
        break;
      }
    case WS_EVT_DATA:
      {
//        Serial.println("WS Data");
        AwsFrameInfo * info = (AwsFrameInfo*)arg;
        //      String msg = "";
        if (info->final && info->index == 0 && info->len == len)
        {
          //the whole message is in a single frame and we got all of it's data
          wsRxReadPtr = 0;
          if (info->opcode == WS_TEXT)
          {
            for (size_t i = 0; i < info->len; i++)
            {
              wsRxBuffer[wsRxReadPtr] = (char) data[i];
              wsRxReadPtr++;
            }
            wsRxBuffer[wsRxReadPtr] = char(0);
            processWsMessage(wsRxBuffer, wsRxReadPtr, client);
          }
          else
          {
            if (info->opcode == WS_BINARY)
            {

            }
          }
        }
        else
        {
          //message is comprised of multiple frames or the frame is split into multiple packets
          if (info->index == 0)
          {
            wsRxReadPtr = 0;
//            Serial.println("Reset Read Ptr");
            //          if(info->num == 0)
            //            Serial.printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
            //          Serial.printf("multi ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
          }

//          Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: \n", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT) ? "text" : "binary", info->index, info->index + len);

          if (info->opcode == WS_TEXT)
          {
//            Serial.println("adding...");
            for (size_t i = 0; i < len; i++)
            {
              wsRxBuffer[wsRxReadPtr] = (char) data[i];
              wsRxReadPtr++;
            }
            wsRxBuffer[wsRxReadPtr] = char(0);
          }
          else
          {
            //no processing of non-text data at this time
          }
          wsRxBuffer[wsRxReadPtr] = char(0);
//          Serial.println(wsRxReadPtr);
          if ((info->index + len) == info->len)
          {
            if (info->final)
            {
              if (info->message_opcode == WS_TEXT)
              {
//                Serial.println("Processing");
//                Serial.println(&wsRxBuffer[0]);                
                processWsMessage(wsRxBuffer, wsRxReadPtr, client);
              }
//              else
//                Serial.println("Type mismatch");
            }
//            else
//              Serial.println(&wsRxBuffer[0]);                
          }
//          else
//            Serial.println("Length mismatch");
        }
        break;
      }
    case WS_EVT_PONG:
      {
        Serial.println("WS Pong");
        break;
      }
    case WS_EVT_ERROR:
      {
        Serial.println("WS Error");
        break;
      }
  }
//  Serial.println("processed");
}
