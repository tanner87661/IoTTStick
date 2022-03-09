#include <Arduino.h>
#include <Math.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
}
#define rawPwrSupplyPin A3

#define numSamples 500 //this is about 3 sinus cycles of 60Hz AC, just in case
#define minVoltage 250 //450 //12V  (1024 / 3.3) * (16/11)
#define maxVoltage 650 //735 //26V  (1024 / 3.3) * (26/11)
#define diffVoltage 10 //28  //1V (1024 / 3.3 * (1 / 11)

bool verifyPowerSignal()
{
  uint16_t minVal = 0xFFFF;
  uint16_t maxVal = 0x0000;
  uint32_t startTime = micros();
  for (uint16_t i = 0; i < numSamples; i++)
  {
    uint16_t currSample = analogRead(rawPwrSupplyPin);
    if (currSample < minVal)
      minVal = currSample;
    if (currSample > maxVal)
      maxVal = currSample;
  }
  uint32_t endTime = micros();
  char txtBuf[150];
  sprintf(txtBuf, "Min: %i Max: %i Time %lu ", minVal, maxVal, endTime - startTime);
  Serial.print(txtBuf);
  return ((minVal > minVoltage) && (maxVal < maxVoltage) && ((maxVal - minVal) <= diffVoltage));
}

void loop() {
  // put your main code here, to run repeatedly:
  if (verifyPowerSignal())
    Serial.println("good");
  else
    Serial.println("bad");
  delay(500);
}
