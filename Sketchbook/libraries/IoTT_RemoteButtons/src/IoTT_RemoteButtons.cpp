#include <IoTT_RemoteButtons.h>

mqttTxFct txbtnMQTT = NULL;

uint8_t mirrorByte(uint8_t byteData)
{
	uint8_t retByte = 0;
	uint8_t inMask = 0x01;
	uint8_t outMask = 0x80;
	while (outMask)
	{
		if ((byteData & inMask) > 0)
			retByte |= outMask;
		inMask <<= 1;
		outMask >>= 1;
	}
	return retByte;
}

buttonType getButtonTypeByName(String typeName)
{
  if (typeName == "off") return btnoff; //btnTypeOff;
  if (typeName == "digital") return digitalAct; //btnTypeDigital;
  if (typeName == "analog") return analog; //btnTypeAnalog;
  if (typeName == "sensor") return sensor; //btnTypeSensor;
  if (typeName == "swireport") return swireport; //btnTypeSwitchReport;
  return digitalAct; //default
}

IoTT_Mux64Buttons::IoTT_Mux64Buttons()
{
//	commType = btnI2C;
//	buttonBaton = xSemaphoreCreateMutex();
}

IoTT_Mux64Buttons::~IoTT_Mux64Buttons()
{
//    xSemaphoreTake(buttonBaton, portMAX_DELAY);
//    xSemaphoreGive(buttonBaton);
//    vSemaphoreDelete(buttonBaton);
}

void IoTT_Mux64Buttons::initButtonsDirect(bool pollBtns) //default false
{
	btnUpdateTimer = millis(); // + btnUpdateInterval;
	if (pollBtns) //if not set, processButtons does not read port data, buttons receive events from external source, eg DigitraxBuffers
		sourceMode = 2; //Poll Mode
	else
		sourceMode = 0; //MUX or Direct Mode
	
}

void IoTT_Mux64Buttons::initButtonsI2C(TwoWire * newWire, uint8_t Addr, uint8_t * analogPins, bool useWifi)
{
	btnUpdateTimer = millis(); // + btnUpdateInterval;
	thisWire = newWire;
	wireAddr = Addr; //
	if (analogPins != NULL)
	{
		sourceMode = 0;
		analogPin = analogPins[0];
		pinMode(analogPin, INPUT_PULLUP);
		numTouchButtons = 16 * analogPins[1]; //byte 1 has number of MUX's served by the 328P at the other end of the wire. byte 0 has the pin number where the signal shows up
	}
	else
	{
		Serial.printf("Init Buttons for GreenHat Addr %i\n", Addr);
		numTouchButtons = 32;
		sourceMode = 1;
		uint8_t regData[3] = {MCP23017_IODIRA, 0xFF, 0xFF};
		writeI2CData(MCP23017_ADDRESS + Addr, &regData[0], 3);  //set to input both ports
		writeI2CData(MCP23017_ADDRESS + Addr+1, &regData[0], 3);  
		regData[0] = MCP23017_GPPUA;
		writeI2CData(MCP23017_ADDRESS + Addr, &regData[0], 3);  //set to input both ports
		writeI2CData(MCP23017_ADDRESS + Addr+1, &regData[0], 3);  
	}
	
	touchArray = (IoTT_ButtonConfig*) realloc (touchArray, numTouchButtons * sizeof(IoTT_ButtonConfig));
	for (int i = 0; i < numTouchButtons; i++)
	{
		IoTT_ButtonConfig * myTouch = &touchArray[i];
		myTouch->btnTypeDetected = digitalAct; //btnTypeDigital; //auto detect	
		myTouch->btnAddr = boardBaseAddress + i; //LocoNet Button Address
		myTouch->btnEventMask = 0x1F; //all events activated

		myTouch->analogAvg.setInitValues(); //initialize with default values
		myTouch->lastPublishedData = 0;
		myTouch->btnStatus = false; //true if pressed
		myTouch->lastStateChgTime[4] = 0; //used to calculate dbl click

		myTouch->nextHoldUpdateTime = millis() + holdThreshold; 
		myTouch->nextPeriodicUpdateTime = millis() + analogRefreshInterval; 
		myTouch->lastEvtPtr = 0;
	}
}

void IoTT_Mux64Buttons::loadButtonCfgDirectJSON(DynamicJsonDocument doc) //used for BlackHat
{
    if (doc.containsKey("RefreshInterval"))
        analogRefreshInterval = doc["RefreshInterval"];
    if (doc.containsKey("BoardBaseAddr"))
        setBoardBaseAddr((int)doc["BoardBaseAddr"]);
    if (doc.containsKey("Buttons"))
    {
        JsonArray Buttons = doc["Buttons"];
		numTouchButtons = Buttons.size();
		touchArray = (IoTT_ButtonConfig*) realloc (touchArray, numTouchButtons * sizeof(IoTT_ButtonConfig));

		for (int i = 0; i < numTouchButtons; i++)
		{
			IoTT_ButtonConfig * myTouch = &touchArray[i];
			myTouch->btnTypeDetected = digitalAct; //btnTypeDigital; //auto detect	
			myTouch->btnAddr = boardBaseAddress + i; //LocoNet Button Address
			myTouch->btnEventMask = 0x1F; //all events activated

			myTouch->analogAvg.setInitValues(); //nitialize with default values
			myTouch->lastPublishedData = 0;
			myTouch->btnStatus = false; //true if pressed
			myTouch->lastStateChgTime[4] = 0; //used to calculate dbl click

			myTouch->nextHoldUpdateTime = millis() + holdThreshold; 
			myTouch->nextPeriodicUpdateTime = millis() + analogRefreshInterval; 
			myTouch->lastEvtPtr = 0;
			myTouch->gpioPin = Buttons[i]["PinNr"];
			pinMode(myTouch->gpioPin, INPUT);
			myTouch->btnAddr = Buttons[i]["ButtonAddr"];
			myTouch->btnTypeDetected = getButtonTypeByName(Buttons[i]["ButtonType"]);
			myTouch->btnEventMask = Buttons[i]["EventMask"];
			if (myTouch->btnEventMask == 0)
				myTouch->btnEventMask = 0x1F; //activate all events in case there was no EventMask field. Use no events is 0x20
		}
	}
    if (doc.containsKey("MQTT"))
    {
		JsonObject myMQTT = doc["MQTT"];
		if (myMQTT.containsKey("Subscribe"))
		{
			JsonArray myMQTTSub = myMQTT["Subscribe"];
			if (myMQTTSub.size() == 1)
				for (int i = 0; i < myMQTTSub.size(); i++)
				{
					strcpy(subTopicList[i].topicName, myMQTTSub[i]["Topic"]);
					subTopicList[i].inclAddr = myMQTTSub[i]["InclAddr"];
				}
		}
		if (myMQTT.containsKey("Publish"))
		{
			JsonArray myMQTTSub = myMQTT["Publish"];
			if (myMQTTSub.size() == 2)
				for (int i = 0; i < myMQTTSub.size(); i++)
				{
					strcpy(pubTopicList[i].topicName, myMQTTSub[i]["Topic"]);
					pubTopicList[i].inclAddr = myMQTTSub[i]["InclAddr"];
				}
		}
	}
}

void IoTT_Mux64Buttons::loadButtonCfgI2CJSON(DynamicJsonDocument doc) //used for YellowHat, 
{
	JsonObject thisObj = doc.as<JsonObject>();
	loadButtonCfgI2CJSONObj(thisObj);
}

void IoTT_Mux64Buttons::loadButtonCfgI2CJSONObj(JsonObject doc) //used GreenHat (called from Switches.cpp)
{
//	Serial.println("loadButtonCfgI2CJSON");
	if (doc.containsKey("DblClickThreshold"))
		setDblClickRate((int)doc["DblClickThreshold"]);
    if (doc.containsKey("HoldThreshold"))
        setHoldDelay((int)doc["HoldThreshold"]);
    if (doc.containsKey("RefreshInterval"))
        analogRefreshInterval = doc["RefreshInterval"];
    if (doc.containsKey("Sensitivity"))
        newAnalogThreshold = doc["Sensitivity"];
    if (doc.containsKey("BoardBaseAddr"))
        setBoardBaseAddr((int)doc["BoardBaseAddr"]);
    if (doc.containsKey("MQTT"))
    {
		JsonObject myMQTT = doc["MQTT"];
		if (myMQTT.containsKey("Subscribe"))
		{
			JsonArray myMQTTSub = myMQTT["Subscribe"];
			if (myMQTTSub.size() == 1)
				for (int i = 0; i < myMQTTSub.size(); i++)
				{
					strcpy(subTopicList[i].topicName, myMQTTSub[i]["Topic"]);
					subTopicList[i].inclAddr = myMQTTSub[i]["InclAddr"];
				}
		}
		if (myMQTT.containsKey("Publish"))
		{
			JsonArray myMQTTSub = myMQTT["Publish"];
			if (myMQTTSub.size() == 2)
				for (int i = 0; i < myMQTTSub.size(); i++)
				{
					strcpy(pubTopicList[i].topicName, myMQTTSub[i]["Topic"]);
					pubTopicList[i].inclAddr = myMQTTSub[i]["InclAddr"];
				}
		}
	}
    if (doc.containsKey("Buttons"))
    {
        JsonArray Buttons = doc["Buttons"];
        uint8_t buttonDefListLen = Buttons.size();
//		Serial.println("load buttons");
//		Serial.println(buttonDefListLen);
        for (int i=0; i<buttonDefListLen;i++)
        {
			uint8_t thisPort = Buttons[i]["PortNr"];
			if ((thisPort >= 0) && (thisPort < numTouchButtons))
			{
				touchArray[thisPort].nextHoldUpdateTime = millis() + holdThreshold; 
				touchArray[thisPort].nextPeriodicUpdateTime = millis() + analogRefreshInterval; 
				touchArray[thisPort].btnAddr = Buttons[i]["ButtonAddr"];
//				touchArray[thisPort].btnTypeRequested = getButtonTypeByName(Buttons[i]["ButtonType"]);
				touchArray[thisPort].btnTypeDetected = getButtonTypeByName(Buttons[i]["ButtonType"]);
				touchArray[thisPort].btnEventMask = Buttons[i]["EventMask"];
				if (touchArray[thisPort].btnEventMask == 0)
					touchArray[thisPort].btnEventMask = 0x1F; //activate all events in case there was no EventMask field. Use no events is 0x20
//				Serial.printf("Button %i type %i addr %i Events %2X \n", thisPort, touchArray[thisPort].btnTypeDetected, touchArray[thisPort].btnAddr, touchArray[thisPort].btnEventMask);
			}
//			Serial.printf("Loaded button %i for BtnNr %i\n", thisPort, touchArray[thisPort].btnAddr);
        } 
    }  
    else
		Serial.println("No buttons defined");
}

void IoTT_Mux64Buttons::setMQTTMode(mqttTxFct txFct)
{
	txbtnMQTT = txFct;
}

void IoTT_Mux64Buttons::subscribeTopics()
{
	for (int i = 0; i < 1; i++)
	{
//		Serial.printf("Subscribe Button Topic %i \n", i);
		String thisTopic = String(subTopicList[i].topicName);
		if (subTopicList[i].inclAddr)
			thisTopic += "/#";
		if (txbtnMQTT) txbtnMQTT(0, &thisTopic[0], NULL);
	}
}

void IoTT_Mux64Buttons::setDblClickRate(int dblClickRate)
{
	dblClickThreshold = dblClickRate;
}

void IoTT_Mux64Buttons::setHoldDelay(int holdDelay)
{
//	Serial.printf("Set Hold Delay to %i\n", holdDelay);
	holdThreshold = holdDelay;
}

void IoTT_Mux64Buttons::setBoardBaseAddr(int boardAddr)
{
	boardBaseAddress = boardAddr;
	for (int i = 0; i < numTouchButtons; i++)
	{
		IoTT_ButtonConfig * myTouch = &touchArray[i];
		myTouch->btnAddr = boardBaseAddress + i; //set address for all buttons if boardBase changes
	}
}

/*
void IoTT_Mux64Buttons::setAnalogRefreshTime(uint16_t newInterval)
{
	analogRefreshInterval = newInterval;
}
*/

uint8_t  IoTT_Mux64Buttons::getButtonMode(int btnNr)
{
	return touchArray[btnNr].btnTypeDetected;
}

uint16_t  IoTT_Mux64Buttons::getButtonAddress(int btnNr)
{
	return touchArray[btnNr].btnAddr;
}

bool IoTT_Mux64Buttons::getButtonState(int btnNr)
{
	return &touchArray[btnNr].btnStatus;
}

void IoTT_Mux64Buttons::sendButtonEvent(uint16_t btnNr, buttonEvent btnEvent)
{
  IoTT_ButtonConfig * thisTouchData = &touchArray[btnNr];
  uint8_t evtMask = 0x01 << btnEvent;
  if ((onButtonEvent) && (startUpCtr==0)) 
    if ((thisTouchData->btnEventMask & evtMask) > 0)
    {
		thisTouchData->lastPublishedEvent = btnEvent;
		if (txbtnMQTT)
			sendBtnStatusMQTT(0, btnNr); //use reply topic
		else
			switch(thisTouchData->btnTypeDetected)
			{
				case digitalAct : onButtonEvent(thisTouchData->btnAddr, btnEvent); break;
//				case sensor : if (evtMask & 0x03) onSensorEvent(thisTouchData->btnAddr, btnEvent, thisTouchData->btnEventMask); break;
//				case swireport : if (evtMask & 0x03) onSwitchReportEvent(thisTouchData->btnAddr, btnEvent, thisTouchData->btnEventMask); break;
				case sensor : ;
				case swireport: thisTouchData->nextHoldUpdateTime = millis() + sensorThreshold; break;

			}
	}
  if ((onBtnDiagnose) && (startUpCtr==0)) 
	onBtnDiagnose(thisTouchData->btnTypeDetected, btnNr, thisTouchData->btnAddr, btnEvent);
}

void IoTT_Mux64Buttons::sendAnalogData(uint8_t btnNr, uint16_t analogValue)
{
	IoTT_ButtonConfig * thisTouchData = &touchArray[btnNr];
	if ((millis() > thisTouchData->nextPeriodicUpdateTime) || (abs(thisTouchData->lastPublishedData - analogValue) > (newAnalogThreshold * analogMaxVal / 100)) || (((analogValue == 0) || (analogValue == analogMaxVal)) && (analogValue != thisTouchData->lastPublishedData)))
	{
		if (millis() > thisTouchData->nextHoldUpdateTime)
		{
			thisTouchData->lastPublishedData = analogValue;
			if ((onAnalogData) && (startUpCtr==0)) 
				if (txbtnMQTT)
					sendBtnStatusMQTT(0, btnNr); //use reply topic
				else
					onAnalogData(thisTouchData->btnAddr, analogValue);
			if ((onBtnDiagnose) && (startUpCtr==0)) 
				onBtnDiagnose(thisTouchData->btnTypeDetected, btnNr, thisTouchData->btnAddr, analogValue);
			thisTouchData->nextHoldUpdateTime = millis() + analogMinMsgDelay;
			thisTouchData->nextPeriodicUpdateTime = millis() + analogRefreshInterval;
		}
	}
}

void IoTT_Mux64Buttons::processSensorHold(uint8_t btnNr) //call onButtonHold function every holdThreshold milliseconds
{
	IoTT_ButtonConfig * thisTouchData = &touchArray[btnNr];
	if (thisTouchData->btnStatus != thisTouchData->btnSentStatus)
		if (millis() > thisTouchData->nextHoldUpdateTime)
		{
			switch(thisTouchData->btnTypeDetected)
			{
				case sensor : if (thisTouchData->btnEventMask & 0x03) onSensorEvent(thisTouchData->btnAddr, thisTouchData->btnStatus, thisTouchData->btnEventMask); break;
				case swireport : if (thisTouchData->btnEventMask & 0x03) onSwitchReportEvent(thisTouchData->btnAddr, thisTouchData->btnStatus, thisTouchData->btnEventMask); break;
			}
			thisTouchData->btnSentStatus = thisTouchData->btnStatus;
		}
}

void IoTT_Mux64Buttons::processDigitalHold(uint8_t btnNr) //call onButtonHold function every holdThreshold milliseconds
{
  IoTT_ButtonConfig * thisTouchData = &touchArray[btnNr];
  if ((thisTouchData->btnStatus) && (millis() > thisTouchData->nextHoldUpdateTime))
  {
	  sendButtonEvent(btnNr, onbtnhold);
	  thisTouchData->nextHoldUpdateTime += holdThreshold;
  }
}

void IoTT_Mux64Buttons::processDigitalInputBuffer(uint8_t btnNr, bool btnPressed)
{
	uint32_t inpMask = 0x00000001 << btnNr;
	if (btnPressed)
		pollBuffer |= inpMask;
	else
		pollBuffer &= (~inpMask);
}

void IoTT_Mux64Buttons::processDigitalButton(uint8_t btnNr, bool btnPressed)
{
//  if (btnNr == 2) Serial.println(btnPressed);
  IoTT_ButtonConfig * thisTouchData = &touchArray[btnNr];
  if (!thisTouchData)
	Serial.printf("No button # %i found\n", btnNr);
  if (btnPressed != thisTouchData->btnStatus)
  {
	thisTouchData->lastEvtPtr++;
	thisTouchData->lastEvtPtr &= 0x03;
	thisTouchData->lastStateChgTime[thisTouchData->lastEvtPtr] = millis();
	thisTouchData->btnStatus = btnPressed;
//	Serial.printf("Button status change %i\n", thisTouchData->btnTypeDetected);
	thisTouchData->nextHoldUpdateTime = millis() + holdThreshold;
	if (btnPressed)
	{
//		Serial.println("Button down");
		sendButtonEvent(btnNr, onbtndown); 
	}
	else
	{
//		Serial.println("Button up");
		sendButtonEvent(btnNr, onbtnup);
		//more processing for click and dblclick
		uint8_t prevUpTime = (thisTouchData->lastEvtPtr + 2) &0x03; //same as mod 4
		if ((thisTouchData->lastStateChgTime[prevUpTime] != 0) && ((thisTouchData->lastStateChgTime[thisTouchData->lastEvtPtr] - thisTouchData->lastStateChgTime[prevUpTime]) < dblClickThreshold))
		{
//		Serial.println("Button Double Click");
			sendButtonEvent(btnNr, onbtndblclick);
		}
		else
		{
//		Serial.println("Button Click");
			sendButtonEvent(btnNr, onbtnclick);
		}
    }
  }
  else
  {
	  switch (thisTouchData->btnTypeDetected)
	  {
		  case digitalAct: if (btnPressed) processDigitalHold(btnNr); break;
		  default: processSensorHold(btnNr); break;
	  } 
  }
} 

void IoTT_Mux64Buttons::writeI2CData(uint8_t devAddr, uint8_t * regData, uint8_t numBytes)
{
	thisWire->beginTransmission(devAddr);
	for (uint8_t i = 0; i < numBytes; i++)
	{
//		Serial.println(*regData, 16);
		thisWire->write(*regData);
		regData++;
	}
	thisWire->endTransmission();
}

uint32_t IoTT_Mux64Buttons::readI2CData(uint8_t devAddr, uint8_t startReg, uint8_t numBytes) //max 4 bytes
{
	writeI2CData(devAddr, &startReg, 1);
	thisWire->requestFrom(devAddr, numBytes);
	uint32_t readRes = 0;
	for (uint8_t i = 0; i < numBytes; i++)
		readRes = (readRes<<8) + thisWire->read();
	return readRes;
}

uint16_t IoTT_Mux64Buttons::readEXTPort(uint8_t extAddr)
{
	return readI2CData(extAddr, MCP23017_GPIOA, 2);
}

uint16_t IoTT_Mux64Buttons::readMUXButton(uint8_t inpLineNr, uint8_t muxNr)
{
	uint8_t newData = ((inpLineNr & 0x0F) | (((muxNr ^ 0xFF) & 0x03) << 4));
	//send command to 328P for line and port
	if (newData != currentChannel)
	{
//		Serial.printf("Send mux %i Line %i Cmd %2x\n", muxNr, inpLineNr, newData);

		writeI2CData(wireAddr, &newData, 1);
		currentChannel = newData;
	}
	//read analog value on sig input
	uint16_t newVal = analogRead(analogPin);
//	Serial.println(newVal);
	return newVal;
}

void IoTT_Mux64Buttons::processButtons()
{
	byte hlpVal;
	startUpCtr = max(startUpCtr-1,0);
	IoTT_ButtonConfig * thisTouchData;
	uint16_t hlpAnalog;
	uint16_t thisAnalogAvg;
	double_t randVal = random(-50, 50) / 1000;


	if (millisElapsed(btnUpdateTimer) > btnUpdateInterval)
	{
		btnUpdateTimer = millis();
		switch (sourceMode)
		{
			case 0: //MUX or Direct GPIO
			{
				if (!thisWire) //currently YellowHat and BlackHat
				{
					pinMode(0, INPUT); //this is a workaround for the moment. Somewhere the settings for pin 0 are accidently changed, leading to constant reading of 1
										//remove when problem is fixed	
//					Serial.println("Btn fix");
				}
				for (uint8_t btnCtr = 0; btnCtr < numTouchButtons; btnCtr++)
				{
					thisTouchData = &touchArray[btnCtr];
					int lineNr = btnCtr & 0x0F; //btnCtr % 16;
					int portNr = 0x01 << (btnCtr >> 4); //0x01 << trunc(btnCtr/16);
	
					if (thisWire)
						hlpAnalog = readMUXButton(lineNr, portNr);
					else
						if (thisTouchData->btnTypeDetected == analog)
							hlpAnalog = analogRead(thisTouchData->gpioPin);
						else
							hlpAnalog = 4095 * digitalRead(thisTouchData->gpioPin);
//					if (btnCtr == 0)
//						Serial.printf("Ctr %i Line %i Value %i\n", btnCtr, thisTouchData->gpioPin, hlpAnalog);
					thisAnalogAvg = round(thisTouchData->analogAvg.getEstimate((double_t) hlpAnalog + randVal)); //artifial flickering to avoid filter lockup

					if (thisTouchData->btnTypeDetected != btnoff)
					{
						if (thisTouchData->btnTypeDetected == analog)
						{
							if ((thisAnalogAvg >= 0) && (thisAnalogAvg <= analogMaxVal))
								sendAnalogData(btnCtr, thisAnalogAvg);
						}
						else if (thisAnalogAvg <= digitalLoMax)
						{
							processDigitalButton(btnCtr, true);
						}
						else if (thisAnalogAvg >= digitalHiMin)
						{
							processDigitalButton(btnCtr, false);
						}
					}

				}
			}
			break;
			case 1: //MCP23017
			{
				uint16_t portData = 0;
				uint8_t bitData[4] = {0,0,0,0}; //0,2: Btn; 1,3: Pos 
				portData = readEXTPort(MCP23017_ADDRESS + wireAddr); //channels 9-16
				bitData[2] = (portData & 0xFF00) >> 8;
				bitData[3] = mirrorByte(portData & 0x00FF);
				portData = readEXTPort(MCP23017_ADDRESS + wireAddr + 1); //channels 1-8
				bitData[0] = (portData & 0xFF00) >> 8;
				bitData[1] = mirrorByte(portData & 0x00FF);
//				Serial.printf("A Btn: %2X A Pos: %2X B Btn: %2X B Pos: %2X \n", bitData[0], bitData[1], bitData[2], bitData[3]);
				for (uint8_t btnCtr = 0; btnCtr < numTouchButtons; btnCtr++)
				{
					uint8_t byteNr = ((btnCtr & 0x10)>>3) + (btnCtr & 1);
					uint8_t bitNr = ((btnCtr>>1) & 0x07);
//					Serial.printf("Btn: %i Byte: %i Bit: %i Result: %i\n", btnCtr, byteNr, bitNr, !(bitData[byteNr] & (0x01 << bitNr)));
					processDigitalButton(btnCtr, !(bitData[byteNr] & (0x01 << bitNr)));
				}
			}
			break;
			case 2: //external
			{
				uint32_t bitMask = 0x00000001;
				for (uint8_t btnCtr = 0; btnCtr < 32; btnCtr++)
				{
					processDigitalButton(btnCtr, pollBuffer & bitMask);
					bitMask = bitMask << 1;
				}
			}
			break;
		}

		uint8_t updateReq = digitraxBuffer->getUpdateReqStatus();
		if (updateReq)
		{
			uint8_t flagMask = 0x01;
			while  (flagMask)
			{
				if (updateReq & flagMask)
				{
					digitraxBuffer->clearUpdateReqFlag(flagMask);
					uint8_t btnOffset = 0;
					while (flagMask)
					{
						btnOffset++;
						flagMask >>= 1;
					}
					btnOffset--;
					for (int i = 0; i < numTouchButtons; i++)
					{
						if ((i % 8) == btnOffset)
						{
							IoTT_ButtonConfig * myTouch = &touchArray[i];
							if ((myTouch->btnTypeDetected == sensor) || (myTouch->btnTypeDetected == swireport))
								myTouch->btnSentStatus = !myTouch->btnStatus;
						}
					}
					return;
				}
				flagMask <<= 1;
			}
		}

	}
}

void IoTT_Mux64Buttons::sendBtnStatusMQTT(uint8_t topicNr, uint16_t btnNr)
{
	IoTT_ButtonConfig * thisTouchData = &touchArray[btnNr];
	
	char myStatusMsg[200];
	DynamicJsonDocument doc(300);
	doc["PortNr"] = btnNr;
	doc["BtnNr"] = thisTouchData->btnAddr;
	doc["BtnType_N"] = thisTouchData->btnTypeDetected;
	switch (thisTouchData->btnTypeDetected)
	{
		case btnoff:
			doc["BtnType_S"] = "off";
			break;
		case digitalAct:
			doc["BtnType_S"] = "digital";
			doc["LastEvent_N"] = thisTouchData->lastPublishedEvent;
			switch (thisTouchData->lastPublishedEvent)
			{
				case onbtndown:
					doc["LastEvent_S"] = "btndown";
					break;
				case onbtnup:
					doc["LastEvent_S"] = "btnup";
					break;
				case onbtnclick:
					doc["LastEvent_S"] = "btnclick";
					break;
				case onbtndblclick:
					doc["LastEvent_S"] = "btndblclick";
					break;
				case onbtnhold:
					doc["LastEvent_S"] = "btnhold";
					break;
				default:
					doc["LastEvent_S"] = "noevent";
					break;
			}
			doc["EventMask"] = thisTouchData->btnEventMask;
			break;
		case analog:
			doc["BtnType_S"] = "analog";
			doc["BtnValue"] = thisTouchData->lastPublishedData;
			break;
	}

	serializeJson(doc, myStatusMsg);
	String thisTopic = String(pubTopicList[topicNr].topicName);
	if (pubTopicList[topicNr].inclAddr)
		thisTopic += ("/" + String(btnNr));
	if (txbtnMQTT)
		txbtnMQTT(1, &thisTopic[0], &myStatusMsg[0]);
}

bool IoTT_Mux64Buttons::processMQTTCmd(char * topic, DynamicJsonDocument doc)
{
	if (!txbtnMQTT) return false; //called in error, so return

	topicStruct * thisTopic = NULL;
	String topicStr = String(topic);
	for (int i = 0; i < 1; i ++)
		if (topicStr.indexOf(subTopicList[i].topicName) >= 0)
		{
			thisTopic = &subTopicList[i];
			break;
		}
	if (!thisTopic)
		for (int i = 0; i < 2; i ++)
			if (topicStr.indexOf(pubTopicList[i].topicName) >= 0)
			{
				thisTopic = &pubTopicList[i];
				break;
			}
	if (!thisTopic) return false; //not a valid topic for the LED Chain
//	Serial.println(thisTopic->topicName);
	
	uint16_t btnAddr[20];
	uint8_t numBtn = 0;
	uint8_t portAddr[20];
	uint8_t numPort = 0;
	
//if address is part of topic, isolate address
	if (thisTopic->inclAddr)
	{
		int lastSlash = topicStr.lastIndexOf('/');
		String addrStr;
		if (lastSlash > 0)
		{
			addrStr = topicStr.substring(lastSlash + 1);
			if (addrStr.length() > 0)
			{
				portAddr[0] = addrStr.toInt();
				if (portAddr[0] < numTouchButtons)
					numPort = 1;
				else
					return false;
			}
		}
//		Serial.printf("Pos: %i Val: %i\n", lastSlash, ledAddr);
	}
//else extract address or address array from payload
	else
	{
		if (doc.containsKey("BtnNr"))
		{
			JsonArray btnList = doc["BtnNr"];
			if (btnList)
			{
				for (int i = 0; i < btnList.size(); i++)
					btnAddr[i] = btnList[i];
				numBtn = btnList.size();
//				Serial.printf("Btn Array %i\n", numBtn);
			}
			else
			{
				btnAddr[0] = doc["BtnNr"];
				numBtn = 1;
//				Serial.printf("Single Button %i\n", btnAddr[0]);
			}
		}
		if (doc.containsKey("PortNr"))
		{
			JsonArray btnList = doc["PortNr"];
			if (btnList)
			{
				for (int i = 0; i < btnList.size(); i++)
					if ((btnList[i] < numTouchButtons) && (numPort < 20))
					{
						portAddr[numPort] = btnList[i];
						numPort++;
					}
//				Serial.printf("Btn Port Array %i\n", numPort);
			}
			else
			{
				portAddr[0] = doc["PortNr"];
				numPort = 1;
//				Serial.printf("Single Button Port %i\n", portAddr[0]);
			}
		}
	}
	
	if ((numBtn == 0) && (numPort == 0)) return false; //no LED Nr found, just return
	
	if (thisTopic == &subTopicList[0])//: //if ASK command
	{
		if (numBtn > 0)
			for (int i = 0; i < numBtn; i++)
				for (int j = 0; j < numTouchButtons; j++)
					if (btnAddr[i] == touchArray[j].btnAddr)
						sendBtnStatusMQTT(1, j); //use reply topic
		if (numPort > 0)
			for (int i = 0; i < numPort; i++)
				sendBtnStatusMQTT(1, portAddr[i]); //use reply topic
		return true;
	}
	return false;
}
