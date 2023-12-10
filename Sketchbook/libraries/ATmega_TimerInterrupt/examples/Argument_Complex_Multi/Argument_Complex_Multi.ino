/****************************************************************************************************************************
  Argument_Complex_Multi.ino
  For AVR ATmega164, ATmega324, ATmega644, ATmega1284 with MightyCore
  Written by Khoi Hoang

  Built by Khoi Hoang https://github.com/khoih-prog/ATmega_TimerInterrupt
  Licensed under MIT license

  Now with we can use these new 16 ISR-based timers, while consuming only 1 hwarware Timer.
  Their independently-selected, maximum interval is practically unlimited (limited only by unsigned long miliseconds)
  The accuracy is nearly perfect compared to software timers. The most important feature is they're ISR-based timers
  Therefore, their executions are not blocked by bad-behaving functions / tasks.
  This important feature is absolutely necessary for mission-critical tasks.

  Notes:
  Special design is necessary to share data between interrupt code and the rest of your program.
  Variables usually need to be "volatile" types. Volatile tells the compiler to avoid optimizations that assume
  variable can not spontaneously change. Because your function may change variables while your program is using them,
  the compiler needs this hint. But volatile alone is often not enough.
  When accessing shared variables, usually interrupts must be disabled. Even with volatile,
  if the interrupt changes a multi-byte variable between a sequence of instructions, it can be read incorrectly.
  If your data is multiple variables, such as an array and a count, usually interrupts need to be disabled
  or the entire sequence of your code which accesses the data.
*****************************************************************************************************************************/

// To demonstrate the usage of complex multiple files to avoid `multi definition linker` error
// by using TimerInterrupt.hpp in multiple files but TimerInterrupt.h in only main file

// These definitions must be placed before #include <TimerInterrupt.h> 
#include "Argument_Complex_Multi.h"

// To be included only in main(), .ino with setup() to avoid `Multiple Definitions` Linker Error
#include "ATmega_TimerInterrupt.h"

volatile pinStruct myOutputPins = { LED_BUILTIN, A0, A1 };

#define TIMER_INTERVAL_MS    1000

void setup()
{
  pinMode(myOutputPins.Pin1, OUTPUT);
  pinMode(myOutputPins.Pin2, OUTPUT);
  pinMode(myOutputPins.Pin3, OUTPUT);
    
  Serial.begin(115200);
  while (!Serial && millis() < 5000);

  Serial.print(F("\nStarting Argument_Complex on ")); Serial.println(BOARD_TYPE);
  Serial.println(ATMEGA_TIMER_INTERRUPT_VERSION);
  Serial.print(F("CPU Frequency = ")); Serial.print(F_CPU / 1000000); Serial.println(F(" MHz"));

  // Timer0 is already used for micros(), millis(), delay(), etc and can't be used
  // Select Timer 1-2
  // Timer 2 is 8-bit timer, only for higher frequency
   
  CurrentTimer.init();

  // Using ATmega324 with 16MHz CPU clock ,
  // For 16-bit timer 1, set frequency from 0.2385 to some KHz
  // For 8-bit timer 2 (prescaler up to 1024, set frequency from 61.5Hz to some KHz

  if (CurrentTimer.attachInterruptInterval(TIMER_INTERVAL_MS, TimerHandler, (unsigned int) &myOutputPins))
  {
    Serial.print(F("Starting ITimer OK, millis() = ")); Serial.println(millis());
  }
  else
    Serial.println(F("Can't set ITimer. Select another freq. or timer"));
}

void loop()
{
}
