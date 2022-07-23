/*
  Blink2 - Blink example modified for using digital I/O 2 instead of standard 
        Arduino digital I/O.
 */
#include <DIO2.h>  // include the fast I/O 2 functions

// The I/O 2 functions use special data type for pin
// Pin codes, such as DP13 are defined in pins2_arduino.h
const GPIO_pin_t led_pin = DP13;

void setup() {                
  pinMode2f(led_pin, OUTPUT);     
}

void loop() {
  digitalWrite2f(led_pin, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);               // wait for a second
  digitalWrite2f(led_pin, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);               // wait for a second
}


