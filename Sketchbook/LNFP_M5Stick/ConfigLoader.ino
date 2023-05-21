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

void deleteAllFiles(String ofNameType, String startDir, String fileExt, bool lastOnly)
{
  Serial.println("Delete all " + ofNameType);
  File root = SPIFFS.open(startDir);
  File thisFile = root.openNextFile();
  String lastFileName;
  while (thisFile)
  {
    String hlpStr = thisFile.name();
//    Serial.println(hlpStr);
    uint8_t extDot = hlpStr.lastIndexOf('.');
    for (int i = 0; i < 3; i++)
      if (isDigit(hlpStr[extDot-i]))
        hlpStr[extDot-i] = '?';
    
    hlpStr.replace("???", "?");
    hlpStr.replace("??", "?");
//    Serial.println(hlpStr);
    if (hlpStr == startDir + "/" + ofNameType + "." + fileExt)
      if (lastOnly)
        lastFileName = thisFile.name();
      else
        deleteFile(thisFile.name());
    if (hlpStr == startDir + "/" + ofNameType + "?." + fileExt)
      if (lastOnly)
        lastFileName = thisFile.name();
      else
        deleteFile(thisFile.name());
    thisFile = root.openNextFile();
  }
  if (lastOnly)
    deleteFile(lastFileName);
}

uint32_t readFileToBuffer(String fileName, char * thisBuffer, uint32_t maxSize)
{
  uint32_t bytesRead = 0;
//  Serial.printf("Trying to read File %s\n", &fileName[0]);
  if (SPIFFS.exists(fileName))
  {
    File dataFile = SPIFFS.open(fileName, "r");
    if (dataFile)
    {
      if (dataFile.size() < maxSize)
      {
        bytesRead = dataFile.size();
        dataFile.read((uint8_t*)thisBuffer, bytesRead);
        thisBuffer[bytesRead] = '\0'; 
      }
      dataFile.close();
    }
    else 
      Serial.printf("Can't open %s \n", &fileName[0]);
  } 
  else 
    Serial.printf("File %s not found\n", &fileName[0]);
  return bytesRead;
}

void prepTempStorage()
{
  
}

void installTempStorage()
{
  
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

bool writeJSONFile(String fileName, const char * fileStr)
{
//  Serial.println(fileName);
  
//  Serial.println(fileStr);
//  Serial.println("Writing Config File from FileStr");
  uint32_t startTime = millis();
  File dataFile = SPIFFS.open(fileName, "w");
  if (dataFile)
  {
    dataFile.println(fileStr);
    dataFile.close();
    Serial.printf("Writing Config File from FileStr complete %i bytes in %i ms\n", strlen(fileStr), millis() - startTime);
    return true;
  }
  else
    return false;
}

DynamicJsonDocument * getDocPtr(String cmdFile, bool duplData)
{
  uint32_t jsonData = readFileToBuffer(cmdFile, wsTxBuffer, wsBufferSize);
//  Serial.println(wsTxBuffer);
  if (jsonData > 0)
  {
    uint16_t docSize = 4096 * (trunc((3 * jsonData) / 4096) + 1);  //.length();
//    Serial.printf("Size: %i Doc Size: %i\n", jsonData, docSize);
    DynamicJsonDocument * thisDoc = new DynamicJsonDocument(docSize);
    DeserializationError error;
    if (duplData)
      error = deserializeJson(*thisDoc, (const char*) wsTxBuffer); //use const to force deserialize to keep copy of buffer data
    else
      error = deserializeJson(*thisDoc, wsTxBuffer); //use const to force deserialize to keep copy of buffer data
    if (!error)
      return thisDoc;
    else
    {
      Serial.println("Deserialization error");
      return NULL;
    }
  }
  else
  {
      Serial.println("File read error");
      return NULL;
  }
}

byte checkI2CPort(byte i2cAddress)
{
  if (i2cAddress > 127)
    return i2cAddress;
  Wire.beginTransmission(i2cAddress);
  delay(10);
  return Wire.endTransmission();
}

void prepareShutDown()
{
  digitraxBuffer->saveToFile(bufferFileName);
  delay(1000);
  if (mySwitchList)
    mySwitchList->saveRunTimeData();
  delay(150);
}
