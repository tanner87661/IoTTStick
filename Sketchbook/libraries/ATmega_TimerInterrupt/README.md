## ATmega_TimerInterrupt Library

[![arduino-library-badge](https://www.ardu-badge.com/badge/ATmega_TimerInterrupt.svg?)](https://www.ardu-badge.com/ATmega_TimerInterrupt)
[![GitHub release](https://img.shields.io/github/release/khoih-prog/ATmega_TimerInterrupt.svg)](https://github.com/khoih-prog/ATmega_TimerInterrupt/releases)
[![GitHub](https://img.shields.io/github/license/mashape/apistatus.svg)](https://github.com/khoih-prog/ATmega_TimerInterrupt/blob/main/LICENSE)
[![contributions welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat)](#Contributing)
[![GitHub issues](https://img.shields.io/github/issues/khoih-prog/ATmega_TimerInterrupt.svg)](http://github.com/khoih-prog/ATmega_TimerInterrupt/issues)

<a href="https://www.buymeacoffee.com/khoihprog6" title="Donate to my libraries using BuyMeACoffee"><img src="https://cdn.buymeacoffee.com/buttons/v2/default-yellow.png" alt="Donate to my libraries using BuyMeACoffee" style="height: 50px !important;width: 181px !important;" ></a>
<a href="https://www.buymeacoffee.com/khoihprog6" title="Donate to my libraries using BuyMeACoffee"><img src="https://img.shields.io/badge/buy%20me%20a%20coffee-donate-orange.svg?logo=buy-me-a-coffee&logoColor=FFDD00" style="height: 20px !important;width: 200px !important;" ></a>

---
---

## Table of Contents

* [Why do we need this ATmega_TimerInterrupt library](#why-do-we-need-this-ATmega_TimerInterrupt-library)
  * [Features](#features)
  * [Why using ISR-based Hardware Timer Interrupt is better](#why-using-isr-based-hardware-timer-interrupt-is-better)
  * [Important Notes about ISR](#important-notes-about-isr)
  * [Currently supported Boards](#currently-supported-boards)
* [Changelog](changelog.md)
* [Prerequisites](#prerequisites)
* [Installation](#installation)
  * [Use Arduino Library Manager](#use-arduino-library-manager)
  * [Manual Install](#manual-install)
  * [VS Code & PlatformIO](#vs-code--platformio)
* [HOWTO Fix `Multiple Definitions` Linker Error](#howto-fix-multiple-definitions-linker-error)
* [More useful Information](#more-useful-information)
  * [1. Timer0](#1-timer0)
  * [2. Timer1](#2-timer1)
  * [3. Timer2](#3-timer2)
  * [4. Timer3 and Timer4](#4-timer3-and-timer4)
  * [5. Important Notes](#5-important-notes)
* [Usage](#usage)
  * [1. Using only Hardware Timer directly](#1-using-only-hardware-timer-directly)
    * [1.1 Init Hardware Timer](#11-init-hardware-timer)
    * [1.2 Set Hardware Timer Interval and attach Timer Interrupt Handler function](#12-set-hardware-timer-interval-and-attach-timer-interrupt-handler-function)
    * [1.3 Set Hardware Timer Frequency and attach Timer Interrupt Handler function](#13-set-hardware-timer-frequency-and-attach-timer-interrupt-handler-function)
  * [2. Using 16 ISR_based Timers from 1 Hardware Timer](#2-using-16-isr_based-timers-from-1-hardware-timer)
    * [2.1 Important Note](#21-important-note)
    * [2.2 Init Hardware Timer and ISR-based Timer](#22-init-hardware-timer-and-isr-based-timer)
    * [2.3 Set Hardware Timer Interval and attach Timer Interrupt Handler functions](#23-set-hardware-timer-interval-and-attach-timer-interrupt-handler-functions)
* [Examples](#examples)
  * [  1. Argument_Complex](examples/Argument_Complex)
  * [  2. Argument_None](examples/Argument_None)
  * [  3. Argument_Simple](examples/Argument_Simple)
  * [  4. Change_Interval](examples/Change_Interval)
  * [  5. FakeAnalogWrite](examples/FakeAnalogWrite)
  * [  6. ISR_16_Timers_Array_Complex](examples/ISR_16_Timers_Array_Complex)
  * [  7. ISR_RPM_Measure](examples/ISR_RPM_Measure)
  * [  8. ISR_Timers_Array_Simple](examples/ISR_Timers_Array_Simple)
  * [  9. RPM_Measure](examples/RPM_Measure)
  * [ 10. SwitchDebounce](examples/SwitchDebounce)
  * [ 11. TimerDuration](examples/TimerDuration)
  * [ 12. ATmega_TimerInterruptTest](examples/ATmega_TimerInterruptTest)
  * [ 13. Change_Interval_HF](examples/Change_Interval_HF).
  * [ 14. Argument_Complex_Multi](examples/Argument_Complex_Multi). **New**
* [Example ISR_16_Timers_Array_Complex](#example-isr_16_timers_array_complex)
* [Debug](#debug)
* [Troubleshooting](#troubleshooting)
* [Issues](#issues)
* [TO DO](#to-do)
* [DONE](#done)
* [Contributions and Thanks](#contributions-and-thanks)
* [Contributing](#contributing)
* [License](#license)
* [Copyright](#copyright)

---
---

### Why do we need this [ATmega_TimerInterrupt library](https://github.com/khoih-prog/ATmega_TimerInterrupt)

### Features

This library enables you to use Interrupt from Hardware Timers on AVR **ATmega164, ATmega324, ATmega644, ATmega1284 using MCUdude MightyCore**

As **Hardware Timers are rare, and very precious assets** of any board, this library now enables you to use up to **16 ISR-based Timers, while consuming only 1 Hardware Timer**. Timers' interval is very long (**ulong millisecs**).

Now with these new **16 ISR-based timers**, the maximum interval is **practically unlimited** (limited only by unsigned long miliseconds) while **the accuracy is nearly perfect** compared to software timers. 

The most important feature is they're ISR-based timers. Therefore, their executions are **not blocked by bad-behaving functions / tasks**. This important feature is absolutely necessary for mission-critical tasks. 

The [**ISR_Timer_Complex**](examples/ISR_Timer_Complex) example will demonstrate the nearly perfect accuracy compared to software timers by printing the actual elapsed millisecs of each type of timers.

Being ISR-based timers, their executions are not blocked by bad-behaving functions / tasks, such as connecting to WiFi, Internet and Blynk services. You can also have many `(up to 16)` timers to use.

This non-being-blocked important feature is absolutely necessary for mission-critical tasks.

You'll see blynkTimer Software is blocked while system is connecting to WiFi / Internet / Blynk, as well as by blocking task 
in loop(), using delay() function as an example. The elapsed time then is very unaccurate

### Why using ISR-based Hardware Timer Interrupt is better

Imagine you have a system with a **mission-critical function**, measuring water level and control the sump pump or doing something much more important. You normally use a **software timer to poll**, or even place the function in loop(). But what if another function is blocking the loop() or setup().

**So your function might not be executed, and the result would be disastrous.**

You'd prefer to have your function called, no matter what happening with other functions (busy loop, bug, etc.).

The correct choice is to use a **Hardware Timer with Interrupt** to call your function.

**These hardware timers, using interrupt**, still work even if other functions are blocking. Moreover, they are **much more precise** (certainly depending on clock frequency accuracy) than other software timers using millis() or micros(). That's necessary if you need to measure some data requiring better accuracy.

Functions using normal software timers, relying on loop() and calling millis(), won't work if the loop() or setup() is blocked by certain operation. For example, certain function is blocking while it's connecting to WiFi or some services.

The catch is your function is now part of an ISR (Interrupt Service Routine), and must be lean / mean, and follow certain rules. More to read on:

[**HOWTO Attach Interrupt**](https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/)

### Important Notes about ISR

1. Inside the attached function, delay() won’t work and the value returned by millis() will not increment. Serial data received while in the function may be lost. You should declare as volatile any variables that you modify within the attached function.

2. Typically global variables are used to pass data between an ISR and the main program. To make sure variables shared between an ISR and the main program are updated correctly, declare them as volatile.

### Currently supported Boards

- **ATmega164(A/P), ATmega324(A/P/PA/PB), ATmega644(A/P), ATmega1284(P)**

### Not yet supported Boards

- **ATmega8535, ATmega16 and ATmega32**

---
---


## Prerequisites

1. [`Arduino IDE 1.8.19+` for Arduino](https://www.arduino.cc/en/Main/Software)
2. [`MCUdude MightyCore v2.1.3+`](https://github.com/MCUdude/MightyCore) for **ATmega164, ATmega324, ATmega644, ATmega1284**. Use Arduino Board Manager to install. [![Latest release](https://img.shields.io/github/release/MCUdude/MightyCore.svg)](https://github.com/MCUdude/MightyCore/releases/latest/)
3. To use with certain example
   - [`SimpleTimer library`](https://github.com/jfturcot/SimpleTimer) for [ISR_Timers_Array_Simple](examples/ISR_Timers_Array_Simple) and [ISR_16_Timers_Array_Complex](examples/ISR_16_Timers_Array_Complex) examples.

---
---

## Installation

### Use Arduino Library Manager

The best and easiest way is to use `Arduino Library Manager`. Search for [**ATmega_TimerInterrupt**](https://github.com/khoih-prog/ATmega_TimerInterrupt), then select / install the latest version.
You can also use this link [![arduino-library-badge](https://www.ardu-badge.com/badge/ATmega_TimerInterrupt.svg?)](https://www.ardu-badge.com/ATmega_TimerInterrupt) for more detailed instructions.

### Manual Install

Another way to install is to:

1. Navigate to [**ATmega_TimerInterrupt**](https://github.com/khoih-prog/ATmega_TimerInterrupt) page.
2. Download the latest release `ATmega_TimerInterrupt-main.zip`.
3. Extract the zip file to `ATmega_TimerInterrupt-main` directory 
4. Copy whole `ATmega_TimerInterrupt-main` folder to Arduino libraries' directory such as `~/Arduino/libraries/`.

### VS Code & PlatformIO:

1. Install [VS Code](https://code.visualstudio.com/)
2. Install [PlatformIO](https://platformio.org/platformio-ide)
3. Install [**ATmega_TimerInterrupt** library](https://registry.platformio.org/libraries/khoih-prog/ATmega_TimerInterrupt) by using [Library Manager](https://registry.platformio.org/libraries/khoih-prog/ATmega_TimerInterrupt/installation). Search for ATmega_TimerInterrupt in [Platform.io Author's Libraries](https://platformio.org/lib/search?query=author:%22Khoi%20Hoang%22)
4. Use included [platformio.ini](platformio/platformio.ini) file from examples to ensure that all dependent libraries will installed automatically. Please visit documentation for the other options and examples at [Project Configuration File](https://docs.platformio.org/page/projectconf.html)

---
---

### HOWTO Fix `Multiple Definitions` Linker Error

The current library implementation, using `xyz-Impl.h` instead of standard `xyz.cpp`, possibly creates certain `Multiple Definitions` Linker error in certain use cases.

You can use

```
#include <ATmega_TimerInterrupt.hpp>	//https://github.com/khoih-prog/ATmega_TimerInterrupt
#include <ISR_Timer.hpp>              //https://github.com/khoih-prog/ATmega_TimerInterrupt
```

in many files. But be sure to use the following `#include <ATmega_TimerInterrupt.h>` or `#include <ISR_Timer.h>` **in just 1 `.h`, `.cpp` or `.ino` file**, which must **not be included in any other file**, to avoid `Multiple Definitions` Linker Error

```
// To be included only in main(), .ino with setup() to avoid `Multiple Definitions` Linker Error
#include "ATmega_TimerInterrupt.h"		//https://github.com/khoih-prog/ATmega_TimerInterrupt

// To be included only in main(), .ino with setup() to avoid `Multiple Definitions` Linker Error
#include "ISR_Timer.h"                //https://github.com/khoih-prog/ATmega_TimerInterrupt
```

Check new [**Argument_Complex_Multi** example](examples/Argument_Complex_Multi) for the demo how to avoid `Multiple Definitions` Linker Error. 

---
---

## More useful Information

From [Arduino 101: Timers and Interrupts](https://www.robotshop.com/community/forum/t/arduino-101-timers-and-interrupts/13072)

### 1. Timer0:

Timer0 is a 8-bit timer.

In the Arduino world, **Timer0 is used for the timer functions**, like delay(), millis() and micros(). If you change **Timer0** registers, this may influence the Arduino timer function. So you should know what you are doing.

### 2. Timer1:

**Timer1** is a 16-bit timer. In the Arduino world, the Servo library uses **Timer1**

### 3. Timer2:

**Timer2** is a 8-bit timer like **Timer0**

In the Arduino world, the **tone() function uses Timer2**.

### 4. Timer3 and Timer4:

**Timer3** is only available on Arduino **ATMEGA_1284(P), ATMEGA_324PB boards**

**Timer4** is only available on Arduino **ATMEGA_324PB boards**

### 5. Important Notes

Before using any Timer, you have to make sure the **Timer has not been used by any other purpose**

- **Timer1 and Timer2 are supported for ATmega164(A/P), ATmega324(A/P/PA), ATmega644(A/P)**

- **Timer1, Timer2 and Timer3 are supported for ATmega1284(P)**

- **Timer1, Timer2, Timer3 and Timer4 are supported for ATmega324PB, which is not yet supported by MightyCore v2.1.3**


---
---

## Usage

Before using any Timer, you have to make sure the Timer has not been used by any other purpose.

### 1. Using only Hardware Timer directly

### 1.1 Init Hardware Timer

```
// Select the timers you're using, here ITimer1
#define USE_TIMER_1     true
#define USE_TIMER_2     false
#define USE_TIMER_3     false
#define USE_TIMER_4     false

// Init timer ITimer1
ITimer1.init();
```

### 1.2 Set Hardware Timer Interval and attach Timer Interrupt Handler function

Use one of these functions with **interval in unsigned long milliseconds**

```
// interval (in ms) and duration (in milliseconds). Duration = 0 or not specified => run indefinitely
template<typename TArg> bool setInterval(unsigned long interval, void (*callback)(TArg), TArg params, unsigned long duration = 0);

// interval (in ms) and duration (in milliseconds). Duration = 0 or not specified => run indefinitely
bool setInterval(unsigned long interval, timer_callback callback, unsigned long duration = 0);

// Interval (in ms) and duration (in milliseconds). Duration = 0 or not specified => run indefinitely
template<typename TArg> bool attachInterruptInterval(unsigned long interval, void (*callback)(TArg), TArg params, unsigned long duration = 0);

// Interval (in ms) and duration (in milliseconds). Duration = 0 or not specified => run indefinitely
bool attachInterruptInterval(unsigned long interval, timer_callback callback, unsigned long duration = 0)
```

as follows

```
void TimerHandler()
{
  // Doing something here inside ISR
}

#define TIMER_INTERVAL_MS        50L

void setup()
{
  ....
  
  // Interval in unsigned long millisecs
  if (ITimer.attachInterruptInterval(TIMER_INTERVAL_MS, TimerHandler))
    Serial.println("Starting ITimer OK, millis() = " + String(millis()));
  else
    Serial.println("Can't set ITimer. Select another freq. or timer");
}  
```

### 1.3 Set Hardware Timer Frequency and attach Timer Interrupt Handler function

Use one of these functions with **frequency in float Hz**

```
// frequency (in hertz) and duration (in milliseconds). Duration = 0 or not specified => run indefinitely
bool setFrequency(float frequency, timer_callback_p callback, /* void* */ uint32_t params, unsigned long duration = 0);

// frequency (in hertz) and duration (in milliseconds). Duration = 0 or not specified => run indefinitely
bool setFrequency(float frequency, timer_callback callback, unsigned long duration = 0);

// frequency (in hertz) and duration (in milliseconds). Duration = 0 or not specified => run indefinitely
template<typename TArg> bool attachInterrupt(float frequency, void (*callback)(TArg), TArg params, unsigned long duration = 0);

// frequency (in hertz) and duration (in milliseconds). Duration = 0 or not specified => run indefinitely
bool attachInterrupt(float frequency, timer_callback callback, unsigned long duration = 0);
```

as follows

```
void TimerHandler()
{
  // Doing something here inside ISR
}

#define TIMER_FREQ_HZ        5555.555

void setup()
{
  ....
  
  // Frequency in float Hz
  if (ITimer.attachInterrupt(TIMER_FREQ_HZ, TimerHandler))
    Serial.println("Starting  ITimer OK, millis() = " + String(millis()));
  else
    Serial.println("Can't set ITimer. Select another freq. or timer");
}  
```


### 2. Using 16 ISR_based Timers from 1 Hardware Timer

### 2.1 Important Note

The 16 ISR_based Timers, designed for long timer intervals, only support using **unsigned long millisec intervals**. If you have to use much higher frequency or sub-millisecond interval, you have to use the Hardware Timers directly as in [1.3 Set Hardware Timer Frequency and attach Timer Interrupt Handler function](#13-set-hardware-timer-frequency-and-attach-timer-interrupt-handler-function)

### 2.2 Init Hardware Timer and ISR-based Timer

```
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

// Init ISR_Timer
// Each ISR_Timer can service 16 different ISR-based timers
ISR_Timer ISR_timer;
```

### 2.3 Set Hardware Timer Interval and attach Timer Interrupt Handler functions

```
void TimerHandler()
{
  ISR_timer.run();
}

#define HW_TIMER_INTERVAL_MS          50L

#define TIMER_INTERVAL_2S             2000L
#define TIMER_INTERVAL_5S             5000L
#define TIMER_INTERVAL_11S            11000L
#define TIMER_INTERVAL_101S           101000L

// In AVR, avoid doing something fancy in ISR, for example complex Serial.print with String() argument
// The pure simple Serial.prints here are just for demonstration and testing. Must be eliminate in working environment
// Or you can get this run-time error / crash
void doingSomething2s()
{
  // Doing something here inside ISR every 2 seconds
}
  
void doingSomething5s()
{
  // Doing something here inside ISR every 5 seconds
}

void doingSomething11s()
{
  // Doing something here inside ISR  every 11 seconds
}

void doingSomething101s()
{
  // Doing something here inside ISR every 101 seconds
}

void setup()
{
  ....
  
	CurrentTimer.init();

  // Using ATmega324 with 16MHz CPU clock ,
  // For 16-bit timer 1, set frequency from 0.2385 to some KHz
  // For 8-bit timer 2 (prescaler up to 1024, set frequency from 61.5Hz to some KHz

  if (CurrentTimer.attachInterruptInterval(TIMER_INTERVAL_MS, TimerHandler))
  {
    Serial.print(F("Starting ITimer OK, millis() = ")); Serial.println(millis());
  }
  else
    Serial.println(F("Can't set ITimer. Select another freq. or timer"));

  // Just to demonstrate, don't use too many ISR Timers if not absolutely necessary
  // You can use up to 16 timer for each ISR_Timer
  ISR_timer.setInterval(TIMER_INTERVAL_2S, doingSomething2s);
  ISR_timer.setInterval(TIMER_INTERVAL_5S, doingSomething5s);
  ISR_timer.setInterval(TIMER_INTERVAL_11S, doingSomething11s);
  ISR_timer.setInterval(TIMER_INTERVAL_101S, doingSomething101s);
}  
```

---
---

### Examples: 

 1. [Argument_Complex](examples/Argument_Complex)
 2. [Argument_None](examples/Argument_None)
 3. [Argument_Simple](examples/Argument_Simple)
 4. [Change_Interval](examples/Change_Interval)
 5. [FakeAnalogWrite](examples/FakeAnalogWrite)
 6. [ISR_16_Timers_Array_Complex](examples/ISR_16_Timers_Array_Complex)
 7. [ISR_RPM_Measure](examples/ISR_RPM_Measure)
 8. [ISR_Timers_Array_Simple](examples/ISR_Timers_Array_Simple)
 9. [RPM_Measure](examples/RPM_Measure)
10. [SwitchDebounce](examples/SwitchDebounce)
11. [TimerDuration](examples/TimerDuration)
12. [ATmega_TimerInterruptTest](examples/ATmega_TimerInterruptTest)
13. [**Change_Interval_HF**](examples/Change_Interval_HF).
14. [**Argument_Complex_Multi**](examples/Argument_Complex_Multi). **New**

---

### Example [ISR_16_Timers_Array_Complex](examples/ISR_16_Timers_Array_Complex)

https://github.com/khoih-prog/ATmega_TimerInterrupt/blob/f3b27af3eba6de4dd56ae49f04e4a8f8c3b18f9d/examples/ISR_16_Timers_Array_Complex/ISR_16_Timers_Array_Complex.ino#L25-L377

---
---


### Debug

Debug is enabled by default on Serial.

You can also change the debugging level from 0 to 3

```cpp
// These define's must be placed at the beginning before #include "ATmega_TimerInterrupt.h"
// _TIMERINTERRUPT_LOGLEVEL_ from 0 to 4
// Don't define _TIMERINTERRUPT_LOGLEVEL_ > 0. Only for special ISR debugging only. Can hang the system.
#define TIMER_INTERRUPT_DEBUG         0
#define _TIMERINTERRUPT_LOGLEVEL_     0
```

---

### Troubleshooting

If you get compilation errors, more often than not, you may need to install a newer version of the core for Arduino boards.

Sometimes, the library will only work if you update the board core to the latest version because I am using newly added functions.


---
---


### Issues

Submit issues to: [ATmega_TimerInterrupt issues](https://github.com/khoih-prog/ATmega_TimerInterrupt/issues)

---
---

### TO DO

1. Search for bug and improvement.


### DONE

 1. Longer Interval for timers.
 2. Reduce code size if use less timers. Eliminate compiler warnings.
 3. Now supporting complex object pointer-type argument.
 3. 16 hardware-initiated software-enabled timers while using only 1 hardware timer.
 4. Fix some bugs in v1.0.0
 5. Add more examples.
 6. Add support to **ATmega164(A/P), ATmega324(A/P/PA/PB), ATmega644(A/P), ATmega1284(P)**-based boards
 7. Optimize code in examples
 8. Fix bug possibly causing system crash when using `_TIMERINTERRUPT_LOGLEVEL_ > 0` 

---
---

### Contributions and Thanks

Many thanks for everyone for bug reporting, new feature suggesting, testing and contributing to the development of this library. Especially to these people who have directly or indirectly contributed to this [ATmega_TimerInterrupt library](https://github.com/khoih-prog/ATmega_TimerInterrupt)

1. Thanks to good work of [Hans](https://github.com/MCUdude) for the [MightyCore](https://github.com/MCUdude/MightyCore)
2. Thanks to [LaurentR59](https://github.com/LaurentR59) to request the enhancement [Support for DX CORE CPU and MightyCORE CPU possible? #8](https://github.com/khoih-prog/TimerInterrupt_Generic/issues/8) leading to this new library.


<table>
  <tr>
    <td align="center"><a href="https://github.com/MCUdude"><img src="https://github.com/MCUdude.png" width="100px;" alt="MCUdude"/><br /><sub><b>⭐️⭐️Hans</b></sub></a><br /></td>
    <td align="center"><a href="https://github.com/LaurentR59"><img src="https://github.com/LaurentR59.png" width="100px;" alt="LaurentR59"/><br /><sub><b>LaurentR59</b></sub></a><br /></td>
  </tr> 
</table>

---

## Contributing

If you want to contribute to this project:
- Report bugs and errors
- Ask for enhancements
- Create issues and pull requests
- Tell other people about this library

---

## License

- The library is licensed under [MIT](https://github.com/khoih-prog/ATmega_TimerInterrupt/blob/main/LICENSE)

---

## Copyright

Copyright 2022- Khoi Hoang


