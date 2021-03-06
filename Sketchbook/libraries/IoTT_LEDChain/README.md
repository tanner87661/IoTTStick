# Esp32 FastLED LED Chain for CTC Panels, Signals, etc.

To include in the application, add:

#include <IoTT_LEDChain.h> //as introduced in video # 30

Note: IoTT_LEDChain is based on FastLED, so FastLED must be installed in the Arduino IDE.
Note: IoTT_LEDChain gets status information from IoTT_DigitraxBuffers, so this library must be available as well

Define a LEDChain variable:

IoTT_ledChain myChain;

and initialize FastLED and myChain in the setup() function:

myChain.loadLEDChainJSON(*jsonDataObj);

jsonDataObj is of type DynamicJsonDocument and has an initialized JSON dataset that defines the settings. See the example section for the syntax of the JSON data

Define the data pin that drives the chain and initialize FastLED:

void setup()
{
	const char LED_DATA_PIN = 12;//GPIO pin Din of of the first pixel of the chain is connected to
	FastLED.addLeds<WS2811, LED_DATA_PIN, GRB>(myChain.getChain(), myChain.getChainLength());
	
	//other stuff
}

In your Loop function, call processChain() to update the LED's

void loop()
{
	myChain.processChain();
	
	//other stuff
}

