#define keepAliveInterval 30000 //send message every 30 secs to keep connection alive
uint32_t keepAlive = millis(); //timer used for periodic message sent over wifi to keep alive while browser is connected. Sent over websocket connection

#define sendDataInterval 250 //send data message every 50 msecs to browser
uint32_t sendDataTimer = millis(); //timer used for periodic message sent over wifi to keep alive while browser is connected. Sent over websocket connection

#define strBufLen 40
typedef struct
{
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
    deleteAllFiles("led", configDir, configExt, true); //delete the last one only to save as much as possible
    request->send(200, "text/plain", "Last set of LED Definitions deleted. Please reload after reboot");
    prepareShutDown();
    delay(1000);
    ESP.restart();
  });
  myWebServer->on("/delete_event", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    deleteAllFiles("btnevt", configDir, configExt, true); //delete the last one only to save as much as possible
    request->send(200, "text/plain", "Last set of event definitions deleted. Please reload after reboot");
    prepareShutDown();
    delay(1000);
    ESP.restart();
  });

//  myWebServer->on("/update", HTTP_POST, [](AsyncWebServerRequest * request) {
//  }, handleUpload);
  //      }, myWebServer->onFileUpload);

/*
  myWebServer->on("/post", HTTP_POST, [](AsyncWebServerRequest * request) {}, NULL, [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total)
  {
    Serial.println("receive post data");
    int headers = request->headers();
    int i;
    String fileName = "";
    for (i = 0; i < headers; i++) {
      AsyncWebHeader* h = request->getHeader(i);
      if (h->name() == "Content-Disposition")
      {
        int first = h->value().indexOf('"');
        int last =  h->value().indexOf('"', first + 1);
        fileName = '/' + h->value().substring(first + 1, last);
        //            fileName = '../update/' + h->value().substring(first+1,last);
        if (!((first > 0) && (last > first)))
          return;
        Serial.printf("Found filename from %i to %i of ", first, last);
        Serial.println(fileName);
        break;
      }
    }
    if (!index)
    {
      uploadFile = SPIFFS.open(fileName.c_str(), "w");
      Serial.printf("UploadStart: %s\n", fileName.c_str());
    }
    int byteOK = uploadFile.write(data, len);
    Serial.printf("writing %i, %i bytes to: %s\n", len, byteOK, fileName.c_str());
    Serial.printf("Result: %i, %i \n", index, total);
    if ((index + len) == total)
    {
      uploadFile.close();
      Serial.printf("Upload Complete: %s\n", fileName.c_str());
      request->send(200, "text/plain", "Upload complete");
      ESP.restart();
    }
  });
*/
/*
  myWebServer->onRequestBody([](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if (!index)
      Serial.printf("BodyStart: %u\n", total);
    Serial.printf("%s", (const char*)data);
    if (index + len == total)
      Serial.printf("BodyEnd: %u\n", total);
  });
*/
  // Send a POST request to <IP>/post with a form field message set to <message>
  ws->onEvent(onWsEvent);
  myWebServer->addHandler(ws);

  myWebServer->onNotFound(notFound);
  myWebServer->serveStatic("/", SPIFFS, "/www/").setDefaultFile("index.htm");
  myWebServer->begin();
  Serial.println("Web Server initialized");
}
/*
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  String hlpStr = "/www/" + filename;
  if (!index) {
    Serial.println((String)"UploadStart: " + hlpStr);
    // open the file on first call and store the file handle in the request object
    request->_tempFile = SPIFFS.open(hlpStr.c_str(), "w");
  }
  if (len) {
    // stream the incoming chunk to the opened file
    request->_tempFile.write(data, len);
  }
  if (final) {
    Serial.println((String)"UploadEnd: " + hlpStr + ", " + index + len);
    // close the file handle as the upload is now done
    request->_tempFile.close();
    request->send(200, "text/plain", "File Uploaded !");
  }
}
*/
/*
   JSON structure for WS communication
   LocoNet commands from LN to app and vice versa
   LED on/off commands from App to CTC panel for testing/identification
   Data load / save for config data and security element/BD/Switch/Route config data{}
   Button report / emulation data
*/

void processStatustoWebClient()
{
  //  Serial.println("Keep alive");
  DynamicJsonDocument doc(512);
  char myStatusMsg[350];
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
//  char buff[10];
  sprintf(myStatusMsg, "%c.%c.%c", BBVersion[0], BBVersion[1], BBVersion[2]);
//  sprintf(buff, "%u.%u.%u", BBVersion[0], BBVersion[1], BBVersion[2]);
  Data["version"] = myStatusMsg;
  Data["ipaddress"] = WiFi.localIP().toString();
  Data["sigstrength"] = WiFi.RSSI();
  Data["apname"] = WiFi.SSID();
/*
  Data["temp"] = M5.Axp.GetTempInAXP192();
  Data["ubat"] = M5.Axp.GetBatVoltage();
  Data["ibat"] =  M5.Axp.GetBatCurrent();
  Data["ubus"] = M5.Axp.GetVBusVoltage();
  Data["ibus"] = M5.Axp.GetVBusCurrent();
  Data["uin"] = M5.Axp.GetVinVoltage();
  Data["iin"] = M5.Axp.GetVinCurrent();
  Data["pwrbat"] = M5.Axp.GetBatPower();
  Data["ibat"] = M5.Axp.GetBatCurrent();
  Data["ubat"] = M5.Axp.GetBatVoltage();
*/
  serializeJson(doc, myStatusMsg);
//  Serial.println(myStatusMsg);
  globalClient->text(myStatusMsg);
  lastWifiUse = millis();
  //  Serial.println("Keep alive done");
}

void sendKeepAlive()
{
  if (fileListRdPtr != fileListWrPtr) //this has priority over keepAlive
  {
    if (millis() > sendDataTimer)
    {
      int hlpRxPtr = (fileListRdPtr + 1) % outListLen;
      sendJSONFile(hlpRxPtr);
      fileListRdPtr = hlpRxPtr;
      sendDataTimer = millis() + sendDataInterval;
    }
  } 
  else
    if (millis() > keepAlive)
    {
      if (globalClient != NULL)
        processStatustoWebClient();
      keepAlive += keepAliveInterval;
    }
}

void sendCTS()
{
  globalClient->text("{\"Cmd\":\"CTS\"}");
}

void sendEndConnection()
{
  globalClient->text("{\"Cmd\":\"EOT\"}");
}

bool addFileToTx(String fileName, int fileIndex, String cmdType, uint8_t multiModeStatus) //0: single file, 1: add multi file 2: reset multifile list 3: write multifile to disk
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
//  Serial.printf("Try to send File %s Type %s as Index %i\n", &outFileList[thisFileIndex].fileName[0], &outFileList[thisFileIndex].cmdType[0], outFileList[thisFileIndex].fileIndex);
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
  if (globalClient != NULL)
    globalClient->text(wsTxBuffer);
}

uint32_t createCfgEntryByName(String fileName, String cmdType, char * toBuffer)
{
  strcat(toBuffer, "\"Type\":\"");
  strcat(toBuffer, cmdType.c_str());
  strcat(toBuffer, "\",\"Data\":");
  uint32_t retStr = readFileToBuffer(configDir + "/" + fileName, &wsTxBuffer[strlen(wsTxBuffer)], wsBufferSize - strlen(wsTxBuffer));
}

/*
String createCfgEntry(String cmdType)
{
  String fileStr = "\"Type\":\"" + cmdType + "\",\"Data\":";
  String fileNameStr;
  if (cmdType == "pgTrMonCfg")
    fileNameStr = configDir + "/trmon" + configDotExt;
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
  if (cmdType == "pgThrottleCfg")
    fileNameStr = configDir + "/throttle" + configDotExt;
  fileStr += readFile(fileNameStr);
  return fileStr;
}
*/

void freeObjects() //free up RAM as ESP will restart at end of updating anyway
{
/*
  if (myChain)
  {
    delete(myChain);
    myChain = NULL;
    yield();
    delay(2000);
  }
*/
/*
  if (eventHandler) 
  {
    delete(eventHandler);
    eventHandler = NULL;
    yield();
    delay(2000);
  }
*/
}

void processWsMessage(char * newMsg, int msgLen, AsyncWebSocketClient * client)
{
  Serial.println(newMsg);
//  Serial.println(String(ESP.getFreeHeap()));
  int docSize = 4096;
//  Serial.println(String(ESP.getMaxAllocHeap()));
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
//      Serial.println(thisCmd);

      if (thisCmd == "ResetDist") //Request to reset distance accumulators
      {
        sensorResetDistance();
      }

      if (thisCmd == "ResetIMU") //Request to reset distance accumulators
      {
        sensorResetIMU();
      }

      if (thisCmd == "ResetPos") //Request to reset distance accumulators
      {
        sensorResetPos();
      }

      if (thisCmd == "StartRecording") //Request to reset distance accumulators
      {
        if (!trackMode)
        {
          Serial.println("Start Rec");
          String newFile = doc["FileName"];
          trackFileName = dataDir + newFile;
          trackMode = openJSONFile(trackFileName);
          lastTrackRecord = millis();
        }
      }

      if (thisCmd == "EndRecording") //Request to reset distance accumulators
      {
        if (trackMode)
        {
          trackMode = false;
          Serial.println("End Rec");
          if (xSemaphoreTake(sensorSemaphore, portMAX_DELAY) == pdPASS) 
          {
            closeJSONFile(trackFileName);
            xSemaphoreGive(sensorSemaphore);
          }
          else
            Serial.println("Failed to get semaphore");
        }
      }

      if (thisCmd == "CfgFiles") //Config Request Format: {"Cmd":"CfgFiles", "Type":"pgxxxxCfg"}
      {
        keepAlive = millis() + keepAliveInterval;
        uint16_t fileSelector = 0xFFFF;
        if (doc.containsKey("Type"))
          fileSelector = doc["Type"];
        addFileToTx("", 0, "pgStartFile", 2); //reset file list
        if (fileSelector & 0x0001)  
          addFileToTx("node", 0, "pgNodeCfg", 1); //add file
        if (fileSelector & 0x0002)  
          addFileToTx("mqtt", 0, "pgMQTTCfg", 1); //add file
        if (fileSelector & 0x0004)  
          addFileToTx("usb", 0, "pgUSBCfg", 1);
        if (fileSelector & 0x0010)  
          addFileToTx("btn", 0, "pgHWBtnCfg", 1);
        if (fileSelector & 0x0100)  
          addFileToTx("btn", 0, "pgThrottleCfg", 1);
        int fileCtr = 0;
        if (fileSelector & 0x0200)  
        {
          if (addFileToTx("greenhat", 0, "pgGreenHatCfg", 1))
          {
            uint8_t modNr = 0;
            if (doc.containsKey("ModuleNr"))
              modNr = doc["ModuleNr"];
            String fileNameStr = "gh/" + String(modNr);
            fileCtr = 0;
            while (addFileToTx(fileNameStr + "/switches", fileCtr, "pgSwitchCfg", 1))
              fileCtr++;
            addFileToTx(fileNameStr + "/btn", 0, "pgHWBtnCfg", 1);
            fileCtr = 0;
            while (addFileToTx(fileNameStr + "/btnevt", fileCtr, "pgBtnHdlrCfg", 1))
              fileCtr++;
            
            addFileToTx(fileNameStr + "/led", 0, "pgLEDCfg", 1);
          }
        }
        if (fileSelector & 0x0400)  
          addFileToTx("lbserver", 0, "pgLBSCfg", 1);
        if (fileSelector & 0x0020)  
          while (addFileToTx("led", fileCtr, "pgLEDCfg", 1))
            fileCtr++;
          fileCtr = 0;
        if (fileSelector & 0x0040)  
          while (addFileToTx("btnevt", fileCtr, "pgBtnHdlrCfg", 1))
            fileCtr++;
        if (fileSelector & 0x0080)  
          while (addFileToTx("trmon", fileCtr, "pgTrMonCfg", 1))
            fileCtr++;
        addFileToTx("", 0, "pgWriteFile", 3); //Write file to disk
      }

      if (thisCmd == "ReqStats") //Request Technical data
        keepAlive = millis();      

      if (thisCmd == "CfgData") //Config Request Format: {"Cmd":"CfgData", "Type":"pgxxxxCfg", "FileName":"name"}
      {
        String cmdType = doc["Type"];
        if (cmdType == "pgLNViewer")
          return;
        if (cmdType == "pgDCCViewer")
          return;
        if (cmdType == "pgOLCBViewer")
          return;
        if (cmdType == "pgTrMonCfg")
          reportSensor = true;

        String fileName = doc["FileName"];
        int fileCtr = 0;
        while (addFileToTx(fileName, fileCtr, cmdType, 0))
          fileCtr++;
      }
      if (thisCmd == "CfgUpdate") //Config Request Format: {"Cmd":"CfgData", "Type":"pgxxxxCfg", "FileType":"xxxx", "FileName":"nnnnx.cfg", "Data":{}}
      {
        execLoop = false;
        const char *  cmdType = doc["Type"];
        const char * fileStr = doc["Data"];
        const char *  fileName = doc["FileName"];
        const char *  fileNameType = doc["FileNameType"];
        int fileIndex = doc["Index"];
        if (strcmp(cmdType, "pgDelete") == 0)
        {
          deleteAllFiles(fileNameType, configDir, configExt, false);
          freeObjects();
          sendCTS();
          return;
        }
        writeJSONFile(configDir + "/" + fileName, fileStr);
        if (!doc.containsKey("Restart")) //if old format (before multi file), then restart
        {
          Serial.println("Restart ESP");
          sendCTS();
          prepareShutDown();
          delay(500);
          ESP.restart(); //configuration update requires restart to be sure dynamic allocation of objects is not messed up
        }
        else
          if (doc["Restart"])
          {
            Serial.println("Reboot ESP");
            sendCTS();
            prepareShutDown();
            delay(500);
            ESP.restart(); //configuration update requires restart to be sure dynamic allocation of objects is not messed up
          }
//          else
//            Serial.println("No Reboot needed");
        sendCTS();
      }
    }
  } 
  else
    Serial.printf("processWsMessage deserializeJson() wsProcessing failed: %s\n", error.c_str());
//  yield();
}

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{
  lastWifiUse = millis();
//  Serial.printf("WS Event %i \n", globalClient);
  switch (type)
  {
    case WS_EVT_CONNECT:
      {
        globalClient = client;
        keepAlive = millis() + 500;
        Serial.printf("Websocket client connection received from %u\n", client->id());
        break;
      }
    case WS_EVT_DISCONNECT:
      {
        globalClient = NULL;
        reportSensor = false;
        Serial.println("Client disconnected");
        break;
      }
    case WS_EVT_DATA:
      {
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
        break;
      }
    case WS_EVT_ERROR:
      {
        break;
      }
  }
}
