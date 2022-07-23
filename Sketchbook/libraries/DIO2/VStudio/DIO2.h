//-------------------------------------------------------------------
#ifndef DIO2_H_
#define DIO2_H_
//-------------------------------------------------------------------

#include "stdafx.h"
#include "Arduino.h"

#if defined(ARDUINO_ARCH_AVR)

// Definitions specific for selected board
#if defined(__AVR_ATmega2560__)
#include "../board/mega/pins2_arduino.h"
#else
#if defined(__AVR_ATmega328P__)
#include "../board/nano/pins2_arduino.h"
#else
#if defined(__AVR_ATmega32U4__)
#include "../board/leonardo/pins2_arduino.h"
#else
#include "../board/standard/pins2_arduino.h"
#endif
#endif
#endif

//#include "../board/mega/pins2_arduino.h"

extern void pinMode2(uint8_t pin, uint8_t mode);
extern void digitalWrite2(uint8_t pin, uint8_t val);
extern uint8_t digitalRead2(uint8_t pin);

extern void pinMode2f(GPIO_pin_t pin, uint8_t mode);
extern uint8_t digitalRead2f(GPIO_pin_t pin);
extern void digitalWrite2f(GPIO_pin_t pin, uint8_t value);

#else

// ARM / ESP / STM / RP2040 versions
// These procs are probably fast enough to access ports very efficiency (and i dont know how to make better !) .
// So the functions are dummies, and conduct to original wiring functions...

typedef int GPIO_pin_t;

/* For RP2040, these functions can also be coded like this:
	static inline void pinMode2(uint8_t pin, uint8_t val) { pinMode(pin, val);}
	static inline void digitalWrite2(uint8_t pin, uint8_t val) { uint32_t mask = 1ul << pin; if (val) gpio_set_mask(mask); else gpio_clr_mask(mask); }
	static inline uint8_t digitalRead2(uint8_t pin) { return gpio_get(pin);}
*/

#define pinMode2(P, M)			pinMode(P, M)
#define digitalWrite2(P, V)		digitalWrite(P, V)
#define digitalRead2(P)			digitalRead(P)

#define pinMode2f(P, M)			pinMode(P, M)
#define digitalWrite2f(P, V)	digitalWrite(P, V)
#define digitalRead2f(P)		digitalRead(P)

#define DP_INVALID	0xFFFF
#define DP0	0
#define DP1	1
#define DP2	2
#define DP3	3
#define DP4	4
#define DP5	5
#define DP6	6
#define DP7	7

//#include "Arduino.h"

#ifdef F
#undef F
#endif

#define F(str)	str
#define __FlashStringHelper		char
#endif

extern int GPIO_to_Arduino_pin(GPIO_pin_t aa);
extern GPIO_pin_t Arduino_to_GPIO_pin(int aa);

//-------------------------------------------------------------------				
#endif
//-------------------------------------------------------------------
