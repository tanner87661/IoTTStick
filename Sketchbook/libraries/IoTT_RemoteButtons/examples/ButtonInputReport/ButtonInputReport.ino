#include <arduino.h>
#include <IoTT_Mux64Buttons.h>

/////////////////////////////////////////////////////////user configurations/////////////////////////////////////////////////////////////////

//define 4 IO pins for address selection of the 4067. These pins must be connected to the address pins (S0 - S3 on the breakout board)
#define ADDR_0 18
#define ADDR_1 5
#define ADDR_2 17
#define ADDR_3 16

//define how many pins will be used for data input. Up to 4 pins, meaning up to 4 mux with 16 inputs each can be supported. Also check the readme file for some pin limitations
#define NUM_DATA_PORTS 1  //number of analog lines used for buttons. Each line gives 16 buttons, maximum is 4 lines

//define each data pin to be used
#define DATA_0 15         //analog line numbers

//define the pin configuration array. The first byte is the number of data pins to be used, followed by the pin numbers for each data pin
uint8_t analogPins[] = {NUM_DATA_PORTS, DATA_0}; //byte 0 is number of defined pins to follow

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


IoTT_Mux64Buttons myButtons;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1000);
  Serial.println("Initializing...");
  myButtons.initButtons(ADDR_0,ADDR_1, ADDR_2, ADDR_3, &analogPins[0], true);
  for (int i = 0; i < 16; i++)
  {
    myButtons.setButtonMode(i,digitalAct, i+100);
  }
  myButtons.setButtonMode(13,touch, 113);
  myButtons.setButtonMode(14,btnoff, 0); //disable unused pin
  myButtons.setButtonMode(15,analog, 115);
}

//application callback procedures for button and analog events
void onButtonEvent(uint16_t btnNr, buttonEvent thisEvent)
{
  switch (thisEvent)
  {
    case onbtndown: Serial.printf("Button %i is down. \n", btnNr); break;
    case onbtnup: Serial.printf("Button %i is up. \n", btnNr); break;
    case onbtnclick: Serial.printf("Button %i was clicked. \n", btnNr); break;
    case onbtndblclick: Serial.printf("Button %i was double-clicked. \n", btnNr); break;
    case onbtnhold: Serial.printf("Button %i is hold down. \n", btnNr); break;
  }
}

void onAnalogData(uint16_t inpNr, uint16_t analogValue )
{
  Serial.printf("Analog Input %i has value %i.\n", inpNr, analogValue);
}

void loop() {
  // call the process function frequently, as it makes the library tick!!
  myButtons.processButtons();
  delay(40); //this simulates a 25Hz update rate. In a real application, the rate will be higher
}
