/*
 * pins2_arduino.h
 *
 *  Created on: 5. 3. 2015
 *      Author: Jan Dolinay
 *
 *  Alternate version of digital input/output for Arduino.
 *  This is dummy file which can be used for Arduino variants which are not
 *  supported by this library to allow building and executing without errors.
 *
 *
  This is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 3 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
 *
 */

#ifndef ADRUINO_PINS2_H_
#define ADRUINO_PINS2_H_


// ===========================================================================
// Configuration
// ===========================================================================
// GPIO2_PREFER_SPEED - User can define this value before including arduino2.h
// or this default value is used.
// Set to 1 to make all digital I/O functions inline.
// This will make them work faster but the resulting code will be bigger.
// In general, you can start with 1 and if you are running out of program memory
// switch to 0.
//
#ifndef GPIO2_PREFER_SPEED
	#define		GPIO2_PREFER_SPEED	1
#endif


// GPIO2_IOREGS_ABOVEFF - set to one if the Atmega MCU used for this Arduino 
// variant has some I/O registers at address higher than 0xff 
// (e.g. Atmega2560 in Arduino Mega).
// In this case we must always disable interrupts in digitalWrite and pinMode,
// because the registers will not be manipulated by single instruction (SBI, CBI),
// because these instructions only work on locations below 0xFF.
//
// For Arduino Standard (ATmega328) set to 0
// For Arduino Mega (ATmega1280 or ATmega2560) set to 1
// If not sure, set to 1 (this is always safe option)
//
#define	  GPIO2_IOREGS_ABOVEFF	0


// Unfortunately we cannot use the PORTA etc. definitions from avr/io.h in the
// GPIO_MAKE_PINCODE macro, so I define the port register addresses here to make
// it more comfortable to define the pins
// This is the address of the port register, e.g. PORTA or PORTB, from the datasheet.
#define		MYPORTB		(0x25)
#define		MYPORTC		(0x28)
#define		MYPORTD		(0x2B)


/*
 * GPIO_pin_t
 * Define the type for digital I/O pin.
 * We will not use simple integer (int) to identify a pin.
 * Instead we use special code, which contains the address of the I/O register
 * for given pin (lower byte) together with its bit mask (upper byte).
 * For this code we create our special data type (GPIO_pin_t) which will prevent
 * the user from calling our digitalRead/Write with invalid pin numbers.
 *
*/
enum GPIO_pin_enum
{
	// Note: The invalid value can be 0 which means digitalWrite will write to
    // reserved address on Atmega 328 used in Arduino Uno,
	// or it can be any valid port register - as long as the bit mask in upper 
    // byte is 0, the operation on this register will have no effect.
	DP_INVALID = 0x0025,
};

typedef	enum GPIO_pin_enum  GPIO_pin_t;

// Number of GPIO pins.
// Used in Arduino_to_GPIO_pin function
#define		GPIO_PINS_NUMBER		(0)

// Macro to obtain bit mask of a pin from its code
#define		GPIO_PIN_MASK(pin)		((uint8_t)((uint16_t)pin >> 8))

// Macros to obtain the addresses of various I/O registers from pin code
#define		GET_PORT_REG_ADR(pin)		((volatile uint8_t*)((pin) & 0x00FF))
#define		GET_PIN_REG_ADR(pin)		(GET_PORT_REG_ADR(pin)-2)
#define		GET_DDR_REG_ADR(pin)		(GET_PORT_REG_ADR(pin)-1)

// Macros which allow us to refer to the I/O registers directly
#define   GPIO_PIN_REG(pin)    (*(volatile uint8_t*)GET_PIN_REG_ADR(pin) )
#define   GPIO_PORT_REG(pin)    (*(volatile uint8_t*)GET_PORT_REG_ADR(pin) )
#define   GPIO_DDR_REG(pin)    (*(volatile uint8_t*)GET_DDR_REG_ADR(pin) )


// ARDUINO2_MAIN should be defined only once in the program so that the
// gpio_pins_progmem is not duplicated. This is done in digital2.c
#ifdef ARDUINO2_MAIN

// This array in program memory (FLASH) maps Arduino pin number (simple integer)
// to our GPIO pin definition.
// The Arduino pin number is used as index into this array. Value at given index
// N is the pin code for the Arduino pin N.
const GPIO_pin_t PROGMEM gpio_pins_progmem[] = {
			DP_INVALID,
};
#else
	extern const GPIO_pin_t PROGMEM gpio_pins_progmem[];
#endif


#endif /* ADRUINO_PINS2_H_ */
