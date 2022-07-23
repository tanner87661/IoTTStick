Fast digital I/O functions for Arduino.
Created by Jan Dolinay, Feb. 2014 [https://www.codeproject.com/Articles/732646/Fast-digital-I-O-for-Arduino]
Works for Arduino Uno and Arduino Mega

Version 1.x.y from Thierry Paris,
Works also for Nano R3, Leonardo, and STM32, ESP32, rp2040 (Raspberry Pi Pico) and Due only for compatibility.

Revision History
----------------
Mai 2021 - Version 1.6.0
- Add Raspberry Pi Pico support by a bypass.

Mai 2019 - Version 1.5.1
- Change Visual Studio project to be conform to ArduiEmulator 2.0.0

December 2018 - Version 1.5.0
- Add support (just a bypass...) for non AVR architectures.

July 2018 - Version 1.4.1
- Fix Keywords.txt format.

July 2017 - Version 1.4
- Add new samples for Arduino Leonardo and Arduino Micro
- Add analog pins declaration for Uno and Nano .

December 2016 - Version 1.3
- Add good definitions for analog pins for Mega.

September 2016 - Version 1.2
- DUE compatibility added. In fact, just redirect the DIO functions to classic Arduino functions...

April 2016 - Version 1.1
- Arduino Nano and Leonardo added. Due has been added also, but only for compatibility, 
no improvementf or that kind of Arduino.
- No more file copy necessary at library installation. The compilation will choose the 
right include file according to the Arduino IDE flags.
- A new function GPIO_to_Arduino_pin has been added to transform a GPIO_pin_t into a
classic pin number in integer type.

March 2015 - Version 1.0
- Directory structure changed so that this package can be installed as an Arduino library.

February 2014 - First release.


Contents of this package:
-------------------------
board - files with pin definitions for supported boards (Uno and Mega). This need to be copied to
	appropriate location in your Arduino installation! (see instructions below).
examples - example sketches for Arduino Uno and Mega to test the new digital I/O.
src - source code of the library


Howto use:
-----------
There are two ways how to use the I/O 2 functions in your program:
1) Install as Arduino library (named Dio2)
OR
2) Copy 3 files into your Arduino installation


Option 1 - Arduino library
---------------------------
Step 1: Install the DIO2 library as any other Arduino library, that is extract the downloaded files 
into you Arduino libraries folder or use the automatic library install from the Arduino drop-down menu Sketch > Import Library

Step 2: Copy the pin2_arduino.h file for the board(s) you plan to use into the appropriate folder 
in your Arduino location.
You will find this file in the attached zip file in 
dio2\board\[board], where [board] is "standard" for Arduino Uno and "mega" for Arduino Mega.
This destination folder is:
For Arduino 1.6.0 IDE:
[your_arduino_location]\hardware\arduino\avr\variants\[board]
Examples: c:\arduino-1.6.0\hardware\arduino\avr\variants\standard  or c:\arduino-1.6.0\hardware\arduino\avr\variants\mega

For the older Arduino 1.0.x IDE:
[your_arduino_location]\hardware\arduino\variants\[board]
Examples: c:\arduino-1.0.5-r2\hardware\arduino\variants\standard or c:\arduino-1.0.5-r2\hardware\arduino\variants\mega.


Please note that the pin2_arduino.h file is different for Arduino standard and Arduino Mega. 
Use the appropriate file for your Arduino variant.
Also note that you are not overwriting anything in your Arduino installation and you can still 
use the original functions.


Option 2 - Copy required files to your Arduino location
-------------------------------------------------------
If you decide to use this option rather than using the library, you need to copy 3 files 
into appropriate folders in your Arduino location. You will find these files in the attached zip file, 
the source and destination locations are as follows:

Arduino 1.6.0
Copy arduino2.h and digital2.c from [zip file]\dio2\src\ to 
[your_arduino_location]\hardware\arduino\avr\cores\arduino\.

Copy pins2_arduino.h from [zip file]\dio2\board\standard or mega to 
[your_arduino_location]\hardware\arduino\avr\variants\standard or mega.

Arduino 1.0.x
Copy arduino2.h and digital2.c from [zip file]\dio2\src\ to 
[your_arduino_location]\hardware\arduino\cores\arduino\

Copy pins2_arduino.h from [zip file]\dio2\board\standard or mega to 
[your_arduino_location]\hardware\arduino\variants\standard or mega.

Note that using this option 2 has one disadvantage: if you need to build a program for other Arduino variant than Uno or Mega, you will encounter build error because of missing pins2_arduino.h file for this variant. To solve this, you can copy the "dummy" pins2_arduino.h file provided in zip file]\dio2\board\dummy to the appropriate folder in the Arduino variants folder.




Start Arduino IDE and create a new program (sketch) as you normally do. 
You can now use the I/O 2 functions described below instead of the standard
Arduino functions.

 
 
Functions which take simple pin number
---------------------------------------

These functions are fully compatible with the original Arduino functions. 
Just add '2' to the names of the original Arduino I/O functions and you are 
using these new functions.

void pinMode2(uint8_t pin, uint8_t mode);
 Set the direction of the pin. Possible options are INPUT, INPUT_PULLUP and OUTPUT.

uint8_t digitalRead2(uint8_t pin);
 Read the value at given pin. The pin should be set as input. Return value is either HIGH or LOW.

void digitalWrite2(uint8_t pin, uint8_t value);
 Set given pin to HIGH or LOW level. The pin should be set as output.


Functions which use pin code
----------------------------
These functions are faster than the previous ones. They differ from the original 
Arduino functions by the type of pin parameter - they use GPIO_pin_t instead of 
uint8_t (int). So in your program, you define the pin as GPIO_pin_t = DP1; 
instead of int pin = 1;.
The other arguments are the same as for standard Arduino functions.

void pinMode2f(GPIO_pin_t pin, uint8_t mode);
uint8_t digitalRead2f(GPIO_pin_t pin);
void digitalWrite2f(GPIO_pin_t pin, uint8_t value);

		

