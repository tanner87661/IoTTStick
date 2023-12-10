/****************************************************************************************************************************
  ISR_Timer.hpp
  For AVR ATmega164, ATmega324, ATmega644, ATmega1284 with MightyCore
  Written by Khoi Hoang

  Built by Khoi Hoang https://github.com/khoih-prog/ATmega_TimerInterrupt
  Licensed under MIT license

  Now with we can use these new 16 ISR-based timers, while consuming only 1 hwarware Timer.
  Their independently-selected, maximum interval is practically unlimited (limited only by unsigned long miliseconds)
  The accuracy is nearly perfect compared to software timers. The most important feature is they're ISR-based timers
  Therefore, their executions are not blocked by bad-behaving functions / tasks.
  This important feature is absolutely necessary for mission-critical tasks.

  Version: 1.1.1

  Version Modified By   Date      Comments
  ------- -----------  ---------- -----------
  1.0.0   K Hoang      22/08/2022 Initial coding for AVR ATmega164, ATmega324, ATmega644, ATmega1284 with MightyCore
  1.1.0   K Hoang      22/08/2022 Fix missing code for Timer3 and Timer4
  1.1.1   K Hoang      27/08/2022 Fix bug and optimize code in examples
*****************************************************************************************************************************/

#pragma once

#ifndef ISR_TIMER_HPP
#define ISR_TIMER_HPP

#ifndef ATMEGA_TIMER_INTERRUPT_VERSION
  #define ATMEGA_TIMER_INTERRUPT_VERSION           "ATmega_TimerInterrupt v1.1.1"
  
  #define ATMEGA_TIMER_INTERRUPT_VERSION_MAJOR     1
  #define ATMEGA_TIMER_INTERRUPT_VERSION_MINOR     1
  #define ATMEGA_TIMER_INTERRUPT_VERSION_PATCH     1

  #define ATMEGA_TIMER_INTERRUPT_VERSION_INT      1001001
#endif

#include "TimerInterrupt_Generic_Debug.h"

#include <stddef.h>
#include <inttypes.h>

#if defined(ARDUINO)
#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif
#endif

typedef void (*timer_callback)(void);
typedef void (*timer_callback_p)(void *);

class ISR_Timer 
{
  public:
    // maximum number of timers
    const static int MAX_TIMERS = 16;

    // setTimer() constants
    const static int RUN_FOREVER = 0;
    const static int RUN_ONCE = 1;

    // constructor
    ISR_Timer();

    void  init();

    // this function must be called inside loop()
    void  run();

    // Timer will call function 'f' every 'd' milliseconds forever
    // returns the timer number (numTimer) on success or
    // -1 on failure (f == NULL) or no free timers
    int setInterval(unsigned long d, timer_callback f);

    // Timer will call function 'f' with parameter 'p' every 'd' milliseconds forever
    // returns the timer number (numTimer) on success or
    // -1 on failure (f == NULL) or no free timers
    int setInterval(unsigned long d, timer_callback_p f, void* p);

    // Timer will call function 'f' after 'd' milliseconds one time
    // returns the timer number (numTimer) on success or
    // -1 on failure (f == NULL) or no free timers
    int setTimeout(unsigned long d, timer_callback f);

    // Timer will call function 'f' with parameter 'p' after 'd' milliseconds one time
    // returns the timer number (numTimer) on success or
    // -1 on failure (f == NULL) or no free timers
    int setTimeout(unsigned long d, timer_callback_p f, void* p);

    // Timer will call function 'f' every 'd' milliseconds 'n' times
    // returns the timer number (numTimer) on success or
    // -1 on failure (f == NULL) or no free timers
    int setTimer(unsigned long d, timer_callback f, unsigned n);

    // Timer will call function 'f' with parameter 'p' every 'd' milliseconds 'n' times
    // returns the timer number (numTimer) on success or
    // -1 on failure (f == NULL) or no free timers
    int setTimer(unsigned long d, timer_callback_p f, void* p, unsigned n);

    // updates interval of the specified timer
    bool changeInterval(unsigned numTimer, unsigned long d);

    // destroy the specified timer
    void deleteTimer(unsigned numTimer);

    // restart the specified timer
    void restartTimer(unsigned numTimer);

    // returns true if the specified timer is enabled
    bool isEnabled(unsigned numTimer);

    // enables the specified timer
    void enable(unsigned numTimer);

    // disables the specified timer
    void disable(unsigned numTimer);

    // enables all timers
    void enableAll();

    // disables all timers
    void disableAll();

    // enables the specified timer if it's currently disabled,
    // and vice-versa
    void toggle(unsigned numTimer);

    // returns the number of used timers
    unsigned  getNumTimers();

    // returns the number of available timers
    unsigned  getNumAvailableTimers() 
    {
      return MAX_TIMERS - numTimers;
    };

  private:
  
    // deferred call constants
    const static int DEFCALL_DONTRUN = 0;       // don't call the callback function
    const static int DEFCALL_RUNONLY = 1;       // call the callback function but don't delete the timer
    const static int DEFCALL_RUNANDDEL = 2;     // call the callback function and delete the timer

    // low level function to initialize and enable a new timer
    // returns the timer number (numTimer) on success or
    // -1 on failure (f == NULL) or no free timers
    int  setupTimer(unsigned long d, void* f, void* p, bool h, unsigned n);

    // find the first available slot
    int  findFirstFreeSlot();

    typedef struct 
    {
      unsigned long prev_millis;        // value returned by the millis() function in the previous run() call
      void* callback;                   // pointer to the callback function
      void* param;                      // function parameter
      bool hasParam;                 // true if callback takes a parameter
      unsigned long delay;              // delay value
      unsigned maxNumRuns;              // number of runs to be executed
      unsigned numRuns;                 // number of executed runs
      bool enabled;                  // true if enabled
      unsigned toBeCalled;              // deferred function call (sort of) - N.B.: only used in run()
    } timer_t;

    volatile timer_t timer[MAX_TIMERS];

    // actual number of timers in use (-1 means uninitialized)
    volatile int numTimers;
};


#endif    // #ifndef ISR_TIMER_HPP
