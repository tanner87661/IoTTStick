//read config files and build object lists for colors, block detectors, switches, buttons, signals etc. 
//this is all dynamic, allocated on the heap when the program is starting up
////////////////////////////////////////////////Config File Loading////////////////////////////////////////////////////////////////////

String readFile(String fileName)
{
  String jsonData = "";
  if (SPIFFS.exists(fileName))
  {
    File dataFile = SPIFFS.open(fileName, "r");
    if (dataFile)
    {
      while (dataFile.position() < dataFile.size())
      {
        jsonData = jsonData + dataFile.readStringUntil('\n');
        jsonData.trim();
      } 
      dataFile.close();
    }
  } else Serial.printf("File %s not found\n", fileName);
  return jsonData;
}

bool writeJSONFile(String fileName, DynamicJsonDocument * writeThis)
{
  String fileStr;
  serializeJson(*writeThis, fileStr);
  Serial.println(fileStr);
  Serial.println("Writing Node Config File");
  File dataFile = SPIFFS.open(fileName, "w");
  if (dataFile)
  {
    dataFile.println(fileStr);
    dataFile.close();
    Serial.println("Writing Config File complete");
    return true;
  }
  else
    return false;
}

bool writeJSONFile(String fileName, String fileStr)
{
  Serial.println(fileStr);
  Serial.println("Writing Node Config File");
  File dataFile = SPIFFS.open(fileName, "w");
  if (dataFile)
  {
    dataFile.println(fileStr);
    dataFile.close();
    Serial.println("Writing Config File complete");
    return true;
  }
  else
    return false;
}

DynamicJsonDocument * getDocPtr(String cmdFile)
{
  String jsonData = readFile(cmdFile);
  if (jsonData != "")
  {
    uint16_t docSize = 3 * jsonData.length();
    DynamicJsonDocument * thisDoc = new DynamicJsonDocument(docSize);
    DeserializationError error = deserializeJson(*thisDoc, jsonData);
    if (!error)
      return thisDoc;
    else
      return NULL;
  }
  else
      return NULL;
}

/* the bin file became too large for OTA. Use ESPTOOL.EXE instead
 *  
 *  Load binary to address 0x00010000:
 *  esptool.exe write_flash 0x00010000 M5.bin
 *  
 *  Load SPIFFS to address 0x00290000
 *  esptool.exe write_flash 0x00290000 SPIFFS.bin
 */

/*
void checkForUpdate(String updateInfoFile)
{
  DynamicJsonDocument * jsonUpdateObj = NULL;
  jsonUpdateObj = getDocPtr(updateInfoFile); //read and decode the update configuration file
  if (jsonUpdateObj != NULL)
  {
    if (jsonUpdateObj->containsKey("Version"))
    {
      JsonArray thisVersion = (*jsonUpdateObj)["Version"];
      if (thisVersion.size() == sizeof(BBVersion)) //BBVersion must be uint8_t
      {
        int oldVersion = 0;
        int newVersion = 0;
        for (uint8_t i = 0; i < thisVersion.size(); i++)
        {
          oldVersion = (oldVersion * 100) + BBVersion[i];
          newVersion = (newVersion * 100) + (int)thisVersion[i];
          if (newVersion > oldVersion)
          {
            if (jsonUpdateObj->containsKey("FileName"))
            {
              char newVersionFile[100];
              strcpy(newVersionFile, (*jsonUpdateObj)["FileName"]);
              Serial.println("Loading new Version");
              installUpdate(newVersionFile);
            }
          }
        }
      }
    }
    delete jsonUpdateObj;
  }
}

void installUpdate(String updateFile)
{
    File file = SPIFFS.open(updateFile, "r");
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }
    Serial.println("Starting update..");
    size_t fileSize = file.size();
    Serial.printf("Update File Size: %i\n", fileSize);
    if(!Update.begin(fileSize))
    {
       Serial.println("Cannot do the update");
       return;
    };
    Update.writeStream(file);
    if(Update.end())
    {
      Serial.println("Successful update");  
    }
    else 
    {
      Serial.println("Error Occurred: " + String(Update.getError()));
      return;
    }
    file.close();
    SPIFFS.remove(updateFile);
    Serial.println("Reset in 4 seconds...");
    delay(4000);
    ESP.restart();
}

*/
