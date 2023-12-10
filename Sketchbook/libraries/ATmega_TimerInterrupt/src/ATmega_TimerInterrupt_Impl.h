/****************************************************************************************************************************
  ATmega_TimerInterrupt.cpp
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

#ifndef ATmega_TimerInterrupt_Impl_h
#define ATmega_TimerInterrupt_Impl_h

#ifndef TIMER_INTERRUPT_DEBUG
  #define TIMER_INTERRUPT_DEBUG      0
#endif

void ATmega_TimerInterrupt::init(int8_t timer)
{    
  // Set timer specific stuff
  // All timers in CTC mode
  // 8 bit timers will require changing prescalar values,
  // whereas 16 bit timers are set to either ck/1 or ck/64 prescalar
  
  //cli();//stop interrupts
  noInterrupts();
  
  switch (timer)
  {    
    #if defined(TCCR1A) && defined(TCCR1B) && defined(WGM12)
    case 1:
      // 16 bit timer
      TCCR1A = 0;
      TCCR1B = 0;
      // Page 172-173. ATmega 328/328P or Page 145-146 of ATmega 640/1280/2560
      // Mode 4 => Clear Timer on Compare match (CTC) using OCR1A for counter value
      bitWrite(TCCR1B, WGM12, 1);
      // No scaling now
      bitWrite(TCCR1B, CS10, 1);
      
      break;
    #endif

    #if defined(TCCR2A) && defined(TCCR2B)
    case 2:
      // 8 bit timer
      TCCR2A = 0;
      TCCR2B = 0;
      // Page 205-206. ATmega328, Page 184-185 ATmega 640/1280/2560
      // Mode 2 => Clear Timer on Compare match (CTC) using OCR2A for counter value
      bitWrite(TCCR2A, WGM21, 1);
      // No scaling now
      bitWrite(TCCR2B, CS20, 1);
      
      break;
    #endif
    
    #if defined(TCCR3A) && defined(TCCR3B) &&  defined(TIMSK3)
    case 3:
      // 16 bit timer
      TCCR3A = 0;
      TCCR3B = 0;
      bitWrite(TCCR3B, WGM32, 1);
      bitWrite(TCCR3B, CS30, 1);
      
      break;
    #endif  

    #if defined(TCCR4A) && defined(TCCR4B) &&  defined(TIMSK4)
    case 4:
      // 16 bit timer
      TCCR4A = 0;
      TCCR4B = 0;
      #if defined(WGM42)
        bitWrite(TCCR4B, WGM42, 1);
      #elif defined(CS43)
        // TODO this may not be correct
        bitWrite(TCCR4B, CS43, 1);
      #endif
      bitWrite(TCCR4B, CS40, 1);
      
      break;
    #endif
  }

  _timer = timer;

  //sei();//enable interrupts
  interrupts();
  
}

void ATmega_TimerInterrupt::set_OCR()
{
  // Run with noInterrupt()
  // Set the OCR for the given timer,
  // set the toggle count,
  // then turn on the interrupts
  uint32_t _OCRValueToUse;

  switch (_timer)
  {
    case 1:
      _OCRValueToUse = min(MAX_COUNT_16BIT, _OCRValueRemaining);
      OCR1A = _OCRValueToUse;
      _OCRValueRemaining -= _OCRValueToUse;

#if defined(OCR1A) && defined(TIMSK1) && defined(OCIE1A)
      // Bit 1 – OCIEA: Output Compare A Match Interrupt Enable
      // When this bit is written to '1', and the I-flag in the Status Register is set (interrupts globally enabled), the
      // Timer/Counter Output Compare A Match interrupt is enabled. The corresponding Interrupt Vector is
      // executed when the OCFA Flag, located in TIFR1, is set.
      bitWrite(TIMSK1, OCIE1A, 1);
#elif defined(OCR1A) && defined(TIMSK) && defined(OCIE1A)
      // this combination is for at least the ATmega32
      bitWrite(TIMSK, OCIE1A, 1);
#endif
      break;

#if defined(OCR2A) && defined(TIMSK2) && defined(OCIE2A)
    case 2:
      _OCRValueToUse = min(MAX_COUNT_8BIT, _OCRValueRemaining);
      OCR2A = _OCRValueToUse;
      _OCRValueRemaining -= _OCRValueToUse;

      bitWrite(TIMSK2, OCIE2A, 1);
      break;
#endif

#if defined(OCR3A) && defined(TIMSK3) && defined(OCIE3A)
    case 3:
      _OCRValueToUse = min(MAX_COUNT_16BIT, _OCRValueRemaining);
      OCR3A = _OCRValueToUse;
      _OCRValueRemaining -= _OCRValueToUse;

      bitWrite(TIMSK3, OCIE3A, 1);
      break;
#endif

#if defined(OCR4A) && defined(TIMSK4) && defined(OCIE4A)
    case 4:
    
      _OCRValueToUse = min(MAX_COUNT_16BIT, _OCRValueRemaining);     
      OCR4A = _OCRValueToUse;
      _OCRValueRemaining -= _OCRValueToUse;

      bitWrite(TIMSK4, OCIE4A, 1);
      break;
#endif
  }

  // Flag _OCRValue == 0 => end of long timer
  if (_OCRValueRemaining == 0)
    _timerDone = true;

}

// frequency (in hertz) and duration (in milliseconds).
// Return true if frequency is OK with selected timer (OCRValue is in range)
bool ATmega_TimerInterrupt::setFrequency(float frequency, timer_callback_p callback, uint32_t params, unsigned long duration)
{
  uint8_t       andMask = 0b11111000;
  unsigned long OCRValue;
  bool isSuccess = false;

  //frequencyLimit must > 1
  float frequencyLimit = frequency * 17179.840;

  // Limit frequency to larger than (0.00372529 / 64) Hz or interval 17179.840s / 17179840 ms to avoid uint32_t overflow
  if ((_timer <= 0) || (callback == NULL) || ((frequencyLimit) < 1) )
  {
    return false;
  }
  else      
  {       
    // Calculate the toggle count. Duration must be at least longer then one cycle
    if (duration > 0)
    {   
      _toggle_count = frequency * duration / 1000;
           
      if (_toggle_count < 1)
      {
        return false;
      }
    }
    else
    {
      _toggle_count = -1;
    }
      
    //Timer0 and timer2 are 8 bit timers, meaning they can store a maximum counter value of 255.
    //Timer2 does not have the option of 1024 prescaler, only 1, 8, 32, 64  
    //Timer1 is a 16 bit timer, meaning it can store a maximum counter value of 65535.
    int prescalerIndexStart;
    
    //Use smallest prescaler first, then increase until fits (<255)
    if (_timer != 2)
    {     
      if (frequencyLimit > 64)
        prescalerIndexStart = NO_PRESCALER;        
      else if (frequencyLimit > 8)
        prescalerIndexStart = PRESCALER_8;
      else
        prescalerIndexStart = PRESCALER_64;

        
      for (int prescalerIndex = prescalerIndexStart; prescalerIndex <= PRESCALER_1024; prescalerIndex++)
      {
        OCRValue = F_CPU / (frequency * prescalerDiv[prescalerIndex]) - 1;
  
        // We use very large _OCRValue now, and every time timer ISR activates, we deduct min(MAX_COUNT_16BIT, _OCRValueRemaining) from _OCRValueRemaining
        // So that we can create very long timer, even if the counter is only 16-bit.
        // Use very high frequency (OCRValue / MAX_COUNT_16BIT) around 16 * 1024 to achieve higher accuracy
    
        if ( (OCRValue / MAX_COUNT_16BIT) < 16384 )
        {
          _OCRValue           = OCRValue;
          _OCRValueRemaining  = OCRValue;
          _prescalerIndex = prescalerIndex;
               
          isSuccess = true;
         
          break;
        }       
      }

      if (!isSuccess)
      {
        // Always do this
        _OCRValue           = OCRValue;
        _OCRValueRemaining  = OCRValue;
        _prescalerIndex = PRESCALER_1024;
      }            
    }
    else
    {
      if (frequencyLimit > 64)
        prescalerIndexStart = T2_NO_PRESCALER;        
      else if (frequencyLimit > 8)
        prescalerIndexStart = T2_PRESCALER_8;
      else if (frequencyLimit > 2)
        prescalerIndexStart = T2_PRESCALER_32;        
      else
        prescalerIndexStart = T2_PRESCALER_64;
          
      // Page 206-207. ATmega328
      //8-bit Timer2 has more options up to 1024 prescaler, from 1, 8, 32, 64, 128, 256 and 1024  
      for (int prescalerIndex = prescalerIndexStart; prescalerIndex <= T2_PRESCALER_1024; prescalerIndex++)
      {
        OCRValue = F_CPU / (frequency * prescalerDivT2[prescalerIndex]) - 1;
  
        // We use very large _OCRValue now, and every time timer ISR activates, we deduct min(MAX_COUNT_8BIT, _OCRValue) from _OCRValue
        // to create very long timer, even if the counter is only 16-bit.
        // Use very high frequency (OCRValue / MAX_COUNT_8BIT) around 16 * 1024 to achieve higher accuracy
        if ( (OCRValue / MAX_COUNT_8BIT) < 16384 )
        {
          _OCRValue           = OCRValue;
          _OCRValueRemaining  = OCRValue;
          // same as prescalarbits
          _prescalerIndex = prescalerIndex;
           
          isSuccess = true;
          
          break;
        }       
      }

      if (!isSuccess)
      {
        // Always do this
        _OCRValue           = OCRValue;
        _OCRValueRemaining  = OCRValue;
        // same as prescalarbits
        _prescalerIndex = T2_PRESCALER_1024;
      } 
    }

    //cli();//stop interrupts
    noInterrupts();

    _frequency = frequency;
    _callback  = (void*) callback;
    _params    = reinterpret_cast<void*>(params);

    _timerDone = false;
          
    // 8 bit timers from here     
    #if defined(TCCR2B)
    if (_timer == 2)
    {
      TCCR2B = (TCCR2B & andMask) | _prescalerIndex;   //prescalarbits;     
    }
    #endif

    // 16 bit timers from here
    #if defined(TCCR1B)
    else if (_timer == 1)
    {
      TCCR1B = (TCCR1B & andMask) | _prescalerIndex;   //prescalarbits;
    }
    #endif
    
    #if defined(TCCR3B)
    else if (_timer == 3)
      TCCR3B = (TCCR3B & andMask) | _prescalerIndex;   //prescalarbits;
    #endif
    
    #if defined(TCCR4B)
    else if (_timer == 4)
      TCCR4B = (TCCR4B & andMask) | _prescalerIndex;   //prescalarbits;
    #endif
             
    // Set the OCR for the given timer,
    // set the toggle count,
    // then turn on the interrupts     
    set_OCR();
    
    //sei();//allow interrupts
    interrupts();

    return true;
  }
}

void ATmega_TimerInterrupt::detachInterrupt(void)
{
  //cli();//stop interrupts
  noInterrupts();
  
  switch (_timer)
  {
    #if defined(TIMSK1) && defined(OCIE1A)
    case 1:
      bitWrite(TIMSK1, OCIE1A, 0);
       
      break;
    #endif

    case 2:
      #if defined(TIMSK2) && defined(OCIE2A)
        bitWrite(TIMSK2, OCIE2A, 0); // disable interrupt
      #endif
            
      break;

#if defined(TIMSK3) && defined(OCIE3A)
    case 3:
      bitWrite(TIMSK3, OCIE3A, 0);
            
      break;
#endif

#if defined(TIMSK4) && defined(OCIE4A)
    case 4:
      bitWrite(TIMSK4, OCIE4A, 0);
            
      break;
#endif
  }
  
  //sei();//allow interrupts
  interrupts();
}

// Duration (in milliseconds). Duration = 0 or not specified => run indefinitely
void ATmega_TimerInterrupt::reattachInterrupt(unsigned long duration)
{
  //cli();//stop interrupts
  noInterrupts();

  // Calculate the toggle count
  if (duration > 0)
  {
    _toggle_count = _frequency * duration / 1000;
  }
  else
  {
    _toggle_count = -1;
  }
    
  switch (_timer)
  {
#if defined(TIMSK1) && defined(OCIE1A)
    case 1:
      bitWrite(TIMSK1, OCIE1A, 1);
       
      break;
#endif

    case 2:
      #if defined(TIMSK2) && defined(OCIE2A)
        bitWrite(TIMSK2, OCIE2A, 1); // enable interrupt
      #endif
            
      break;

#if defined(TIMSK3) && defined(OCIE3A)
    case 3:
      bitWrite(TIMSK3, OCIE3A, 1);
            
      break;
#endif

#if defined(TIMSK4) && defined(OCIE4A)
    case 4:
      bitWrite(TIMSK4, OCIE4A, 1);
            
      break;
#endif
  }
  
  //sei();//allow interrupts
  interrupts();
}

// Just stop clock source, still keep the count
void ATmega_TimerInterrupt::pauseTimer(void)
{
  uint8_t andMask = 0b11111000;
  
  //Just clear the CSx2-CSx0. Still keep the count in TCNT and Timer Interrupt mask TIMKSx. 
  
  // 8 bit timers from here     
  #if defined(TCCR2B)
  if (_timer == 2)
  {
    TCCR2B = (TCCR2B & andMask);
  }
  #endif
  
  // 16 bit timers from here
  #if defined(TCCR1B)
  else if (_timer == 1)
  {
    TCCR1B = (TCCR1B & andMask);
  }
  #endif
  
  #if defined(TCCR3B)
  else if (_timer == 3)
    TCCR3B = (TCCR3B & andMask);
  #endif
  
  #if defined(TCCR4B)
  else if (_timer == 4)
    TCCR4B = (TCCR4B & andMask);
  #endif
}

// Just reconnect clock source, continue from the current count
void ATmega_TimerInterrupt::resumeTimer(void)
{
  uint8_t andMask = 0b11111000;
  
  //Just restore the CSx2-CSx0 stored in _prescalerIndex. Still keep the count in TCNT and Timer Interrupt mask TIMKSx. 
  // 8 bit timers from here     
  #if defined(TCCR2B)
  if (_timer == 2)
  {
    TCCR2B = (TCCR2B & andMask) | _prescalerIndex;   //prescalarbits;
  }
  #endif

  // 16 bit timers from here
  #if defined(TCCR1B)
  else if (_timer == 1)
  {
    TCCR1B = (TCCR1B & andMask) | _prescalerIndex;   //prescalarbits;
  }
  #endif
  
  #if defined(TCCR3B)
  else if (_timer == 3)
    TCCR3B = (TCCR3B & andMask) | _prescalerIndex;   //prescalarbits;
  #endif
  
  #if defined(TCCR4B)
  else if (_timer == 4)
    TCCR4B = (TCCR4B & andMask) | _prescalerIndex;   //prescalarbits;
  #endif
}

//////////////////////////////////////////////

#if USE_TIMER_1 
  #ifndef TIMER1_INSTANTIATED
    // To force pre-instatiate only once
    #define TIMER1_INSTANTIATED
    static ATmega_TimerInterrupt ITimer1(HW_TIMER_1);
    
    // Timer0 is used for micros(), millis(), delay(), etc and can't be used
    // Pre-instatiate

    ISR(TIMER1_COMPA_vect)
    {
      long countLocal = ITimer1.getCount();
           
      if (ITimer1.getTimer() == 1)
      {
        if (countLocal != 0)
        {
          if (ITimer1.checkTimerDone())
          {            
            ITimer1.callback();

            if (ITimer1.get_OCRValue() > MAX_COUNT_16BIT)
            {
              // To reload _OCRValueRemaining as well as _OCR register to MAX_COUNT_16BIT if _OCRValueRemaining > MAX_COUNT_16BIT
              ITimer1.reload_OCRValue();
            }
                 
            if (countLocal > 0)                  
              ITimer1.setCount(countLocal - 1);
          }
          else
          {
            //Deduct _OCRValue by min(MAX_COUNT_16BIT, _OCRValue)
            // If _OCRValue == 0, flag _timerDone for next cycle  
            // If last one (_OCRValueRemaining < MAX_COUNT_16BIT) => load _OCR register _OCRValueRemaining
            ITimer1.adjust_OCRValue();
          }         
        }
        else
        {
          ITimer1.detachInterrupt();
        }
      }
    }
    
  #endif  //#ifndef TIMER1_INSTANTIATED
#endif    //#if USE_TIMER_1

#if USE_TIMER_2
  #ifndef TIMER2_INSTANTIATED
    #define TIMER2_INSTANTIATED
    static ATmega_TimerInterrupt ITimer2(HW_TIMER_2);
    
    ISR(TIMER2_COMPA_vect)
    {
      long countLocal = ITimer2.getCount();
     
      if (ITimer2.getTimer() == 2)
      {
        if (countLocal != 0)
        {
          if (ITimer2.checkTimerDone())
          {
            ITimer2.callback();
            
            if (ITimer2.get_OCRValue() > MAX_COUNT_8BIT)
            {
              // To reload _OCRValueRemaining as well as _OCR register to MAX_COUNT_8BIT if _OCRValueRemaining > MAX_COUNT_8BIT
              ITimer2.reload_OCRValue();
            }
            
            if (countLocal > 0)
             ITimer2.setCount(countLocal - 1);

          }
          else
          {           
            //Deduct _OCRValue by min(MAX_COUNT_8BIT, _OCRValue)
            // If _OCRValue == 0, flag _timerDone for next cycle
            ITimer2.adjust_OCRValue();
          }          
        }    
        else
        {
          ITimer2.detachInterrupt();
        }
      }
    }    
  #endif  //#ifndef TIMER2_INSTANTIATED
#endif    //#if USE_TIMER_2

#if ( TIMER_INTERRUPT_USING_ATMEGA_1284 || TIMER_INTERRUPT_USING_ATMEGA_324PB )

  // Pre-instatiate
  #if USE_TIMER_3
    #ifndef TIMER3_INSTANTIATED
      // To force pre-instatiate only once
      #define TIMER3_INSTANTIATED
      static ATmega_TimerInterrupt ITimer3(HW_TIMER_3);
      
      ISR(TIMER3_COMPA_vect)
      {
        long countLocal = ITimer3.getCount();
        
        if (ITimer3.getTimer() == 3)
        {
          if (countLocal != 0)
          {
            if (ITimer3.checkTimerDone())
            { 
              ITimer3.callback();

              if (ITimer3.get_OCRValue() > MAX_COUNT_16BIT)
              {
                // To reload _OCRValueRemaining as well as _OCR register to MAX_COUNT_16BIT if _OCRValueRemaining > MAX_COUNT_16BIT
                ITimer3.reload_OCRValue();
              }
                           
              if (countLocal > 0)
                ITimer3.setCount(countLocal - 1);     
            }
            else
            {
              //Deduct _OCRValue by min(MAX_COUNT_16BIT, _OCRValue)
              // If _OCRValue == 0, flag _timerDone for next cycle          
              // If last one (_OCRValueRemaining < MAX_COUNT_16BIT) => load _OCR register _OCRValueRemaining
              ITimer3.adjust_OCRValue();
            }
          }
          else
          {
            ITimer3.detachInterrupt();
          }
        }
      }  
      
    #endif  //#ifndef TIMER3_INSTANTIATED
  #endif    //#if USE_TIMER_3

#endif      //#if ( TIMER_INTERRUPT_USING_ATMEGA_1284 || TIMER_INTERRUPT_USING_ATMEGA_324PB )

#if (TIMER_INTERRUPT_USING_ATMEGA_324PB)

  #if USE_TIMER_4
    #ifndef TIMER4_INSTANTIATED
      // To force pre-instatiate only once
      #define TIMER4_INSTANTIATED
      static ATmega_TimerInterrupt ITimer4(HW_TIMER_4);
      
      ISR(TIMER4_COMPA_vect)
      {
        long countLocal = ITimer4.getCount();
                
        if (ITimer4.getTimer() == 4)
        {
          if (countLocal != 0)
          {
            if (ITimer4.checkTimerDone())
            {  
              ITimer4.callback();
              
              if (ITimer4.get_OCRValue() > MAX_COUNT_16BIT)
              {
                // To reload _OCRValueRemaining as well as _OCR register to MAX_COUNT_16BIT if _OCRValueRemaining > MAX_COUNT_16BIT
                ITimer4.reload_OCRValue();
              }
                           
              if (countLocal > 0)
                ITimer4.setCount(countLocal - 1);       
            }
            else
            {
              //Deduct _OCRValue by min(MAX_COUNT_16BIT, _OCRValue) or min(MAX_COUNT_8BIT, _OCRValue)
              // If _OCRValue == 0, flag _timerDone for next cycle     
              // If last one (_OCRValueRemaining < MAX_COUNT_16BIT / MAX_COUNT_8BIT) => load _OCR register _OCRValueRemaining
              ITimer4.adjust_OCRValue();
            }
          }
          else
          {
            ITimer4.detachInterrupt();
          }
        }
      }
      
      
    #endif  //#ifndef TIMER4_INSTANTIATED
  #endif    //#if USE_TIMER_4

#endif      //#if (TIMER_INTERRUPT_USING_ATMEGA_324PB)

#endif // ATmega_TimerInterrupt_Impl_h
