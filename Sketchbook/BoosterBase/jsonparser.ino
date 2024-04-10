void procInput()
{
  while (Serial.available())
  {
    serBuffer[readPtr] = Serial.read();
    if (readPtr == 0)
    {
      if ((serBuffer[0] != '\r') && (serBuffer[0] != '\n'))
        readPtr++;
    }
    else
      if ((serBuffer[readPtr] == '\r') || (serBuffer[readPtr] == '\n'))
      {
        serBuffer[readPtr] = '\0';
        cmdParser(serBuffer);
        readPtr = 0;
      }
      else
        readPtr++;
  }
}

void cmdParser(char newCmd[])
{
//  Serial.println(newCmd);
  int docSize = 256;
  DynamicJsonDocument doc(docSize);
  DeserializationError error = deserializeJson(doc, newCmd);//, msgLen);
  if (!error)
  {

    if (doc.containsKey("Cmd"))
    {
      const char* CmdName = doc["Cmd"];
      int8_t nodeNr = -1; //all nodes
      if (doc.containsKey("Node"))
        nodeNr = doc["Node"];
/*
      if (CmdName == "SCT") //{"Cmd":"SCT"}
        bList.startSCTest(nodeNr);
      if (CmdName == "PWR") //{"Cmd":"PWR"}
      {
        uint8_t pwrStat = 0;
        if (doc.containsKey("State")) //0: off 1: on 
          pwrStat = doc["State"];
        bList.setExtStatus(nodeNr, pwrStat);
      }
      if (CmdName == "RST") //{"Cmd":"RST"} Reset
        bList.resetBooster(nodeNr);
      if (CmdName == "CFG") //{"Cmd":"CFG"} get Config
        bList.requestBoosterReport(nodeNr, 0x01);
*/
      if (strcmp(CmdName, "LN") == 0) //Set booster config elements
      {
        if (doc.containsKey("Prm"))
        {
          lnActivatorDef lnAct;
          uint8_t svOpc = doc["Prm"]["Opc"];
          JsonArray srcData = doc["Prm"]["Vals"];
          uint8_t srcVal = srcData[0];
          srcVal &= 0x1F;//5bit value
          lnAct.devAddr = doc["Prm"]["Addr"];
          switch (svOpc)
          {
            case 0xB0:; //switch
            case 0xBD: //switch ack
              lnAct.trigDef = (0x10 << 1) + ((srcVal & 0x02) >> 1); 
              lnAct.devAddr++;
              break; 
            case 0xED: //DCC Signal (Imm Packet)
              lnAct.trigDef = (0x20 << 1) + srcVal; 
              break;
            case 0xB2:  //block detector
              lnAct.trigDef = (0x40 << 1) + (srcVal & 0x01); 
              lnAct.devAddr++;
              break;
            case 0x71: //SV button command
              {
                if (srcVal == 0x01)
                {
                  srcVal = srcData[1];
                  lnAct.trigDef = (0x30 << 1) + (srcVal & 0x03); 
                }
                else
                  return;
              }
              break;
            default: return; //no valid opcode
          }
//          lnAct.trigDef;
          bList.processExtCommand(0x3F, lnAct);
        }
      }
      if (strcmp(CmdName, "BTN") == 0) //Stick button pressed
      {
        bList.setExtStatus(-1, 0xFE);
      }
      if (strcmp(CmdName, "SV") == 0) //Set booster config elements
      {
        if (doc.containsKey("Prm"))
        {
          uint8_t svOpc = doc["Prm"]["Opc"];
          uint16_t memAddr = doc["Prm"]["Addr"];
          uint16_t lnAddr = doc["Prm"]["Dst"];
          JsonArray memData = doc["Prm"]["Vals"];
          uint8_t svDataSize = memData.size();
          uint8_t svData[svDataSize];
          switch (svOpc)
          {
            case 0x01: //write 1 byte from D1 to mem loc
            {
              if ((svDataSize > 0) && (lnAddr == bList.getLnAddress()))
              {
                svData[0] = memData[0];
                bList.writeSVData(svOpc, memAddr, svData);
              }
            }
            break;
            case 0x02: //read 1 byte from mem loc and return in D1
              if (lnAddr == bList.getLnAddress())
                bList.requestSVData(svOpc, memAddr);
            break;
            case 0x05: //write 4 bytes from D1..D4 to mem loc
              if ((svDataSize > 3) && (lnAddr == bList.getLnAddress()))
              {
                for (uint8_t i = 0; i < svDataSize; i++)
                  svData[i] = memData[i];
                bList.writeSVData(svOpc, memAddr, svData);
              }
            break;
            case 0x06: //read 4 bytes from mem loc and return in D1..D4
            {
              if (lnAddr == bList.getLnAddress())
                bList.requestSVData(svOpc, memAddr);
            }
            break;
            case 0x07: bList.requestSVData(svOpc, memAddr);
            break;
            case 0x08: if (lnAddr == bList.getLnAddress()) bList.requestSVData(svOpc, memAddr);
            break;
            case 0x09: bList.requestSVData(svOpc, memAddr);
              if ((svDataSize > 3) && (memAddr == bList.getLnAddress()))
              {
                for (uint8_t i = 0; i < svDataSize; i++)
                  svData[i] = memData[i];
                bList.writeSVData(svOpc, lnAddr, svData);
              }
            break;
          }
        }
/*        
        if (doc.containsKey("SF"))
          bList.setSensorFactor(nodeNr, doc["SF"]);
        if (doc.containsKey("NC"))
          bList.setNominalCurrent(nodeNr, doc["NC"]);
        if (doc.containsKey("FS"))
          bList.setFuseValue(nodeNr, doc["FS"]);
        if (doc.containsKey("RS"))
          bList.setResetMode(nodeNr, doc["RS"]);
        if (doc.containsKey("AR"))
          bList.setARMode(nodeNr, doc["AR"]);
        if (doc.containsKey("EE"))
          bList.writeEEPROM(nodeNr, doc["EE"]);
*/
      }
    }
  }
  else
    Serial.println(error.c_str());
}

/*
void getBoosterOpsReport(Booster* thisBooster)
{
  char myMqttMsg[50];
//  sprintf(myMqttMsg, "{\"I\":[%i,%i,%i,%i,%i,%i,%i,%i]}\n",thisBooster->nodeNr, (round(thisBooster->bOps.currOLF * 100)), (round(thisBooster->bOps.currRMS)), thisBooster->bOps.intStat, thisBooster->bOps.extStat, digitalRead(thisBooster->bCfg.reversePin));
//  Serial.print(myMqttMsg);
}

void getBoosterHwCfgReport(Booster* thisBooster)
{
  char myMqttMsg[50];
//  sprintf(myMqttMsg, "{\"HW\":[%i,%i,%i,%i,%i]}\n",thisBooster->nodeNr, thisBooster->bCfg.ctrlPin, thisBooster->bCfg.sensePin-14, thisBooster->bCfg.reversePin,round(thisBooster->bCfg.senseFactor * 1000));
//  Serial.print(myMqttMsg);
}

void getBoosterElCfgReport(Booster* thisBooster)
{
  char myMqttMsg[50];
//  sprintf(myMqttMsg, "{\"EL\":[%i,%i,%i,%i,%i]}\n",thisBooster->nodeNr, thisBooster->bCfg.currNominal, thisBooster->bCfg.fuseMode, thisBooster->bCfg.autoResetMode, thisBooster->bCfg.autoReverseMode);
//  Serial.print(myMqttMsg);
}
*/
