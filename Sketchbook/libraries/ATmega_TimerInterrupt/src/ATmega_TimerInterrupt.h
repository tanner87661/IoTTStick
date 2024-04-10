/****************************************************************************************************************************
  ATmega_TimerInterrupt.h
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

#ifndef ATmega_TimerInterrupt_h
#define ATmega_TimerInterrupt_h

#include "ATmega_TimerInterrupt.hpp"
#include "ATmega_TimerInterrupt_Impl.h"

#endif      //#ifndef ATmega_TimerInterrupt_h
