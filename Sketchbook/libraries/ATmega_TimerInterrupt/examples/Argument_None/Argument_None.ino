/****************************************************************************************************************************
  Argument_None.ino
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

// These define's must be placed at the beginning before #include "TimerInterrupt.h"
// _TIMERINTERRUPT_LOGLEVEL_ from 0 to 4
// Don't define _TIMERINTERRUPT_LOGLEVEL_ > 0. Only for special ISR debugging only. Can hang the system.
#define TIMER_INTERRUPT_DEBUG         0
#define _TIMERINTERRUPT_LOGLEVEL_     0

#define USE_TIMER_1     true

// Select just 1 TIMER to be true
#define USE_TIMER_2     true
// TIMER_3 Only valid for ATmega1284 and ATmega324PB (not ready in core yet)
#define USE_TIMER_3     false
// TIMER_4 Only valid for ATmega324PB, not ready in core yet
#define USE_TIMER_4     false

#if USE_TIMER_2
  #define CurrentTimer   ITimer2
#elif USE_TIMER_3
  #define CurrentTimer   ITimer3
#elif USE_TIMER_4
  #define CurrentTimer   ITimer4
#else
  #error You must select one Timer  
#endif

#if (_TIMERINTERRUPT_LOGLEVEL_ > 3)
  #if (USE_TIMER_2)
    #warning Using Timer1 and Timer2
  #elif (USE_TIMER_3)
    #warning Using Timer1 and Timer3
  #elif (USE_TIMER_4)
    #warning Using Timer1 and Timer4
  #endif
#endif

// To be included only in main(), .ino with setup() to avoid `Multiple Definitions` Linker Error
#include "ATmega_TimerInterrupt.h"

#define TIMER1_INTERVAL_MS    1000
#define TIMER_INTERVAL_MS     2000

#ifndef LED_BUILTIN
  #define LED_BUILTIN   13
#endif

void TimerHandler1(void)
{
  static bool toggle1 = false;

  //timer interrupt toggles pin LED_BUILTIN
  digitalWrite(LED_BUILTIN, toggle1);
  toggle1 = !toggle1;
}

void TimerHandler(void)
{
  static bool toggle = false;
 
  //timer interrupt toggles outputPin
  digitalWrite(A0, toggle);
  toggle = !toggle;
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(A0, OUTPUT);
  
  Serial.begin(115200);
  while (!Serial && millis() < 5000);

  Serial.print(F("\nStarting Argument_None on ")); Serial.println(BOARD_TYPE);
  Serial.println(ATMEGA_TIMER_INTERRUPT_VERSION);
  Serial.print(F("CPU Frequency = ")); Serial.print(F_CPU / 1000000); Serial.println(F(" MHz"));

  // Timer0 is already used for micros(), millis(), delay(), etc and can't be used
  // Select Timer 1-2
  // Timer 2 is 8-bit timer, only for higher frequency
  
  ITimer1.init();

  // Using ATmega324 with 16MHz CPU clock ,
  // For 16-bit timer 1, set frequency from 0.2385 to some KHz
  // For 8-bit timer 2 (prescaler up to 1024, set frequency from 61.5Hz to some KHz

  if (ITimer1.attachInterruptInterval(TIMER1_INTERVAL_MS, TimerHandler1))
  {
    Serial.print(F("Starting ITimer1 OK, millis() = ")); Serial.println(millis());
  }
  else
    Serial.println(F("Can't set ITimer1. Select another freq. or timer"));

  // Using ATmega324 with 16MHz CPU clock ,
  // For 16-bit timer 1, set frequency from 0.2385 to some KHz
  // For 8-bit timer 2 (prescaler up to 1024, set frequency from 61.5Hz to some KHz

  ///////////////////////////////////////////////
  
  // Init second timer
  CurrentTimer.init();

  if (CurrentTimer.attachInterruptInterval(TIMER_INTERVAL_MS, TimerHandler))
  {
    Serial.print(F("Starting ITimer OK, millis() = ")); Serial.println(millis());
  }
  else
    Serial.println(F("Can't set ITimer. Select another freq. or timer"));
}

void loop()
{

}
