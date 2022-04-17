#include "RHBoardManager.h"

void(* resetFunc) (void) = 0; //declare reset function @ address 0

BoardManager::BoardManager()
{
}

void BoardManager::intializeBoard()
{
  RHCtrlLEDs = new LEDChain;
  RHCtrlLEDs->begin();
  RHSensorBlock = new Sensor;
  RHSensorBlock->begin();
  initPins();
  DCC::begin(MOTOR_SHIELD_TYPE); 
  setBoardModeOff();
}

void BoardManager::processLoop()
{
  if (millis() - lastPwrCheck > pwrCheckInterval)
  {
    verifyPowerSignal(1);
    setLEDDispStatus();
  }
}

void BoardManager::setDeviceMode(uint8_t newMode) //0: Off 1: Cmd Stn 2: Booster with LN 3: Booster no LN 65535: Reboot 328P
{
  Serial.println(newMode);
  switch (newMode)
  {
    case 0: //OFF
      setBoardModeOff();
      break;
    case 1: // Cmd Stn
      setBoardModeCmdStn();
      break;
    case 2: //booster with LN
      setBoardModeBooster(true);
      break;
    case 3: //booster no LN
      setBoardModeBooster(false);
      break;
  }
}

void BoardManager::setDeviceOutput(uint8_t flagNr, uint8_t flagStatus)
{
  StringFormatter::send(Serial, F("Set device output %i %i\n"), flagNr, flagStatus);
  switch (flagNr)
  {
    case 0:
      setIBT2Enable(flagStatus > 0);
      break;
    case 1:
      setProgTrack(flagStatus > 0);
      break;
    case 2:
      setRailSyncEnable(flagStatus > 0);
      break;
    case 3:
      setDCCSource(flagStatus > 0); //1: INTERNAL 2: LOCONET
      break;
  }
}

void BoardManager::setOutputCurrent(uint8_t outMod, uint16_t newCurrent)
{
  StringFormatter::send(Serial, F("Set output current %i %i\n"), outMod, newCurrent);
  switch (outMod)
  {
    case 0: //IBT2
      break;
    case 1: //Prog Ack
      break;
    case 2: //RailSync
      break;
  }
}

/*
 * RedHat setup Codes starting at 1025
 * 1025 Device Mode: 0: Off 1: Cmd Stn 2: Booster with LocoNet 3: Booster no LocoNet 65535: Reboot 328P
 * 1026 Output Enable: Bit Nr, Status --  Bit 0: IBT-2 Bit 1: ProgSignal enable Bit 2: RailSync enable   Bit 3: SourceSelect 
 * 1027 Set current IBT-2 VarVal has current in mAmps
 * 1028 Set ack current Prog Track
 * 1029 Set max current RailSync
 * 1030 LED Brightness analog value 0-100 in percent ofull brightness
*/

uint8_t BoardManager::getDeviceMode()
{
  return deviceConfig;  
}

Sensor * BoardManager::getSensor()
{
  return RHSensorBlock;
}

void BoardManager::checkAllSensors(Print *stream)
{
  if ((powerStatus & 0xFC00) == 0)
    if (!digitalRead(pinProgTrackActive))
      RHSensorBlock->checkAll(stream);
}

void BoardManager::printAllSensors(Print *stream)
{
  RHSensorBlock->printAll(stream);
}

LEDChain * BoardManager::getLEDChain()
{
  return RHCtrlLEDs;
}

void BoardManager::setLED(uint8_t ledNr, CHSV ledColor)
{
  RHCtrlLEDs->setLED(ledNr, ledColor);
}

void BoardManager::setLEDBrightness(uint8_t percentLevel)
{
  RHCtrlLEDs->setLEDBrightness(percentLevel);
}

void BoardManager::setIBT2Enable(bool setEnable)
{
  setProgTrack(true);
  digitalWrite(pinIBT2Enable, setEnable);
  if (RHCtrlLEDs)
  {
    if (setEnable)
      RHCtrlLEDs->setLED(1, CHSV(75,255,255));
    else
      RHCtrlLEDs->setLED(1, CHSV(0,255,255));
    FastLED.show();
  }
}

void BoardManager::setProgTrack(bool setOn)
{
  Serial.println("Set prog Track");
  digitalWrite(pinProgTrackActive, setOn);
}

void BoardManager::initPins()
{
  PORTB &= 0xF0; //IOs 8-11 
  digitalWrite(pinSetSupplyPwr, 0); //also pins0
  digitalWrite(pinS1, 0);
  digitalWrite(pinS2, 0);
  digitalWrite(pinS3, 0);
  digitalWrite(pinProgTrackActive, 0);
  digitalWrite(pinCmdStationMode, 1);
  digitalWrite(pinIBT2Enable, 0);
  digitalWrite(pinLowPwrDCC, 0);
  pinMode(pinSetSupplyPwr, OUTPUT);
  pinMode(pinS1, OUTPUT);
  pinMode(pinS2, OUTPUT);
  pinMode(pinS3, OUTPUT);
  pinMode(pinProgTrackActive, OUTPUT);
  pinMode(pinCmdStationMode, OUTPUT);
  pinMode(pinIBT2Enable, OUTPUT);
  pinMode(pinLowPwrDCC, OUTPUT);
}

bool BoardManager::verifyPowerSignal(uint16_t numChecks)
{
  uint16_t minVal = 0xFFFF;
  uint16_t maxVal = 0x0000;
  for (uint16_t i = 0; i < numChecks; i++)
  {
    uint16_t currSample = analogRead(pinRawPwrSupply); //10bit ADC
    if (currSample < minVal)
      minVal = currSample;
    if (currSample > maxVal)
      maxVal = currSample;
  }
  powerStatus = (maxVal + minVal) >> 1; //average value
  if ((maxVal - minVal) > diffVoltage)
    powerStatus |= 0x1000; //invalid polarity or AC
  if ((minVal < minVoltage) || (maxVal > maxVoltage))
    powerStatus |= 0x2000; //invalid voltage range
  return ((powerStatus & 0xFC00) == 0);
}

uint16_t BoardManager::getExtPwrStatus()
{
  return powerStatus;
}

void BoardManager::setLEDDispStatus()
{
  if (RHCtrlLEDs)
  {
    if ((powerStatus & 0xFC00) == 0) //voltage ok, polarity ok
      RHCtrlLEDs->setLED(0, CHSV(map(powerStatus, minVoltage, maxVoltage, 40, 130),255,255)); //color depends on voltage from yellow to cyan
    else
      if (powerStatus |= 0x1000)
        RHCtrlLEDs->setLED(0, CHSV(210,255,255));
      else
        RHCtrlLEDs->setLED(0, CHSV((powerStatus & 0x3FF) > maxVoltage ? 0 : 170,255,255));
    FastLED.show();
  }
}

void BoardManager::setProgOutputEnable(bool setEnable)
{
  Serial.println("Set prog Track Disable");
  digitalWrite(pinProgTrackActive, 0);
  RHCtrlLEDs->setLED(2, CHSV(setEnable ? 85 : 0,255,255));
}

void BoardManager::setRailSyncEnable(bool setEnable)
{
  Serial.println(setEnable ? "Enable RailSync" : "Disable RailSync");
  digitalWrite(pinLowPwrDCC, setEnable);
}

void BoardManager::setDCCSource(bool setInternal)
{
  digitalWrite(pinCmdStationMode, setInternal); //also activates Loconet master
}

void BoardManager::setPowerRelay(bool newStat)
{
  digitalWrite(pinSetSupplyPwr, newStat);
}

void BoardManager::setBoardModeOff()
{
  DIAG(F("Off Mode"));
  RHSensorBlock->setEnable(false);  
  setIBT2Enable(false);
  setProgTrack(false);
  setProgOutputEnable(false);
  setRailSyncEnable(false);
  setDCCSource(true); //internal
  deviceConfig = 0;
}

void BoardManager::setBoardModeCmdStn()
{
  DIAG(F("Cmd Stn Mode"));
  RHSensorBlock->setEnable(true);  
  setIBT2Enable(true);
  setProgOutputEnable(false);
  setRailSyncEnable(true);
  setDCCSource(true); //internal
  deviceConfig = 1;
}

void BoardManager::setBoardModeBooster(bool useLN = true)
{
  DIAG(F("Booster Mode"));
  RHSensorBlock->setEnable(useLN ? true : false);  
  setIBT2Enable(true);
  setProgOutputEnable(false);
  setRailSyncEnable(false);
  setDCCSource(false); //internal
  deviceConfig = useLN ? 2 : 3;
}

void BoardManager::resetBoard()
{
  resetFunc();
}
