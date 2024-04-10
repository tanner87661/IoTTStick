/*
  Thanks for supporting Open-Hard/Soft-ware and thanks
  for all of the contributors to this project.

  For extra info on GRBL please have a look at my blog :
    http://blog.protoneer.co.nz/tag/grbl/

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  http://www.gnu.org/licenses/
*/

/*
    Supported hardware:
        Arduino Duemilanove
        Arduino Uno
        Arduino Mega 2560 (Limited Testing)

*/
#define Set_Bit(val, bitn) (val |= (1 << (bitn)))
#define Clr_Bit(val, bitn) (val &= ~(1 << (bitn)))
#define Get_Bit(val, bitn) (val & (1 << (bitn)))
#include <grblmain.h>
#define LedRed1        \
    Set_Bit(PORTB, 4); \
    Set_Bit(PORTC, 3)
#define LedRed0        \
    Clr_Bit(PORTB, 4); \
    Clr_Bit(PORTC, 3)
#define LedBlue1       \
    Set_Bit(PORTB, 5); \
    Set_Bit(PORTC, 2)
#define LedBlue0       \
    Clr_Bit(PORTB, 5); \
    Clr_Bit(PORTC, 2)

#define Limit (PINB & 0x02) != 0x02
void setup() {
    DDRC  = 0xff;
    DDRB  = 0xff;
    DDRD  = 0xff;
    PORTB = 0xff;
    LedBlue0;
    LedRed0;
    for (int i = 0; i < 5; i++) {
        LedBlue1;
        delay(20);
        LedRed0;
        delay(20);
        LedBlue0;
        delay(20);
        LedRed1;
        delay(20);
    }
    LedRed0;
    LedBlue0;
    startGrbl();
}

void loop() {
}
