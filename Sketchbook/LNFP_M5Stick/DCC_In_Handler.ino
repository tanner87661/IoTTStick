
typedef struct
{
  uint32_t lastUpdate;
  uint16_t slotID;
  char dispStr[22];  
  bool validEntry = false;
} refreshEntry;

#define oneShotBufferSize 4
#define refreshBufferSize 100
#define ackPulseLen 6 //6 ms ack pulse for SM programming

refreshEntry refreshBuffer[refreshBufferSize];
refreshEntry oneShotBuffer[oneShotBufferSize];
uint8_t oneShotWrPtr = 0;
uint8_t oneShotRdPtr = 0;

uint32_t initPulse = millis();
uint8_t lastCVVal = 3;

bool sendIdle = true;

String lastWSRefreshStr = "";
String lastNextionRefreshStr = "";

void dccClientCallback(char* topic, byte *  payload, unsigned int length) //this is the landing point for incoming DCC messages from MQTT
{
  DynamicJsonDocument doc(4 * length);
  DeserializationError error = deserializeJson(doc, payload);
  uint16_t dccAddr = 0;
  uint16_t param1 = 0;
  uint16_t param2 = 0;
  uint16_t param3 = 0;
  String tempStr;
  if (!error)
  {
    if (doc.containsKey("type"))
    {
      dccAddr = doc["addr"];
      String thisType = doc["type"];
      if (thisType == "switch")
      {
        String tempStr1 = doc["dir"];
        param1 = (tempStr1 == "closed")? 1:0;
        String tempStr2 = doc["power"];
        param2 = (tempStr2 == "on")? 1:0;
        notifyDccAccTurnoutOutput(dccAddr, param1, param2);
      }
      if (thisType == "signal")
      {
        param1 = doc["aspect"];
        notifyDccSigOutputState(dccAddr, param1);
      }
      if (thisType == "loco_speed")
      {
        param1 = doc["speed"];
        param2 = doc["speedsteps"];
        String tempStr1 = doc["dir"];
        DCC_DIRECTION param3 = (tempStr1 == "reverse")? DCC_DIR_REV:DCC_DIR_FWD;
        String tempStr2 = doc["addr_type"];
        DCC_ADDR_TYPE param4 = (tempStr2 == "long")? DCC_ADDR_LONG:DCC_ADDR_SHORT;
        notifyDccSpeed(dccAddr, param4, param1, param3, (DCC_SPEED_STEPS)param2);
      }
      if (thisType == "loco_function")
      {
        String tempStr2 = doc["addr_type"];
        DCC_ADDR_TYPE param4 = (tempStr2 == "long")? DCC_ADDR_LONG:DCC_ADDR_SHORT;
        param1 = doc["func_group"];
        FN_GROUP param3 = FN_0;
        switch (param1)
        {
          case 0: param3 = FN_0; break;
          case 1: param3 = FN_0_4; break;
          case 2: param3 = FN_5_8; break;
          case 3: param3 = FN_9_12; break;
          case 4: param3 = FN_13_20; break;
          case 5: param3 = FN_21_28; break;
        }
        param2 = doc["func_value"];
        notifyDccFunc(dccAddr, param4, param3, param2);
      }
      if (thisType == "loco_remove")
      {
        Serial.printf("Remove %u\n", dccAddr);        
        stopRefresh(dccAddr);
        sendRefreshBuffer();
      }
    }
  }
  else
    Serial.println("dccBC deserialization error");
}

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
    processDCCtoWebClient(false, outStr);
    if ((useM5Viewer == 2) && (!isOneTime()))
      processDCCtoM5(false, outStr);
    lastWSRefreshStr = outStr;
  }
}

void stopRefresh(uint16_t dccAddr)
{
  char dispStr[150];  
  for (uint8_t i=0; i < refreshBufferSize; i++)
  {
    if ((refreshBuffer[i].slotID & 0x3FFF) == dccAddr)
    {
      refreshBuffer[i].validEntry = false;
      if (lnMQTTServer) //cascaded MQTT Server
      {
        sprintf(dispStr, "{\"type\":\"loco_remove\", \"addr\": %u}", dccAddr);
        lnMQTTServer->sendDCCMsg(dispStr);
      }
    }
  }
}

void clearRefreshBuffer()
{
  char dispStr[150];  
  for (uint8_t i=0; i < refreshBufferSize; i++)
  {
    if (refreshBuffer[i].validEntry)
    {
      refreshBuffer[i].validEntry = false;
      if (lnMQTTServer)
      {
        sprintf(dispStr, "{\"type\":\"loco_remove\", \"addr\": %u}", refreshBuffer[i].slotID & 0x3FFF);
        lnMQTTServer->sendDCCMsg(dispStr);
      }
    }
  }
}

void verifyRefreshAge()
{
  if (useInterface.devId == 1) //DCC Interface
  {
    uint32_t ageLimit = millis() - 500;
    char dispStr[150];  
    for (uint8_t i=0; i < refreshBufferSize; i++)
    {
      if (refreshBuffer[i].validEntry)
        if (refreshBuffer[i].lastUpdate < ageLimit)
        {
          refreshBuffer[i].validEntry = false;
          if (lnMQTTServer)
          {
            sprintf(dispStr, "{\"type\":\"loco_remove\", \"addr\": %u}", refreshBuffer[i].slotID & 0x3FFF);
            lnMQTTServer->sendDCCMsg(dispStr);
          }
        }
    }
  }
}

bool updateRefreshBuffer(uint16_t thisID, const char dispStr[]) //make pointer
{
  for (uint8_t i=0; i < refreshBufferSize; i++)
    if ((refreshBuffer[i].slotID == thisID) && refreshBuffer[i].validEntry)
    {
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
//      Serial.print("Add ");
//      Serial.println(dispStr);
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
  processDCCtoWebClient(true, dispStr);
//  Serial.println("sendOneTime");
  if ((useM5Viewer == 2) && isOneTime())
    processDCCtoM5(true, dispStr);
}

// This function is called whenever a normal DCC Turnout Packet is received and we're in Output Addressing Mode

void notifyDccAccTurnoutOutput(uint16_t Addr, uint8_t Direction, uint8_t OutputPower )
{
  digitraxBuffer->setSwiStatus(Addr-1, Direction, OutputPower);
  char dispStr[150];  
  sprintf(dispStr, "Swi %u Pos %s %s", Addr, Direction==0? "TH":"CL", OutputPower==0?"OFF":"ON");
//  Serial.println(dispStr);
  updateOneShotBuffer(dispStr);
  if (lnMQTTServer)
  {
    sprintf(dispStr, "{\"type\":\"switch\", \"addr\": %u, \"dir\": \"%s\", \"power\":\"%s\"}", Addr, Direction==0? "thrown":"closed", OutputPower==0?"off":"on");
//    Serial.println(dispStr);
    lnMQTTServer->sendDCCMsg(dispStr);
  }
  if (useHat.devId == 8) //SilverHat USB Serial Injector
    if (usbSerial)
      usbSerial->dccToBooster(0xB0, Addr-1, (Direction << 1) + OutputPower);
}

// This function is called whenever a DCC Signal Aspect Packet is received
void notifyDccSigOutputState(uint16_t Addr, uint8_t State)
{
  digitraxBuffer->setSignalAspect(Addr, State);
  char dispStr[150];  
  sprintf(dispStr, "Sig %u Asp %i", Addr, State);
//  Serial.println(dispStr);

  updateOneShotBuffer(dispStr);
  if (lnMQTTServer)
  {
    sprintf(dispStr, "{\"type\":\"signal\", \"addr\": %u, \"aspect\": %i}", Addr, State);
//    Serial.println(dispStr);
    lnMQTTServer->sendDCCMsg(dispStr);
  }
  if (useHat.devId == 8) //SilverHat USB Serial Injector
    if (usbSerial)
      usbSerial->dccToBooster(0xED, Addr+1, State);
}

void    notifyDccIdle(void)
{
  if (sendIdle)
  {
    clearRefreshBuffer();
    updateRefreshBuffer(0, "DCC Idle");
    sendRefreshBuffer();
    char dispStr[25];  
    if (lnMQTTServer && sendIdle)
    {
      sprintf(dispStr, "{\"type\":\"idle\"}");
      lnMQTTServer->sendDCCMsg(dispStr);
    }
    sendIdle = false;
  }
}

void    notifyDccSpeed(uint16_t Addr, DCC_ADDR_TYPE AddrType, uint8_t Speed, DCC_DIRECTION Dir, DCC_SPEED_STEPS SpeedSteps )
{
  sendIdle = true;
  char dispStr[150];  
  sprintf(dispStr, "#%i T%i S%i/%i %c", Addr, AddrType, Speed, SpeedSteps, Dir == DCC_DIR_REV?'R':'F'); 
//  Serial.println(dispStr);
  if (updateRefreshBuffer((Addr & 0x3FFF), dispStr))
    if (lnMQTTServer)
    {
      sprintf(dispStr, "{\"type\":\"loco_speed\", \"addr\": %u, \"addr_type\": \"%s\", \"speed\": %i, \"speedsteps\": %i, \"dir\": \"%s\"}", Addr, AddrType == 0?"short":"long", Speed, SpeedSteps, Dir == DCC_DIR_REV?"reverse":"forward");
//      Serial.println(dispStr);
      lnMQTTServer->sendDCCMsg(dispStr);
    }
}

void    notifyDccFunc(uint16_t Addr, DCC_ADDR_TYPE AddrType, FN_GROUP FuncGrp, uint8_t FuncState)
{
  char dispStr[150];  
  sprintf(dispStr, "#%i T%i G%i F%02X", Addr, AddrType, FuncGrp, FuncState); 
//  Serial.println(dispStr);
  uint16_t thisID = Addr & 0x3FFF;
  switch (FuncGrp)
  {
    case 1: thisID |= 0x4000; break;
    default: thisID |= 0x8000; break;
  }
  if (updateRefreshBuffer(thisID, dispStr))
    if (lnMQTTServer)
    {
//      sprintf(dispStr, "{\"type\":\"loco_function\", \"addr\": %u, \"addr_type\": \"%s\", \"func_group\": %i, \"func_value\": %02X}", Addr, AddrType == 0?"short":"long", FuncGrp, FuncState);
      sprintf(dispStr, "{\"type\":\"loco_function\", \"addr\": %u, \"addr_type\": \"%s\", \"func_group\": %i, \"func_value\": %i}", Addr, AddrType == 0?"short":"long", FuncGrp, FuncState);
//    Serial.println(dispStr);
      lnMQTTServer->sendDCCMsg(dispStr);
    }
}

void    notifyDccMsg (DCC_MSG * Msg)
{
  if (digitalRead(groveTxD))
  {
    if ((millis() - initPulse) > ackPulseLen)
    {
      digitalWrite(groveTxD, 0);
//      Serial.println("Clr ACK");
    }
  }
}

uint8_t notifyCVValid (uint16_t CV, uint8_t Writable)
{
//  Serial.printf("Validate CV %i\n", CV);
  return 1;
}

uint8_t notifyCVRead (uint16_t CV)
{
  Serial.printf("Read CV %i\n", CV);
  return lastCVVal;
}

uint8_t notifyCVWrite (uint16_t CV, uint8_t Value)
{
  lastCVVal = Value;
  Serial.printf("Write CV %i to %i\n", CV, Value);
  return Value;
}

void    notifyCVAck (void)
{
//  Serial.println("Req ACK");
  digitalWrite(groveTxD, 1);
  initPulse = millis();
}


void processDCCtoWebClient(bool oneTime, String dispText) //if a web browser is conneted, DCC messages are sent via Websockets
                                                          //this is the hook for a web based DCC viewer
{
  int8_t currClient = getWSClientByPage(0, "pgDCCViewer");
  if (currClient >= 0)
  {
    DynamicJsonDocument doc(1200);
    char myMqttMsg[800];
    doc["Cmd"] = "DCC";
    JsonObject data = doc.createNestedObject("Data");
    data["oneTime"] = oneTime;
    data["dispStr"] = dispText;
    serializeJson(doc, myMqttMsg);
    while (currClient >= 0)
    {
      globalClients[currClient].wsClient->text(myMqttMsg);
      currClient = getWSClientByPage(currClient+1, "pgDCCViewer");
    }
    lastWifiUse = millis();
//    Serial.println(myMqttMsg);
  }
}
