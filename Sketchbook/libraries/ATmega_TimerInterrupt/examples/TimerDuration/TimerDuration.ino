/****************************************************************************************************************************
  TimerDuration.ino
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

#if !defined(LED_BUILTIN)
  #define LED_BUILTIN     13
#endif

#if USE_TIMER_1

void TimerHandler1(unsigned int outputPin = LED_BUILTIN)
{
  static bool toggle1 = false;

  //timer interrupt toggles pin LED_BUILTIN
  digitalWrite(outputPin, toggle1);
  toggle1 = !toggle1;
}

#endif

void TimerHandler(unsigned int outputPin = LED_BUILTIN)
{
  static bool toggle = false;

  //timer interrupt toggles outputPin
  digitalWrite(outputPin, toggle);
  toggle = !toggle;
}

unsigned int outputPin1 = LED_BUILTIN;
unsigned int outputPin  = A0;

#define TIMER1_INTERVAL_MS    10000
#define TIMER1_FREQUENCY      (float) (1000.0f / TIMER1_INTERVAL_MS)
#define TIMER1_DURATION_MS    0 //(10 * TIMER1_INTERVAL_MS)

#define TIMER_INTERVAL_MS    13000
#define TIMER_FREQUENCY      (float) (1000.0f / TIMER_INTERVAL_MS)
#define TIMER_DURATION_MS    0   //(20 * TIMER_INTERVAL_MS)

void setup()
{
  pinMode(outputPin1, OUTPUT);
  pinMode(outputPin, OUTPUT);
  
  Serial.begin(115200);
  while (!Serial && millis() < 5000);

  Serial.print(F("\nStarting TimerDuration on ")); Serial.println(BOARD_TYPE);
  Serial.println(ATMEGA_TIMER_INTERRUPT_VERSION);
  Serial.print(F("CPU Frequency = ")); Serial.print(F_CPU / 1000000); Serial.println(F(" MHz"));

  // Timer0 is already used for micros(), millis(), delay(), etc and can't be used
  // Select Timer 1-2
  // Timer 2 is 8-bit timer, only for higher frequency
  
#if USE_TIMER_1

  ITimer1.init();

  // Using ATmega324 with 16MHz CPU clock ,
  // For 16-bit timer 1, set frequency from 0.2385 to some KHz
  // For 8-bit timer 2 (prescaler up to 1024, set frequency from 61.5Hz to some KHz

  if (ITimer1.attachInterruptInterval(TIMER1_INTERVAL_MS, TimerHandler1, outputPin1, TIMER1_DURATION_MS))
  {
    Serial.print(F("Starting ITimer1 OK, millis() = ")); Serial.println(millis());
  }
  else
    Serial.println(F("Can't set ITimer1. Select another freq. or timer"));

#endif

  ///////////////////////////////////////////////
  
  // Init second timer
  CurrentTimer.init();

  if (CurrentTimer.attachInterruptInterval(TIMER_INTERVAL_MS, TimerHandler, outputPin))
  {
    Serial.print(F("Starting ITimer OK, millis() = ")); Serial.println(millis());

#if (TIMER_INTERRUPT_DEBUG > 1)    
    Serial.print(F("OutputPin = ")); Serial.print(outputPin);
    Serial.print(F(" address: ")); Serial.println((uint32_t) &outputPin );
#endif    
  }
  else
    Serial.println(F("Can't set ITimer. Select another freq. or timer"));
}

void loop()
{
}
