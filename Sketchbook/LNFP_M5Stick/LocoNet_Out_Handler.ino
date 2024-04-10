
//routines to prepare LocoNet Messages and send them to IoTT_LocoNetESP32
//used by ButtonHandler library to send commands to LocoNet

void sendSVCommand2(uint8_t SRC, uint8_t SV_CMD, uint16_t DST, uint16_t ADDR, uint8_t DTA[4])
{
  lnTransmitMsg txData;
  txData.lnData[0] = 0xE5;
  txData.lnData[1] = 0x10;
  txData.lnData[2] = SRC;
  txData.lnData[3] = SV_CMD;
  txData.lnData[4] = 0x02;
  txData.lnData[5] = 0x10 + ((ADDR & 0x8000) >> 12)  + ((ADDR & 0x0080) >> 5)  + ((DST & 0x8000) >> 14)  + ((DST & 0x0080) >> 7);  
  txData.lnData[6] = DST & 0x007F;
  txData.lnData[7] = (DST & 0x7F00) >> 8;
  txData.lnData[8] = ADDR & 0x007F;
  txData.lnData[9] = (ADDR & 0x7F00) >> 8;
  txData.lnData[10]= 0x10 + ((DTA[3] & 0x80) >> 4)  + ((DTA[2] & 0x80) >> 5)  + ((DTA[1] & 0x80) >> 6)  + ((DTA[0] & 0x80) >> 7);  
  txData.lnData[11]= (DTA[0] & 0x7F);
  txData.lnData[12]= (DTA[1] & 0x7F);
  txData.lnData[13]= (DTA[2] & 0x7F);
  txData.lnData[14]= (DTA[3] & 0x7F);
  txData.lnMsgSize = 16;
  setXORByte(&txData.lnData[0]);
  sendMsg(txData);
}

void sendSwitchCommand(uint8_t opCode, uint16_t swiNr, uint8_t swiTargetPos, uint8_t coilStatus)
{
//  Serial.printf("Req: %2X %i %i\n", opCode, swiNr, swiTargetPos);
  uint8_t currPos = digitraxBuffer->getSwiStatus(swiNr); //((swiPos[swiNr >> 2] >> (2 * (swiNr % 4))) & 0x03) << 4);
  lnTransmitMsg txData;
  txData.lnData[0] = opCode;
  txData.lnData[1] = swiNr & 0x007F;
  txData.lnData[2] = (swiNr & 0x0780)>>7;
  switch (opCode)
  {
    case 0xB1: //SWI_REP
    {
//      Serial.printf("TargetPos %i\n", swiTargetPos);
      txData.lnData[2] |= 0x60; //use switch input
      if (swiTargetPos == 0)
        txData.lnData[2] &= 0x6F;
      else
        txData.lnData[2] |= 0x10;
    }
    break;
    case 0xBD:; //SWI_ACK, same structure as next
    case 0xB0: //SWI_REQ
    {
      switch (swiTargetPos)
      {
        case thrown: break;
        case closed: txData.lnData[2] |= 0x20; break;
        case toggle: txData.lnData[2] |= ((currPos & 0x20) ^ 0x20); break;
        case nochange: txData.lnData[2] = (txData.lnData[2] & 0xDF) | (currPos & 0x20); break;
      }
      if (coilStatus > 0)
        txData.lnData[2] |= 0x10;
    }
    break;
  }
  txData.lnMsgSize = 4;
//  Serial.printf("LN Out: 1 %2X 2 %2X 3 %2X 4 %2X\n", txData.lnData[0],txData.lnData[1],txData.lnData[2],txData.lnData[3]);
  setXORByte(&txData.lnData[0]);
//  Serial.printf("LN Out: 1 %2X 2 %2X 3 %2X 4 %2X\n", txData.lnData[0],txData.lnData[1],txData.lnData[2],txData.lnData[3]);
  sendMsg(txData);
}

void sendSignalCommand(uint16_t signalNr, uint8_t signalAspect)
{
//  Serial.printf("Signal Nr %i Aspect %i \n", signalNr, signalAspect);
  lnTransmitMsg txData;
  uint8_t boardAddr = (((signalNr-1) & 0x07FC)>>2) + 1;
  uint8_t turnoutIndex = (signalNr-1) & 0x03;
  txData.lnData[0] = 0xED; //OPC_IMM_
  txData.lnData[1] = 0x0B;
  txData.lnData[2] = 0x7F;
  txData.lnData[3] = 0x31; //3 IM Bytes, 3 repetitions
  txData.lnData[5] = ((boardAddr & 0x003F) | 0x80); //IM1
  txData.lnData[6] = ((~boardAddr & 0x01C0)>>2) | ((turnoutIndex & 0x03)<<1) | 0x01; //IM2
  txData.lnData[7] = (signalAspect & 0x01FF); //IM3
  txData.lnData[4] = ((txData.lnData[5] & 0x80)>>7) + ((txData.lnData[6] & 0x80)>>6) + 0x20; //DHI
  txData.lnData[5] &= 0x7F;
  txData.lnData[6] &= 0x7F;
  txData.lnData[8] = 0x00;
  txData.lnData[9] = 0x00;
  txData.lnData[10] = 0xED;
  txData.lnMsgSize = 11;
  setXORByte(&txData.lnData[0]);
  sendMsg(txData);
}

void sendBlockDetectorCommand(uint16_t bdNr, uint8_t bdStatus) 
{ 
//  Serial.printf("Block Detector Nr %i Status %i \n", bdNr, bdStatus);
  lnTransmitMsg txData;
  txData.lnData[0] = 0xB2; //OPC_INPUT_REP
  txData.lnData[1] = (bdNr & 0x00FE)>>1;
  txData.lnData[2] = (bdNr & 0x0F00)>>8;
  if ((bdNr & 0x0001) > 0)
    txData.lnData[2] |= 0x20;
  if (bdStatus > 0)
    txData.lnData[2] |= 0x10;
  txData.lnMsgSize = 4;
  setXORByte(&txData.lnData[0]);
  sendMsg(txData);
}

void sendSwiReportMessage(uint16_t inpAddr, uint8_t newPos)
{
  lnTransmitMsg txData;
  txData.lnData[0] = 0xB1; 
  txData.lnData[1] = (inpAddr & 0x007F); 
  txData.lnData[2] = ((inpAddr & 0x0780) >> 7) + 0x60 + ((newPos & 0x01) << 4); 
  txData.lnMsgSize = 4;
  setXORByte(&txData.lnData[0]);
  sendMsg(txData);
}

void sendButtonCommand(uint16_t btnNr, uint8_t  btnEvent)
{
//  Serial.printf("Button Nr %i Command %i \n", btnNr, btnEvent);
  lnTransmitMsg txData;
  txData.lnData[0] = 0xE5; //OPC_PEER_XFER
  txData.lnData[1] = 0x10; //16 byte message
  txData.lnData[2] = 0x0C; //can be any number from 0x01 to 0x6F
  txData.lnData[3] = 0x71;
  txData.lnData[4] = 0x02;
  
  txData.lnData[5] = 0x10;
  txData.lnData[6] = 0x00;
  txData.lnData[7] = 0x00;
  
  txData.lnData[8] = (btnNr & 0x3F); //addr low 6  bit
  txData.lnData[9] = (btnNr & 0xFC0) >> 6; //addr high 6 bits
  
  txData.lnData[10] = 0x10;
  txData.lnData[11] = 0x01; //button message
  txData.lnData[12] = btnEvent;
  txData.lnData[13] = 0x00;
  txData.lnData[14] = 0x00;
  
  txData.lnMsgSize = 16;
  setXORByte(&txData.lnData[0]);
  sendMsg(txData);
}

void sendAnalogCommand(uint16_t btnNr, uint16_t analogVal)
{
//  Serial.printf("Analog Command Addr %i Value %i \n", btnNr, analogVal);
  lnTransmitMsg txData;
  txData.lnData[0] = 0xE5; //OPC_PEER_XFER
  txData.lnData[1] = 0x10; //16 byte message
  txData.lnData[2] = 0x0C; //can be any number from 0x01 to 0x6F
  txData.lnData[3] = 0x71;
  txData.lnData[4] = 0x02;
  
  txData.lnData[5] = 0x00;
  txData.lnData[6] = 0x00;
  txData.lnData[7] = 0x00;
  
  txData.lnData[8] = (btnNr & 0x3F); //addr low 6  bit
  txData.lnData[9] = (btnNr & 0xFC) >> 6; //addr high 6 bits
  
  txData.lnData[10] = 0x10;
  txData.lnData[11] = 0x00; //analog message
  txData.lnData[12] = (analogVal & 0x3F); //data low
  txData.lnData[13] = (analogVal & 0x0FC0) >> 6; //data high
  txData.lnData[14] = 0x00;

  txData.lnMsgSize = 16;
  setXORByte(&txData.lnData[0]);
  sendMsg(txData);
}

void sendPowerCommand(uint8_t cmdType, uint8_t pwrStatus)
{ 
//  Serial.printf("Power Command %i %i %i\n", cmdType, pwrStatus, digitraxBuffer->getPowerStatus());
  lnTransmitMsg txData;
  txData.lnMsgSize = 2;
  if (cmdType == toggleVal)
    switch (digitraxBuffer->getPowerStatus())
    {
      case 0: txData.lnData[0] = 0x85; break; //off to OPC_IDLE
      case 1: txData.lnData[0] = 0x85; break; //on to OPC_IDLE
      case 2: txData.lnData[0] = 0x83; break; //idle to OPC_GPON
      default: txData.lnData[0] = 0x85; break; //OPC_IDLE
    }
  else
    switch (pwrStatus)
    {
      case 0x82:;
      case 0: txData.lnData[0] = 0x82; break; //OPC_GPOFF
      case 0x83:;
      case 1: txData.lnData[0] = 0x83; break; //OPC_GPON
      case 2: txData.lnData[0] = 0x85; break; //OPC_IDLE
      default: txData.lnData[0] = 0x85; break; //OPC_IDLE
    }
//  txData.lnData[1] = ~txData.lnData[0];
  txData.lnMsgSize = 2;
  setXORByte(&txData.lnData[0]);
  sendMsg(txData);
//  dispMsg(&txData.lnData[0]);
}

uint16_t sendLocoNetReply(lnTransmitMsg txData)
{
//  Serial.printf("Send reply: %2X", txData.lnData[0]);
//  for (uint8_t i = 1; i < txData->lnMsgSize; i ++)
//    Serial.printf(" %2X ", txData->lnData[i]);
//  Serial.println();
  if (lnSerial)
    return lnSerial->lnWriteReply(&txData);
  else
    return 0;
}
/*
uint16_t sendDCCCmdGen(lnTransmitMsg txData)
{
//  Serial.printf("Send it %i %i\n", txData.lnData[0], txData.lnData[1]);
  if (usbSerial)
    usbSerial->lnWriteMsg(txData);
}
*/
//callback interface to the IoTT_Buttons library. If a Button or Analog event occurs, the library calles one of these functions
//all we do here is sending the information to LocoNet, from where we will receive it back as serial port echo that can be processed like any regular LocoNet message

void onSensorEvent(uint16_t sensorAddr, uint8_t sensorEvent, uint8_t eventMask)
{
//  Serial.println(eventMask);
//  Serial.println(sensorEvent);
  if (sensorEvent == onbtndown)
    sendBlockDetectorCommand(sensorAddr, (eventMask & 0x04) == 0 ? 0 : 1);
  if (sensorEvent == onbtnup)
    sendBlockDetectorCommand(sensorAddr, (eventMask & 0x04) == 0 ? 1 : 0);
}

void onSwitchReportEvent(uint16_t switchAddr, uint8_t switchEvent, uint8_t eventMask)
{
//  Serial.printf("Switch %i has position %i. \n", switchAddr, switchEvent);
  if (switchEvent == onbtndown) //this really is the status, not the event, so we need to change the logic
    sendSwitchCommand(0xB1, switchAddr, switchEvent, (eventMask & 0x04)== 0 ? 1 : 0);
  if (switchEvent == onbtnup)
    sendSwitchCommand(0xB1, switchAddr, switchEvent, (eventMask & 0x04)== 0 ? 0 : 1);
}

void onSwitchRequestEvent(uint16_t switchAddr, uint8_t switchEvent)
{
}

void onButtonEvent(uint16_t btnAddr, buttonEvent thisEvent)
{
//  Serial.printf("Button %i has event %i. \n", btnAddr, thisEvent);
  sendButtonCommand(btnAddr, thisEvent);
}

void onAnalogData(uint16_t inpNr, uint16_t analogValue )
{
//  Serial.printf("Analog Input %i has value %i.\n", inpNr, analogValue);
  sendAnalogCommand(inpNr, analogValue);
}

void onBtnDiagnose(uint8_t evtType, uint8_t portNr, uint16_t inpAddr, uint16_t btnValue)
{
  int8_t currClient = getWSClientByPage(0, "pgHWBtnCfg");
  if (currClient >= 0)
  {
//    Serial.printf("Button %i Diagnose %i alias %i has value %i.\n", evtType, portNr, inpAddr, btnValue);
    DynamicJsonDocument doc(1200);
    char myMqttMsg[400];
    
    doc["Cmd"] = "HWBtn";
    JsonArray data = doc.createNestedArray("Data");
    data.add(evtType);
    data.add(portNr);
    data.add(inpAddr);
    data.add(btnValue);
    serializeJson(doc, myMqttMsg);
    while (currClient >= 0)
    {
      globalClients[currClient].wsClient->text(myMqttMsg);
      currClient = getWSClientByPage(currClient + 1, "pgHWBtnCfg");
    }
//    Serial.println(myMqttMsg);
    lastWifiUse = millis();
  }
}
