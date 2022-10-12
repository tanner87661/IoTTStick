/*
 *  © 2021 Mike S
 *  © 2021 Harald Barth
 *  © 2021 Fred Decker
 *  All rights reserved.
 *  
 *  This file is part of CommandStation-EX
 *
 *  This is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  It is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CommandStation.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef DCCCurrentSensor_h
#define DCCCurrentSensor_h
#include "Arduino.h"
#include "DCCTimer.h"

#define millisamples 26 //number of samples in a millisecond
#define reportIntv 50 //number of sampling cycles averaged for reporting

class DCCCurrentSensor {
  public:
    static void begin();
    static void begin(INTERRUPT_CALLBACK adc_interrupt);
    static float getCurrent(byte track);
/*
    static float sampleAccu;
    static uint8_t sampleCtr;
    static float msVal[2];
    static float intvAccu;
    static uint16_t intvCtr;
    static float trackVal[2];
    static byte trackSel; //0: main track 1: prog track
*/
  private:
    static void adcinterruptHandler();
};
#endif
