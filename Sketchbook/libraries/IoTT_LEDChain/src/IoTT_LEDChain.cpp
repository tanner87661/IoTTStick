#include <IoTT_LEDChain.h>

mqttTxFct txMQTT = NULL;

transitionType getTransitionTypeByName(String transName)
{
  if (transName == "soft") return soft;
  if (transName == "direct") return direct;
  if (transName == "merge") return merge;
  return direct; 
}

colorMode getColorModeByName(String colModeName)
{
  if (colModeName == "static") return constlevel;
  if (colModeName == "localblink") return localblinkpos;
  if (colModeName == "localblinkpos") return localblinkpos;
  if (colModeName == "localblinkneg") return localblinkneg;
  if (colModeName == "localrampup") return localrampup;
  if (colModeName == "localrampdown") return localrampdown;
  if (colModeName == "blink") return globalblinkpos;
  if (colModeName == "globalblink") return globalblinkpos;
  if (colModeName == "globalblinkpos") return globalblinkpos;
  if (colModeName == "globalblinkneg") return globalblinkneg;
  if (colModeName == "globalrampup") return globalrampup;
  if (colModeName == "globalrampdown") return globalrampdown;
  return constlevel; 
}

displayType getDisplayTypeByName(String displayTypeName)
{
  if (displayTypeName == "discrete") return discrete;
  if (displayTypeName == "linear") return linear;
  return discrete; 
}

//enum sourceType : uint8_t {button=0, analogvalue=1, trackswitch=2, signalmastdcc=3, signalmastdyn=4, blockdetector=5, transponder=6, powerstat=7, alwayson=8, nosource=255};

sourceType getLEDActionTypeByName(String actionName)
{ //blockdet, dccswitch, dccsignal, svbutton, analoginp, powerstat
  if (actionName == "block") return evt_blockdetector; 
  if (actionName == "switch") return evt_trackswitch;
  if (actionName == "signal") return evt_signalmastdcc;
  if (actionName == "signaldyn") return evt_signalmastdyn;
  if (actionName == "signalstat") return evt_trackswitch;
  if (actionName == "button") return evt_button;
  if (actionName == "analog") return evt_analogvalue;
  if (actionName == "power") return evt_powerstat;
  if (actionName == "transponder") return evt_transponder;
  if (actionName == "constant") return evt_alwayson;
  return evt_nosource; 
}
/*
  if (transName == "button") return button;
  if (transName == "analogvalue") return analogvalue;
  if (transName == "switch") return trackswitch;
  if (transName == "dccsignal") return signalmastdcc;
  if (transName == "dynsignal") return signalmastdyn;
  if (transName == "blockdetector") return blockdetector;
  if (transName == "transponder") return transponder;
  if (transName == "power") return transponder;
*/
IoTT_ColorDefinitions::IoTT_ColorDefinitions()
{
}

IoTT_ColorDefinitions::~IoTT_ColorDefinitions()
{
}

void IoTT_ColorDefinitions::loadColDefJSON(JsonObject thisObj)
{
	String colName = thisObj["Name"];
    strcpy(colorName, colName.c_str()); 
    if (thisObj.containsKey("HSVVal"))
    {
		byte HSV0 = (byte)thisObj["HSVVal"][0];
        byte HSV1 = (byte)thisObj["HSVVal"][1];
        byte HSV2 = (byte)thisObj["HSVVal"][2];
        HSVVal = CHSV(HSV0, HSV1, HSV2);
        hsv2rgb_raw(HSVVal, RGBVal);
//	Serial.print(colName);
//	Serial.printf(" HSV: %i %i %i\n", HSVVal.h, HSVVal.s, HSVVal.v);
    }
    else if (thisObj.containsKey("RGBVal"))
    {
        byte RGB0 = (byte)thisObj["RGBVal"][0];
        byte RGB1 = (byte)thisObj["RGBVal"][1];
        byte RGB2 = (byte)thisObj["RGBVal"][2];
        RGBVal = CRGB(RGB0, RGB1, RGB2);
        HSVVal = rgb2hsv_approximate(RGBVal); //FastLED function
//	Serial.print(colName);
//	Serial.printf(" HSV from RGB: %i %i %i\n", HSVVal.h, HSVVal.s, HSVVal.v);
    }
    else
    {
		RGBVal = CRGB(0,0,0);
        HSVVal = rgb2hsv_approximate(RGBVal); //FastLED function
	}
}

uint32_t endIdentify = 0;
#define  identifyTimeout 5000
#define  staticTimeout 2000  //time before static signal position becomes permanent

//******************************************************************************************************************************************************************
IoTT_LEDCmdList::IoTT_LEDCmdList()
{
}

IoTT_LEDCmdList::~IoTT_LEDCmdList()
{
}

void IoTT_LEDCmdList::updateLEDs()
{
}


void IoTT_LEDCmdList::loadCmdListJSON(JsonObject thisObj)
{
//	Serial.println("Loading Cmd Seq");
	upToVal = thisObj["Val"];
	if (parentObj->multiColor)
	{
		colOn = (IoTT_ColorDefinitions**) realloc (colOn, parentObj->ledAddrListLen * sizeof(IoTT_ColorDefinitions*));
		colOff = (IoTT_ColorDefinitions**) realloc (colOff, parentObj->ledAddrListLen * sizeof(IoTT_ColorDefinitions*));
		JsonArray colOnArray = thisObj["ColOn"];
		JsonArray colOffArray = thisObj["ColOff"];
		for (int i=0; i<parentObj->ledAddrListLen;i++)
		{
			colOn[i] = parentObj->parentObj->getColorByName(colOnArray[i]);
			colOff[i] = parentObj->parentObj->getColorByName(colOffArray[i]);
		}
		
	}
	else
	{
		colOn = (IoTT_ColorDefinitions**) realloc (colOn, sizeof(IoTT_ColorDefinitions*));
		colOff = (IoTT_ColorDefinitions**) realloc (colOff, sizeof(IoTT_ColorDefinitions*));
		if (thisObj.containsKey("ColOn"))
			colOn[0] = parentObj->parentObj->getColorByName(thisObj["ColOn"]);
		else
			colOn[0] = NULL;
		if (thisObj.containsKey("ColOff"))
			colOff[0] = parentObj->parentObj->getColorByName(thisObj["ColOff"]);
		else
			colOff[0] = NULL;
	}
	
	dispMode = (uint8_t*) realloc (dispMode, parentObj->ledAddrListLen * sizeof(uint8_t));
	blinkRate = (uint16_t*) realloc (blinkRate, parentObj->ledAddrListLen * sizeof(uint16_t));
	transType = (uint8_t*) realloc (transType, parentObj->ledAddrListLen * sizeof(uint8_t));
	JsonArray dispModeArray = thisObj["Mode"];
	JsonArray blinkRateArray = thisObj["Rate"];
	JsonArray transTypeArray = thisObj["Transition"];
	for (uint16_t i = 0; i < parentObj->ledAddrListLen; i++)
	{
		if (dispModeArray.isNull())
			dispMode[i] = getColorModeByName(thisObj["Mode"]);
		else
			dispMode[i] = getColorModeByName(dispModeArray[i]);

		if (blinkRateArray.isNull())
			blinkRate[i] = thisObj["Rate"];
		else
			blinkRate[i] = blinkRateArray[i];

		if (transTypeArray.isNull())
			transType[i] = getTransitionTypeByName(thisObj["Transition"]);
		else
			transType[i] = getTransitionTypeByName(transTypeArray[i]);
	}
}

//******************************************************************************************************************************************************************
IoTT_LEDHandler::IoTT_LEDHandler()
{
}

IoTT_LEDHandler::~IoTT_LEDHandler()
{
	freeObjects();
}

void IoTT_LEDHandler::freeObjects()
{
	if (cmdListLen > 0)
	{
		for (uint16_t i = 0; i < cmdListLen; i++)
			delete cmdList[i];
		cmdListLen = 0;
		free(cmdList);
	}
}

void IoTT_LEDHandler::updateLocalBlinkValues()
{
}

bool IoTT_LEDHandler::identifyLED(uint16_t ledNr)
{
	bool idResult = false;
	if (ledAddrListLen > 0)
	{
		for (int i = 0; i < ledAddrListLen; i++)
		{
			if (ledAddrList[i] == ledNr)
			{
				idResult = true;
				currentColor[i] = CHSV(0,0,255);
				parentObj->setCurrColHSV(ledAddrList[i], CHSV(0,0,255));
			}
		}
	}
	return idResult;
}


void IoTT_LEDHandler::updateChainDataForColor(uint8_t colorNr, IoTT_LEDCmdList * cmdDef, IoTT_LEDCmdList * cmdDefLin, uint8_t distance)
{
	CHSV targetCol, targetColLin;
	bool flipBlink = false;
	bool useGlobal = true;
	uint16_t timeElapsed;
//	Serial.printf("Disp Mode %i \n", cmdDef->dispMode[colorNr]);
//	cmdDef->dispMode[colorNr] = 0;
	switch (cmdDef->dispMode[colorNr])
	{
		case constlevel: 
			targetCol = (cmdDef->colOn[colorNr] != NULL) ? cmdDef->colOn[colorNr]->HSVVal : CHSV(0,0,0); 
			if (cmdDefLin != NULL)
				targetColLin = (cmdDefLin->colOn[colorNr] != NULL) ? cmdDefLin->colOn[colorNr]->HSVVal : CHSV(0,0,0); 
			break;
		case globalrampdown: 
			flipBlink = true;
		case globalrampup: 
			targetCol = cmdDef->colOn[colorNr]->HSVVal; 
			if (flipBlink)
				targetCol.v = round(targetCol.v * (1 - parentObj->globFaderValue));
			else
				targetCol.v = round(targetCol.v * parentObj->globFaderValue);
			
			if (cmdDefLin != NULL)
			{
				targetColLin = cmdDefLin->colOn[colorNr]->HSVVal; 
				if (flipBlink)
					targetColLin.v = round(targetColLin.v * (1 - parentObj->globFaderValue));
				else
					targetColLin.v = round(targetColLin.v * parentObj->globFaderValue);
			}
			break;
		case globalblinkneg: 
			flipBlink = true;
		case globalblinkpos: 
			if (parentObj->getBlinkStatus() ^ flipBlink)
			{
				targetCol = (cmdDef->colOn[colorNr] != NULL) ? cmdDef->colOn[colorNr]->HSVVal : CHSV(0,0,0); 
				if (cmdDefLin != NULL)
					targetColLin = (cmdDefLin->colOn[colorNr] != NULL) ? cmdDefLin->colOn[colorNr]->HSVVal : CHSV(0,0,0); 
			}
			else 
			{
				targetCol = (cmdDef->colOff[0] != NULL) ? cmdDef->colOff[0]->HSVVal : CHSV(0,0,0); 
				if (cmdDefLin != NULL)
					targetColLin = (cmdDefLin->colOff[colorNr] != NULL) ? cmdDefLin->colOff[colorNr]->HSVVal : CHSV(0,0,0); 
			}
			break;

		case localrampdown: 
			flipBlink = true;
		case localrampup: 
			timeElapsed = blinkTimer - millis();
			if (millis() > blinkTimer)
			{
				blinkStatus = !blinkStatus;
				blinkTimer += cmdDef->blinkRate[colorNr];
				timeElapsed = 0; //blinkInterval;
				while (millis() > blinkTimer) //exception correction in case something is not initialized.
					blinkTimer = millis() + cmdDef->blinkRate[colorNr];
			}
			locFaderValue = 1 - ((float_t)timeElapsed/(float_t)cmdDef->blinkRate[colorNr]);  //positive slope ramp from 0 to 1
			targetCol = cmdDef->colOn[colorNr]->HSVVal; 
			if (flipBlink)
				targetCol.v = round(targetCol.v * (1 - locFaderValue));
			else
				targetCol.v = round(targetCol.v * locFaderValue);
			if (cmdDefLin != NULL)
			{
				targetColLin = cmdDefLin->colOn[colorNr]->HSVVal; 
				if (flipBlink)
					targetColLin.v = round(targetColLin.v * (1 - locFaderValue));
				else
					targetColLin.v = round(targetColLin.v * locFaderValue);
			}
//			Serial.printf("h: %i s: %i v: %i %i %i %g\n", targetCol.h, targetCol.s, targetCol.v, timeElapsed, cmdDef->blinkRate[colorNr], locFaderValue);
			break;

		case localblinkneg: 
			flipBlink = true;
		case localblinkpos: 
			useGlobal = false;
			timeElapsed = blinkTimer - millis();
			if (millis() > blinkTimer)
			{
				blinkStatus = !blinkStatus;
				blinkTimer += cmdDef->blinkRate[colorNr];
				timeElapsed = 0; //blinkInterval;
				while (millis() > blinkTimer) //exception correction in case something is not initialized.
					blinkTimer = millis() + cmdDef->blinkRate[colorNr];
			}
			
			if (blinkStatus ^ flipBlink) 
			{
				targetCol = (cmdDef->colOn[colorNr] != NULL) ? cmdDef->colOn[colorNr]->HSVVal : CHSV(0,0,0); 
				if (cmdDefLin != NULL)
					targetColLin = (cmdDefLin->colOn[colorNr] != NULL) ? cmdDefLin->colOn[colorNr]->HSVVal : CHSV(0,0,0); 
			}
			else 
			{
				targetCol = (cmdDef->colOff[colorNr] != NULL) ? cmdDef->colOff[colorNr]->HSVVal : CHSV(0,0,0); 
				if (cmdDefLin != NULL)
					targetColLin = (cmdDefLin->colOff[colorNr] != NULL) ? cmdDefLin->colOff[colorNr]->HSVVal : CHSV(0,0,0); 
			}
			break;
	}
	if (cmdDefLin != NULL)
	{
		int16_t hueSpan = targetCol.h - targetColLin.h;
		int8_t hueSign = hueSpan >= 0 ? 1 : -1;
		if (abs(hueSpan) > 128)
		{
			hueSpan = 255 - abs(hueSpan);
			hueSign *= -1;
		}
		float_t distFact = (float_t)distance / 100;
		int hueDist = hueSign * round((float_t)abs(hueSpan) * distFact);
		targetCol.h = targetColLin.h + hueDist; 

		int16_t sSpan = targetCol.s - targetColLin.s;
		int sDist = round((float_t) sSpan * distFact);
		targetCol.s = targetColLin.s  + sDist;
		
		int16_t vSpan = targetCol.v - targetColLin.v;
		int vDist = round((float_t) vSpan * distFact);
		targetCol.v = targetColLin.v  + vDist;

//		Serial.printf("t: %i t1: %i d: %f r: %i %i %i %i %i %i\n", targetColLin.h, hueSpan, distFact, hueDist, sDist, vDist, targetCol.h, targetCol.s, targetCol.v);
	}
	
	targetCol.v = round(targetCol.v * parentObj->getBrightness()); //this is the final target color, now we calculate the next step on the way there, if needed

	if ((targetCol.h != currentColor[colorNr].h) || (targetCol.s != currentColor[colorNr].s) || (targetCol.v != currentColor[colorNr].v) || parentObj->refreshAnyway)
	{
		uint16_t blinkPeriod;
		if (useGlobal)
			blinkPeriod = parentObj->blinkInterval;
		else
			blinkPeriod = cmdDef->blinkRate[colorNr];
		uint16_t rateH;
		if (blinkPeriod > 0)
			rateH = round((255/(float)blinkPeriod)*(1000/(float)parentObj->ledUpdateInterval)); //val_units per LED refresh interval at given blink period
		else
			rateH = 255; //immediate change, period 0
//		Serial.printf("Period: %i Rate %i\n", blinkPeriod, rateH);
		int16_t hueChange = targetCol.h - currentColor[colorNr].h;
		int16_t satChange = targetCol.s - currentColor[colorNr].s;
		float satRatio = satChange / hueChange;
		int16_t valChange = targetCol.v - currentColor[colorNr].v;
		float valRatio = valChange / hueChange;

		int newTarget_h = targetCol.h;
		int newTarget_v = targetCol.v;
		
//		Serial.printf("Trans Type %i \n", cmdDef->transType[colorNr]);
//		cmdDef->transType[colorNr] = 1;
		switch (cmdDef->transType[colorNr])
		{
			case direct: //just go to the next value
				currentColor[colorNr] = targetCol;
				break;
			case soft: //reduce brightness, change hue and saturation, increase brightness
				rateH *= 4;
//			    Serial.printf("Setting Soft h %i s %i v %i to h %i s %i v %i rate %i\n", currentColor.h, currentColor.s, currentColor.v, targetCol.h, targetCol.s, targetCol.v, rateH); 
				if (currentColor[colorNr].h != targetCol.h)
				{
					if (currentColor[colorNr].v > rateH)
						currentColor[colorNr].v -= rateH;
					else
						currentColor[colorNr].v = 0;
					if (currentColor[colorNr].v == 0)
					{
						currentColor[colorNr].h = targetCol.h;
						currentColor[colorNr].s = targetCol.s;
					}
				}
				else
				{
					if ((currentColor[colorNr].v + rateH) < targetCol.v)
						currentColor[colorNr].v += rateH;
					else
						currentColor[colorNr].v = targetCol.v;
					currentColor[colorNr].s = targetCol.s;
				}
				break;
			case merge: //change hue, saturation, brightness simultaneously
//			    Serial.printf("Setting Merge h %i s %i v %i to h %i s %i v %i rate %i\n", currentColor[colorNr].h, currentColor[colorNr].s, currentColor[colorNr].v, targetCol.h, targetCol.s, targetCol.v, rateH); 
				int16_t newTarget_h = targetCol.h;
				int16_t myDeltaH = rateH;
//				Serial.println(hueChange);
				switch (hueChange)
				{
					case -260 ... -128: //turning down on long way, we better turn up
						newTarget_h += 255;
						break;
					case -127 ... -1: //turning down on direct way
						myDeltaH = -rateH;
						break; 
					case 0 ... 127: //turning up on direct way
						break;
					case 128 ... 260: //turning up, but more than half way, so we turn down
						newTarget_h -= 255;
						myDeltaH = -rateH;
						break;
				}
//				Serial.printf("new target hue: %i new Delta: %i\n", newTarget_h, myDeltaH);
				if (myDeltaH >= 0)
					if ((currentColor[colorNr].h + rateH) > newTarget_h)
						currentColor[colorNr].h = targetCol.h;
					else
						currentColor[colorNr].h += rateH;
				else
					if ((currentColor[colorNr].h - rateH) < newTarget_h)
						currentColor[colorNr].h = targetCol.h;
					else
						currentColor[colorNr].h -= rateH;
				if (currentColor[colorNr].h == targetCol.h)
				{
					currentColor[colorNr].s = targetCol.s;
					currentColor[colorNr].v = targetCol.v;
				}
				else
				{
					currentColor[colorNr].s = currentColor[colorNr].s + round(myDeltaH * satRatio);
					currentColor[colorNr].v = currentColor[colorNr].v + round(myDeltaH * valRatio);
				}
				break;
		}
		if (multiColor)
			parentObj->setCurrColHSV(ledAddrList[colorNr], currentColor[colorNr]);
		else
			for (int i = 0; i < ledAddrListLen; i++)
				parentObj->setCurrColHSV(ledAddrList[i], currentColor[colorNr]);
	}
}

void IoTT_LEDHandler::updateChainData(IoTT_LEDCmdList * cmdDef, IoTT_LEDCmdList * cmdDefLin, uint8_t distance)
{
	if (multiColor)
		for (int i = 0; i < ledAddrListLen; i++)
			updateChainDataForColor(i, cmdDef, cmdDefLin, distance);
	else
		updateChainDataForColor(0, cmdDef, cmdDefLin, distance);
}

void IoTT_LEDHandler::updateBlockDet()
{
	IoTT_LEDCmdList * cmdDef = NULL;
	//get target color based on status

	uint16_t blockStatus = 0;
	for (int8_t i = (ctrlAddrListLen-1); i >= 0; i--) //check for the latest position
		blockStatus = (2 * blockStatus) + (getBDStatus(ctrlAddrList[i]) & 0x01);

	if (lastValue != blockStatus)
	{
		lastValue = blockStatus;
		blinkTimer = millis();
	}
	cmdDef = cmdList[blockStatus];
	//update chain LED's
	if (cmdDef != NULL)
		updateChainData(cmdDef);
}

void IoTT_LEDHandler::updateSwSignalPos(bool isDynamic)
{
	IoTT_LEDCmdList * cmdDef = NULL;
	int16_t nextVal = -1;
	int16_t nextInd = -1;
	uint16_t swiStatus = 0;
	uint8_t swiBitMask = 0x01;
	uint8_t swiChgMask = 0;
	if (isDynamic)
	{
		swiStatus = lastValue;
		uint32_t lastAct = 0;
		uint8_t dynSwi = 0;
		for (int i = 0; i < ctrlAddrListLen; i++) //check for the latest activity
		{
			uint32_t hlpAct = getLastSwitchActivity(ctrlAddrList[i]);
			if (hlpAct > lastAct)
			{
				dynSwi = i;
				lastAct = hlpAct;
			}
		}
		if (lastAct != 0) //we have activity
		{
			if (lastAct != lastActivity)
			{
//				Serial.printf("Updating %i activity for Switch %i\n", dynSwi, ctrlAddrList[dynSwi]);
				swiStatus = 2 * dynSwi;
				if (((getSwiStatus(ctrlAddrList[dynSwi]) >> 4) & 0x02) > 0)
					swiStatus++;
				lastActivity = lastAct;
			}
		}
	}
	else //static switch
	{
		for (int i = 0; i < ctrlAddrListLen; i++)
		{
			if (((getSwiStatus(ctrlAddrList[i]) >> 4) & 0x02) > 0)
				swiStatus |= swiBitMask;
			swiBitMask <<= 1;
		}
	}
	if (lastValue != swiStatus)
	{
		if ((isDynamic) || (ctrlAddrListLen == 1)) //react immediately
		{
			lastValue = swiStatus; //this is the static value
			blinkTimer = millis();
		}
		else //delayed reaction
		{
			if (lastStatValue != swiStatus)
			{
				lastStatValue = swiStatus;
				lastActivity = millis();
			}
			else
				if (millis() > lastActivity + staticTimeout)
				{
					lastValue = swiStatus; //this is the static value
					blinkTimer = millis();
				}
				else
					swiStatus = lastValue; //waiting for timeout, but processing any LED changes like blink
		}
	}
//	Serial.printf("Checking Swi %i Stat %i\n", ctrlAddrList[0], swiStatus);
	for (int i = 0; i < cmdListLen; i++)
	{
//		Serial.printf("Testing Value %i Status %i in Loop %i\n", cmdList[i]->upToVal, swiStatus, i);
		if ((swiStatus <= cmdList[i]->upToVal) && (swiStatus > nextVal))
		{
			nextInd = i;
			nextVal = cmdList[i]->upToVal;
		}
	}
	if (nextInd >= 0)
	{
//		Serial.printf("Updating Switch %i to Position %i Cmd %i \n", ctrlAddrList[0], swiStatus, nextInd);
		cmdDef = cmdList[nextInd];
		updateChainData(cmdDef);
	}
}

void IoTT_LEDHandler::updateSignalPos()
{
	IoTT_LEDCmdList * cmdDef = NULL;
	IoTT_LEDCmdList * cmdDefLin = NULL;
	uint16_t sigAddress = ctrlAddrList[0];
	uint16_t sigAspect = getSignalAspect(ctrlAddrList[0]);
	if (lastValue != sigAspect)
	{
		lastValue = sigAspect;
		blinkTimer = millis();
	}
//    Serial.printf("Checking Signal %i to Aspect %i  \n", sigAddress, sigAspect);
	int16_t nextVal = -1;
	int16_t nextInd = -1;
	uint8_t distance = 0;
	for (int i = 0; i < cmdListLen; i++)
	{
//		if (sigAddress == 799)
//			Serial.printf("%i %i %i %i \n", i, sigAspect, cmdList[i]->upToVal, nextInd);
		if ((sigAspect <= cmdList[i]->upToVal) && (sigAspect > nextVal))
		{
			nextInd = i;
			nextVal = cmdList[i]->upToVal;
		}
	}
	if (nextInd >= 0)
	{
//		if (sigAddress == 799)
//			Serial.printf("Updating Signal %i to Aspect %i Cmd %i \n", sigAddress, sigAspect, nextInd);
		cmdDef = cmdList[nextInd];
		if ((nextInd > 0) && (displType == linear))
		{
 			cmdDefLin = cmdList[nextInd-1];
			distance = round(((float_t)(sigAspect - cmdDefLin->upToVal) / (float_t)(cmdDef->upToVal - cmdDefLin->upToVal)) * 100);
//			Serial.printf("Linear Signal %i to Aspect %i Curr %i Prev %i Dist %i \n", sigAddress, sigAspect, cmdDef->upToVal, cmdDefLin->upToVal, distance);
			if (distance < 100)
				updateChainData(cmdDef, cmdDefLin, distance);
			else
				updateChainData(cmdDef);
		}
		else
			updateChainData(cmdDef);
	}
}

void IoTT_LEDHandler::updateButtonPos()
{
	IoTT_LEDCmdList * cmdDef = NULL;
	uint16_t btnNr = ctrlAddrList[0];
	uint16_t btnState = getButtonValue(btnNr);
	int16_t nextVal = -1;
	int16_t nextInd = -1;
	if (lastValue != btnState)
	{
		lastValue = btnState;
		blinkTimer = millis();
	}
	for (int i = 0; i < cmdListLen; i++)
	{
		if ((btnState <= cmdList[i]->upToVal) && (btnState > nextVal))
		{
			nextInd = i;
			nextVal = cmdList[i]->upToVal;
		}
	}
	if (nextInd >= 0)
	{
//    Serial.printf("Updating Button %i to Aspect %i Cmd %i \n", btnNr, btnState, nextInd);
		cmdDef = cmdList[nextInd];
		updateChainData(cmdDef);
	}
}

void IoTT_LEDHandler::updateAnalogValue()
{
	IoTT_LEDCmdList * cmdDef = NULL;
	IoTT_LEDCmdList * cmdDefLin = NULL;
	uint16_t analogNr = ctrlAddrList[0];
	uint16_t analogVal = getAnalogValue(analogNr);
	if (lastValue != analogVal)
	{
		lastValue = analogVal;
		blinkTimer = millis();
	}
	int16_t nextVal = -1;
	int16_t nextInd = -1;
	int16_t prevInd = -1;
	uint8_t distance = 0;
//    Serial.printf("Analog Input %i to Value %i %i levels\n", analogNr, analogVal, cmdListLen);
	for (int i = 0; i < cmdListLen; i++)
	{
//		Serial.printf("Checking %i < %i \n", analogVal, cmdList[i]->upToVal);
		if ((analogVal <= cmdList[i]->upToVal) && (analogVal > nextVal))
		{
			prevInd = nextInd;
			nextInd = i;
			nextVal = cmdList[i]->upToVal;
		}
	}
	cmdDef = cmdList[nextInd];
	if ((nextInd > 0) && (displType == linear))
	{
		cmdDefLin = cmdList[nextInd-1];
		distance = round(((float_t)(analogVal - cmdDefLin->upToVal) / (float_t)(cmdDef->upToVal - cmdDefLin->upToVal)) * 100);
//			Serial.printf("Linear Signal %i to Aspect %i Curr %i Prev %i Dist %i \n", sigAddress, analogVal, cmdDef->upToVal, cmdDefLin->upToVal, distance);
		if (distance < 100)
			updateChainData(cmdDef, cmdDefLin, distance);
		else
			updateChainData(cmdDef);
	}
	else
		updateChainData(cmdDef);
}

void IoTT_LEDHandler::updateTransponder()
{
	IoTT_LEDCmdList * cmdDef = NULL;
	//get target color based on status

	if (lastValue != lastExtStatus)
	{
		lastValue = lastExtStatus;
		blinkTimer = millis();
	}
	cmdDef = cmdList[lastExtStatus];
	//update chain LED's
	if (cmdDef != NULL)
		updateChainData(cmdDef);
}

void IoTT_LEDHandler::updatePowerStatus()
{
	IoTT_LEDCmdList * cmdDef = NULL;
	int16_t nextVal = -1;
	int16_t nextInd = -1;
	if (lastValue != getPowerStatus())
	{
		lastValue = getPowerStatus();
		blinkTimer = millis();
	}
	for (int i = 0; i < cmdListLen; i++)
	{
		if ((getPowerStatus() <= cmdList[i]->upToVal) && (getPowerStatus() > nextVal))
		{
			nextInd = i;
			nextVal = cmdList[i]->upToVal;
		}
	}
	if (nextInd >= 0)
	{
//    Serial.printf("Updating Power Status to Status %i Cmd %i \n", getPowerStatus(), nextInd);
		cmdDef = cmdList[nextInd];
		updateChainData(cmdDef);
	}
}

void IoTT_LEDHandler::updateConstantLED()
{
	IoTT_LEDCmdList * cmdDef = cmdList[0];
	updateChainData(cmdDef);
//    Serial.printf("Updating Constant LED %i Cmd %i \n", 0, 0);
}

//enum sourceType : uint8_t {evt_button=0, evt_analogvalue=1, evt_trackswitch=2, evt_signalmastdcc=3, evt_signalmastdyn=4, evt_blockdetector=5, evt_transponder=6, evt_powerstat=7, evt_alwayson=8, evt_nosource=255};

void IoTT_LEDHandler::updateLEDs()
{
//	Serial.print("2");
    switch (ctrlSource)
    {
		case evt_blockdetector: updateBlockDet(); break;
		case evt_signalmastdyn: updateSwSignalPos(true); break;
		case evt_trackswitch:  updateSwSignalPos(false); break;
		case evt_signalmastdcc: updateSignalPos(); break;
		case evt_button: updateButtonPos(); break;
		case evt_analogvalue: updateAnalogValue(); break;
		case evt_transponder: updateTransponder(); break;
		case evt_powerstat: updatePowerStatus(); break;
		case evt_alwayson: updateConstantLED(); break;
    }
}

void IoTT_LEDHandler::processTranspEvent(uint16_t btnAddr, uint16_t eventValue)
{
	Serial.printf("Transponder event Zone %i Move %i Addr %i\n", btnAddr, (eventValue & 0x8000) >> 15, eventValue & 0x7FFF);
	if (ctrlAddrListLen > 0)
		if (ctrlAddrList[0] == btnAddr)
			if (condAddrListLen > 0)
				for (uint16_t i = 0; i < condAddrListLen; i++)
					if (condAddrList[i] == (eventValue & 0x7FFF))
					{
						Serial.printf("Execute Transponder event Zone %i Move %i Addr %i\n", btnAddr, (eventValue & 0x8000) >> 15, eventValue & 0x7FFF);
						lastExtStatus = ((eventValue & 0x8000)>>15) ^ 0x01;
						break;
					}
}

void IoTT_LEDHandler::loadLEDHandlerJSON(JsonObject thisObj)
{
	freeObjects();
//	Serial.println("Loading LED Object");
	if (thisObj.containsKey("LEDNums"))
	{
		JsonArray LEDNums = thisObj["LEDNums"];
		ledAddrListLen = LEDNums.size();
		ledAddrList = (uint16_t*) realloc (ledAddrList, ledAddrListLen * sizeof(uint16_t));
		currentColor = (CHSV*) realloc (currentColor, ledAddrListLen * sizeof(CHSV));
		for (int i=0; i<ledAddrListLen;i++)
		{
			ledAddrList[i] = LEDNums[i];
			currentColor[i] = CHSV(0,0,0);
		}
	}
	if (thisObj.containsKey("MultiColor"))
		multiColor = thisObj["MultiColor"];
	else
		multiColor = false;
	if (thisObj.containsKey("CtrlSource"))
		ctrlSource = getLEDActionTypeByName(thisObj["CtrlSource"]);
	if (thisObj.containsKey("CtrlAddr"))
	{
		JsonArray CtrlAddr = thisObj["CtrlAddr"];
		if (CtrlAddr.isNull())
			ctrlAddrListLen = 1;
		else
			ctrlAddrListLen = CtrlAddr.size();
		ctrlAddrList = (uint16_t*) realloc (ctrlAddrList, ctrlAddrListLen * sizeof(uint16_t));
		if (CtrlAddr.isNull())
			ctrlAddrList[0] = thisObj["CtrlAddr"];
		else
			for (int i=0; i<ctrlAddrListLen;i++)
				ctrlAddrList[i] = CtrlAddr[i];
	}
	if (thisObj.containsKey("CondAddr"))
	{
		JsonArray CondAddr = thisObj["CondAddr"];
		if (!CondAddr.isNull())
		{
			condAddrListLen = CondAddr.size();
			condAddrList = (uint16_t*) realloc (condAddrList, condAddrListLen * sizeof(uint16_t));
			for (int i=0; i<condAddrListLen;i++)
				condAddrList[i] = CondAddr[i];
		}
	}
	if (thisObj.containsKey("DisplayType"))
		displType = getDisplayTypeByName(thisObj["DisplayType"]);
	if (thisObj.containsKey("LEDCmd"))
	{
		JsonArray LEDCmd = thisObj["LEDCmd"];
		cmdListLen = LEDCmd.size();
		bool useOldFormat = ((ctrlSource == evt_trackswitch) && (ctrlAddrListLen == 1) && (cmdListLen == 4)); //old format
		if (useOldFormat)
			cmdListLen = 2;
		
        cmdList = (IoTT_LEDCmdList**) realloc (cmdList, cmdListLen * sizeof(IoTT_LEDCmdList*));
		for (int i=0; i<cmdListLen;i++)
		{
          IoTT_LEDCmdList * thisCmdEntry = new(IoTT_LEDCmdList);
          thisCmdEntry->parentObj = this;
          if (useOldFormat)
          {
			thisCmdEntry->loadCmdListJSON(LEDCmd[2*i]);
			thisCmdEntry->upToVal = i;
		  }
          else
			thisCmdEntry->loadCmdListJSON(LEDCmd[i]);
          cmdList[i] = thisCmdEntry;
		}
	}
}


//******************************************************************************************************************************************************************
IoTT_ledChain::IoTT_ledChain(TwoWire * newWire, uint16_t useI2CAddr, bool multiRequest)
{
	thisWire = newWire;
	if (thisWire)
	{
		Serial.printf("Init LED Chain I2C Mode %2X\n", useI2CAddr);
		chainMode = hatI2C;
		I2CAddr = useI2CAddr;
	}
	else
	{
		Serial.printf("Init LED Chain direct Mode %2X \n", useI2CAddr);
		chainMode = hatDirect;
	}
	useMultiByte = multiRequest;
#ifdef useRTOS
	ledBaton = xSemaphoreCreateMutex();
#endif
}

IoTT_ledChain::~IoTT_ledChain()
{
	freeObjects();
}

void IoTT_ledChain::freeObjects()
{
	if (colorDefListLen > 0)
	{
//		for (uint16_t i = 0; i < colorDefListLen; i++)
		for (uint16_t i = 0; i < (sizeof(colorDefinitionList) / sizeof(colorDefinitionList[0])); i++)
			delete colorDefinitionList[i];
		colorDefListLen = 0;
		free(colorDefinitionList);
	}
	if (LEDHandlerListLen > 0)
	{
		for (uint16_t i = 0; i < LEDHandlerListLen; i++)
			delete LEDHandlerList[i];
		LEDHandlerListLen = 0;
		free(LEDHandlerList);
	}
}

void IoTT_ledChain::loadLEDChainJSON(DynamicJsonDocument doc, bool resetList)
{
	JsonObject thisObj = doc.as<JsonObject>();
	loadLEDChainJSONObj(thisObj, resetList);
}

void IoTT_ledChain::loadLEDChainJSONObj(JsonObject doc, bool resetList)
{
	if (resetList)
		freeObjects();
//	Serial.println("Load Chain Params");
	if (doc.containsKey("ChainParams"))
    {
        chainLength = doc["ChainParams"]["NumLEDs"];
//        Serial.println(chainLength);
        if (thisWire)
			if (chainLength > i2cMaxChainLength)
				chainLength = i2cMaxChainLength; //for safety. Value limited in config already
        currentBrightness = doc["ChainParams"]["Brightness"]["InitLevel"];
        if (doc.containsKey("MQTT"))
        {
			JsonObject myMQTT = doc["MQTT"];
			if (myMQTT.containsKey("Subscribe"))
			{
				JsonArray myMQTTSub = myMQTT["Subscribe"];
				if (myMQTTSub.size() == 2)
					for (int i = 0; i < myMQTTSub.size(); i++)
					{
						strcpy(subTopicList[i].topicName, myMQTTSub[i]["Topic"]);
						subTopicList[i].inclAddr = myMQTTSub[i]["InclAddr"];
					}
			}
			if (myMQTT.containsKey("Publish"))
			{
				JsonArray myMQTTSub = myMQTT["Publish"];
				if (myMQTTSub.size() == 1)
					for (int i = 0; i < myMQTTSub.size(); i++)
					{
						strcpy(pubTopicList[i].topicName, myMQTTSub[i]["Topic"]);
						pubTopicList[i].inclAddr = myMQTTSub[i]["InclAddr"];
					}
			}
		}
		brightnessControlType = getLEDActionTypeByName(doc["ChainParams"]["Brightness"]["CtrlSource"]);
        brightnessControlAddr = doc["ChainParams"]["Brightness"]["Addr"];
        blinkInterval = doc["ChainParams"]["BlinkPeriod"];
        char colType[10];
        strcpy(colType, doc["ChainParams"]["ColorSeq"]);
		colTypeNum = 0x66;
        if (strcmp(colType, "RGB") == 0)
			colTypeNum = 0x0C;
        if (strcmp(colType, "GRB") == 0)
			colTypeNum = 0x66;
			
        initChain(chainLength);
		switch (brightnessControlType)
		{
			case evt_analogvalue: setAnalogValue(brightnessControlAddr, round(4095 * currentBrightness)); break;
			default: currentBrightness = 1.0; break;
		}
    }
    else
		Serial.println("No Chain Params");
	Serial.println("Load Colors");
    if (doc.containsKey("LEDCols"))
    {
        JsonArray LEDCols = doc["LEDCols"];
        colorDefListLen = LEDCols.size();
        colorDefinitionList = (IoTT_ColorDefinitions**) realloc (colorDefinitionList, colorDefListLen * sizeof(IoTT_ColorDefinitions*));
        for (int i=0; i < colorDefListLen; i++)
        {
			
          IoTT_ColorDefinitions * thisColorDefEntry = new(IoTT_ColorDefinitions);
          thisColorDefEntry->loadColDefJSON(LEDCols[i]);
          colorDefinitionList[i] = thisColorDefEntry;
        }
    }
	else
		Serial.println("No LED Colors defined");
//	Serial.println("Load LED Defs");
    if (doc.containsKey("LEDDefs"))
    {
        JsonArray LEDDefs = doc["LEDDefs"];
        uint16_t newListLen = LEDDefs.size();
        LEDHandlerList = (IoTT_LEDHandler**) realloc (LEDHandlerList, (newListLen + LEDHandlerListLen) * sizeof(IoTT_LEDHandler*));
        for (int i=0; i < newListLen; i++)
        {
			IoTT_LEDHandler * thisLEDHandlerEntry = new(IoTT_LEDHandler);
			thisLEDHandlerEntry->parentObj = this;
			thisLEDHandlerEntry->loadLEDHandlerJSON(LEDDefs[i]);
			LEDHandlerList[LEDHandlerListLen + i] = thisLEDHandlerEntry;
		}
		LEDHandlerListLen += newListLen;
        Serial.printf("%i LED Defs loaded\n", LEDHandlerListLen);
	}
	else
		Serial.println("No LED Chain defined");
//	Serial.println("Load LED Defs Complete");
}

IoTT_ColorDefinitions * IoTT_ledChain::getColorByName(String colName)
{
	for (int i=0; i<colorDefListLen;i++)
	{
		IoTT_ColorDefinitions * thisPointer = colorDefinitionList[i];
		if (String(thisPointer->colorName) == colName)
			return thisPointer;
	}
	return NULL;
}

uint16_t IoTT_ledChain::getChainLength()
{
	return chainLength;
}

CRGB * IoTT_ledChain::getChain()
{
	return ledChain;
}

float_t IoTT_ledChain::getBrightness()
{
	return currentBrightness;
}

void IoTT_ledChain::setBrightness(float_t newVal)
{
	currentBrightness = newVal;
}

sourceType IoTT_ledChain::getBrightnessControlType()
{
	return brightnessControlType;
}

uint16_t IoTT_ledChain::getBrightnessControlAddr()
{
	return brightnessControlAddr;
}


CRGB * IoTT_ledChain::initChain(word numLEDs)
{
    ledChain = (CRGB*) realloc (ledChain, numLEDs * sizeof(CRGB));
    for (int i = 0; i < numLEDs; i++)
		setCurrColHSV(i, CHSV(1,255,0)); //initialize dark
	needUpdate = true;
	refreshAnyway = true;
	blinkTimer = millis() + blinkInterval;
	ledUpdateTimer = millis() + ledUpdateInterval + 5; //5ms ofset to Buttons
	globFaderValue = 0;
	return ledChain;
}

void IoTT_ledChain::setMQTTMode(mqttTxFct txFct)
{
	txMQTT = txFct;
}

void IoTT_ledChain::subscribeTopics()
{
	for (int i = 0; i < 2; i++)
	{
		Serial.printf("Subscribe LED Topic %i \n", i);
		String thisTopic = String(subTopicList[i].topicName);
		if (subTopicList[i].inclAddr)
			thisTopic += "/#";
		if (txMQTT) txMQTT(0, &thisTopic[0], NULL);
	}
}

void IoTT_ledChain::setCurrColHSV(uint16_t ledNr, CHSV newCol)
{
//	Serial.printf("Set HSV %i to %i\n", ledNr, newCol);
	if ((ledNr >= 0) && (ledNr < chainLength))
	{
	#ifdef useRTOS
		xSemaphoreTake(ledBaton, portMAX_DELAY);
	#endif
	    switch (chainMode)
	    {
			case hatDirect:
//			    Serial.println("call Direct");
				ledChain[ledNr] = newCol;
				break;
			case hatI2C:
//			    Serial.printf("call setI2CLED for LED %i\n", ledNr);
				setI2CLED(ledNr, newCol);
//			    Serial.println("call SerComm Done");
				break;
		}
		needUpdate = true;
	#ifdef useRTOS
		xSemaphoreGive(ledBaton);
	#endif
	}
}

void IoTT_ledChain::setBlinkRate(uint16_t blinkVal)
{
	blinkInterval = blinkVal;
}

void IoTT_ledChain::identifyLED(uint16_t ledNr)
{
	bool idResult = false;
	if (ledNr < getChainLength())
		if (LEDHandlerListLen > 0)
		{
			for (int i = 0; i < LEDHandlerListLen; i++)
			{
				if (LEDHandlerList[i]->identifyLED(ledNr))
				{
					idResult = true;
					tempLEDBuffer[tempLEDCtr] = ledNr;
					tempLEDCtr++;
					break;
				}
			}
			if (!idResult) //LED not defined, so lighten manually, but need to switch off later
			{
				setCurrColHSV(ledNr, CHSV(0,0,255));
				tempLEDBuffer[tempLEDCtr] = ledNr;
				tempLEDCtr++;
			}
			endIdentify = millis() + identifyTimeout;
		}
}

void IoTT_ledChain::setRefreshInterval(uint16_t newInterval)
{
	ledUpdateInterval = newInterval;
}

bool IoTT_ledChain::getBlinkStatus()
{
	return blinkStatus;
}

float_t IoTT_ledChain::getFaderValue()
{
	if (blinkStatus)
	  return globFaderValue;
	else
	  return (1 - globFaderValue);
}

/*
CRGB IoTT_ledChain::getCurrColRGB(uint16_t ledNr)
{
	return ledChain[ledNr];
}
*/

CHSV IoTT_ledChain::getCurrColHSV(uint16_t ledNr)
{
	return rgb2hsv_approximate(ledChain[ledNr]);
}

void IoTT_ledChain::updateLEDs()
{
	switch (getBrightnessControlType())
	{
		case evt_analogvalue: setBrightness((float)getAnalogValue(getBrightnessControlAddr())/4095); break;
		default: setBrightness(1.0); break;
	}
//	Serial.print("1");
	if (LEDHandlerListLen > 0)
		for (uint16_t i = 0; i < LEDHandlerListLen; i++)
			LEDHandlerList[i]->updateLEDs();
		
}

void IoTT_ledChain::showI2CLED()
{
//	Serial.printf("Show LED Chain I2C Mode %2X\n", I2CAddr);
	thisWire->beginTransmission(I2CAddr);
	thisWire->write(0xFF);
	thisWire->endTransmission();
}

void IoTT_ledChain::setI2CLED(uint16_t ledNr, CHSV newCol)
{
//	Serial.printf("LED to %2X data %2X %2X %2X %2X %2X \n", I2CAddr, (ledNr & 0xFF00)>>8, ledNr & 0x00FF, newCol.h, newCol.s, newCol.v);
	thisWire->beginTransmission(I2CAddr);
	thisWire->write((ledNr & 0xFF00)>>8);
	thisWire->write(ledNr & 0x00FF);
	if (newCol != lastCol)
	{
		thisWire->write(newCol.h);
		thisWire->write(newCol.s);
		thisWire->write(newCol.v);
		lastCol = newCol;
	}
	thisWire->endTransmission(false);
	ledChain[ledNr] = newCol;
}
/*
void IoTT_ledChain::initI2CLED()
{
	int devID = -1;
	int successCtr = 0;
	for (int i = 0; i < 25; i++)
	{
		devID = pingI2CDevice();
		Serial.println(devID);
		if (devID >= 0)
		{
			successCtr++;
			if (successCtr > 5)
				break;
		}
		else
			delay(100);
	}
	if (devID < 0)
		ESP.restart();
	Serial.printf("Set LED params L %i T %i\n", chainLength, colTypeNum);
	setI2CChainLen(chainLength);
	setI2CLEDType(colTypeNum);
	resetI2CDevice(false);
	successCtr = 0;
	for (int i = 0; i < 25; i++)
	{
		devID = pingI2CDevice();
		Serial.println(devID);
		if (devID >= 0)
		{
			successCtr++;
			if (successCtr > 5)
			break;
		}
		else
			delay(100);
	}
	if (devID < 0)
		ESP.restart();
	setI2CLED(0xFF, CHSV(0,0,0)); //set all dark
	Serial.printf("Init I2C Chain Addr %2X with %i LED Type %i\n", I2CAddr, chainLength, colTypeNum);
}
*/

void IoTT_ledChain::resetI2CWDT()
{
//	Serial.printf("Reset WDT %i  \n", millis());
	thisWire->beginTransmission(I2CAddr);
	thisWire->write(0x00); //reset prgPtr
	thisWire->endTransmission();
}

bool IoTT_ledChain::isVerified()
{
	if (chainMode == hatDirect)
		return true;
	else
		return i2cVerified;
}

int8_t IoTT_ledChain::pingI2CDevice(uint8_t numBytes)
{
	while (thisWire->available())
		char c = thisWire->read();
	uint8_t devData[numBytes];
//	Serial.printf("Ping device %2X for %i bytes \n", I2CAddr, numBytes);
	
	uint8_t byteCount = 0;
	if (useMultiByte)
	{
		thisWire->requestFrom(I2CAddr, numBytes);
		uint32_t untilTime = millis() + 100;
		while (millis() < untilTime)
		{
			while (thisWire->available())
			{
				devData[byteCount] = (uint8_t)thisWire->read();
				byteCount++;
			}
		}
	}
	else
	{
		resetI2CWDT();
		for (byteCount = 0; byteCount < 5; byteCount++ )
		{
			thisWire->requestFrom(I2CAddr, 1);
			if (thisWire->available())
			{
				devData[byteCount] = (uint8_t)thisWire->read();
//				Serial.print(devData[byteCount],16);
//				Serial.print(", ");
			}
		}
	}
//	Serial.println();
//	Serial.println(byteCount);
	if (byteCount >= numBytes)
	{
		if (devData[0] > 0)
		{
			i2cDevID = devData[0]; //0x55 YellowHat 0x56 GreenHat
			i2cChainLength = (devData[1]<<8) + devData[2];
			i2cChainType = (devData[3]<<8) + devData[4];
		}
		return devData[0];
	}
	return -1;
}

void IoTT_ledChain::resetI2CDevice(bool forceReset)
{
	Serial.println("Cond restart");
	thisWire->beginTransmission(I2CAddr);
	thisWire->write(0xFE);
	thisWire->write(0xFF); //restart command
	thisWire->write(0);
	thisWire->write(forceReset ? 1:0);
	thisWire->endTransmission();
	Serial.println(forceReset ? 1:0);
	delay(50);
}
void IoTT_ledChain::setI2CChainLen(uint16_t chainLen)
{
	Serial.printf("Send Chain Length %i\n", chainLen);
	thisWire->beginTransmission(I2CAddr);
	thisWire->write(0xFE);
	thisWire->write(0x00);
	thisWire->write((chainLen & 0xFF00) >> 8);
	thisWire->write(chainLen & 0x00FF);
	thisWire->endTransmission();
	Serial.println((chainLen & 0xFF00) >> 8);
	Serial.println(chainLen & 0x00FF);
	delay(50);
}

void IoTT_ledChain::setI2CLEDType(uint16_t ledType)
{
	Serial.printf("Send Chain LED Type %i\n", ledType);
	thisWire->beginTransmission(I2CAddr);
	thisWire->write(0xFE);
	thisWire->write(0x01);
	Serial.println((ledType & 0xFF00) >> 8);
	Serial.println(ledType & 0x00FF);
	thisWire->write((ledType & 0xFF00) >> 8);
	thisWire->write(ledType & 0x00FF);
	thisWire->endTransmission();
	delay(50);
}

void IoTT_ledChain::processBtnEvent(sourceType inputEvent, uint16_t btnAddr, uint16_t eventValue)
{
	switch (inputEvent)
	{
		case evt_transponder:
//			Serial.println("Step 1");
			if (LEDHandlerListLen > 0)
			{
//				Serial.println("Step 2");
				for (uint16_t i = 0; i < LEDHandlerListLen; i++)
				{
//					Serial.printf("Step 3 Handler %i Evt %i\n", i, LEDHandlerList[i]->ctrlSource);
					if (LEDHandlerList[i]->ctrlSource == inputEvent)
					{
//						Serial.println("Step 4");
						LEDHandlerList[i]->processTranspEvent(btnAddr, eventValue);
					}
				}
				
			}
			break;
	}
}

void IoTT_ledChain::processChain()
{
	if (txMQTT) return;
//	Serial.print("-");
	uint32_t currMillis = millis();
	bool rollOver = (currMillis < procRollover);
	procRollover = millis();
    uint16_t timeElapsed = blinkTimer - millis();
	if ((millis() > blinkTimer) || rollOver)
	{
		blinkStatus = !blinkStatus;
		blinkTimer += blinkInterval;
        timeElapsed = 0; 
		if (millis() > blinkTimer) //exception correction in case something is not initialized.
		{
		  blinkTimer = millis() + blinkInterval;
		  timeElapsed = 0;
	    }
	}
    globFaderValue = 1 - ((float_t)timeElapsed/(float_t)blinkInterval);  //positive slope ramp from 0 to 1
//    Serial.printf("%f\n", globFaderValue);
	if ((millis() > ledUpdateTimer) || rollOver)
	{
//		Serial.print(".");
		ledUpdateTimer += ledUpdateInterval;
		if (millis() > ledUpdateTimer) //exception correction in case something is not initialized.
		  ledUpdateTimer = millis() + ledUpdateInterval;

		if ((thisWire) && (!i2cVerified))
		{
			pingCtr++;
			if (pingCtr > 20)
			{
				pingCtr = 0;
				i2cDevID = pingI2CDevice(5);
				if (i2cChainType != colTypeNum)
					setI2CLEDType(colTypeNum);
				else
					if (i2cChainLength != chainLength)
						setI2CChainLen(chainLength);
					else
					{
						resetI2CDevice(false);
						i2cVerified = true;
					}
			}
		}	
		else
		{
			if ((millis() > endIdentify) || rollOver)
			{
				updateLEDs();
				if (tempLEDCtr > 0)
					while (tempLEDCtr > 0)
					{
						tempLEDCtr--;
						setCurrColHSV(tempLEDBuffer[tempLEDCtr], CHSV(0,0,0));
					}
			}
		}
	}
	
	if ((needUpdate) || refreshAnyway)
	{
//		Serial.println("update LEDs");
#ifdef useRTOS
		xSemaphoreTake(ledBaton, portMAX_DELAY);
#endif
	    switch (chainMode)
	    {
			case hatDirect:
				FastLED.show();
				break;
			case hatI2C:
				showI2CLED();
				break;
		}
		needUpdate = false;
		refreshAnyway = false;
#ifdef useRTOS
		xSemaphoreGive(ledBaton);
#endif
	}
}

void IoTT_ledChain::sendLEDStatusMQTT(uint16_t ledNr)
{
//	Serial.printf("Send LED %i\n", ledNr);
	CRGB currRGB = ledChain[ledNr];
	CHSV currHSV = rgb2hsv_approximate(currRGB);
	char myStatusMsg[200];
	DynamicJsonDocument doc(300);
	doc["LEDNr"] = ledNr;
	JsonArray thisRGB = doc.createNestedArray("RGB");
	JsonArray thisHSV = doc.createNestedArray("HSV");
	thisRGB.add(currRGB.r);
	thisRGB.add(currRGB.g);
	thisRGB.add(currRGB.b);
	thisHSV.add(currHSV.h);
	thisHSV.add(currHSV.s);
	thisHSV.add(currHSV.v);
	serializeJson(doc, myStatusMsg);
	String thisTopic = String(pubTopicList[0].topicName);
	if (pubTopicList[0].inclAddr)
		thisTopic += ("/" + String(ledNr));
//	Serial.println(thisTopic);
//	Serial.println(myStatusMsg);
	if (txMQTT)
		txMQTT(1, &thisTopic[0], &myStatusMsg[0]);
}

bool IoTT_ledChain::processMQTTCmd(char * topic, DynamicJsonDocument doc)
{
	if (!txMQTT) return false; //called in error, so return

	topicStruct * thisTopic = NULL;
	String topicStr = String(topic);
	for (int i = 0; i < 2; i ++)
		if (topicStr.indexOf(subTopicList[i].topicName) >= 0)
		{
			thisTopic = &subTopicList[i];
			break;
		}
	if (!thisTopic)
		for (int i = 0; i < 1; i ++)
			if (topicStr.indexOf(pubTopicList[i].topicName) >= 0)
			{
				thisTopic = &pubTopicList[i];
				break;
			}
	if (!thisTopic) return false; //not a valid topic for the LED Chain
//	Serial.println(thisTopic->topicName);
	
	int ledAddr[20];
	uint16_t numLED = 0;
	
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
				ledAddr[0] = addrStr.toInt();
				numLED = 1;
			}
		}
//		Serial.printf("Pos: %i Val: %i\n", lastSlash, ledAddr);
	}
//else extract address or address array from payload
	else
	{
		if (doc.containsKey("LEDNr"))
		{
			JsonArray ledList = doc["LEDNr"];
			if (ledList)
			{
				for (int i = 0; i < ledList.size(); i++)
					ledAddr[i] = ledList[i];
				numLED = ledList.size();
//				Serial.printf("LED Array %i\n", numLED);
			}
			else
			{
				ledAddr[0] = doc["LEDNr"];
				numLED = 1;
//				Serial.printf("Single LED %i\n", ledAddr[0]);
			}
		}
	}
	
	if (numLED == 0) return false; //no LED Nr found, just return
	
	CHSV thisCol;
	if (thisTopic == &subTopicList[0])//: //if SET command
	{
		//extract RGB or HSV value 
		if (doc.containsKey("RGB"))
		{
			JsonArray HSVArray = doc["RGB"];
			thisCol = rgb2hsv_approximate(CRGB(HSVArray[0], HSVArray[1], HSVArray[2]));
		}
		else
			if (doc.containsKey("HSV"))
			{
				JsonArray HSVArray = doc["HSV"];
				thisCol = CHSV(HSVArray[0], HSVArray[1], HSVArray[2]);
			}
		//set LEDs to value
		for (int i = 0; i < numLED; i++)
			if (ledAddr[i] < chainLength)
			{
				switch (chainMode)
				{
					case hatDirect:
						ledChain[ledAddr[i]] = thisCol;
						break;
					case hatI2C:
						setI2CLED(ledAddr[i], thisCol);
						break;
				}
			}
		switch (chainMode)
		{
			case hatDirect:
				FastLED.show();
				break;
			case hatI2C:
				showI2CLED();
				break;
		}
		return true;
	}
	if (thisTopic == &subTopicList[1])//: //if ASK command
	{
		//send HSV Value of requested LEDs
		for (int i = 0; i < numLED; i++)
			if (ledAddr[i] < chainLength)
				sendLEDStatusMQTT(ledAddr[i]);
		return true;
	}
}


