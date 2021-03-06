CD74HC4067 Button library for the ESP32 
Breakout Board: https://www.sparkfun.com/products/9056

Arduino platform

This library polls 16 input lines of up to 4 4064 analog MUX and returns button events or analog values.

Each channel can be set to Touch Input, Analog Input, Digital Input, or Autodetect. 

In order to make Autodetect work properly, digital inputs must be connected to an external pullup or pulldown resistor. 
If left open, the input will be interpreted as touch input.

For button inputs, the library generates five different button events: Button Down, Button Hold (every 3 seconds, interval is adjustable), 
Button Up, Button Clicked, or Button Double-Clicked when button is clicked twice in an adjustable time interval

For analog inputs, an analog event is created as soon as the inpput value deviates more than 5% from the previously announced value.
If no change occurs, the analog event is repeated periodically. Also, the number of events per channel is limited to avoid overloading
of the communication network (typically LocoNet)

The application should implement two callback functions to receive the events:

onButtonEvent(uint16_t btnAddr, buttonEvent btnEvent);
onAnalogData(uint16_t inpAddr, uint16_t analogValue );
 
defined in IoTT_ButtonTypeDef.h
	enum buttonEvent : byte {onbtndown=0, onbtnup=1, onbtnclick=2, onbtndblclick=4, onbtnhold=3, noevent=255};

The application must define 4 address pins to select one of 1`6 input channels of the MUX:

#define ADDR_0 18
#define ADDR_1 5
#define ADDR_2 17
#define ADDR_3 16

Then, it must define how many MUX are used in parallel. Up to 4 devices are possible, resulting in 64 input lines

#define NUM_DATA_PORTS 2  //number of analog lines used for buttons. Each line gives 16 buttons, maximum is 4 lines
#define DATA_0 15         //analog line numbers
#define DATA_1 35         //analog line numbers
  
uint8_t analogPins[] = {NUM_DATA_PORTS, DATA_0, DATA_1}; //byte 0 is number of defined pins to follow

Finally, the library can be initialized 

IoTT_Mux64Buttons myButtons;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1000);
  Serial.println("Initializing...");
  myButtons.initButtons(ADDR_0,ADDR_1, ADDR_2, ADDR_3, &analogPins[0], true);
}

The last parameter indicates whether WiFi of the ESP32 is used or not, as this limit the usage of some pins for analog data

Pin modes can be set by pin, specifying the pin number, input mode, and button number that should be used
  myButtons.setButtonMode(0,digitalAct, 15);
  myButtons.setButtonMode(1,btnoff, 0);
  myButtons.setButtonMode(2,analog, 25);
  myButtons.setButtonMode(3,touch, 26);
  myButtons.setButtonMode(4,autodetect, 40);

etc.

Alternatively, pins can be defined by sending a JSON string. 

    DynamicJsonDocument doc(docSize);
    DeserializationError error = deserializeJson(doc, jsonData);
    if (!error)
      myButtons.loadButtonCfgJSON(doc);

jsonData is a JSON string with this syntax:

{
"HoldThreshold":500, "DblClickThreshold":1000, "BoardBaseAddr":0,
"Buttons":
[
{
	"PortNr":0,
	"ButtonType": "off",
	"ButtonAddr": 0
},
{
	"PortNr":1,
	"ButtonType": "off",
	"ButtonAddr": 0
}
]
}



