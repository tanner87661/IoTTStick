#ifndef IoTT_RemoteButtons_h
#define IoTT_RemoteButtons_h

#include <Arduino.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <IoTT_CommDef.h>
#include <IoTT_ButtonTypeDef.h>
#include <IoTT_DigitraxBuffers.h>
#include <OneDimKalman.h>

//defs for MCP23017
#define MCP23017_ADDRESS 0x20 //!< MCP23017 Address
//registers
#define MCP23017_IODIRA 0x00   //!< I/O direction register A
#define MCP23017_IPOLA 0x02    //!< Input polarity port register A
#define MCP23017_GPINTENA 0x04 //!< Interrupt-on-change pins A
#define MCP23017_DEFVALA 0x06  //!< Default value register A
#define MCP23017_INTCONA 0x08  //!< Interrupt-on-change control register A
#define MCP23017_IOCONA 0x0A   //!< I/O expander configuration register A
#define MCP23017_GPPUA 0x0C    //!< GPIO pull-up resistor register A
#define MCP23017_INTFA 0x0E    //!< Interrupt flag register A
#define MCP23017_INTCAPA 0x10  //!< Interrupt captured value for port register A
#define MCP23017_GPIOA 0x12    //!< General purpose I/O port register A
#define MCP23017_OLATA 0x14    //!< Output latch register 0 A

#define MCP23017_IODIRB 0x01   //!< I/O direction register B
#define MCP23017_IPOLB 0x03    //!< Input polarity port register B
#define MCP23017_GPINTENB 0x05 //!< Interrupt-on-change pins B
#define MCP23017_DEFVALB 0x07  //!< Default value register B
#define MCP23017_INTCONB 0x09  //!< Interrupt-on-change control register B
#define MCP23017_IOCONB 0x0B   //!< I/O expander configuration register B
#define MCP23017_GPPUB 0x0D    //!< GPIO pull-up resistor register B
#define MCP23017_INTFB 0x0F    //!< Interrupt flag register B
#define MCP23017_INTCAPB 0x11  //!< Interrupt captured value for port register B
#define MCP23017_GPIOB 0x13    //!< General purpose I/O port register B
#define MCP23017_OLATB 0x15    //!< Output latch register 0 B


#define avgBase 5  //numer of values to be considered for rolling average

#define noAnalogMin 1000
#define noAnalogMax 3000

#define digitalLoMax 500 //5
#define digitalHiMin 2500 //4090

//#define btnTypeOff		0x00
//#define btnTypeTouch	0x01
//#define btnTypeDigital	0x02
//#define btnTypeAnalog	0x40
//#define btnAutoDetect	0x80

#define analogMaxVal 4095
/* 
 * supported button types:
 * 1: digital ButtonN0
 * 2: digital ButtonNC
 * 9: Analog Input
 * 
 * Messages for digital buttons: Button down, Button up, Button Click, Button Hold, Button Double Click
 * Messages Analog Input: Value Change with new value
*/ 

typedef struct 
{
//set from outside
  buttonType  btnTypeDetected = digitalAct; //(analog | digitalAct); //auto detect	
  uint16_t btnAddr = 0xFFFF; //LocoNet Button Address
  uint8_t  btnEventMask = 0x1F; //all events activated. Bits 0: down 1: up 2: click 3: dblclick 4: hold
  uint8_t  gpioPin = 0;
//internal usage
  OneDimKalman    analogAvg; //Kalman filtered analog reading
  uint16_t lastPublishedData = 0;
  buttonEvent lastPublishedEvent = onbtnup;
  bool     btnStatus = false; //true if pressed
  bool     btnSentStatus = false; //last sent status, used for sensor debouncing
  uint32_t lastStateChgTime[4]; //used to calculate dbl click events
  uint8_t  lastEvtPtr = 0; //bufferPtr to the above
  uint32_t nextHoldUpdateTime; //timer for repeating hold events
  uint32_t nextPeriodicUpdateTime; //timer for repeating hold events
} IoTT_ButtonConfig;

class IoTT_Mux64Buttons
{
	public:
		IoTT_Mux64Buttons();
		~IoTT_Mux64Buttons();

	private:
		IoTT_ButtonConfig * touchArray = NULL;
		TwoWire * thisWire = NULL;
		uint8_t sourceMode = 0; //by default, MUX or GPIO is used. 0: MUX or GPIO (if no Wire); 1: MCP23017 port commands; 2: Bit buffer lookup
		uint8_t wireAddr = 0;
		uint8_t analogPin = 0;
		uint8_t numTouchButtons = 0;
		uint16_t dblClickThreshold = 1000;
		uint16_t holdThreshold = 500;
		uint16_t sensorThreshold = 500;
		uint16_t analogMinMsgDelay = 750; //minimum time between 2 analog messages. This limits the bandwidth usage
		uint16_t analogRefreshInterval = 30000; //by default resend analog information every 30 seconds
		uint16_t boardBaseAddress = 0;
		int16_t startUpCtr = 50;
		float newAnalogThreshold = 1;  //new analog value gets sent out if deviation is more than x%
		int currentChannel = -1;
		uint32_t pollBuffer = 0; //used by event driven approach (e.g. RedHat, to set status lines)
		bool mqttMode = false;
		topicStruct subTopicList[1] = {{"BTNASK", false}};
		topicStruct pubTopicList[2] = {{"BTNREPORT", false}, {"BTNREPLY", false}};

	public: 
		void initButtonsI2C(TwoWire * newWire, uint8_t Addr, uint8_t * analogPins, bool useWifi = false); //analogPins = NULL makes it a GreenHat
		void loadButtonCfgI2CJSON(DynamicJsonDocument doc);
		void loadButtonCfgI2CJSONObj(JsonObject doc);

		void initButtonsDirect(bool pollBtns = false);
		void loadButtonCfgDirectJSON(DynamicJsonDocument doc);

		void setMQTTMode(mqttTxFct txFct);
		void subscribeTopics();

//		void setButtonMode(uint8_t btnNr, uint8_t btnMode, uint16_t btnAddress);
		void setDblClickRate(int dblClickRate);
		void setHoldDelay(int holdDelay);
		void setBoardBaseAddr(int boardAddr);
//		void setAnalogRefreshTime(uint16_t newInterval);
		uint8_t  getButtonMode(int btnNr);
		uint16_t  getButtonAddress(int btnNr);
		uint32_t btnUpdateTimer = millis();
		uint16_t btnUpdateInterval = 50; //milliseconds, gives 20Hz
		bool getButtonState(int btnNr);
		void processButtons();
		bool processMQTTCmd(char * topic, DynamicJsonDocument doc);
		void sendBtnStatusMQTT(uint8_t topicNr, uint16_t btnNr);
		void processDigitalInputBuffer(uint8_t btnNr, bool btnPressed);
	private:
		void processDigitalButton(uint8_t btnNr, bool btnPressed);
		void processDigitalHold(uint8_t btnNr);
		void processSensorHold(uint8_t btnNr);
		void sendButtonEvent(uint16_t btnAddr, buttonEvent btnEvent);
		void sendAnalogData(uint8_t btnNr, uint16_t analogValue );
		void writeI2CData(uint8_t devAddr, uint8_t * regData, uint8_t numBytes);
		uint32_t readI2CData(uint8_t devAddr, uint8_t startReg, uint8_t numBytes); //max 4 bytes
		uint16_t readMUXButton(uint8_t inpLineNr, uint8_t muxNr);
		uint16_t readEXTPort(uint8_t extAddr);
};

extern void onButtonEvent(uint16_t btnAddr, buttonEvent btnEvent) __attribute__ ((weak));
extern void onAnalogData(uint16_t inpAddr, uint16_t analogValue ) __attribute__ ((weak));
extern void onBtnDiagnose(uint8_t evtType, uint8_t portNr, uint16_t inpAddr, uint16_t btnValue) __attribute__ ((weak));
extern void onSensorEvent(uint16_t sensorAddr, uint8_t sensorEvent, uint8_t eventMask) __attribute__ ((weak));
extern void onSwitchReportEvent(uint16_t switchAddr, uint8_t switchEvent, uint8_t eventMask) __attribute__ ((weak));
extern void onSwitchRequestEvent(uint16_t switchAddr, uint8_t switchEvent) __attribute__ ((weak));

#endif
