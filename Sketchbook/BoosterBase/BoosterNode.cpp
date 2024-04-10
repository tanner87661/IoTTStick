#include "BoosterNode.h"

//char myMqttMsg[80];

Booster::Booster()
{
}

Booster::~Booster()
{
}

void Booster::initNode(uint8_t nNr, uint16_t lnAddr) //, nodeConfigData* initData)
{
//  Serial.print("Init Booster ");
//  Serial.println(nNr);
  nodeNr = nNr;
  setLNAddr(lnAddr);
  bCfg = readEEPROM(); //(*initData);
  if (bCfg.checkByte == 0) 
  {
    Serial.println("Invalid Booster Node Data");
    return;
  }
  updateNominalTemp();
  configPins();
  tripLine(false);
  bOps.currCtr = random(numSamples);
  bOps.tripCurrVal = round(fuseTripPoint * bCfg.currNominal / bCfg.senseFactor); //trip current = 135% of nominal current, in analog port value
//  Serial.print("Nominal Temp: ");
//  Serial.print(bCfg.fuseMode);
//  Serial.print(" ");
//  Serial.print(bCfg.currNominal);
//  Serial.print(" ");
//  Serial.println(bOps.nominalTemp);
//  Serial.println(bOps.tripCurrVal);
  bOps.intStat = allok;
  bOps.extStat = extStop;
  bOps.sigStat = noSig;
  bOps.scTrigVal = max(1, trunc(bCfg.fuseMode/10)); //req
  
//  Serial.println(bCfg.currNominal);
//  Serial.println(bOps.tripCurrVal);
//  Serial.println(bOps.nominalTemp);
//  Serial.println(bOps.scTrigVal);
}

void Booster::updateNominalTemp()
{
  bOps.nominalTemp = round(bCfg.fuseMode * (bCfg.currNominal / 10)); //centigrade
  bOps.currTemp = 0;
}

void Booster::configPins()
{
  if (bCfg.ctrlPin > 3) //pins 0,1,2,3 reserved for Serial and DCC
    pinMode(bCfg.ctrlPin, OUTPUT);  
  if (bCfg.reversePin > 3) //pins 0,1,2,3 reserved for Serial and DCC
    pinMode(bCfg.reversePin, OUTPUT);  
}

void Booster::releasePins()
{
  if (bCfg.ctrlPin > 3) //pins 0,1,2,3 reserved for Serial and DCC
    pinMode(bCfg.ctrlPin, INPUT);  
  if (bCfg.reversePin > 3) //pins 0,1,2,3 reserved for Serial and DCC
    pinMode(bCfg.reversePin, INPUT);  
}

//uint8_t Booster::getTripStatus()
//{
//  return bOps.tripFlag;
//}

void Booster::setLNAddr(uint16_t lnAddr)
{
  lnAddress = lnAddr;
}

void Booster::tripLine(bool newStat)
{
//  Serial.print(bCfg.ctrlPin);
//  Serial.print(" ");
//  Serial.println(newStat);
  digitalWrite(bCfg.ctrlPin, newStat);
  if (!newStat)
  {
    bOps.scTimer = millis();
    bOps.scCtr = 0;
  }
}

uint8_t Booster::getOutputStatus()
{
  return digitalRead(bCfg.ctrlPin);
}

void Booster::updateRMS_T(uint8_t currBuffer)
{
  float tmpRes = sqrt(bOps.currData[currBuffer] / numSamples) * 4 * bCfg.senseFactor;
  uint16_t addTemp = round(sq(tmpRes/10)/(bCfg.currNominal/10));
  uint16_t decrTemp = max(10, round(bOps.currTemp/bCfg.fuseMode));
  bOps.currTemp = bOps.currTemp + addTemp;
  if (bOps.currTemp > decrTemp)
    bOps.currTemp -= decrTemp;
  else
    bOps.currTemp = 0;
  bOps.currOLF = round(100 * ((float)bOps.currTemp / (float)bOps.nominalTemp));
  bOps.currRMS = round(tmpRes);

//  Serial.print("Nom: ");
//  Serial.print(bOps.nominalTemp);
//  Serial.print(" add: ");
//  Serial.print(addTemp);
//  Serial.print(" decr: ");
//  Serial.print(decrTemp);
//  Serial.print(" Curr: ");
//  Serial.println(bOps.currTemp);
  
}

void Booster::setARPolarity(uint8_t newPolarity) //0: forward 1: reverse; 2: toggle
{
//  Serial.print("AR Polarity Mod ");
//  Serial.print(nodeNr);
//  Serial.print(" pos ");
//  Serial.println(newPolarity);
//  if (bCfg.autoReverseMode)
  if (bCfg.reversePin > 1)
  {
    if (newPolarity < 2)
      digitalWrite(bCfg.reversePin, newPolarity);
    else
      digitalWrite(bCfg.reversePin, !digitalRead(bCfg.reversePin));
  }
}

void Booster::setExtStatus(extNodeStatus newStatus)
{
//  Serial.println("Ext Stat ");
//  Serial.print(nodeNr);
//  Serial.print(" pos ");
//  Serial.println(newStatus);
  bOps.extStat = newStatus;
  bOps.intStat = allok;
  tripLine((bOps.extStat == extRun) && (bOps.extStat != noSig));
}

void Booster::setSignalStatus(extSignalStatus newStatus)
{
  bOps.sigStat = newStatus;
  tripLine((bOps.extStat == extRun) && (bOps.extStat != noSig));
}

bool Booster::processReportRequest()
{
  char myMqttMsg[50];
  if (reportFlags & 0x01) //pins PWR, Analog, AR 3 nibbles pin nr, Sensor Rate 2 bytes (*100)
  {
    reportFlags &= ~0x01;
    uint16_t sensVal = round(bCfg.senseFactor * 100);
//    Serial.println(bCfg.ctrlPin);
//    Serial.println(bCfg.sensePin);
    sprintf(myMqttMsg, quadByteResponse, lnAddress, 0x46, ((nodeNr+1) << 8) + 0x00, (bCfg.ctrlPin << 4) + (bCfg.sensePin-A0), bCfg.reversePin, sensVal >> 8, sensVal & 0x00FF);
    Serial.print(myMqttMsg);
//    Serial.println(sensVal);
    return true;
  }
  if (reportFlags & 0x02) //AR 1bit, Reset Mode 2bits, Fuse Value 1byte, nominal Current 2byte
  {
    reportFlags &= ~0x02;
//    Serial.println(bCfg.autoReverseMode);
    sprintf(myMqttMsg, quadByteResponse, lnAddress, 0x46, ((nodeNr+1) << 8) + 0x04, (bCfg.autoReverseMode << 2) + (bCfg.autoResetMode & 0x03), bCfg.fuseMode, bCfg.currNominal >> 8, bCfg.currNominal & 0x00FF);
    Serial.print(myMqttMsg);
    return true;
  }
  if (reportFlags & 0x04) //LN Actuator On type, state, addr
  {
    reportFlags &= ~0x04;
    sprintf(myMqttMsg, quadByteResponse, lnAddress, 0x46, ((nodeNr+1) << 8) + 0x08, bCfg.actSetOn.trigDef, (bCfg.actSetOn.devAddr >> 8),(bCfg.actSetOn.devAddr & 0x00FF),0);
    Serial.print(myMqttMsg);
    return true;
  }
  if (reportFlags & 0x08) //LN Actuator Off type, state, addr
  {
    reportFlags &= ~0x08;
    sprintf(myMqttMsg, quadByteResponse, lnAddress, 0x46, ((nodeNr+1) << 8) + 0x0C, bCfg.actSetOff.trigDef, (bCfg.actSetOff.devAddr >> 8),(bCfg.actSetOff.devAddr & 0x00FF),0);
    Serial.print(myMqttMsg);
    return true;
  }
  if (reportFlags & 0x10) //LN Actuator Reset type, state, addr
  {
    reportFlags &= ~0x10;
    sprintf(myMqttMsg, quadByteResponse, lnAddress, 0x46, ((nodeNr+1) << 8) + 0x10, bCfg.actResetNode.trigDef, (bCfg.actResetNode.devAddr >> 8),(bCfg.actResetNode.devAddr & 0x00FF),0);
    Serial.print(myMqttMsg);
    return true;
  }
  if (reportFlags & 0x20) //LN Actuator AR Pos type, state, addr            
  {
    reportFlags &= ~0x20;
    sprintf(myMqttMsg, quadByteResponse, lnAddress, 0x46, ((nodeNr+1) << 8) + 0x14, bCfg.actARPolarityPos.trigDef, (bCfg.actARPolarityPos.devAddr >> 8),(bCfg.actARPolarityPos.devAddr & 0x00FF),0);
    Serial.print(myMqttMsg);
    return true;
  }
  if (reportFlags & 0x40) //LN Actuator AR Neg type, state, addr
  {
    reportFlags &= ~0x40;
    sprintf(myMqttMsg, quadByteResponse, lnAddress, 0x46, ((nodeNr+1) << 8) + 0x18, bCfg.actARPolarityNeg.trigDef, (bCfg.actARPolarityNeg.devAddr >> 8),(bCfg.actARPolarityNeg.devAddr & 0x00FF),0);
    Serial.print(myMqttMsg);
    return true;
  }
  if (reportFlags & 0x80) //LN Actuator Short circuit type, state, addr
  {
    reportFlags &= ~0x80;
    sprintf(myMqttMsg, quadByteResponse, lnAddress, 0x46, ((nodeNr+1) << 8) + 0x1C, bCfg.repShortCircuit.trigDef, (bCfg.repShortCircuit.devAddr >> 8),(bCfg.repShortCircuit.devAddr & 0x00FF),0);
    Serial.print(myMqttMsg);
    return true;
  }
  if (reportFlags & 0x100) //LN Actuator Overload type, state, addr
  {
    reportFlags &= ~0x100;
    sprintf(myMqttMsg, quadByteResponse, lnAddress, 0x46, ((nodeNr+1) << 8) + 0x20, bCfg.repOverload.trigDef, (bCfg.repOverload.devAddr >> 8),(bCfg.repOverload.devAddr & 0x00FF),0);
    Serial.print(myMqttMsg);
    return true;
  }
  if (reportFlags & 0x200) //Short circuit current
  {
    reportFlags &= ~0x200;
    sprintf(myMqttMsg, quadByteResponse, lnAddress, 0x46, ((nodeNr+1) << 8) + 0x24, bCfg.scCurr >> 8, bCfg.scCurr & 0x00FF,0, 0);
    Serial.print(myMqttMsg);
    return true;
  }

  if (bOps.rdPtr != bOps.wrPtr) //ready to read data
  {  
    updateRMS_T(bOps.rdPtr);
    if ((bOps.lastCurrRead + readIntv) < millis()) //ready to read data
    {
      bool sendIntStat = ((bCfg.repShortCircuit.trigDef & 0xE0) > 0) && (bOps.intStat != intStatTx);
      bool sendOLStat = ((bCfg.repOverload.trigDef & 0xE0) > 0) && (bOps.currOLF != olfTx);
      uint16_t OLFVal = round(bOps.currOLF);
      uint16_t TVal = round(bOps.currTemp/100);
      sprintf(myMqttMsg, DCCAmpResponse, nodeNr, OLFVal, TVal, bOps.currRMS, olfTx, bOps.intStat, bOps.extStat, digitalRead(bCfg.reversePin));
      Serial.print(myMqttMsg);
      if (sendIntStat)
      {
        sendIntStatLN();
        intStatTx = bOps.intStat;
      }
      else
        if (sendOLStat)
        {
          sendOLFStatLN();
          olfTx = bOps.currOLF;
        }
      bOps.lastCurrRead += readIntv;
    }
    bOps.rdPtr = (bOps.rdPtr + 1) % smplBufSize;
    if (bOps.currOLF > olfThreshold)
    {
      tripLine(false);
      bOps.intStat = olCool;
    }
    return true;
  }
  return false;
}

bool Booster::processLoop()
{
//  processReportRequest();
  return false;
}

void Booster::restartNode()
{
  bOps.intStat = allok;
  bOps.extStat = extRun;
  tripLine(true);
}

void Booster::resetBooster()
{
//  Serial.print("Restart Node ");
//  Serial.println(nodeNr);
  if (bOps.intStat == awReset)
    restartNode();
}


void Booster::requestSVData(uint8_t svCmd, int16_t memLoc)
{
  switch (memLoc)
  {
    case 0x00: reportFlags |= 0x01; break;
    case 0x04: reportFlags |= 0x02; break;
    case 0x08: reportFlags |= 0x04; break;
    case 0x0C: reportFlags |= 0x08; break;
    case 0x10: reportFlags |= 0x10; break;
    case 0x14: reportFlags |= 0x20; break;
    case 0x18: reportFlags |= 0x40; break;
    case 0x1C: reportFlags |= 0x80; break;
    case 0x20: reportFlags |= 0x100; break;
    case 0x24: reportFlags |= 0x200; break;
    case 0xF0: break;
    case 0xF1: break;
    case 0xF2: break;
    case 0xF3: break;
    case 0xFF: reportFlags = 0xFFFF; break;
  }
}

void Booster::writeSVData(uint8_t svCmd, int16_t memLoc, uint8_t svData[]) //write SV data
{
  //write memLoc
//  Serial.println(memLoc);
  switch (memLoc)
  {
    case 0x00: 
    {
      releasePins();
      bCfg.ctrlPin = svData[0] >> 4;
      bCfg.sensePin = (svData[0] & 0x0F) + A0;
      bCfg.reversePin = svData[1] & 0x0F;
      float sensVal = (float)((svData[2] << 8) + svData[3]) / 100;
      setSensorFactor(sensVal);
      configPins();
    }
      break;
    case 0x04: 
      bCfg.autoReverseMode = svData[0] >> 2;
      bCfg.autoResetMode = svData[0] & 0x03;
      bCfg.fuseMode = svData[1];
      bCfg.currNominal = (svData[2] << 8) + svData[3];
      updateNominalTemp();
//    Serial.println(bCfg.autoReverseMode);
      break;
    case 0x08: 
      bCfg.actSetOn.trigDef = svData[0];
      bCfg.actSetOn.devAddr = (svData[1]<<8) + svData[2];
      break;
    case 0x0C: 
      bCfg.actSetOff.trigDef = svData[0];
      bCfg.actSetOff.devAddr = (svData[1]<<8) + svData[2];
      break;
    case 0x10: 
      bCfg.actResetNode.trigDef = svData[0];
      bCfg.actResetNode.devAddr = (svData[1]<<8) + svData[2];
      break;
    case 0x14: 
      bCfg.actARPolarityPos.trigDef = svData[0];
      bCfg.actARPolarityPos.devAddr = (svData[1]<<8) + svData[2];
      break;
    case 0x18: 
      bCfg.actARPolarityNeg.trigDef = svData[0];
      bCfg.actARPolarityNeg.devAddr = (svData[1]<<8) + svData[2];
      break;
    case 0x1C: 
      bCfg.repShortCircuit.trigDef = svData[0];
      bCfg.repShortCircuit.devAddr = (svData[1]<<8) + svData[2];
      break;
    case 0x20: 
      bCfg.repOverload.trigDef = svData[0];
      bCfg.repOverload.devAddr = (svData[1]<<8) + svData[2];
      break;
    case 0xF0: 
      if (svData[0] == 0xFF) //toggle
        if (bOps.extStat == extStop)
          setExtStatus(extRun);
        else
          setExtStatus(extStop);
      else
        setExtStatus(svData[0]);
      break;
    case 0xF1: 
      setARPolarity(svData[0]);
      break;
    case 0xF2: 
      resetBooster();
      break;
    case 0xF3: 
      startSCTest();
      break;
  }
  requestSVData(svCmd, memLoc);
}

void Booster::setSensorFactor(double newSensFact)
{
  bCfg.senseFactor = newSensFact;
//  Serial.println(bCfg.senseFactor);
  reportFlags |= 0x01;
}

void Booster::setNominalCurrent(uint16_t newCurrent)
{
  bCfg.currNominal = newCurrent;
  reportFlags |= 0x02;
}

void Booster::setFuseValue(uint8_t newFuseMode)
{
  bCfg.fuseMode = newFuseMode;
  reportFlags |= 0x02;
}

void Booster::setResetMode(uint8_t newResetMode)
{
  bCfg.autoResetMode = newResetMode;
  reportFlags |= 0x02;
}

void Booster::setARMode(bool newARMode)
{
  bCfg.autoReverseMode = newARMode;
  reportFlags |= 0x02;
}

void Booster::writeEEPROM(bool writeEE)
{
  uint8_t * wrPtr = &bCfg.ctrlPin;
  uint16_t baseAddr = (128 * (nodeNr+1)) + 1;
  uint8_t recSize = sizeof(bCfg);
  bCfg.checkByte = 0;
  for (uint8_t i = 0; i <  recSize-1; i++)
  {
    bCfg.checkByte ^= (*wrPtr);
    wrPtr++;
  }
  EEPROM.put(baseAddr, bCfg);
}

nodeConfigData Booster::readEEPROM()
{
  nodeConfigData readData;
  uint8_t * rdPtr = &readData.ctrlPin;
  uint16_t baseAddr = (128 * (nodeNr+1)) + 1;
  uint8_t chkSum = 0;
  uint8_t recSize = sizeof(readData);
  EEPROM.get(baseAddr, readData);
  for (uint8_t i = 0; i < recSize-1;  i++)
  {
    chkSum ^= (*rdPtr);
//    Serial.print((*rdPtr));
//    Serial.print(", ");
    rdPtr++;
  }
//  Serial.println(chkSum);
  if (chkSum != readData.checkByte)
  {
//    Serial.println("EE invalid");
    readData.ctrlPin = 0;
    readData.sensePin = A0; //analog pins starting with 14
    readData.reversePin = 0;
    readData.senseFactor = 0; //Current Factor: 8500 A/A, Rsense = 5kOhm Umax at 8.5 Amps = 5V = Analog Value 1023 Sense Factor = 8500 / 1023 = 8.31
    readData.currNominal = 0; //[mA] set trip current, must be smaller than achievable short circuit current. 80% of this is nominal current (infinite RMS load) if higher, warmup is calculated
    readData.fuseMode = 30; //10,20,30,..,100
    readData.autoResetMode = 2; //0: manual only 1: limited 2: full autoreset
    readData.autoReverseMode = false;
    readData.actSetOn.trigDef = 0;
    readData.actSetOff.trigDef = 0;
    readData.actResetNode.trigDef = 0;
    readData.actARPolarityPos.trigDef = 0;
    readData.actARPolarityNeg.trigDef = 0;
    readData.repShortCircuit.trigDef = 0;
    readData.repOverload.trigDef = 0;
    readData.checkByte = 0;
  }
//  else
//      Serial.println("EE ok");
  return readData;
}

void Booster::processTimerInterrupt()
{
  switch (bOps.extStat)
  {
    case extStop: processTimerOps(); break; //booster is external off
    case extRun: processTimerOps(); break;
    case extScDetect: processTimerSCDetect(); break;
    default: break;
  }
}

void Booster::processTimerOps()
{
  //current Measurement, independent of status
  if (bOps.currCtr > numSamples)
  {
    bOps.wrPtr = (bOps.wrPtr + 1) % smplBufSize;
    bOps.currData[bOps.wrPtr] = 0;
    bOps.currCtr = 0;
  }
  uint16_t currVal = analogRead(bCfg.sensePin);
  bOps.currData[bOps.wrPtr] += (double)sq(currVal >> 2);
  bOps.currCtr++;
  bool isShort = currVal > bOps.tripCurrVal; 
  switch (bOps.intStat)
  {
    case allok: 
    {
      if (isShort) 
      {
        if (bCfg.autoReverseMode)
        {
          setARPolarity(0xFF);
          bOps.intStat = scAR0;
//          Serial.print(currVal);
//          Serial.print(" ");
//          Serial.println("A");
        }
        else
        {
          bOps.scCtr++;
          if (bOps.scCtr > bOps.scTrigVal)
          {
            tripLine(false);
            bOps.intStat = scOff;
//            Serial.print(currVal);
//            Serial.print(" ");
//            Serial.println("S");
          }
          
        }
      }
      else
        bOps.scWaitTime = 500;
    }
    break;
    case scAR0:
    {
      if (isShort) 
      {
        bOps.scCtr++;
        if (bOps.scCtr > bOps.scTrigVal)
        {
          tripLine(false);
          bOps.intStat = scOff;
//            Serial.print(currVal);
//            Serial.print(" ");
//          Serial.println("X");
        }
      }
      else
      {
        bOps.intStat = allok;
//            Serial.print(bOps.scCtr);
//            Serial.print(" ");
//          Serial.println("Z");
      }
    break;
    case scOff: 
    {
      switch (bCfg.autoResetMode)
      {
        case 0: //manual reset only
        {
          bOps.intStat = awReset;
        }
        break;
        case 1: //limited autoreset only, 3 times, 0.5sec distance
        {
          if (millis() - bOps.scTimer > limWaitTime)
          {
            bOps.scWaitTime += 500;
            if (bOps.scWaitTime < (500 * limRetry))
            {
              bOps.intStat = allok;
              tripLine(true);
            }
            else
              bOps.intStat = awReset; //manual reset expected
          }
        }
        break;
        case 2: //full auto reset, progressive delay up to 10 secs
        {
          if (millis() - bOps.scTimer > bOps.scWaitTime)
          {
            if (bOps.scWaitTime < 10000)
              bOps.scWaitTime += 500;
            bOps.intStat = allok;
            tripLine(true);
          }
        }
        break;
      }
    }
    break;
    case olOff: 
    break; 
    case olCool: 
    {
//      Serial.println(bOps.currOLF);
      if (bOps.currOLF < olfRestoreThreshold)
      {
        if (bCfg.autoResetMode > 0)
        {
          bOps.intStat = allok;
          tripLine(true);    
        }
        else    
          bOps.intStat = awReset;
      }
    }
    break;
    case awReset: //await reset 
    break;
  }
//  Serial.println(bOps.tripCurrVal);
//  if ((bOps.tripCurrVal > bOps.tripThreshold) || (bOps.overloadValue > 0)) 
//  {
//    int16_t netOL = bOps.tripCurrVal - bOps.tripThreshold;
//    bOps.overloadValue = max(bOps.overloadValue + netOL, 0);
//  }

  }
}

void Booster::startSCTest()
{
  if (bOps.extStat == extStop) //only start if booster is off
  {
    bOps.scCtr = 0;
    bOps.scVal = 0;
    bOps.extStat = extScDetect;
  }
  else
    setExtStatus(extStop);
}

void Booster::processTimerSCDetect()
{
  if (bOps.scCtr < scTestSamples)
  {
    if (getOutputStatus()== 0)
      tripLine(true);
    else
    {
      bOps.scVal += analogRead(bCfg.sensePin);
      tripLine(false);
      bOps.scCtr++;
    }
  }
  else
  {
    bCfg.scCurr = round((float)bOps.scVal * bCfg.senseFactor / (float)bOps.scCtr);
    reportFlags |= 0x200;
    bOps.extStat = extStop; 
  } 
}

void Booster::processExtCommand(lnActivatorDef newCmd)
{
  if ((newCmd.trigDef == bCfg.actSetOn.trigDef) && (newCmd.devAddr == bCfg.actSetOn.devAddr)) 
    setExtStatus(extRun);
  if ((newCmd.trigDef == bCfg.actSetOff.trigDef) && (newCmd.devAddr == bCfg.actSetOff.devAddr)) 
    setExtStatus(extStop);
  if ((newCmd.trigDef == bCfg.actResetNode.trigDef) && (newCmd.devAddr == bCfg.actResetNode.devAddr)) 
    resetBooster();
  if ((newCmd.trigDef == bCfg.actARPolarityPos.trigDef) && (newCmd.devAddr == bCfg.actARPolarityPos.devAddr)) 
    setARPolarity(0);
  if ((newCmd.trigDef == bCfg.actARPolarityNeg.trigDef) && (newCmd.devAddr == bCfg.actARPolarityNeg.devAddr)) 
    setARPolarity(1);
}
  
  void Booster::sendLNMsg(uint8_t opCode, uint16_t lnAddr, uint8_t lnStatus)
  {
    char myMqttMsg[50];
    sprintf(myMqttMsg, LNResponse, opCode, lnAddr, lnStatus); 
    Serial.print(myMqttMsg);
  }

  void Booster::sendIntStatLN()
  {
    switch (bCfg.repOverload.trigDef >> 5)
    {
      case 1: sendLNMsg(0xB0, bCfg.repOverload.devAddr, bOps.intStat < 3 ? bCfg.repShortCircuit.trigDef & 0x01 : (bCfg.repShortCircuit.trigDef & 0x01) ^ 0x01); break;
      case 2: sendLNMsg(0xED, bCfg.repOverload.devAddr, bOps.intStat); break;
      case 3: sendLNMsg(0xE5, bCfg.repOverload.devAddr, bOps.intStat < 3 ? bCfg.repShortCircuit.trigDef & 0x01 : (bCfg.repShortCircuit.trigDef & 0x01) ^ 0x01); break;
      case 4: sendLNMsg(0xB2, bCfg.repOverload.devAddr, bOps.intStat < 3 ? bCfg.repShortCircuit.trigDef & 0x01 : (bCfg.repShortCircuit.trigDef & 0x01) ^ 0x01); break;
    }
  }

  void Booster::sendOLFStatLN()
  {
    switch (bCfg.repOverload.trigDef >> 5)
    {
      case 1: sendLNMsg(0xB0, bCfg.repOverload.devAddr, bOps.currOLF > 100 ? bCfg.repOverload.trigDef & 0x01 : (bCfg.repOverload.trigDef & 0x01) ^ 0x01); break;
      case 2: sendLNMsg(0xED, bCfg.repOverload.devAddr, round(bOps.currOLF / 4)); break;
      case 3: sendLNMsg(0xE5, bCfg.repOverload.devAddr, bOps.currOLF > 100 ? bCfg.repOverload.trigDef & 0x01 : (bCfg.repOverload.trigDef & 0x01) ^ 0x01); break;
      case 4: sendLNMsg(0xB2, bCfg.repOverload.devAddr, bOps.currOLF > 100 ? bCfg.repOverload.trigDef & 0x01 : (bCfg.repOverload.trigDef & 0x01) ^ 0x01); break;
    }
  }
