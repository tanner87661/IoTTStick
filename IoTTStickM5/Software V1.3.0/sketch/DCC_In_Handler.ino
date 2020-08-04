typedef struct
{
  uint32_t lastUpdate;
  char dispStr[25];  
  bool validEntry = false;
} refreshEntry;

#define oneShotBufferSize 4
#define refreshBufferSize 10
refreshEntry refreshBuffer[refreshBufferSize];
refreshEntry oneShotBuffer[oneShotBufferSize];
uint8_t oneShotWrPtr = 0;
uint8_t oneShotRdPtr = 0;

String lastWSRefreshStr = "";
String lastNextionRefreshStr = "";

void sendRefreshBuffer()
{
  verifyRefreshAge();
  String outStr = "";
  for (uint8_t i=0; i < refreshBufferSize; i++)
    if (refreshBuffer[i].validEntry)
    {
//      Serial.println(refreshBuffer[i].dispStr);
      outStr += refreshBuffer[i].dispStr;
      outStr += "\r\n";
    }
  if (outStr != lastWSRefreshStr)
  {
    if (globalClient != NULL)
      processDCCtoWebClient(false, outStr);
    if ((useM5Viewer == 2) && (!isOneTime()))
      processDCCtoM5(false, outStr);
    lastWSRefreshStr = outStr;
  }
}

void verifyRefreshAge()
{
  uint32_t ageLimit = millis() - 500;
  for (uint8_t i=0; i < refreshBufferSize; i++)
  {
    if (refreshBuffer[i].lastUpdate < ageLimit)
      refreshBuffer[i].validEntry = false;
  }
}

void updateRefreshBuffer(char dispStr[])
{
  for (uint8_t i=0; i < refreshBufferSize; i++)
  {
    if (strcmp(refreshBuffer[i].dispStr, dispStr) == 0) //the same, just update with new time
    {
      refreshBuffer[i].validEntry = true;
      refreshBuffer[i].lastUpdate = millis();
      return;
    }
  }
  for (uint8_t i=0; i < refreshBufferSize; i++)
  {
    if (!refreshBuffer[i].validEntry)
    {
      refreshBuffer[i].validEntry = true;
      refreshBuffer[i].lastUpdate = millis();
      strcpy(refreshBuffer[i].dispStr, dispStr);
      return;
    }
  }
}

void updateOneShotBuffer(char dispStr[])
{
  uint8_t tempPtr = (oneShotWrPtr + 1) % oneShotBufferSize;
  oneShotBuffer[tempPtr].lastUpdate = millis();
  strcpy(oneShotBuffer[tempPtr].dispStr, dispStr);
  oneShotBuffer[tempPtr].validEntry = true;
  oneShotWrPtr = tempPtr;
  Serial.println(dispStr);
  if (globalClient != NULL)
    processDCCtoWebClient(true, dispStr);
  Serial.println("sendOneTime");
  if ((useM5Viewer == 2) && isOneTime())
    processDCCtoM5(true, dispStr);
}

// This function is called whenever a normal DCC Turnout Packet is received and we're in Output Addressing Mode

void notifyDccAccTurnoutOutput(uint16_t Addr, uint8_t Direction, uint8_t OutputPower )
{
  setSwitchStatus(Addr-1, Direction, OutputPower);
  char dispStr[25];  
  sprintf(dispStr, "Swi %u %s %s", Addr, Direction==0? "Th":"Cl", OutputPower==0?"Off":"On");
  updateOneShotBuffer(dispStr);
}

// This function is called whenever a DCC Signal Aspect Packet is received
void notifyDccSigOutputState(uint16_t Addr, uint8_t State)
{
  setSignalAspect(Addr, State);
  char dispStr[25];  
  sprintf(dispStr, "Sig %u A%i", Addr, State);
  updateOneShotBuffer(dispStr);
}

void    notifyDccIdle(void)
{
  updateRefreshBuffer("DCC Idle");
}

void    notifyDccSpeed(uint16_t Addr, DCC_ADDR_TYPE AddrType, uint8_t Speed, DCC_DIRECTION Dir, DCC_SPEED_STEPS SpeedSteps )
{
  char dispStr[40];  
  sprintf(dispStr, "#%i S%i/%i %c", Addr, Speed, SpeedSteps, Dir == DCC_DIR_REV?'R':'F'); 
  updateRefreshBuffer(dispStr);
}

void    notifyDccFunc(uint16_t Addr, DCC_ADDR_TYPE AddrType, FN_GROUP FuncGrp, uint8_t FuncState)
{
  char dispStr[100];  
  sprintf(dispStr, "#%i G%i F%X", Addr, FuncGrp, FuncState); 
  updateRefreshBuffer(dispStr);
}

void processDCCtoWebClient(bool oneTime, String dispText) //if a web browser is conneted, DCC messages are sent via Websockets
                                                          //this is the hook for a web based DCC viewer
{
    DynamicJsonDocument doc(1200);
    char myMqttMsg[400];
    doc["Cmd"] = "DCC";
    JsonObject data = doc.createNestedObject("Data");
    data["oneTime"] = oneTime;
    data["dispStr"] = dispText;
    serializeJson(doc, myMqttMsg);
    globalClient->text(myMqttMsg);
    lastWifiUse = millis();
//    Serial.println(myMqttMsg);
}
