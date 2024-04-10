#include <Arduino.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include "DCCTimer.h"
#include "BoosterNode.h"
#include "BoosterAdmin.h"

BoosterGroup bList;

char serBuffer[150];
uint8_t readPtr = 0;

void setup() {
  delay(1000);
  Serial.begin(115200);
  while (Serial.available())
    char c = Serial.read();
  Serial.println("IoTT SilverHat Booster");
  randomSeed(micros());
  bList.initBooster();
  bList.initNodes(); 
  bList.setExtStatus(-1, 0);
  delay(10);
  DCCTimer::begin(interruptHandler);   
  delay(10);
  bList.setExtStatus(-1, 0xFF); //bList.bConfig.devMode & 0x0F);
  bList.requestSVData(-1, 0x00FF);
  bList.requestSVData(-1, 0xFFFF);
}

void interruptHandler()
{
  bList.processTimerInterrupt();
}

void loop() 
{
  procInput(); //get commands
  bList.processLoop(); //send out reports and manage resets
}
