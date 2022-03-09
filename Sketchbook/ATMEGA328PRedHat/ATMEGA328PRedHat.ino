#include <Arduino.h>
#include <avr/eeprom.h>
#include "ModuleSetup.h"
#include "DCCTimer.h"
#include "LedChain.h"
#include "DCCExParser.h"

DCCTimer * TrackTimer = NULL;
LEDChain * OutLED = NULL;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  OutLED = new LEDChain();
  if (OutLED)
    OutLED->begin();
  BoardManager thisBoard = new BoardManager();
  while (true)
  {
    thisBoard->verifyPowerSignal();
    delay(500);
  }
  


  initInterrupts();
}

void initInterrupts()
{
  TrackTimer = new DCCTimer();
  if (TrackTimer)
    TrackTimer->begin(&interruptHandler);
}

static void interruptHandler()
{
    if (OCR1A == 464)     
      OCR1A = 1856;
    else   
      OCR1A = 464;
  PORTD ^= 0x60;
//  digitalWrite(5, !digitalRead(5));
//  digitalWrite(6, !digitalRead(6));
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available())
  {
    while(Serial.available())
      char c = Serial.read();
//    noInterrupts();  
    if (OCR1A == 928)     
      OCR1A = 1856;
    else   
      OCR1A = 928;
//    interrupts();          
  }
}
