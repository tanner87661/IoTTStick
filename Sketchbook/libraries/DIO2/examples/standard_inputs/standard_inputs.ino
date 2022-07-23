/*
Input test of fast digital I/O for Arduino Uno (Standard)
14.2.2014
*/

// Set to 1 to have fast but (maybe) bigger program (inline digital I/O functions)
// set 0 to have slower and (sometimes) smaller program 
#define	GPIO2_PREFER_SPEED	1
#include <DIO2.h>  // include the fast I/O 2 functions


void test_inputs_pullup(void);
void test_inputs_no_pullup(void);

void setup() {
  // put your setup code here, to run once:
  // Will use serial port to display which pin is low
  Serial.begin(9600);
}

void loop() {
    // put your main code here, to run repeatedly: 
    test_inputs_pullup();
    //test_inputs_no_pullup();  // not reliable, please see comment below.
}

/* HOWTO test:
 * Run program with this function called in the loop().
 *  With INPUT_PULLUP mode: connect any pin to GND (directly by a wire, or use
 * e.g. a 1k resistor if you want to be extra safe). The program will print
 * into serial terminal the number of the pin you connected to GND.
 
 *  With INPUT (without pull up): the logical level at the pin is not defined and
 *  the program would print most of them as LOW. That's why the test is for HIGH
 *  and we are printing pins which are high. 
 *  NOTE that it is normar that some pins will be reported high even if not 
 *  connected to 5V! But if you connect any pin to +5V (directly or through 
 *  e.g. 1 k resistor) it should be printed as HIGH for sure.
 *
 *  NOTE: pins 0 and 1 are not working (used for serial communication)
 *  pin 13 is not working (used for on-board LED)
 *
 * */
void test_inputs_pullup(void)
{
  uint8_t pin;
  for ( pin=2; pin<20; pin++ )
  {
    pinMode2(pin, INPUT_PULLUP);
 
  }

  // LED pin cannot be used as input
  pinMode2(13, OUTPUT);	

  // scan all pins and print the one which is low
  // excluding pins for serial line (0 and 1)!
  // and pin for LED
  for ( pin=2; pin<20; pin++ )
  {
    if ( pin != 13 && digitalRead2(pin) == LOW )
    {
	   Serial.print("pin low: ");
	   Serial.println(pin);
	   delay(500);
    }
  }

}


void test_inputs_no_pullup(void)
{
  uint8_t pin;
  for ( pin=2; pin<20; pin++ )
  {
    pinMode2(pin, INPUT);
  }

  // LED pin cannot be used as input anyway
  pinMode2(13, OUTPUT);	// LED for signalling

  // scan all pins and print the one which is high
  // excluding pins for serial line (0 and 1)!
  // and pin for LED
  for ( pin=2; pin<70; pin++ )
  {
    if ( pin != 13 && digitalRead2(pin) == HIGH )
    {
	   Serial.print("pin high: ");
	   Serial.println(pin);
	   delay(500);
    }
  }

}
 
