#include "ModuleSetup.h"

void(* resetFunc) (void) = 0; //declare reset function @ address 0

void BoardManager::setDeviceMode(uint8_t newMode)
{
  deviceConfig = newMode;
  resetFunc();
}

uint8_t BoardManager::getDeviceMode()
{
  return deviceConfig;  
}

bool BoardManager::verifyPowerSignal()
{
  uint16_t minVal = 0xFFFF;
  uint16_t maxVal = 0x0000;
  for (uint16_t i = 0; i < numSamples; i++)
  {
    uint16_t currSample = analogRead(rawPwrSupplyPin);
    if (currSample > minVal)
      minVal = currSample;
    if (currSample < maxVal)
      maxVal = currSample;
  }
//  Serial.printf("Min: %i Max: %i Diff: %i\n", minVal, maxVal, maxVal - minVal);
  return ((minVal > minVoltage) && (maxVal < maxVoltage) && ((maxVal - minVal) <= diffVoltage));
}

void BoardManager::setBoosterMode(bool useLoconet)
{
  
}

void BoardManager::setCmdStationMode()
{
  
}
