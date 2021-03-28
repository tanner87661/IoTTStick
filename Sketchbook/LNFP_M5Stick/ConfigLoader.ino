//read config files and build object lists for colors, block detectors, switches, buttons, signals etc. 
//this is all dynamic, allocated on the heap when the program is starting up
////////////////////////////////////////////////Config File Loading////////////////////////////////////////////////////////////////////

int getFileSize(String fileName)
{
  if (SPIFFS.exists(fileName))
  {
    File dataFile = SPIFFS.open(fileName, "r");
    if (dataFile)
    {
      int thisSize = dataFile.size();
      dataFile.close();
      return thisSize;
    }
  }
  return -1;
}

bool deleteFile(String fileName)
{
  Serial.printf("Deleting %s\n", &fileName[0]);
  if (SPIFFS.exists(fileName))
  {
    SPIFFS.remove(fileName);
    return true;
  }
  return false;
}

void deleteAllFiles(String ofNameType, String startDir, String fileExt)
{
  File root = SPIFFS.open(startDir);
  File thisFile = root.openNextFile();
  while (thisFile)
  {
    String hlpStr = thisFile.name();
    for (int i = 0; i < 10; i++)
      hlpStr.replace(String(i), "?");
    hlpStr.replace("???", "?");
    hlpStr.replace("??", "?");
    if (hlpStr == startDir + "/" + ofNameType + "." + fileExt)
      deleteFile(thisFile.name());
    if (hlpStr == startDir + "/" + ofNameType + "?." + fileExt)
      deleteFile(thisFile.name());
    thisFile = root.openNextFile();
  }
}

String readFile(String fileName)
{
  String jsonData = "";
//  Serial.printf("Trying to read File %s\n", &fileName[0]);
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
    else 
      Serial.printf("Can't open %s \n", &fileName[0]);
  } 
  else 
    Serial.printf("File %s not found\n", &fileName[0]);
//  Serial.println(jsonData);
  return jsonData;
}

bool writeJSONFile(String fileName, DynamicJsonDocument * writeThis)
{
  String fileStr;
  serializeJson(*writeThis, fileStr);
//  Serial.println(fileName);
//  Serial.println(fileStr);
  uint32_t startTime = millis();
  File dataFile = SPIFFS.open(fileName, "w");
  if (dataFile)
  {
    dataFile.println(fileStr);
    dataFile.close();
//    Serial.printf("Writing Config File from JSON complete %i bytes in %i ms\n", fileStr.length(), millis() - startTime);
    return true;
  }
  else
    return false;
}

bool writeJSONFile(String fileName, String * fileStr)
{
//  Serial.println(fileName);
  
//  Serial.println(*fileStr);
//  Serial.println("Writing Config File from FileStr");
  uint32_t startTime = millis();
  File dataFile = SPIFFS.open(fileName, "w");
  if (dataFile)
  {
    dataFile.println(*fileStr);
    dataFile.close();
    Serial.printf("Writing Config File from FileStr complete %i bytes in %i ms\n", fileStr->length(), millis() - startTime);
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

void prepareShutDown()
{
  saveToFile(bufferFileName);
  if (mySwitchList)
    mySwitchList->saveRunTimeData();
}
