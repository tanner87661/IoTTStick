/*
  Blink2 - Blink example modified for using digital I/O 2 instead of standard 
        Arduino digital I/O.
 */

#include <DIO2.h>  // include the fast I/O 2 functions
 
// Pin 13 has an LED connected on most Arduino boards.
// give it a name:
const int led = 13;

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin as an output.
  pinMode2(led, OUTPUT);     
}

// the loop routine runs over and over again forever:
void loop() {
  digitalWrite2(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);               // wait for a second
  digitalWrite2(led, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);               // wait for a second
}
