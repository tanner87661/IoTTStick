/*
Output test of fast digital I/O for Arduino Leonardo
30.05.2017
*/

// Set to 1 to have fast but bigger program (inline digital I/O functions)
// set 0 to have slower and smaller program 
#define	GPIO2_PREFER_SPEED	1
#include <DIO2.h>  // include the fast I/O 2 functions


void test_outputs(void);

void setup() {
  // put your setup code here, to run once:
  // Set all pins to output mode
  uint8_t pin;
  for ( pin=0; pin<20; pin++ )
  {
    pinMode2(pin, OUTPUT);
  }
  //Serial.begin(9600);
}

void loop() {
      // put your main code here, to run repeatedly: 
      test_outputs();
}


/* HOWTO test:
 * Run program with this function called in the loop().
 * Connect an LED to any pin (through 1 k resistor! NOT DIRECTLY!).
 * The LED should blink (same as the LED 13 on the board)
 *
 * Note: All pins are working if you are not using serial communication.
 * If using Serial, pins 0 and 1 will not work.
 * */
void test_outputs(void)
{
  uint8_t pin;

  while( 1 )
  {
    // Set all outputs to HIGH
    for ( pin=0; pin<30; pin++ )
    {
	digitalWrite2(pin, HIGH);
    }
    //Serial.println("on");
    delay(600);
    
    // Set all outputs to LOW
    for ( pin=0; pin<30; pin++ )
    {
	digitalWrite2(pin, LOW);
    }
    //Serial.println("off");
    delay(600);
  }
}


