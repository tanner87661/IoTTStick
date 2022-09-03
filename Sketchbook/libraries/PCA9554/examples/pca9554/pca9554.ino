/***************************************************************************
Title: PCA9554 sketch (PCA9554) - Output Example
Created by: Mach-1 Electronics - AD0ND
For: Arduino Users Everywhere!

This file is free software; you can redistribute it and/or modify
it under the terms of either the GNU General Public License version 2
or the GNU Lesser General Public License version 2.1, both as
published by the Free Software Foundation.
***************************************************************************/

#include <PCA9554.h>  // Load the PCA9554 Library
#include <Wire.h>     // Load the Wire Library

PCA9554 ioCon1(0x20);  // Create an object at this address
//  PCA9554 Addressing
//  Address     A2  A1  A0
//  0x20        L   L   L
//  0x21        L   L   H
//  0x22        L   H   L
//  0x23        L   H   H
//  0x24        H   L   L
//  0x25        H   L   H
//  0x26        H   H   L
//  0x27        H   H   H

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  ioCon1.portMode(ALLOUTPUT); //Set the port as all output

  //  Can also set pins individually using 
  //  ioCon1.pinmode(pin number, Input/Output);
  //  IE: ioCon1.pinmode(1, INPUT); 
}



void loop()
{
   ioCon1.digitalWrite(0, LOW);
   ioCon1.digitalWrite(1, LOW);
   ioCon1.digitalWrite(2, LOW);
   ioCon1.digitalWrite(3, LOW);
   ioCon1.digitalWrite(4, LOW);
   ioCon1.digitalWrite(5, LOW);
   ioCon1.digitalWrite(6, LOW);
   ioCon1.digitalWrite(7, HIGH);
   delay(25);
   ioCon1.digitalWrite(0, LOW);
   ioCon1.digitalWrite(1, LOW);
   ioCon1.digitalWrite(2, LOW);
   ioCon1.digitalWrite(3, LOW);
   ioCon1.digitalWrite(4, LOW);
   ioCon1.digitalWrite(5, LOW);
   ioCon1.digitalWrite(6, HIGH);
   ioCon1.digitalWrite(7, HIGH);
   delay(25);
   ioCon1.digitalWrite(0, LOW);
   ioCon1.digitalWrite(1, LOW);
   ioCon1.digitalWrite(2, LOW);
   ioCon1.digitalWrite(3, LOW);
   ioCon1.digitalWrite(4, LOW);
   ioCon1.digitalWrite(5, HIGH);
   ioCon1.digitalWrite(6, HIGH);
   ioCon1.digitalWrite(7, HIGH);
   delay(25);
   ioCon1.digitalWrite(0, LOW);
   ioCon1.digitalWrite(1, LOW);
   ioCon1.digitalWrite(2, LOW);
   ioCon1.digitalWrite(3, LOW);
   ioCon1.digitalWrite(4, HIGH);
   ioCon1.digitalWrite(5, HIGH);
   ioCon1.digitalWrite(6, HIGH);
   ioCon1.digitalWrite(7, HIGH);
   delay(25);
   ioCon1.digitalWrite(0, LOW);
   ioCon1.digitalWrite(1, LOW);
   ioCon1.digitalWrite(2, LOW);
   ioCon1.digitalWrite(3, HIGH);
   ioCon1.digitalWrite(4, HIGH);
   ioCon1.digitalWrite(5, HIGH);
   ioCon1.digitalWrite(6, HIGH);
   ioCon1.digitalWrite(7, HIGH);
   delay(25);
   ioCon1.digitalWrite(0, LOW);
   ioCon1.digitalWrite(1, LOW);
   ioCon1.digitalWrite(2, HIGH);
   ioCon1.digitalWrite(3, HIGH);
   ioCon1.digitalWrite(4, HIGH);
   ioCon1.digitalWrite(5, HIGH);
   ioCon1.digitalWrite(6, HIGH);
   ioCon1.digitalWrite(7, LOW);
   delay(25);
   ioCon1.digitalWrite(0, LOW);
   ioCon1.digitalWrite(1, HIGH);
   ioCon1.digitalWrite(2, HIGH);
   ioCon1.digitalWrite(3, HIGH);
   ioCon1.digitalWrite(4, HIGH);
   ioCon1.digitalWrite(5, HIGH);
   ioCon1.digitalWrite(6, LOW);
   ioCon1.digitalWrite(7, LOW);
   delay(25);
   ioCon1.digitalWrite(0, HIGH);
   ioCon1.digitalWrite(1, HIGH);
   ioCon1.digitalWrite(2, HIGH);
   ioCon1.digitalWrite(3, HIGH);
   ioCon1.digitalWrite(4, HIGH);
   ioCon1.digitalWrite(5, LOW);
   ioCon1.digitalWrite(6, LOW);
   ioCon1.digitalWrite(7, LOW);
   delay(25);
   ioCon1.digitalWrite(0, HIGH);
   ioCon1.digitalWrite(1, HIGH);
   ioCon1.digitalWrite(2, HIGH);
   ioCon1.digitalWrite(3, HIGH);
   ioCon1.digitalWrite(4, LOW);
   ioCon1.digitalWrite(5, LOW);
   ioCon1.digitalWrite(6, LOW);
   ioCon1.digitalWrite(7, LOW);
   delay(25);
   ioCon1.digitalWrite(0, HIGH);
   ioCon1.digitalWrite(1, HIGH);
   ioCon1.digitalWrite(2, HIGH);
   ioCon1.digitalWrite(3, LOW);
   ioCon1.digitalWrite(4, LOW);
   ioCon1.digitalWrite(5, LOW);
   ioCon1.digitalWrite(6, LOW);
   ioCon1.digitalWrite(7, LOW);
   delay(25);
   ioCon1.digitalWrite(0, HIGH);
   ioCon1.digitalWrite(1, HIGH);
   ioCon1.digitalWrite(2, LOW);
   ioCon1.digitalWrite(3, LOW);
   ioCon1.digitalWrite(4, LOW);
   ioCon1.digitalWrite(5, LOW);
   ioCon1.digitalWrite(6, LOW);
   ioCon1.digitalWrite(7, LOW);
   delay(25);
   ioCon1.digitalWrite(0, HIGH);
   ioCon1.digitalWrite(1, LOW);
   ioCon1.digitalWrite(2, LOW);
   ioCon1.digitalWrite(3, LOW);
   ioCon1.digitalWrite(4, LOW);
   ioCon1.digitalWrite(5, LOW);
   ioCon1.digitalWrite(6, LOW);
   ioCon1.digitalWrite(7, LOW);
   delay(25);
   ioCon1.digitalWrite(0, LOW);
   ioCon1.digitalWrite(1, LOW);
   ioCon1.digitalWrite(2, LOW);
   ioCon1.digitalWrite(3, LOW);
   ioCon1.digitalWrite(4, LOW);
   ioCon1.digitalWrite(5, LOW);
   ioCon1.digitalWrite(6, LOW);
   ioCon1.digitalWrite(7, LOW);
   delay(25);
   for(byte i = 0; i < 16; i++)
  {
    ioCon1.digitalWritePort(~(1 << i));
    delay(75);
  }
}
