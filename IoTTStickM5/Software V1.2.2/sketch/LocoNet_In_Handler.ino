//onLocoNetMessage is called everytime a LocoNet message is received
//it should decode valid messages, update any status buffer and call event handlers as needed

void callbackLocoNetMessage(lnReceiveBuffer * newData) //this is the landing point for incoming LocoNet messages
                                                       //from LocoNet, MQTT, or Gateway
{
  if ((newData->errorFlags & (~msgEcho)) == 0) //filter out echo flag
    processLNValidMsg(newData);
  else
    processLNError(newData);
}

void processLNError(lnReceiveBuffer * newData)
{
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
}

void processLNValidMsg(lnReceiveBuffer * newData)
{
  processLocoNetMsg(newData);
  if (globalClient != NULL)
    processLNtoWebClient(newData);
  if (useM5Viewer == 1)
    processLNtoM5(newData);
}

void processLNtoWebClient(lnReceiveBuffer * newData) //if a web browser is conneted, all LN messages are sent via Websockets
                                                     //this is the hook for a web based LcooNet viewer
{
    DynamicJsonDocument doc(1200);
    char myMqttMsg[400];
    doc["Cmd"] = "LN";
    JsonArray data = doc.createNestedArray("Data");
    for (byte i=0; i < newData->lnMsgSize; i++)
      data.add(newData->lnData[i]);
    serializeJson(doc, myMqttMsg);
    globalClient->text(myMqttMsg);
    lastWifiUse = millis();
}

//the following event handlers are called from processLocoNetMsg after decoding incoming LocoNet messages. Normally it is not necessary to use an event handler for status related data, e.g. inputs, switch, etc. The only
//message where an event handler is preferrable is button events as they are very volatile and just evaluating button status information mey lead to missing some statuses when changing fast

void handleSwiEvent(uint16_t swiAddr, uint8_t swiPos, uint8_t coilStat)
{
  //add code here for event actions other than updating internal buffer
  Serial.printf("Incoming Switch Command for Switch %i Position %i Status %i\n", swiAddr, swiPos, coilStat);
}

void handleInputEvent(uint16_t inpAddr, uint8_t inpStatus)
{
  //add code here for event actions other than updating internal buffer
  Serial.printf("Incoming Input Command for Detector %i Level %i\n", inpAddr, inpStatus);
}

void handleSignalEvent(uint16_t sigAddr, uint8_t sigAspect)
{
  //add code here for event actions other than updating internal buffer
  Serial.printf("Incoming Signal Command for Signal %i Aspect %i\n", sigAddr, sigAspect);
}

void handlePowerStatus()
{
  //add code here for event actions other than updating internal buffer
  Serial.printf("Incoming Power Status Event. New Status: %i\n", getPowerStatus());
}

void handleAnalogValue(uint16_t analogAddr, uint16_t inputValue)
{
  //for buttons, we always check if there is an entry in the command list, and execute if ture
  Serial.printf("Incoming Analog Event for Input %i Value %i\n", analogAddr, inputValue);
}

void handleButtonValue(uint16_t btnAddr, uint8_t inputValue)
{
  //for buttons, we always check if there is an entry in the command list, and execute if ture
  Serial.printf("Incoming Button Event for Button %i Event %i\n", btnAddr, inputValue);
  if (buttonHandler) buttonHandler->processBtnEvent(btnAddr, (buttonEvent)inputValue);
}
