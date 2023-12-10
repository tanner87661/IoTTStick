/****************************************************************************************************************************
  ATmega_TimerInterrupt.hpp
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
****************************************************************************************************************************/

#pragma once

#ifndef ATmega_TimerInterrupt_hpp
#define ATmega_TimerInterrupt_hpp

#if defined(BOARD_TYPE)
  #undef BOARD_TYPE
#endif

#if ( defined(__AVR_ATmega164P__) || defined(__AVR_ATmega164A__)  || \
      defined(__AVR_ATmega324P__) || defined(__AVR_ATmega324PA__) || defined(__AVR_ATmega324A__) || defined(__AVR_ATmega324PB__) || \
      defined(__AVR_ATmega644__)  || defined(__AVR_ATmega644A__)  || defined(__AVR_ATmega644P__) || \
			defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__) )

	#if ( defined(__AVR_ATmega164P__) || defined(__AVR_ATmega164A__) )
	
		#if defined(TIMER_INTERRUPT_USING_ATMEGA_164)
		  #undef TIMER_INTERRUPT_USING_ATMEGA_164
		#endif
		#define TIMER_INTERRUPT_USING_ATMEGA_164      true
  
		#define BOARD_TYPE    "MightyCore ATmega164(A/P)"
		
	#elif ( defined(__AVR_ATmega324P__) || defined(__AVR_ATmega324PA__) || defined(__AVR_ATmega324A__) )
	
		#if defined(TIMER_INTERRUPT_USING_ATMEGA_324)
		  #undef TIMER_INTERRUPT_USING_ATMEGA_324
		#endif
		#define TIMER_INTERRUPT_USING_ATMEGA_324      true
		
		#define BOARD_TYPE    "MightyCore ATmega324(A/P/PA)"
			
  #elif ( defined(__AVR_ATmega324PB__) )
  
  	#if defined(TIMER_INTERRUPT_USING_ATMEGA_324PB)
		  #undef TIMER_INTERRUPT_USING_ATMEGA_324PB
		#endif
		#define TIMER_INTERRUPT_USING_ATMEGA_324PB    true
		
		#define BOARD_TYPE    "MightyCore ATmega324PB"
		
	#elif ( defined(__AVR_ATmega644__)  || defined(__AVR_ATmega644A__)  || defined(__AVR_ATmega644P__) )
	
	  #if defined(TIMER_INTERRUPT_USING_ATMEGA_644)
		  #undef TIMER_INTERRUPT_USING_ATMEGA_644
		#endif
		#define TIMER_INTERRUPT_USING_ATMEGA_644    true
		
		#define BOARD_TYPE    "MightyCore ATmega644(A/P)"
		
	#elif ( defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__) )
	
	  #if defined(TIMER_INTERRUPT_USING_ATMEGA_1284)
		  #undef TIMER_INTERRUPT_USING_ATMEGA_1284
		#endif
		#define TIMER_INTERRUPT_USING_ATMEGA_1284   true
		
		#define BOARD_TYPE    "MightyCore ATmega1284(P)"
		
	#else
		#define BOARD_TYPE    "Unknown Board"
	#endif
	
#elif ( defined(__AVR_ATmega8535__) || defined(__AVR_ATmega16__) || defined(__AVR_ATmega32__) )

  #error Old MightyCore AVR ATmega8535, ATmega16, ATmega32 not supported, try ATmega164, ATmega324, ATmega644, ATmega1284
  
#else
  #error This is designed only for MightyCore ATmega164, ATmega324, ATmega644, ATmega1284! Please check your Tools->Board setting.
#endif

#if (_TIMERINTERRUPT_LOGLEVEL_ > 2)
	#if ( defined(__AVR_ATmega164P__) || defined(__AVR_ATmega164A__) )
		#warning MightyCore ATmega164(A/P) with available Timer1-2
	#elif ( defined(__AVR_ATmega324P__) || defined(__AVR_ATmega324PA__) || defined(__AVR_ATmega324A__) || defined(__AVR_ATmega324PB__) )
		#warning MightyCore ATmega324(A/P/PA/PB) with available Timer1-2
  #elif ( defined(__AVR_ATmega324PB__) )
		#warning MightyCore ATmega324PB with available Timer1-4	
	#elif ( defined(__AVR_ATmega644__)  || defined(__AVR_ATmega644A__)  || defined(__AVR_ATmega644P__) )
		#warning MightyCore ATmega644(A/P) with available Timer1-2
	#elif TIMER_INTERRUPT_USING_ATMEGA_1284
		#warning MightyCore ATmega1284(P)  with available Timer1-3
	#else
		#warning Unknown Board
	#endif
#endif

//////////////////////////////////////////////////////////////////////////////////

#ifndef TIMER_INTERRUPT_DEBUG
  #define TIMER_INTERRUPT_DEBUG      0
#endif

#include "TimerInterrupt_Generic_Debug.h"

#ifndef ATMEGA_TIMER_INTERRUPT_VERSION
  #define ATMEGA_TIMER_INTERRUPT_VERSION           "ATmega_TimerInterrupt v1.1.1"
  
  #define ATMEGA_TIMER_INTERRUPT_VERSION_MAJOR     1
  #define ATMEGA_TIMER_INTERRUPT_VERSION_MINOR     1
  #define ATMEGA_TIMER_INTERRUPT_VERSION_PATCH     1

  #define ATMEGA_TIMER_INTERRUPT_VERSION_INT      1001001
#endif

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "Arduino.h"
#include "pins_arduino.h"

#if defined(min)
	#undef min
	#define min(a,b) ((a)<(b)?(a):(b))
#endif
	
#if defined(max)
	#undef max	
	#define max(a,b) ((a)>(b)?(a):(b))
#endif	

#define MAX_COUNT_8BIT            255
#define MAX_COUNT_10BIT           1023
#define MAX_COUNT_16BIT           65535

typedef void (*timer_callback)();
typedef void (*timer_callback_p)(void *);

// HW_TIMER_0 is already used for micros(), millis(), delay(), etc and can't be used
enum
{
  HW_TIMER_0 = 0,
  HW_TIMER_1,
  HW_TIMER_2,
  HW_TIMER_3,
  HW_TIMER_4,
  NUM_HW_TIMERS
};

enum
{
  NO_CLOCK_SOURCE = 0,
  NO_PRESCALER,
  PRESCALER_8,
  PRESCALER_64,
  PRESCALER_256,
  PRESCALER_1024,
  NUM_ITEMS
};

enum
{
  T2_NO_CLOCK_SOURCE = 0,
  T2_NO_PRESCALER,
  T2_PRESCALER_8,
  T2_PRESCALER_32,
  T2_PRESCALER_64,
  T2_PRESCALER_128,
  T2_PRESCALER_256,
  T2_PRESCALER_1024,
  T2_NUM_ITEMS
};

const unsigned int prescalerDiv   [NUM_ITEMS]     = { 1, 1, 8, 64, 256, 1024 };
const unsigned int prescalerDivT2 [T2_NUM_ITEMS]  = { 1, 1, 8, 32,  64,  128, 256, 1024 };

class ATmega_TimerInterrupt
{
  private:

    bool            _timerDone;
    int8_t          _timer;
    unsigned int    _prescalerIndex;
    uint32_t        _OCRValue;
    uint32_t        _OCRValueRemaining;
    volatile long   _toggle_count;
    double           _frequency;

    void*           _callback;        // pointer to the callback function
    void*           _params;          // function parameter

    void set_OCR();

  public:

    ATmega_TimerInterrupt()
    {
      _timer              = -1;
      _frequency          = 0;
      _callback           = NULL;
      _params             = NULL;
      _timerDone          = false;
      _prescalerIndex     = NO_PRESCALER;
      _OCRValue           = 0;
      _OCRValueRemaining  = 0;
      _toggle_count       = -1;
    };

    explicit ATmega_TimerInterrupt(uint8_t timerNo)
    {
      _timer              = timerNo;
      _frequency          = 0;
      _callback           = NULL;
      _params             = NULL;
      _timerDone          = false;
      _prescalerIndex     = NO_PRESCALER;
      _OCRValue           = 0;
      _OCRValueRemaining  = 0;
      _toggle_count       = -1;
    };

    void callback() __attribute__((always_inline))
    {
      if (_callback != NULL)
      {
        if (_params != NULL)
          (*(timer_callback_p)_callback)(_params);
        else
          (*(timer_callback)_callback)();
      }
    }

    void init(int8_t timer);

    void init()
    {
      init(_timer);
    };

    // frequency (in hertz) and duration (in milliseconds). Duration = 0 or not specified => run indefinitely
    bool setFrequency(float frequency, timer_callback_p callback, /* void* */ uint32_t params, unsigned long duration = 0);

    // frequency (in hertz) and duration (in milliseconds). Duration = 0 or not specified => run indefinitely
    bool setFrequency(float frequency, timer_callback callback, unsigned long duration = 0)
    {
      return setFrequency(frequency, reinterpret_cast<timer_callback_p>(callback), /*NULL*/ 0, duration);
    }

    // interval (in ms) and duration (in milliseconds). Duration = 0 or not specified => run indefinitely
    template<typename TArg>
    bool setInterval(unsigned long interval, void (*callback)(TArg), TArg params, unsigned long duration = 0)
    {
      static_assert(sizeof(TArg) <= sizeof(uint32_t), "setInterval() callback argument size must be <= 4 bytes");
      return setFrequency((float) (1000.0f / interval), reinterpret_cast<timer_callback_p>(callback), (uint32_t) params, duration);
    }

    // interval (in ms) and duration (in milliseconds). Duration = 0 or not specified => run indefinitely
    bool setInterval(unsigned long interval, timer_callback callback, unsigned long duration = 0)
    {
      return setFrequency((float) (1000.0f / interval), reinterpret_cast<timer_callback_p>(callback), /*NULL*/ 0, duration);
    }

    template<typename TArg>
    bool attachInterrupt(float frequency, void (*callback)(TArg), TArg params, unsigned long duration = 0)
    {
      static_assert(sizeof(TArg) <= sizeof(uint32_t), "attachInterrupt() callback argument size must be <= 4 bytes");
      return setFrequency(frequency, reinterpret_cast<timer_callback_p>(callback), (uint32_t) params, duration);
    }

    bool attachInterrupt(float frequency, timer_callback callback, unsigned long duration = 0)
    {
      return setFrequency(frequency, reinterpret_cast<timer_callback_p>(callback), /*NULL*/ 0, duration);
    }

    // Interval (in ms) and duration (in milliseconds). Duration = 0 or not specified => run indefinitely
    template<typename TArg>
    bool attachInterruptInterval(unsigned long interval, void (*callback)(TArg), TArg params, unsigned long duration = 0)
    {
      static_assert(sizeof(TArg) <= sizeof(uint32_t), "attachInterruptInterval() callback argument size must be <= 4 bytes");
      return setFrequency( (float) ( 1000.0f / interval), reinterpret_cast<timer_callback_p>(callback), (uint32_t) params, duration);
    }

    // Interval (in ms) and duration (in milliseconds). Duration = 0 or not specified => run indefinitely
    bool attachInterruptInterval(unsigned long interval, timer_callback callback, unsigned long duration = 0)
    {
      return setFrequency( (float) ( 1000.0f / interval), reinterpret_cast<timer_callback_p> (callback), /*NULL*/ 0, duration);
    }

    void detachInterrupt();

    void disableTimer()
    {
      detachInterrupt();
    }

    // Duration (in milliseconds). Duration = 0 or not specified => run indefinitely
    void reattachInterrupt(unsigned long duration = 0);

    // Duration (in milliseconds). Duration = 0 or not specified => run indefinitely
    void enableTimer(unsigned long duration = 0) __attribute__((always_inline))
    {
      reattachInterrupt(duration);
    }

    // Just stop clock source, still keep the count
    void pauseTimer();

    // Just reconnect clock source, continue from the current count
    void resumeTimer();

    // Just stop clock source, clear the count
    void stopTimer()
    {
      detachInterrupt();
    }

    // Just reconnect clock source, start current count from 0
    void restartTimer(unsigned long duration = 0)
    {
      reattachInterrupt(duration);
    }

    int8_t getTimer() __attribute__((always_inline))
    {
      return _timer;
    };

    long getCount() __attribute__((always_inline))
    {
      return _toggle_count;
    };

    void setCount(long countInput) __attribute__((always_inline))
    {
      //cli();//stop interrupts
      //noInterrupts();

      _toggle_count = countInput;

      //sei();//enable interrupts
      //interrupts();
    };

    long get_OCRValue() __attribute__((always_inline))
    {
      return _OCRValue;
    };

    long get_OCRValueRemaining() __attribute__((always_inline))
    {
      return _OCRValueRemaining;
    };

    void adjust_OCRValue() //__attribute__((always_inline))
    {
      //cli();//stop interrupts
      noInterrupts();

      if (_timer != 2)
      {
        if (_OCRValueRemaining < MAX_COUNT_16BIT)
        {
          set_OCR();
        }
          
        _OCRValueRemaining -= min(MAX_COUNT_16BIT, _OCRValueRemaining);
      }
      else
      {
        if (_OCRValueRemaining < MAX_COUNT_8BIT)
        {
          set_OCR();
        }
          
        _OCRValueRemaining -= min(MAX_COUNT_8BIT, _OCRValueRemaining);
      }

      if (_OCRValueRemaining <= 0)
      {
        // Reset value for next cycle
        _OCRValueRemaining = _OCRValue;
        _timerDone = true;
      }
      else
        _timerDone = false;

      //sei();//enable interrupts
      interrupts();
    };

    void reload_OCRValue() //__attribute__((always_inline))
    {
      //cli();//stop interrupts
      noInterrupts();

      // Reset value for next cycle, have to deduct the value already loaded to OCR register
      
      _OCRValueRemaining = _OCRValue;
      set_OCR();

      _timerDone = false;

      //sei();//enable interrupts
      interrupts();
    };

    bool checkTimerDone() //__attribute__((always_inline))
    {
      return _timerDone;
    };

}; // class ATmega_TimerInterrupt

//////////////////////////////////////////////

// To be sure not used Timers are disabled
#if !defined(USE_TIMER_1)
  #define USE_TIMER_1     false
#endif

#if !defined(USE_TIMER_2)
  #define USE_TIMER_2     false
#endif

#if !defined(USE_TIMER_3)
  #define USE_TIMER_3     false
#elif ( USE_TIMER_3 && ( TIMER_INTERRUPT_USING_ATMEGA_1284 || TIMER_INTERRUPT_USING_ATMEGA_324PB ) )
  #warning Timer3 (16-bit) is OK to use for ATMEGA_1284(P) or ATMEGA_324PB
#elif USE_TIMER_3
  #error Timer3 is only available for ATMEGA_1284(P) and ATMEGA_324PB
#endif

#if !defined(USE_TIMER_4)
  #define USE_TIMER_4     false
#elif ( USE_TIMER_4 && ( TIMER_INTERRUPT_USING_ATMEGA_324PB ) )
  #warning Timer4 is OK to use for ATMEGA_324PB
#elif USE_TIMER_4
  #error Timer4 is only available for ATMEGA_324PB
#endif


//////////////////////////////////////////////

#endif      //#ifndef ATmega_TimerInterrupt_hpp
