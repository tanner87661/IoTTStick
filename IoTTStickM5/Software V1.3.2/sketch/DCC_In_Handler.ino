typedef struct
{
  uint32_t lastUpdate;
  uint16_t slotID;
  char dispStr[20];  
  bool validEntry = false;
} refreshEntry;

#define oneShotBufferSize 4
#define refreshBufferSize 100
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

bool updateRefreshBuffer(uint16_t thisID, char dispStr[])
{
  for (uint8_t i=0; i < refreshBufferSize; i++)
    if (refreshBuffer[i].slotID == thisID)
    {
      refreshBuffer[i].validEntry = true;
      refreshBuffer[i].lastUpdate = millis();
      if (strcmp(refreshBuffer[i].dispStr, dispStr) == 0) //the same, just update with new time
        return false;
      else
      {
        strcpy(refreshBuffer[i].dispStr, dispStr);
        return true;
      }
    }
  for (uint8_t i=0; i < refreshBufferSize; i++)
  {
    if (!refreshBuffer[i].validEntry) //find unused buffer space
    {
      refreshBuffer[i].slotID = thisID;
      refreshBuffer[i].validEntry = true;
      refreshBuffer[i].lastUpdate = millis();
      strcpy(refreshBuffer[i].dispStr, dispStr);
      return true;
    }
  }
  return false;
}

void updateOneShotBuffer(char dispStr[])
{
  uint8_t tempPtr = (oneShotWrPtr + 1) % oneShotBufferSize;
  oneShotBuffer[tempPtr].lastUpdate = millis();
  strcpy(oneShotBuffer[tempPtr].dispStr, dispStr);
  oneShotBuffer[tempPtr].validEntry = true;
  oneShotWrPtr = tempPtr;
//  Serial.println(dispStr);
  if (globalClient != NULL)
    processDCCtoWebClient(true, dispStr);
//  Serial.println("sendOneTime");
  if ((useM5Viewer == 2) && isOneTime())
    processDCCtoM5(true, dispStr);
}

// This function is called whenever a normal DCC Turnout Packet is received and we're in Output Addressing Mode

void notifyDccAccTurnoutOutput(uint16_t Addr, uint8_t Direction, uint8_t OutputPower )
{
  setSwitchStatus(Addr-1, Direction, OutputPower);
  char dispStr[100];  
  sprintf(dispStr, "Swi %u %s %s", Addr, Direction==0? "Th":"Cl", OutputPower==0?"Off":"On");
  updateOneShotBuffer(dispStr);
  if (lnMQTT)
  {
    sprintf(dispStr, "{\"type\":\"switch\", \"addr\": %u, \"dir\": \"%s\", \"power\":\"%s\"}", Addr, Direction==0? "thrown":"closed", OutputPower==0?"off":"on");
    lnMQTT->sendDCCMsg(dispStr);
  }
}

// This function is called whenever a DCC Signal Aspect Packet is received
void notifyDccSigOutputState(uint16_t Addr, uint8_t State)
{
  setSignalAspect(Addr, State);
  char dispStr[100];  
  sprintf(dispStr, "Sig %u A%i", Addr, State);
  updateOneShotBuffer(dispStr);
  if (lnMQTT)
  {
    sprintf(dispStr, "{\"type\":\"signal\", \"addr\": %u, \"aspect\": %i}", Addr, State);
    lnMQTT->sendDCCMsg(dispStr);
  }
}

void    notifyDccIdle(void)
{
  updateRefreshBuffer(0, "DCC Idle");
  char dispStr[25];  
  if (lnMQTT)
  {
    sprintf(dispStr, "{\"type\":\"idle\"}");
    lnMQTT->sendDCCMsg(dispStr);
  }
}

void    notifyDccSpeed(uint16_t Addr, DCC_ADDR_TYPE AddrType, uint8_t Speed, DCC_DIRECTION Dir, DCC_SPEED_STEPS SpeedSteps )
{
  char dispStr[150];  
  sprintf(dispStr, "#%i T%i S%i/%i %c", Addr, AddrType, Speed, SpeedSteps, Dir == DCC_DIR_REV?'R':'F'); 
  if (updateRefreshBuffer((Addr & 0x3FFF), dispStr))
    if (lnMQTT)
    {
      sprintf(dispStr, "{\"type\":\"loco_speed\", \"addr\": %u, \"addr_type\": \"%s\", \"speed\": %i, \"speedsteps\": %i, \"dir\": \"%s\"}", Addr, AddrType == 0?"short":"long", Speed, SpeedSteps, Dir == DCC_DIR_REV?"reverse":"forward");
      lnMQTT->sendDCCMsg(dispStr);
    }
}

void    notifyDccFunc(uint16_t Addr, DCC_ADDR_TYPE AddrType, FN_GROUP FuncGrp, uint8_t FuncState)
{
  char dispStr[150];  
  sprintf(dispStr, "#%i T%i G%i F%02X", Addr, AddrType, FuncGrp, FuncState); 
  uint16_t thisID = Addr & 0x3FFF;
  switch (FuncGrp)
  {
    case 1: thisID |= 0x4000; break;
    default: thisID |= 0x8000; break;
  }
  if (updateRefreshBuffer(thisID, dispStr))
    if (lnMQTT)
    {
//      sprintf(dispStr, "{\"type\":\"loco_function\", \"addr\": %u, \"addr_type\": \"%s\", \"func_group\": %i, \"func_value\": %02X}", Addr, AddrType == 0?"short":"long", FuncGrp, FuncState);
      sprintf(dispStr, "{\"type\":\"loco_function\", \"addr\": %u, \"addr_type\": \"%s\", \"func_group\": %i, \"func_value\": %i}", Addr, AddrType == 0?"short":"long", FuncGrp, FuncState);
      lnMQTT->sendDCCMsg(dispStr);
    }
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
