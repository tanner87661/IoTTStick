#define ARDUINO2_MAIN

#include "stdafx.h"
#include "DIO2.hpp"
#include "DIO2.h"
#include "ArduiEmulator.hpp"

#ifdef ARDUINO_ARCH_AVR
void pinMode2(uint8_t pin, uint8_t mode)
{
	pinMode(pin, mode);
}

void digitalWrite2(uint8_t pin, uint8_t val)
{
	digitalWrite(pin, val);
}

uint8_t digitalRead2(uint8_t pin)
{
	return digitalRead(pin);
}

void pinMode2f(GPIO_pin_t pin, uint8_t mode)
{
	pinMode(GPIO_to_Arduino_pin(pin), mode);
}

uint8_t digitalRead2f(GPIO_pin_t pin)
{
	return digitalRead(GPIO_to_Arduino_pin(pin));
}

void digitalWrite2f(GPIO_pin_t pin, uint8_t value)
{
	digitalWrite(GPIO_to_Arduino_pin(pin), value);
}	

int GPIO_to_Arduino_pin(GPIO_pin_t inPin)
{
	int i;
	for (i = 0; i < GPIO_PINS_NUMBER; i++)
	{
		if (inPin == gpio_pins_progmem[i])
			return i;
	}

	return -1;
}

GPIO_pin_t Arduino_to_GPIO_pin(int inPin)
{
	return gpio_pins_progmem[inPin];
}
#else
int GPIO_to_Arduino_pin(GPIO_pin_t inPin)
{
	return inPin;
}

GPIO_pin_t Arduino_to_GPIO_pin(int inPin)
{
	return inPin;
}

#endif
