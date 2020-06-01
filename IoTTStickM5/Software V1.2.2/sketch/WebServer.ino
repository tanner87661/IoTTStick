
void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void startWebServer()
{
  if (!myWebServer) return;
  myWebServer->on("/heap", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });

  /*    myWebServer->on("/update", HTTP_POST, [](AsyncWebServerRequest *request)
      {
    //      request->sendHeader("Connection", "close");
        request->send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
        ESP.restart();
      }, []()
      {
        HTTPUpload& upload = server.upload();
        if (upload.status == UPLOAD_FILE_START)
        {
          Serial.printf("Update: %s\n", upload.filename.c_str());
          if (!Update.begin(UPDATE_SIZE_UNKNOWN))
          { //start with max available size
            Update.printError(Serial);
          }
        }
        else
          if (upload.status == UPLOAD_FILE_WRITE)
          {
          // flashing firmware to ESP
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
            {
              Update.printError(Serial);
            }
          }
            else
              if (upload.status == UPLOAD_FILE_END)
              {
                if (Update.end(true))
                { //true to set the size to the current progress
                  Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
                }
                else
                {
                  Update.printError(Serial);
                }
              }
      }
    );
  */
  myWebServer->on("/update", HTTP_POST, [](AsyncWebServerRequest * request) {
  }, handleUpload);
  //      }, myWebServer->onFileUpload);

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

  myWebServer->onRequestBody([](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if (!index)
      Serial.printf("BodyStart: %u\n", total);
    Serial.printf("%s", (const char*)data);
    if (index + len == total)
      Serial.printf("BodyEnd: %u\n", total);
  });

  // Send a POST request to <IP>/post with a form field message set to <message>
  ws->onEvent(onWsEvent);
  myWebServer->addHandler(ws);

  myWebServer->onNotFound(notFound);
  myWebServer->serveStatic("/", SPIFFS, "/www/").setDefaultFile("index.htm");
  myWebServer->begin();
  Serial.println("Web Server initialized");
}

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
  DynamicJsonDocument doc(1200);
  char myStatusMsg[400];
  doc["Cmd"] = "STATS";
  JsonObject Data = doc.createNestedObject("Data");
  Data["uptime"] = millis();
  if (useNTP)// && ntpOK) no longer needed in stick as we have RTC
  {
    now = time(0);
    char buff[40]; //39 digits plus the null char
    strftime(buff, 40, "%m-%d-%Y %H:%M:%S", localtime(&now));
    Data["systime"] = buff;
  }
  Data["freemem"] = String(ESP.getFreeHeap());
  Data["totaldisk"] = String(SPIFFS.totalBytes());
  Data["useddisk"] = String(SPIFFS.usedBytes());
  Data["freedisk"] = String(SPIFFS.totalBytes() - SPIFFS.usedBytes());
  char buff[10];
  sprintf(buff, "%u.%u.%u", BBVersion[0], BBVersion[1], BBVersion[2]);
  Data["version"] = buff;
  Data["ipaddress"] = WiFi.localIP().toString();
  Data["sigstrength"] = WiFi.RSSI();
  serializeJson(doc, myStatusMsg);
  //    Serial.println(myStatusMsg);
  globalClient->text(myStatusMsg);
  lastWifiUse = millis();
  //  Serial.println("Keep alive done");
}

void sendKeepAlive()
{
  if (millis() > keepAlive)
  {
    if (globalClient != NULL)
      processStatustoWebClient();
    keepAlive += keepAliveInterval;
  }
}

String createCfgEntry(String cmdType)
{
  String fileStr = "\"Type\":\"" + cmdType + "\",\"Data\":";
  if (cmdType == "pgNodeCfg")
    fileStr += readFile("/configdata/node.cfg");
  if (cmdType == "pgMQTTCfg")
    fileStr += readFile("/configdata/mqtt.cfg");
  if (cmdType == "pgHWBtnCfg")
    fileStr += readFile("/configdata/btn.cfg");
  if (cmdType == "pgBtnHdlrCfg")
    fileStr += readFile("/configdata/btnevt.cfg");
  if (cmdType == "pgLEDCfg")
    fileStr += readFile("/configdata/led.cfg");
//  Serial.println(fileStr);
  return fileStr;
}

void processWsMessage(String newMsg, AsyncWebSocketClient * client)
{
  Serial.println(newMsg);
  DynamicJsonDocument doc(3 * newMsg.length());
  DeserializationError error = deserializeJson(doc, newMsg);
  if (!error)
  {
    if (doc.containsKey("Cmd"))
    {
      String thisCmd = doc["Cmd"];
      if (thisCmd == "SetLED") //Request to switch on LED for identification purposes
      {
        JsonArray ledList = doc["LedNr"];
        for (int i = 0; i < ledList.size(); i++)
        {
//          Serial.printf("Setting Test LED %i\n", (uint16_t)ledList[i]);
          if (myChain) myChain->identifyLED(ledList[i]);
        }
      }
      if (thisCmd == "CfgFiles") //Config Request Format: {"Cmd":"CfgData", "Type":"pgxxxxCfg"}
      {
        String fileStr = "{\"Cmd\":\"CfgFiles\", \"FileEntries\":[";
        fileStr += "{" + createCfgEntry("pgNodeCfg")+ "},"; //this always first, is used to validate file
        fileStr += "{" + createCfgEntry("pgMQTTCfg")+ "},";
        fileStr += "{" + createCfgEntry("pgHWBtnCfg")+ "},";
        fileStr += "{" + createCfgEntry("pgBtnHdlrCfg")+ "},";
        fileStr += "{" + createCfgEntry("pgLEDCfg")+ "}";
        fileStr += "]}";
        Serial.println(fileStr);
        client->text(fileStr);
      }
      if (thisCmd == "CfgData") //Config Request Format: {"Cmd":"CfgData", "Type":"pgxxxxCfg"}
      {
        String cmdType = doc["Type"];
        String fileStr = "{\"Cmd\":\"CfgData\", ";

        if (cmdType == "pgLNViewer")
          return;
        if (cmdType == "pgDCCViewer")
          return;
        fileStr += createCfgEntry(cmdType);
        fileStr += "}";
        Serial.println(fileStr);
        client->text(fileStr);
      }
      if (thisCmd == "CfgUpdate") //Config Request Format: {"Cmd":"CfgData", "Type":"pgxxxxCfg", "Data":{}}
      {
        String cmdType = doc["Type"];
        String fileStr = doc["Data"];
        if (cmdType == "pgNodeCfg")
          writeJSONFile("/configdata/node.cfg", fileStr);
        if (cmdType == "pgMQTTCfg")
          writeJSONFile("/configdata/mqtt.cfg", fileStr);
        if (cmdType == "pgHWBtnCfg")
          writeJSONFile("/configdata/btn.cfg", fileStr);
        if (cmdType == "pgBtnHdlrCfg")
          writeJSONFile("/configdata/btnevt.cfg", fileStr);
        if (cmdType == "pgLEDCfg")
          writeJSONFile("/configdata/led.cfg", fileStr);
        if (!doc.containsKey("Restart")) //if old format, then restart
          ESP.restart(); //configuration update requires restart to be sure dynamic allocation of objects is not messed up
        else
          if (doc["Restart"])
            ESP.restart(); //configuration update requires restart to be sure dynamic allocation of objects is not messed up
      }
    }
  }
}

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{
  lastWifiUse = millis();
  Serial.printf("WS Event %i \n", globalClient);
  switch (type)
  {
    case WS_EVT_CONNECT:
      {
        globalClient = client;
        keepAlive = millis() + 500;
        Serial.printf("Websocket client connection received from %u", client->id());
        break;
      }
    case WS_EVT_DISCONNECT:
      {
        globalClient = NULL;
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
          wsReadPtr = 0;
          if (info->opcode == WS_TEXT)
          {
            for (size_t i = 0; i < info->len; i++)
            {
              wsBuffer[wsReadPtr] = (char) data[i];
              wsReadPtr++;
            }
            wsBuffer[wsReadPtr] = char(0);
            processWsMessage(String(wsBuffer), client);
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
            wsReadPtr = 0;
            Serial.println("Reset Read Ptr");
            //          if(info->num == 0)
            //            Serial.printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
            //          Serial.printf("multi ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
          }

          Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: \n", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT) ? "text" : "binary", info->index, info->index + len);

          if (info->opcode == WS_TEXT)
          {
            Serial.println("adding...");
            for (size_t i = 0; i < len; i++)
            {
              wsBuffer[wsReadPtr] = (char) data[i];
              wsReadPtr++;
            }
            wsBuffer[wsReadPtr] = char(0);
          }
          else
          {
            //no processing of non-text data at this time
          }
          wsBuffer[wsReadPtr] = char(0);
          Serial.println(wsReadPtr);
          if ((info->index + len) == info->len)
          {
            if (info->final)
            {
              if (info->message_opcode == WS_TEXT)
                processWsMessage(String(wsBuffer), client);
            }
          }
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
