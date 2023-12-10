#include "BoosterNode.h"

Booster::Booster()
{
}

Booster::~Booster()
{
}

void Booster::initNode(uint8_t nNr, nodeConfigData* initData)
{
  bCfg = (*initData);
  nodeNr = nNr;
  pinMode(bCfg.ctrlPin, OUTPUT);
  tripLine(false);
  bOps.currCtr = random(numSamples);
  bOps.tripCurrVal = round(fuseTripPoint * bCfg.currNominal / bCfg.senseFactor); //trip current = 120% of nominal current, in analog port value
  bOps.nominalTemp = (float)bCfg.fuseMode * ((float)bCfg.currNominal / 1000);
  bOps.intStat = allok;
  bOps.extStat = extStop;
  bOps.sigStat = noSig;
  bOps.scTrigVal = max(1, trunc(bCfg.fuseMode/10)); //req
  if (bCfg.autoReverseMode)
    pinMode(bCfg.reversePin, OUTPUT);
  
//  Serial.println(bCfg.currNominal);
//  Serial.println(bOps.tripCurrVal);
//  Serial.println(bOps.nominalTemp);
//  Serial.println(bOps.scTrigVal);
}

//uint8_t Booster::getTripStatus()
//{
//  return bOps.tripFlag;
//}

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
  bOps.currRMS = sqrt(bOps.currData[currBuffer] / numSamples) * 4 * bCfg.senseFactor;
  bOps.currTemp = bOps.currTemp + (bOps.currRMS)/1000 - (bOps.currTemp/bCfg.fuseMode);
  bOps.currOLF = (double)round(100 * bOps.currTemp / bOps.nominalTemp)/100;
  bOps.currRMS = (double)round(100 * bOps.currRMS)/100;
  bOps.currTemp = (double)round(100 * bOps.currTemp)/100;
}

void Booster::setARPolarity(uint8_t newPolarity) //0: forward 1: reverse; 2: toggle
{
  if (bCfg.autoReverseMode)
  {
    if (newPolarity < 2)
      digitalWrite(bCfg.reversePin, newPolarity);
    else
      digitalWrite(bCfg.reversePin, !digitalRead(bCfg.reversePin));
  }
}

void Booster::setExtStatus(extNodeStatus newStatus)
{
//  Serial.println("Ext Stat");
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
    sprintf(myMqttMsg, "{\"Cmd\":\"SVR\",\"Prm\":{\"Src\":%i,\"Opc\":%i,\"Addr\":%i,\"Vals\":[%i,%i,%i,%i]}}\n", bCfg.lnAddress, 0x46, ((nodeNr+1) << 8) + 0x00, (bCfg.ctrlPin << 4) + (bCfg.sensePin-14), bCfg.reversePin, sensVal >> 8, sensVal & 0x00FF);
    Serial.print(myMqttMsg);
    return true;
  }
  if (reportFlags & 0x02) //AR 1bit, Reset Mode 2bits, Fuse Value 1byte, nominal Current 2byte
  {
    reportFlags &= ~0x02;
    sprintf(myMqttMsg, "{\"Cmd\":\"SVR\",\"Prm\":{\"Src\":%i,\"Opc\":%i,\"Addr\":%i,\"Vals\":[%i,%i,%i,%i]}}\n", bCfg.lnAddress, 0x46, ((nodeNr+1) << 8) + 0x04, (bCfg.autoReverseMode << 2) + (bCfg.autoResetMode & 0x03), bCfg.fuseMode, bCfg.currNominal >> 8, bCfg.currNominal & 0x00FF);
    Serial.print(myMqttMsg);
    return true;
  }
  if (reportFlags & 0x04) //LN Actuator On type, state, addr
  {
    reportFlags &= ~0x04;
    sprintf(myMqttMsg, "{\"Cmd\":\"SVR\",\"Prm\":{\"Src\":%i,\"Opc\":%i,\"Addr\":%i,\"Vals\":[%i,%i,%i,%i]}}\n", bCfg.lnAddress, 0x46, ((nodeNr+1) << 8) + 0x08, bCfg.actSetOn.trigDef, (bCfg.actSetOn.devAddr >> 8),(bCfg.actSetOn.devAddr & 0x00FF),0);
    Serial.print(myMqttMsg);
    return true;
  }
  if (reportFlags & 0x08) //LN Actuator Off type, state, addr
  {
    reportFlags &= ~0x08;
    sprintf(myMqttMsg, "{\"Cmd\":\"SVR\",\"Prm\":{\"Src\":%i,\"Opc\":%i,\"Addr\":%i,\"Vals\":[%i,%i,%i,%i]}}\n", bCfg.lnAddress, 0x46, ((nodeNr+1) << 8) + 0x0C, bCfg.actSetOff.trigDef, (bCfg.actSetOff.devAddr >> 8),(bCfg.actSetOff.devAddr & 0x00FF),0);
    Serial.print(myMqttMsg);
    return true;
  }
  if (reportFlags & 0x10) //LN Actuator Reset type, state, addr
  {
    reportFlags &= ~0x10;
    sprintf(myMqttMsg, "{\"Cmd\":\"SVR\",\"Prm\":{\"Src\":%i,\"Opc\":%i,\"Addr\":%i,\"Vals\":[%i,%i,%i,%i]}}\n", bCfg.lnAddress, 0x46, ((nodeNr+1) << 8) + 0x10, bCfg.actResetNode.trigDef, (bCfg.actResetNode.devAddr >> 8),(bCfg.actResetNode.devAddr & 0x00FF),0);
    Serial.print(myMqttMsg);
    return true;
  }
  if (reportFlags & 0x20) //LN Actuator AR Pos type, state, addr            
  {
    reportFlags &= ~0x20;
    sprintf(myMqttMsg, "{\"Cmd\":\"SVR\",\"Prm\":{\"Src\":%i,\"Opc\":%i,\"Addr\":%i,\"Vals\":[%i,%i,%i,%i]}}\n", bCfg.lnAddress, 0x46, ((nodeNr+1) << 8) + 0x14, bCfg.actARPolarityPos.trigDef, (bCfg.actARPolarityPos.devAddr >> 8),(bCfg.actARPolarityPos.devAddr & 0x00FF),0);
    Serial.print(myMqttMsg);
    return true;
  }
  if (reportFlags & 0x40) //LN Actuator AR Neg type, state, addr
  {
    reportFlags &= ~0x40;
    sprintf(myMqttMsg, "{\"Cmd\":\"SVR\",\"Prm\":{\"Src\":%i,\"Opc\":%i,\"Addr\":%i,\"Vals\":[%i,%i,%i,%i]}}\n", bCfg.lnAddress, 0x46, ((nodeNr+1) << 8) + 0x18, bCfg.actARPolarityNeg.trigDef, (bCfg.actARPolarityNeg.devAddr >> 8),(bCfg.actARPolarityNeg.devAddr & 0x00FF),0);
    Serial.print(myMqttMsg);
    return true;
  }
  if (reportFlags & 0x80) //LN Actuator Short circuit type, state, addr
  {
    reportFlags &= ~0x80;
    sprintf(myMqttMsg, "{\"Cmd\":\"SVR\",\"Prm\":{\"Src\":%i,\"Opc\":%i,\"Addr\":%i,\"Vals\":[%i,%i,%i,%i]}}\n", bCfg.lnAddress, 0x46, ((nodeNr+1) << 8) + 0x1C, bCfg.repShortCircuit.trigDef, (bCfg.repShortCircuit.devAddr >> 8),(bCfg.repShortCircuit.devAddr & 0x00FF),0);
    Serial.print(myMqttMsg);
    return true;
  }
  if (reportFlags & 0x100) //LN Actuator Overload type, state, addr
  {
    reportFlags &= ~0x100;
    sprintf(myMqttMsg, "{\"Cmd\":\"SVR\",\"Prm\":{\"Src\":%i,\"Opc\":%i,\"Addr\":%i,\"Vals\":[%i,%i,%i,%i]}}\n", bCfg.lnAddress, 0x46, ((nodeNr+1) << 8) + 0x20, bCfg.repOverload.trigDef, (bCfg.repOverload.devAddr >> 8),(bCfg.repOverload.devAddr & 0x00FF),0);
    Serial.print(myMqttMsg);
    return true;
  }

  if (bOps.rdPtr != bOps.wrPtr) //ready to read data
  {  
    updateRMS_T(bOps.rdPtr);
    if ((bOps.lastCurrRead + readIntv) < millis()) //ready to read data
    {
      getBoosterOpsReport(this);
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
    
    case 0xFF: reportFlags = 0xFFFF; break;
  }
}

void Booster::writeSVData(uint8_t svCmd, int16_t memLoc, uint8_t svData[]) //write SV data
{
  //write memLoc
  switch (memLoc)
  {
    case 0x00: break;
    case 0x04: 
      bCfg.autoReverseMode = svData[0] >> 2;
      bCfg.autoResetMode = svData[0] & 0x03;
      bCfg.fuseMode = svData[1];
      bCfg.currNominal = (svData[2] << 8) + svData[3];
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
  }
  requestSVData(svCmd, memLoc);
}

void Booster::setSensorFactor(double newSensFact)
{
  bCfg.senseFactor = newSensFact;
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
  uint16_t baseAddr = 128 * (nodeNr+1);
  for (uint8_t i = 0; i <  sizeof(bCfg); i++)
  {
    EEPROM.update(baseAddr + 1 + i, (*wrPtr));
    bCfg.checkByte ^= (*wrPtr);
    wrPtr++;
  }
}

nodeConfigData Booster::readEEPROM()
{
  nodeConfigData eeData;
  uint8_t * rdPtr = &eeData.ctrlPin;
  uint16_t baseAddr = 128 * (nodeNr+1);
  uint8_t chkSum = 0;
  for (uint8_t i = 0; i <  sizeof(eeData); i++)
  {
    EEPROM.get(baseAddr + 1 + i, (*rdPtr));
    rdPtr++;
  }
  return eeData;
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
          setARPolarity(2);
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
    char myMqttMsg[50];
    sprintf(myMqttMsg, "{\"SCT\":[%i,%i,%i]}\n",nodeNr, bOps.scVal, round((float)bOps.scVal * bCfg.senseFactor / (float)bOps.scCtr));
    Serial.print(myMqttMsg);
    bOps.extStat = extStop; 
  } 
}

void Booster::processExtCommand(lnActivatorDef newCmd)
{
  if ((newCmd.trigDef == bCfg.actSetOn.trigDef) && (newCmd.devAddr == bCfg.actSetOn.devAddr)) 
    setExtStatus(extRun);
  if ((newCmd.trigDef == bCfg.actSetOff.trigDef) && (newCmd.devAddr == bCfg.actSetOff.devAddr)) 
    setExtStatus(extStop);
  if ((newCmd.trigDef == bCfg.actARPolarityPos.trigDef) && (newCmd.devAddr == bCfg.actARPolarityPos.devAddr)) 
    setARPolarity(0);
  if ((newCmd.trigDef == bCfg.actARPolarityNeg.trigDef) && (newCmd.devAddr == bCfg.actARPolarityNeg.devAddr)) 
    setARPolarity(1);
}
