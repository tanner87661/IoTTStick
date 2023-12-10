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

  Serial.println(bConfig.numMods);
  Serial.println(bConfig.serNr);
  Serial.println(bConfig.checkByte);

  while (true)
    delay(100);
    
  for (uint8_t i = 0; i < 6; i++)
    bData[i].nodeNr = i;
//  bConfig = *initData;
  sigMask = (bConfig.devMode & 0xF0) >> 4;
//  sigMask = (bConfig.acceptPWM << 1) + bConfig.acceptDCC;
}

void BoosterGroup::initNodes(nodeConfigData* initData, uint8_t numBoosters)
{
  numDevs = min(numBoosters, maxPins);
  for (uint8_t i = 0; i < numDevs; i++)
    bData[i].initNode(i, &(initData[i]));
//  Serial.println(numDevs);
}

void BoosterGroup::setExtStatus(int8_t boosterNr, uint8_t newStat)
{
  pwrStatus = newStat;
  for (uint8_t i = 0; i < numDevs; i++)
    if ((i == boosterNr) || (boosterNr < 0))
      bData[i].setExtStatus(newStat);
}

void BoosterGroup::resetBooster(int8_t boosterNr)
{
  for (uint8_t i = 0; i < numDevs; i++)
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
        switch (memLoc)
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
      case 0x01:;
      case 0x05:
        switch (memLoc)
        {
          case 0x00: //set/get number of modules, LN Mode, LN Addr
            numDevs = svData[0];
            bConfig.useLN = svData[1];
            bConfig.lnAddress = (svData[2] << 8) + svData[3];
            break;
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
  for (uint8_t i = 0; i < numDevs; i++)
    if ((i == nodeNr) || (nodeNr < 0))
      bData[i].setSensorFactor(newSensFact);
}

void BoosterGroup::setNominalCurrent(int8_t nodeNr, uint16_t newCurrent)
{
  for (uint8_t i = 0; i < numDevs; i++)
    if ((i == nodeNr) || (nodeNr < 0))
      bData[i].setNominalCurrent(newCurrent);
}

void BoosterGroup::setFuseValue(int8_t nodeNr, uint8_t newFuseMode)
{
  for (uint8_t i = 0; i < numDevs; i++)
    if ((i == nodeNr) || (nodeNr < 0))
      bData[i].setFuseValue(newFuseMode);
}

void BoosterGroup::setResetMode(int8_t nodeNr, uint8_t newResetMode)
{
  resetMode = newResetMode;
  for (uint8_t i = 0; i < numDevs; i++)
    if ((i == nodeNr) || (nodeNr < 0))
      bData[i].setResetMode(newResetMode);
}

void BoosterGroup::setARMode(int8_t nodeNr, bool newARMode)
{
  for (uint8_t i = 0; i < numDevs; i++)
    if ((i == nodeNr) || (nodeNr < 0))
      bData[i].setARMode(newARMode);
}

void BoosterGroup::setSignalStatus(extSignalStatus newStatus)
{
  for (uint8_t i = 0; i < numDevs; i++)
    bData[i].setSignalStatus(newStatus);
}

void BoosterGroup::writeEEPROM(int8_t nodeNr, bool writeEE)
{
  uint8_t * wrPtr = &bConfig.numMods;
  uint16_t baseAddr = 0;
  uint8_t chkSum = 0;
  for (uint8_t i = 0; i <  sizeof(bConfig); i++)
  {
    EEPROM.update(baseAddr + 1 + i, (*wrPtr));
    bConfig.checkByte ^= (*wrPtr);
    wrPtr++;
  }
  for (uint8_t i = 0; i < numDevs; i++)
    if ((i == nodeNr) || (nodeNr < 0))
      bData[i].writeEEPROM(writeEE);
}

boosterConfigData BoosterGroup::readEEPROM()
{
  boosterConfigData readData;
  uint8_t * rdPtr = &readData.numMods;
  uint16_t baseAddr = 0;
  uint8_t chkSum = 0;
  for (uint8_t i = 0; i <  sizeof(readData); i++)
  {
    EEPROM.get(baseAddr + 1 + i, (*rdPtr));
    chkSum ^= (*rdPtr);
    rdPtr++;
  }
  if (chkSum != readData.checkByte)
    readData.numMods = 0;
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
  
  if (devIndex < numDevs) //if the booster is defined, we process it
  {
    Booster* thisBooster = &bData[devIndex];
    thisBooster->processTimerInterrupt();
  }
  devIndex = (devIndex + 1) % (maxPins+1); //move ptr to next
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
    bool isDCC = activeA && activeB && diffSig; //2 input DCC only
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
      for (uint8_t i = 0; i < numDevs; i++)
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
  for (uint8_t i = 0; i < maxPins; i++)
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
      sprintf(myMqttMsg, "{\"Cmd\":\"SVR\",\"Prm\":{\"Src\":%i,\"Opc\":%i,\"Vals\":[%i,%i,%i,%i]}}\n", bConfig.lnAddress, 0x47, manufID, devID, prodID, bConfig.serNr);
      Serial.print(myMqttMsg);
      return true;
    }
    if (reportFlags & 0x02) //
    {
      reportFlags &= ~0x02;
      sprintf(myMqttMsg, "{\"Cmd\":\"SVR\",\"Prm\":{\"Src\":%i,\"Opc\":%i,\"Vals\":[%i,%i,%i,%i]}}\n", bConfig.lnAddress, 0x48, manufID, devID, prodID, bConfig.serNr);
      Serial.print(myMqttMsg);
      return true;
    }
    if (reportFlags & 0x04) //set/get number of modules, LN Mode, LN Addr
    {
      reportFlags &= ~0x04;
      sprintf(myMqttMsg, "{\"Cmd\":\"SVR\",\"Prm\":{\"Src\":%i,\"Opc\":%i,\"Addr\":%i,\"Vals\":[%i,%i,%i,%i]}}\n", bConfig.lnAddress, 0x46, 0x00, numDevs, bConfig.useLN, ((bConfig.lnAddress & 0xFF00)>>8), (bConfig.lnAddress & 0x00FF));
      Serial.print(myMqttMsg);
      return true;
    }
    if (reportFlags & 0x08) //set/get power on mode, accepted signal type
    {
      reportFlags &= ~0x08;
      sprintf(myMqttMsg, "{\"Cmd\":\"SVR\",\"Prm\":{\"Src\":%i,\"Opc\":%i,\"Addr\":%i,\"Vals\":[%i]}}\n", bConfig.lnAddress, 0x42, 0x04, bConfig.devMode);
      Serial.print(myMqttMsg);
      return true;
    } 
    if (reportFlags & 0x10) //LN Actuator On type, state, addr
    {
      reportFlags &= ~0x10;
      sprintf(myMqttMsg, "{\"Cmd\":\"SVR\",\"Prm\":{\"Src\":%i,\"Opc\":%i,\"Addr\":%i,\"Vals\":[%i,%i,%i,%i]}}\n", bConfig.lnAddress, 0x46, 0x08, bConfig.globalOn.trigDef, (bConfig.globalOn.devAddr >> 8),(bConfig.globalOn.devAddr & 0x00FF),0);
      Serial.print(myMqttMsg);
      return true;
    }
    if (reportFlags & 0x20) //LN Actuator Off type, state, addr
    {
      reportFlags &= ~0x20;
      sprintf(myMqttMsg, "{\"Cmd\":\"SVR\",\"Prm\":{\"Src\":%i,\"Opc\":%i,\"Addr\":%i,\"Vals\":[%i,%i,%i,%i]}}\n", bConfig.lnAddress, 0x46, 0x0C, bConfig.globalOff.trigDef, (bConfig.globalOff.devAddr >> 8),(bConfig.globalOff.devAddr & 0x00FF),0);
      Serial.print(myMqttMsg);
      return true;
    }
    if (reportFlags & 0x0100) //report signal status
    {
      reportFlags &= ~0x0100;
      sprintf(myMqttMsg, "{\"Cmd\":\"SVR\",\"Prm\":{\"Src\":%i,\"Opc\":%i,\"Addr\":%i,\"Vals\":[%i]}}\n", bConfig.lnAddress, 0x42, 0xF0, sigStatus);
      Serial.print(myMqttMsg);
      return true;
    }
    if (reportFlags & 0x0200) //get current global power stauts
    {
      reportFlags &= ~0x0200;
      sprintf(myMqttMsg, "{\"Cmd\":\"SVR\",\"Prm\":{\"Src\":%i,\"Opc\":%i,\"Addr\":%i,\"Vals\":[%i]}}\n", bConfig.lnAddress, 0x42, 0xF1, pwrStatus);
      Serial.print(myMqttMsg);
      return true;
    }
    for (uint8_t i = 0; i < numDevs; i++)
      if (bData[i].processReportRequest())
        return true;

    return false;
  }
  else
    return false;
}

void BoosterGroup::processExtCommand(int8_t boosterNr, lnActivatorDef newCmd)
{
  if (boosterNr & 0x20)
  {
    if ((newCmd.trigDef == bConfig.globalOn.trigDef) && (newCmd.devAddr == bConfig.globalOn.devAddr)) 
      setExtStatus(0x3F, extRun);
    if ((newCmd.trigDef == bConfig.globalOff.trigDef) && (newCmd.devAddr == bConfig.globalOff.devAddr)) 
      setExtStatus(0x3F, extStop);
  }
  if (boosterNr & 0x10)
    for (uint8_t i = 0; i < numDevs; i++)
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
  if (ofIndex < numDevs)
    return &bData[ofIndex];
  else
    return NULL;
}

/*
void BoosterGroup::requestBoosterReport(int8_t boosterNr, uint16_t reportType); //set report flags
{
  DynamicJsonDocument doc(100);
  char myMqttMsg[100];
  doc["Cmd"] = "B";
//  doc["Stat"] = currStatus == 0? "OFF" : "ON";
  doc["Sig"] = sigStatus;
  serializeJson(doc, myMqttMsg);
  Serial.println(String(myMqttMsg));
}
*/
