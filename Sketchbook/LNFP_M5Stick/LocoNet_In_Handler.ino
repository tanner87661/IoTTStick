
//callbackLocoNetMessage is called everytime a LocoNet message is received
//it should decode valid messages, update any status buffer and call event handlers as needed
//LocoNet messages coming in from Communication module side, e.g. LN Driver, MQTT, or Gateway

void callbackLocoNetMessageUpstream(lnReceiveBuffer * newData) //this is the landing point for incoming LocoNet messages
                                                       //from LocoNet interfaces
{
//  Serial.printf("Receive downstream Callback ID %2X %2X\n", newData->requestID, newData->errorFlags);
  if (lnSubnet) //if subnet is active, it first needs to go to the main Loconet via ln over tcp or mqtt, then being handled as echo
  {
//    if ((newData->errorFlags & (msgEcho)) == 0) //filter out echo messages
    if (newData->errorFlags == 0) //filter out echo messages
    {
      newData->requestID = random(0xFF) + fromLNSubnet;
//      Serial.printf("Forward to Upstream Loconet ID %2X %2X\n", newData->requestID, newData->errorFlags);
      sendMsg(*(lnTransmitMsg*)newData);
    }
  }
}

void callbackLocoNetMessage(lnReceiveBuffer * newData) //this is the landing point for incoming LocoNet messages
                                                       //from LocoNet interfaces
{
//  Serial.printf("Loconet Callback ID %2X %2X\n", newData->requestID, newData->errorFlags);
  if ((newData->errorFlags & (~msgEcho)) == 0) //filter out echo flag
    processLNValidMsg(newData);
  else
    processLNError(newData);
}

void processLNError(lnReceiveBuffer * newData)
{
//   Serial.printf("LN Msg %i ReqID %2X with %i bytes requested %i: ", newData->errorFlags, newData->requestID, newData->lnMsgSize, newData->reqRecTime); 
//   for (int i=0; i<newData->lnMsgSize; i++)
//     Serial.printf("0x%02X ", newData->lnData[i]);
//   Serial.println();//requestID

  if ((newData->errorFlags & errorCollision) > 0)
    Serial.println("LocoNet Error: LocoNet Collision detected");
  if ((newData->errorFlags & errorFrame) > 0)
    Serial.println("LocoNet Error: Message Framing Error");
  if ((newData->errorFlags & errorTimeout) > 0)
    Serial.println("LocoNet Error: LocoNet Timeout detected");
  if ((newData->errorFlags & errorCarrierLoss) > 0)
    Serial.println("LocoNet Error: LocoNet not connected");
  if ((newData->errorFlags & msgIncomplete) > 0)
    Serial.println("LocoNet Error: Message is incomplete");
  if ((newData->errorFlags & msgXORCheck) > 0)
    Serial.println("LocoNet Error: XOR Check failed on message");
  if ((newData->errorFlags & msgStrayData) > 0)
    Serial.println("LocoNet Error: unexpected data received");
  if ((newData->errorFlags & msgEcho) > 0)
    Serial.println("LocoNet/MQTT/TCP Echo message");
}

void processLNValidMsg(lnReceiveBuffer * newData)
{
//  Serial.printf("LN Msg %i ReqID %2x with %i bytes requested %i: ", newData->errorFlags, newData->requestID, newData->lnMsgSize, newData->reqRecTime); 
//  for (int i=0; i<newData->lnMsgSize; i++)
//    Serial.printf("0x%02X ", newData->lnData[i]);
//  Serial.println();
  if (lnMQTTClient)
    if ((newData->requestID & fromMQTTGW) == 0) return; //we are a client and this message is not from the gateway
  digitraxBuffer->processLocoNetMsg(newData); //send it to DigitraxBuffers
  processDataToWebClient("LN", newData);
  if (useM5Viewer == 1)
    processLNtoM5(newData);
  if (usbSerial)
  {
    switch (usbSerial->getMsgType())
    {
      case LocoNet:
//        Serial.println("Loconet USB");
        usbSerial->lnWriteMsg(*newData);
        break;
      case DCCEx:
//        usbSerial->lnWriteMsg(*newData);
        break;
      case DCCBoost:
//        Serial.println("Booster");
        usbSerial->lnWriteMsg(*newData);
        break;
      default: break;       
    }
  }
//  if (secElHandlerList) secElHandlerList->processLocoNetMsg(newData); //do not call this before buffer processing as it will read new buffer values
  if (lbServer)
    lbServer->lnWriteMsg(newData);
  if ((lnMQTTServer) && ((newData->requestID & fromMQTTGW) == 0)) //not coming from MQTT Gateway (ourself)
  {
//    Serial.println("MQTT Callback");
    lnMQTTServer->lnWriteMsg(newData);
  }
  if (wiThServer)
    wiThServer->lnWriteMsg(newData);
//   Serial.println("Done");
  if (lnSubnet && ((newData->requestID & fromLNSubnet) != fromLNSubnet)) //not originally from the subnet
  {
//    Serial.println("Send to Subnet");
    newData->requestID = random(0xFF);
    lnSubnet->lnWriteMsg(newData);
  }
}

void processDataToWebClient(String thisCmd, lnReceiveBuffer * newData)  //if a web browser is conneted, all LN messages are sent via Websockets
                                                                        //this is the hook for a web based LcooNet viewer
{
  int8_t currClient = getWSClientByPage(0, "pgLNViewer");
  if (currClient >= 0)
  {
    DynamicJsonDocument doc(1200);
    char myMqttMsg[400];
    doc["Cmd"] = thisCmd;
    JsonArray data = doc.createNestedArray("Data");
    for (byte i=0; i < newData->lnMsgSize; i++)
      data.add(newData->lnData[i]);
    serializeJson(doc, myMqttMsg);
    while (currClient >= 0)
    {
      globalClients[currClient].wsClient->text(myMqttMsg);
      currClient = getWSClientByPage(currClient+1, "pgLNViewer");
    }
    lastWifiUse = millis();
  }
}

//the following event handlers are called from processLocoNetMsg after decoding incoming LocoNet messages. Normally it is not necessary to use an event handler for status related data, e.g. inputs, switch, etc. The only
//message where an event handler is preferrable is button events as they are very volatile and just evaluating button status information mey lead to missing some statuses when changing fast

void handleTranspondingEvent(uint16_t zoneAddr, uint16_t locoAddr, uint8_t eventVal)
{
//  Serial.printf("Incoming Transponder Command for Zone %i Loco %i Status %i\n", zoneAddr, locoAddr, eventVal);
  if (eventHandler) eventHandler->processBtnEvent(evt_transponder, zoneAddr, locoAddr | (eventVal << 15));
  if (mySwitchList) mySwitchList->processBtnEvent(evt_transponder, zoneAddr, locoAddr | (eventVal << 15));
  if (myChain) myChain->processBtnEvent(evt_transponder, zoneAddr, locoAddr | (eventVal << 15));
}

void handleSwiEvent(uint16_t swiAddr, uint8_t swiPos, uint8_t coilStat)
{
  //add code here for event actions other than updating internal buffer
//  Serial.printf("Incoming Switch Command for Switch %i Position %i Status %i\n", swiAddr, swiPos, coilStat);
  if (eventHandler) eventHandler->processBtnEvent(evt_trackswitch, swiAddr, swiPos);
  if (mySwitchList) mySwitchList->processBtnEvent(evt_trackswitch, swiAddr, swiPos);
//  if (myChain) myChain->processBtnEvent(evt_trackswitch, swiAddr, swiPos);
}

void handleInputEvent(uint16_t inpAddr, uint8_t inpStatus)
{
  //add code here for event actions other than updating internal buffer
//  Serial.printf("Incoming Input Command for Detector %i Level %i\n", inpAddr, inpStatus);
  if (eventHandler) eventHandler->processBtnEvent(evt_blockdetector, inpAddr, inpStatus);
  if (mySwitchList) mySwitchList->processBtnEvent(evt_blockdetector, inpAddr, inpStatus);
}

void handleSignalEvent(uint16_t sigAddr, uint8_t sigAspect)
{
  //add code here for event actions other than updating internal buffer
//  Serial.printf("Incoming Signal Command for Signal %i Aspect %i\n", sigAddr, sigAspect);
  if (eventHandler) eventHandler->processBtnEvent(evt_signalmastdcc, sigAddr, sigAspect);
  if (mySwitchList) mySwitchList->processBtnEvent(evt_signalmastdcc, sigAddr, sigAspect);
}

void handlePowerStatus()
{
  //add code here for event actions other than updating internal buffer
//  Serial.printf("Incoming Power Status Event. New Status: %i\n", digitraxBuffer->getPowerStatus());
  if (eventHandler) eventHandler->processBtnEvent(evt_powerstat, 0, digitraxBuffer->getPowerStatus());
}

void handleAnalogValue(uint16_t analogAddr, uint16_t inputValue)
{
  //add code here for event actions other than updating internal buffer
//  Serial.printf("Incoming Analog Event for Input %i Value %i\n", analogAddr, inputValue);
  if (eventHandler) eventHandler->processBtnEvent(evt_analogvalue, analogAddr, inputValue);
  if (mySwitchList) mySwitchList->processBtnEvent(evt_analogvalue, analogAddr, inputValue);
}

void handleButtonValue(uint16_t btnAddr, uint8_t inputValue)
{
  //for buttons, we always check if there is an entry in the command list, and execute if ture
//  Serial.printf("Incoming Button Event for Button %i Event %i\n", btnAddr, inputValue);
  if (eventHandler) eventHandler->processBtnEvent(evt_button, btnAddr, inputValue);
  if (mySwitchList) mySwitchList->processBtnEvent(evt_button, btnAddr, inputValue);
}

void handleProgrammerEvent(uint8_t *  programmerSlot)
{
//  Serial.printf("Prog Stat: %i CV: %i Val: %i\n", programmerSlot[1], (programmerSlot[5]<<7) + (programmerSlot[6] & 0x7F), programmerSlot[7]);
  if (trainSensor) trainSensor->programmerReturn(programmerSlot);
  //add code for RedHat
}
