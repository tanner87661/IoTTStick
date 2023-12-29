#include "BoosterAdmin.h"


BoosterGroup::BoosterGroup()
{
}

BoosterGroup::~BoosterGroup()
{
}

void BoosterGroup::initBooster()
{
  bConfig = readEEPROM();
  for (uint8_t i = 0; i < 6; i++)
    bData[i].nodeNr = i;
//  numDevs = bConfig.numMods;
//  Serial.println(bConfig.numMods);
//  sigMask = (bConfig.devMode & 0xF0) >> 4;
//  sigMask = (bConfig.acceptPWM << 1) + bConfig.acceptDCC;
}

void BoosterGroup::initNodes() //nodeConfigData* initData, uint8_t numBoosters)
{
  for (uint8_t i = 0; i < bConfig.numMods; i++)
    bData[i].initNode(i, bConfig.lnAddress); //, &(initData[i]));
//  Serial.println(bConfig.numMods);
}

void BoosterGroup::setExtStatus(int8_t boosterNr, uint8_t newStat)
{
//  Serial.print("Booster ");
//  Serial.print(boosterNr);
//  Serial.print(" Status ");
//  Serial.println(newStat);
  if (newStat == 0xFF) //dev mode
    pwrStatus = (bConfig.devMode & 0x0F);
  else
    if (newStat == 0xFE) //toggle
      pwrStatus ^= 0x01;
    else
      pwrStatus = newStat;
  for (uint8_t i = 0; i < bConfig.numMods; i++)
    if ((i == boosterNr) || (boosterNr < 0))
      bData[i].setExtStatus(pwrStatus);
}

void BoosterGroup::resetBooster(int8_t boosterNr)
{
  for (uint8_t i = 0; i < bConfig.numMods; i++)
    if ((i == boosterNr) || (boosterNr < 0))
      bData[i].resetBooster();
}

void BoosterGroup::requestSVData(uint8_t svCmd, int16_t memLoc) //set report flags
{
  if ((memLoc & 0xFF00) == 0) //256 bytes device memory
  {
    switch (svCmd)
    {
      case 7: reportFlags |= 0x01; break;
      case 8: reportFlags |= 0x02; break;
      default:
        switch (memLoc & 0x00FF)
        {
          case 0x00: reportFlags |= 0x0004; break; //get number of modules, LN Mode, LN Addr
          case 0x04: reportFlags |= 0x0008; break; //get power on mode, accepted signal type
          case 0x08: reportFlags |= 0x0010; break; //get LN Actuator for Device on type, state, addr
          case 0x0C: reportFlags |= 0x0020; break; //get LN Actuator for Device off type, state, addr

          
          case 0xF0: reportFlags |= 0x0100; break; //get current signal status
          case 0xF1: reportFlags |= 0x0200; break; //get current global power status

          
          case 0xFF: reportFlags = 0xFFFF; break; //report all
        }
      break;
    }
  }
  else
  {
    uint8_t boosterNr = (memLoc >> 8) - 1;
    for (uint8_t i = 0; i < 6; i++)
      if ((i == boosterNr) || (memLoc & 0xFF00) == 0xFF00)
        bData[i].requestSVData(svCmd, memLoc & 0x00FF);
  }
}

void BoosterGroup::writeSVData(uint8_t svCmd, int16_t memLoc, uint8_t svData[]) //write SV data
{
  if (((memLoc & 0xFF00) == 0) || (svCmd == 0x09))
  {
    //write SV data
    switch (svCmd)
    {
      case 0x07:;
      case 0x08: 
        break;
      case 0x09: //set LN Address
        if (false)
        {
          bConfig.lnAddress = memLoc;
          requestSVData(0x08, 0);
        }
        break;
      case 0x01: ;
      case 0x05:
        switch (memLoc)
        {
          case 0x00: //set/get number of modules, LN Mode, LN Addr
          {
            uint8_t oldNumMods = bConfig.numMods;
            bConfig.numMods = svData[0];
            bConfig.useLN = svData[1];
            bConfig.lnAddress = (svData[2] << 8) + svData[3];
            if (oldNumMods < bConfig.numMods)
            {
              for (uint8_t i = oldNumMods; i < bConfig.numMods; i++)
                bData[i].initNode(i, bConfig.lnAddress);
            }
            break;
          }
          case 0x04: //set/get power on mode, accepted signal type
            bConfig.devMode = svData[0];
            break;
          case 0x08: //set LN Actuator for Device on type, state, addr
            bConfig.globalOn.trigDef = svData[0];
            bConfig.globalOn.devAddr = (svData[1] << 8) + svData[2];
            break;
          case 0x0C: //set LN Actuator for Device off type, state, addr
            bConfig.globalOff.trigDef = svData[0];
            bConfig.globalOff.devAddr = (svData[1] << 8) + svData[2];
            break;
//          case 0xF0: //signal status is read only
//            break;
          case 0xF1: //set global power status on/off
            setExtStatus(-1, svData[0]); //include updating all subnodes
            break;
          case 0xFF: //write all data to EEPROM
//            Serial.println("write EEPROM");
            writeEEPROM(-1, true); //include updating all subnodes
            break;
        }
        requestSVData(svCmd & 0x0F, memLoc);
        break;
      default: 
        break;
    }
  }
  else
  {
    uint8_t boosterNr = (memLoc >> 8) - 1;
    for (uint8_t i = 0; i < 6; i++)
      if ((i == boosterNr) || (memLoc & 0xFF00) == 0xFF00)
        bData[boosterNr].writeSVData(svCmd, memLoc & 0x00FF, svData);
  }
}

void BoosterGroup::setSensorFactor(int8_t nodeNr, double newSensFact)
{
  for (uint8_t i = 0; i < bConfig.numMods; i++)
    if ((i == nodeNr) || (nodeNr < 0))
      bData[i].setSensorFactor(newSensFact);
}

void BoosterGroup::setNominalCurrent(int8_t nodeNr, uint16_t newCurrent)
{
  for (uint8_t i = 0; i < bConfig.numMods; i++)
    if ((i == nodeNr) || (nodeNr < 0))
      bData[i].setNominalCurrent(newCurrent);
}

void BoosterGroup::setFuseValue(int8_t nodeNr, uint8_t newFuseMode)
{
  for (uint8_t i = 0; i < bConfig.numMods; i++)
    if ((i == nodeNr) || (nodeNr < 0))
      bData[i].setFuseValue(newFuseMode);
}

void BoosterGroup::setResetMode(int8_t nodeNr, uint8_t newResetMode)
{
  resetMode = newResetMode;
  for (uint8_t i = 0; i < bConfig.numMods; i++)
    if ((i == nodeNr) || (nodeNr < 0))
      bData[i].setResetMode(newResetMode);
}

void BoosterGroup::setARMode(int8_t nodeNr, bool newARMode)
{
  for (uint8_t i = 0; i < bConfig.numMods; i++)
    if ((i == nodeNr) || (nodeNr < 0))
      bData[i].setARMode(newARMode);
}

void BoosterGroup::setSignalStatus(extSignalStatus newStatus)
{
  for (uint8_t i = 0; i < bConfig.numMods; i++)
    bData[i].setSignalStatus(newStatus);
}

void BoosterGroup::writeEEPROM(int8_t nodeNr, bool writeEE)
{
  uint16_t baseAddr = 1;
  bConfig.checkByte = 0;
  uint8_t * wrPtr = &bConfig.numMods;
  uint8_t recSize = sizeof(bConfig);
  for (uint8_t i = 0; i < recSize-1; i++)
  {
//    Serial.print(*wrPtr);
//    Serial.print(",");
    bConfig.checkByte ^= (*wrPtr);
    wrPtr++;
  }
  EEPROM.put(baseAddr, bConfig);
//  Serial.println();
//  Serial.println(bConfig.checkByte);
  for (uint8_t i = 0; i < bConfig.numMods; i++)
    if ((i == nodeNr) || (nodeNr < 0))
      bData[i].writeEEPROM(writeEE);
}

boosterConfigData BoosterGroup::readEEPROM()
{
  boosterConfigData readData;
  uint8_t * rdPtr = &readData.numMods;
  uint16_t baseAddr = 1;
  uint8_t chkSum = 0;
  uint8_t recSize = sizeof(bConfig);
  EEPROM.get(baseAddr, readData);
  for (uint8_t i = 0; i <  recSize-1; i++)
  {
//    Serial.print(*rdPtr);
//    Serial.print(",");
    chkSum ^= (*rdPtr);
    rdPtr++;
  }
  Serial.println();
  Serial.println(readData.checkByte);
  Serial.println(chkSum);
  if (chkSum != readData.checkByte)
  {
    readData.numMods = 0;
    readData. serNr = random(255);
    readData.lnAddress = 1234;
    readData.useLN = false;
    readData.devMode = 0x00; //DCC/pwrup OFF
    readData.globalOn.trigDef = 0;
    readData.globalOn.devAddr = 0;
    readData.globalOff.trigDef = 0;
    readData.globalOff.devAddr = 0;
  }
  return readData;
}

void BoosterGroup::processTimerInterrupt() //called by interrupt handler
{
//  digitalWrite(LED_BUILTIN, 1);
  if (digitalRead(DCCA_PIN))
    posCtrA++;
  else
    negCtrA++;
  if (digitalRead(DCCB_PIN))
    posCtrB++;
  else
    negCtrB++;
  splCtr++;
  
  if (devIndex < bConfig.numMods) //if the booster is defined, we process it
  {
    Booster* thisBooster = &bData[devIndex];
    thisBooster->processTimerInterrupt();
  }
  devIndex = (devIndex + 1) % (maxDefs+1); //move ptr to next
//  digitalWrite(LED_BUILTIN, 0);
}

void BoosterGroup::checkInputSignal()
{
  int8_t hasSig = -1;
  if (splCtr > qSamples)
  {
    uint8_t okVal = qSamples >> 2;
    bool activeA = (posCtrA > 0) && (negCtrA > 0);
    bool activeB = (posCtrB > 0) && (negCtrB > 0);
    bool diffSig = abs((posCtrA - negCtrB)) < okVal;
    bool isPWM = activeA ^ activeB;
    isPWM &= ((bConfig.devMode & 0xF0) == 0x10);
    bool isDCC = activeA && activeB && diffSig; //2 input DCC only
    isDCC &= ((bConfig.devMode & 0xF0) == 0x00);
    hasSig = (isPWM << 1) + isDCC;
    if (hasSig != sigStatus)
    {
/*
      Serial.print(negCtrA);
      Serial.print(" ");
      Serial.print(posCtrA);
      Serial.print(" ");
      Serial.print(negCtrB);
      Serial.print(" ");
      Serial.print(posCtrB);
      Serial.print(" ");
      Serial.print(okVal);
      Serial.print(" ");
      Serial.print(diffSig);
      Serial.print(" ");
      Serial.print(isPWM);
      Serial.print(" ");
      Serial.print(isDCC);
      Serial.println(" ");
*/
      sigStatus = hasSig;
      for (uint8_t i = 0; i < bConfig.numMods; i++)
        bData[i].setSignalStatus(sigStatus);
      reportFlags |= 0x0100;  
    }
    negCtrA = 0;  
    posCtrA = 0; 
    negCtrB = 0;  
    posCtrB = 0; 
    splCtr = 0;
  }
}

void BoosterGroup::processLoop()
{
  checkInputSignal();
  processReportRequest();
  for (uint8_t i = 0; i < maxDefs; i++)
  {
    Booster* thisBooster = getBooster(i);
    if (thisBooster)
    {
      if (thisBooster->processLoop())
        break;
    }
  }
}

/*
  0x01: manufacturer, developer, product, serial number
  0x02: # of modules,  LN address (default 25)
  0x04: power up mode, accepted signal type, LN mode
  0x10: LN Actuator for Device On type, state, addr
  0x20: LN Actuator for Device Off type, state, addr
*/

bool BoosterGroup::processReportRequest()
{
  if ((millis() - lastReportTime) > reportInterval) //rollover safe
  {
    lastReportTime = millis();
    char myMqttMsg[100];
    if (reportFlags & 0x01) //
    {
      reportFlags &= ~0x01;
      sprintf(myMqttMsg, quadByteResponse, bConfig.lnAddress, 0x47, 0x00, manufID, devID, prodID, bConfig.serNr);
      Serial.print(myMqttMsg);
      return true;
    }
    if (reportFlags & 0x02) //
    {
      reportFlags &= ~0x02;
      sprintf(myMqttMsg, quadByteResponse, bConfig.lnAddress, 0x48, 0x00, manufID, devID, prodID, bConfig.serNr);
      Serial.print(myMqttMsg);
      return true;
    }
    if (reportFlags & 0x04) //set/get number of modules, LN Mode, LN Addr
    {
      reportFlags &= ~0x04;
      sprintf(myMqttMsg, quadByteResponse, bConfig.lnAddress, 0x46, 0x00, bConfig.numMods, bConfig.useLN, ((bConfig.lnAddress & 0xFF00)>>8), (bConfig.lnAddress & 0x00FF));
      Serial.print(myMqttMsg);
      return true;
    }
    if (reportFlags & 0x08) //set/get power on mode, accepted signal type
    {
      reportFlags &= ~0x08;
      sprintf(myMqttMsg, singleByteResponse, bConfig.lnAddress, 0x42, 0x04, bConfig.devMode);
      Serial.print(myMqttMsg);
      return true;
    } 
    if (reportFlags & 0x10) //LN Actuator On type, state, addr
    {
      reportFlags &= ~0x10;
      sprintf(myMqttMsg, quadByteResponse, bConfig.lnAddress, 0x46, 0x08, bConfig.globalOn.trigDef, (bConfig.globalOn.devAddr >> 8),(bConfig.globalOn.devAddr & 0x00FF),0);
      Serial.print(myMqttMsg);
      return true;
    }
    if (reportFlags & 0x20) //LN Actuator Off type, state, addr
    {
      reportFlags &= ~0x20;
      sprintf(myMqttMsg, quadByteResponse, bConfig.lnAddress, 0x46, 0x0C, bConfig.globalOff.trigDef, (bConfig.globalOff.devAddr >> 8),(bConfig.globalOff.devAddr & 0x00FF),0);
      Serial.print(myMqttMsg);
      return true;
    }
    if (reportFlags & 0x0100) //report signal status
    {
      reportFlags &= ~0x0100;
      sprintf(myMqttMsg, singleByteResponse, bConfig.lnAddress, 0x42, 0xF0, sigStatus);
      Serial.print(myMqttMsg);
      return true;
    }
    if (reportFlags & 0x0200) //get current global power stauts
    {
      reportFlags &= ~0x0200;
      sprintf(myMqttMsg, singleByteResponse, bConfig.lnAddress, 0x42, 0xF1, pwrStatus);
      Serial.print(myMqttMsg);
      return true;
    }
    for (uint8_t i = 0; i < bConfig.numMods; i++)
      if (bData[i].processReportRequest())
        return true;

    return false;
  }
  else
    return false;
}

void BoosterGroup::processExtCommand(int8_t boosterNr, lnActivatorDef newCmd) //called from parser for device and modules (0x3F)
{
  if (boosterNr & 0x20)
  {
    if ((newCmd.trigDef == bConfig.globalOn.trigDef) && (newCmd.devAddr == bConfig.globalOn.devAddr)) 
      setExtStatus(-1, extRun); //extStatus calls modules setExtStatus as well
    if ((newCmd.trigDef == bConfig.globalOff.trigDef) && (newCmd.devAddr == bConfig.globalOff.devAddr)) 
      setExtStatus(-1, extStop);
  }
  if (boosterNr & 0x10)
    for (uint8_t i = 0; i < bConfig.numMods; i++)
      if ((i == (boosterNr & 0x0F)) || ((boosterNr & 0x0F) == 0x0F))
        bData[i].processExtCommand(newCmd);
}

void BoosterGroup::startSCTest(int8_t nodeNr)
{
  if (nodeNr >= 0)
  {
    Booster* thisBooster = getBooster(nodeNr);
    if (thisBooster)
      thisBooster->startSCTest();
  }
}

Booster* BoosterGroup::getBooster(uint8_t ofIndex)
{
  if (ofIndex < bConfig.numMods)
    return &bData[ofIndex];
  else
    return NULL;
}
