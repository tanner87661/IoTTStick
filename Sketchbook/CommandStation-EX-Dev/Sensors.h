/*
 *  © 2020, Chris Harlow. All rights reserved.
 *  
 *  This file is part of Asbelos DCC API
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
#ifndef Sensor_h
#define Sensor_h

#include "Arduino.h"
#include <Math.h>
#include "DIAG.h"

#define  SENSOR_DECAY  0.03

#define PortBank1 A2
#define PortBank2 A7


class  Sensor
{
  public:
    void begin();
    void setEnable(bool newStatus);
    void checkAll(Print *stream);
    void printAll(Print *stream);
  private:
    uint32_t outStatus;
    uint32_t inpStatusABCD;
    uint32_t verifyStatus;
    uint8_t currentSensor = 0;
    byte latchdelay;
    bool isActive = false;
}; // Sensor


#endif
