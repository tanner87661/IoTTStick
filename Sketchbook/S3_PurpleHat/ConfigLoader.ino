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
  USBSerial.printf("Deleting %s\n", &fileName[0]);
  if (SPIFFS.exists(fileName))
  {
    SPIFFS.remove(fileName);
    return true;
  }
  return false;
}

void deleteAllFiles(String ofNameType, String startDir, String fileExt, bool lastOnly)
{
  USBSerial.println("Delete all " + ofNameType);
  File root = SPIFFS.open(startDir);
  File thisFile = root.openNextFile();
  String lastFileName;
  while (thisFile)
  {
    String hlpStr = thisFile.name();
//    USBSerial.println(hlpStr);
    uint8_t extDot = hlpStr.lastIndexOf('.');
    for (int i = 0; i < 3; i++)
      if (isDigit(hlpStr[extDot-i]))
        hlpStr[extDot-i] = '?';
    
    hlpStr.replace("???", "?");
    hlpStr.replace("??", "?");
//    USBSerial.println(hlpStr);
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
//  USBSerial.printf("Trying to read File %s\n", &fileName[0]);
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
      USBSerial.printf("Can't open %s \n", &fileName[0]);
  } 
  else 
    USBSerial.printf("File %s not found\n", &fileName[0]);
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
//  USBSerial.println(fileName);
//  USBSerial.println(fileStr);
  uint32_t startTime = millis();
  File dataFile = SPIFFS.open(fileName, "w");
  if (dataFile)
  {
    dataFile.println(fileStr);
    dataFile.close();
//    USBSerial.printf("Writing Config File from JSON complete %i bytes in %i ms\n", fileStr.length(), millis() - startTime);
    return true;
  }
  else
    return false;
}

bool writeJSONFile(String fileName, const char * fileStr)
{
//  USBSerial.println(fileName);
  
//  USBSerial.println(fileStr);
//  USBSerial.println("Writing Config File from FileStr");
  uint32_t startTime = millis();
  File dataFile = SPIFFS.open(fileName, "w");
  if (dataFile)
  {
    dataFile.println(fileStr);
    dataFile.close();
    USBSerial.printf("Writing Config File from FileStr complete %i bytes in %i ms\n", strlen(fileStr), millis() - startTime);
    return true;
  }
  else
    return false;
}

DynamicJsonDocument * getDocPtr(String cmdFile, bool duplData)
{
  uint32_t jsonData = readFileToBuffer(cmdFile, wsTxBuffer, wsBufferSize);
//  USBSerial.println(wsTxBuffer);
  if (jsonData > 0)
  {
    uint16_t docSize = 4096 * (trunc((3 * jsonData) / 4096) + 1);  //.length();
//    USBSerial.printf("Size: %i Doc Size: %i\n", jsonData, docSize);
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
      USBSerial.println("Deserialization error");
      return NULL;
    }
  }
  else
  {
      USBSerial.println("File read error");
      return NULL;
  }
}

byte checkI2CPort(byte i2cAddress)
{
/*
  if (i2cAddress > 127)
    return i2cAddress;
  Wire.beginTransmission(i2cAddress);
  delay(10);
  return Wire.endTransmission();
*/
}

void prepareShutDown()
{
/*
  digitraxBuffer->saveToFile(bufferFileName);
  delay(1000);
  if (mySwitchList)
    mySwitchList->saveRunTimeData();
  delay(150);
*/
}

/*
void initWire(uint8_t pinSDA, uint8_t pinSCL)
{
  USBSerial.println("Init TwoWire");  
  Wire.setPins(pinSDA,pinSCL);
  Wire.begin();
  delay(10);
  Wire.setClock(i2cClock);
  delay(10);

//        Wire.begin(hatSDA, hatSCL);//, 400000); //initialize the I2C interface 400kHz
  check_if_exist_I2C();
  delay(10);
}

void check_if_exist_I2C() 
{
  byte error, address;
  int nDevices;
  nDevices = 0;
//for (address = 0x36; address < 0x37; address++ )  
  for (address = 1; address < 127; address++ )  
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    delay(10);
    error = Wire.endTransmission();

    if (error == 0){
      USBSerial.print("I2C device found at address 0x");
      if (address < 16)
        USBSerial.print("0");
      USBSerial.print(address, HEX);
      USBSerial.println("  !");
      nDevices++;
    } else if (error == 4) {
      USBSerial.print("Unknow error at address 0x");
      if (address < 16)
        USBSerial.print("0");
      USBSerial.println(address, HEX);
    }
  } //for loop
  if (nDevices == 0)
    USBSerial.println("No I2C devices found");
  else
    USBSerial.println("**********************************\n");
  //delay(1000);           // wait 1 seconds for next scan, did not find it necessary
}
*/
