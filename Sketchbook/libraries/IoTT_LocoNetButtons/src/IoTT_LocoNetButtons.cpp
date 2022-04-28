#include <IoTT_LocoNetButtons.h>

sourceType getSourceTypeByName(String transName)
{
  if (transName == "button") return evt_button;
  if (transName == "analogvalue") return evt_analogvalue;
  if (transName == "analogscaler") return evt_analogscaler;
  if (transName == "switch") return evt_trackswitch;
  if (transName == "dccsignal") return evt_signalmastdcc;
  if (transName == "dynsignal") return evt_signalmastdyn;
  if (transName == "blockdetector") return evt_blockdetector;
  if (transName == "transponder") return evt_transponder;
  if (transName == "power") return evt_powerstat;
  return evt_trackswitch;
};

enableType getEnableTypeByName(String enableName)
{
  if (enableName == "on") return ent_alwayson;
  if (enableName == "off") return ent_alwaysoff;
  if (enableName == "switch") return ent_switch;
  if (enableName == "button") return ent_button;
  if (enableName == "block") return ent_block;
  return ent_alwayson;
};

outputType getActionTypeByName(String actionName)
{ //blockdet, dccswitch, dccswitchack, dccswitchreport, dccsignal, svbutton, analoginp, powerstat
  if (actionName == "block") return blockdet; 
  if (actionName == "switch") return dccswitchreq;
  if (actionName == "switchack") return dccswitchack;
  if (actionName == "switchrep") return dccswitchrep;
  if (actionName == "signal") return dccsignalnmra;
  if (actionName == "button") return svbutton;
  if (actionName == "analog") return analoginp;
  if (actionName == "power") return powerstat;
  return unknown; 
}

buttonEvent getButtonEventByName(String typeName)
{
  if (typeName == "btndown") return onbtndown;
  if (typeName == "btnup") return onbtnup;
  if (typeName == "btnclick") return onbtnclick;
  if (typeName == "btnhold") return onbtnhold;
  if (typeName == "btncblclick") return onbtndblclick;
  return noevent;
}

ctrlTypeType getCtrlTypeByName(String typeName)
{
  if (typeName == "closed") return closed;
  if (typeName == "thrown") return thrown;
  if (typeName == "toggle") return toggle;
  if (typeName == "nochange") return nochange;
  if (typeName == "input") return input;
  return (ctrlTypeType) typeName.toInt();;
}

ctrlValueType getCtrlValueByName(String valueName)
{
	if (valueName == "on") return onVal;
	if (valueName == "off") return offVal;
	if (valueName == "idle") return idleVal;
    return (ctrlValueType) valueName.toInt();
}

ctrlValueType getPowerEventByName(String valueName)
{
	if (valueName == "on") return onVal;
	if (valueName == "off") return offVal;
	if (valueName == "idle") return idleVal;
	if (valueName == "toggle") return toggleVal;
	return idleVal;
}


/*----------------------------------------------------------------------------------------------------------------------*/

IoTT_BtnHandlerCmd::IoTT_BtnHandlerCmd()
{
//	Serial.println("Obj BtnHdlCmd");
}

IoTT_BtnHandlerCmd::~IoTT_BtnHandlerCmd()
{
}

void IoTT_BtnHandlerCmd::loadButtonCfgJSON(JsonObject thisObj)
{
	targetType = getActionTypeByName(thisObj["CtrlTarget"]);
	targetAddr = thisObj["CtrlAddr"];
	switch (targetType)
	{
		case svbutton:
			cmdType = getButtonEventByName(thisObj["CtrlType"]);
			break;
		case powerstat:
			cmdType = getPowerEventByName(thisObj["CtrlType"]);
//			Serial.println(cmdType);			
			break;
		default:
			cmdType = getCtrlTypeByName(thisObj["CtrlType"]);
			break;
	}
	cmdValue = getCtrlValueByName(thisObj["CtrlValue"]);
	if (thisObj.containsKey("ExecDelay"))
		execDelay = thisObj["ExecDelay"];
	else
		execDelay = 250;
}

void IoTT_BtnHandlerCmd::executeBtnEvent()
{
//	Serial.printf("Handling Button Command to Addr %i Type %i Value %i\n", targetAddr, cmdType, cmdValue);
    switch (targetType)
    {
      case dccswitchreq: if (sendSwitchCommand) sendSwitchCommand(0xB0, targetAddr, cmdType, cmdValue); break; //switch
      case dccswitchack: if (sendSwitchCommand) sendSwitchCommand(0xBD, targetAddr, cmdType, cmdValue); break; //switch
      case dccswitchrep: if (sendSwitchCommand) sendSwitchCommand(0xB1, targetAddr, cmdType, cmdValue); break; //switch
      case dccsignalnmra: if (sendSignalCommand) sendSignalCommand(targetAddr, cmdValue); break; //signal
      case powerstat: 
			if (sendPowerCommand) 
				if (cmdType == 3) //toggle
					sendPowerCommand(cmdType, 0); 
				else
					sendPowerCommand(0, cmdType); 
			break; //power status
      case blockdet: if (sendBlockDetectorCommand) sendBlockDetectorCommand(targetAddr, cmdValue); break; //analog
      case svbutton:  if (sendButtonCommand) sendButtonCommand(targetAddr, cmdType); break; //button command
      case analoginp:  if (sendAnalogCommand) sendAnalogCommand(targetAddr, ((cmdType & 0x0F) << 8) + cmdValue); break; //analog value, strip down to 12 bit
    }
}


/*----------------------------------------------------------------------------------------------------------------------*/

IoTT_BtnHandler::IoTT_BtnHandler()
{
}

IoTT_BtnHandler::~IoTT_BtnHandler()
{
	for (uint16_t i = 0; i < numCmds; i++)
	{
		IoTT_BtnHandlerCmd * thisPointer = cmdList[i];
		delete thisPointer;
		thisPointer = NULL;
	}
	numCmds = 0;
	free(cmdList);
	btnCondAddrLen = 0;
	free(btnCondAddr);
}

IoTT_BtnHandlerCmd * IoTT_BtnHandler::getCmdByIndex(uint16_t thisCmd)
{
	return cmdList[thisCmd];
}

IoTT_BtnHandlerCmd * IoTT_BtnHandler::getCmdByTypeAddr(uint8_t cmdType, uint16_t cmdAddr)
{
	for (uint16_t i = 0; i < numCmds; i++)
		if ((cmdList[i]->targetType == cmdType) && (cmdList[i]->targetAddr == cmdAddr))
			return cmdList[i];
	return NULL;
}

void IoTT_BtnHandler::loadButtonCfgJSON(JsonObject thisObj)
{
	if (thisObj.containsKey("BtnCondAddr"))
	{
		btnCondAddrLen = 1;
		JsonArray btnCondList = thisObj["BtnCondAddr"];
		if (btnCondList)
		{
			btnCondAddrLen = btnCondList.size();
			btnCondAddr = (uint16_t*) realloc (btnCondAddr, btnCondAddrLen * sizeof(uint16_t));
			for (uint8_t i = 0; i < btnCondAddrLen; i++)
				btnCondAddr[i] = btnCondList[i];
		}
		else
		{
			btnCondAddr = (uint16_t*) realloc (btnCondAddr, btnCondAddrLen * sizeof(uint16_t));
			btnCondAddr[0] = thisObj["BtnCondAddr"];
		}
	}
	if (thisObj.containsKey("CmdList"))
	{
		JsonArray btnCmdList = thisObj["CmdList"];
		numCmds = btnCmdList.size();
//		Serial.printf("Load %i commands btnCon %i\n", numCmds, btnCondAddr);
		cmdList = (IoTT_BtnHandlerCmd**) realloc (cmdList, numCmds * sizeof(IoTT_BtnHandlerCmd*));
		for (uint16_t i = 0; i < numCmds; i++)
		{
			IoTT_BtnHandlerCmd * thisCmd = new(IoTT_BtnHandlerCmd);
			thisCmd->loadButtonCfgJSON(btnCmdList[i]);
			thisCmd->parentObj = this;
			cmdList[i] = thisCmd;
		}
	}
}

int16_t IoTT_BtnHandler::getCondAddrIndex(uint16_t ofAddr)
{
	for (uint16_t i = 0; i < btnCondAddrLen; i++)
		if (btnCondAddr[i] == ofAddr)
			return i;
	return -1;
}

uint16_t IoTT_BtnHandler::getCondAddr(uint8_t addrIndex)
{
	if (btnCondAddrLen == 0)
		return 0xFFFF;
	return btnCondAddr[addrIndex];
}

//add command list to buffer for timed execution
//change to allow for merging two lists
void IoTT_BtnHandler::processBtnEvent()
{
//	Serial.println("Call process");
	uint32_t lastExecTime = millis();
    cmdBuffer * thisOutBuffer = &parentObj->parentObj->outBuffer;
    //for all lines in the command 
	for (uint16_t i = 0; i < numCmds; i++)
	{
//		Serial.printf("Add Cmd %i of %i to buffer\n", i, numCmds);
		//find a spot in the buffer
		int nextEmptySlot = -1;
		for (int j = 0; j < cmdBufferLen; j++)
		{
			if (!thisOutBuffer->cmdOutBuffer[j].tbd)
			{
				nextEmptySlot = j;
				break;
			}
		}
		if (nextEmptySlot >= 0) //overwrite protection. If no slot, ignore command
		{
			//place in the buffer
			IoTT_BtnHandlerCmd * thisPointer = cmdList[i];
			thisOutBuffer->cmdOutBuffer[nextEmptySlot].nextCommand = thisPointer;
			//set the exectime and the tbd flag
			lastExecTime = lastExecTime + thisPointer->execDelay;
			thisOutBuffer->cmdOutBuffer[nextEmptySlot].execTime = lastExecTime;
			thisOutBuffer->cmdOutBuffer[nextEmptySlot].tbd = true;
		}
 
	}
}


/*----------------------------------------------------------------------------------------------------------------------*/

IoTT_LocoNetButtons::IoTT_LocoNetButtons()
{
}

IoTT_LocoNetButtons::~IoTT_LocoNetButtons()
{
	for (uint16_t i = 0; i < eventTypeListLen; i++)
	{
		IoTT_BtnHandler * thisPointer = eventTypeList[i];
		delete thisPointer;
		thisPointer = NULL;
	}
	eventTypeListLen = 0;
	free(eventTypeList);
}

void IoTT_LocoNetButtons::loadButtonCfgJSON(JsonObject thisObj)
{
	if (thisObj.containsKey("ButtonNr"))
	{
		JsonArray BtnAddr = thisObj["ButtonNr"];
		if (BtnAddr.isNull())
			btnAddrListLen = 1;
		else
			btnAddrListLen = BtnAddr.size();
		btnAddrList = (uint16_t*) realloc (btnAddrList, btnAddrListLen * sizeof(uint16_t));
		if (BtnAddr.isNull())
			btnAddrList[0] = thisObj["ButtonNr"];
		else
			for (int i=0; i<btnAddrListLen;i++)
				btnAddrList[i] = BtnAddr[i];
	}
	if (thisObj.containsKey("CondData"))
	{
		JsonArray CondData = thisObj["CondData"];
		if (CondData.isNull())
			condDataListLen = 1;
		else
			condDataListLen = CondData.size();
		condDataList = (uint16_t*) realloc (condDataList, condDataListLen * sizeof(uint16_t));
		if (CondData.isNull())
			condDataList[0] = thisObj["CondData"];
		else
			for (int i=0; i<condDataListLen;i++)
				condDataList[i] = CondData[i];
	}
//	if (thisObj.containsKey("CurrDisp"))
//		currEvent = thisObj["CurrDisp"];

	if (thisObj.containsKey("EnableSource"))
		enableInput = getEnableTypeByName(thisObj["EnableSource"]);
	if (thisObj.containsKey("EnableAddr"))
		enableAddr = thisObj["EnableAddr"];
	if (thisObj.containsKey("EnableState"))
		enableStatus = thisObj["EnableState"];

	if (thisObj.containsKey("EventSource"))
		eventInput = getSourceTypeByName(thisObj["EventSource"]);
	else	
		eventInput = getSourceTypeByName("button");
	if (thisObj.containsKey("CtrlCmd"))
	{
		JsonArray btnCmd = thisObj["CtrlCmd"];
		eventTypeListLen = btnCmd.size();
		eventTypeList = (IoTT_BtnHandler**) realloc (eventTypeList, eventTypeListLen * sizeof(IoTT_BtnHandler*));
//		Serial.printf("Load Button Addr %i\n", btnAddr);
		for (uint16_t i = 0; i < eventTypeListLen; i++)
		{
			IoTT_BtnHandler * thisEvent = new(IoTT_BtnHandler);
			thisEvent->loadButtonCfgJSON(btnCmd[i]);
			thisEvent->parentObj = this;
			eventTypeList[i] = thisEvent;
		}
	}
}

sourceType IoTT_LocoNetButtons::getEventSource()
{
	return eventInput;
}

sourceType IoTT_LocoNetButtons::hasEventSource(sourceType thisSource)
{
//	Serial.printf("found event source %i is %i\n", thisSource, eventInput);
	switch (thisSource)
	{
		case evt_trackswitch: ;
		case evt_signalmastdyn:
			if ((eventInput == evt_trackswitch) || (eventInput == evt_signalmastdyn))
				return eventInput;
			else
				return evt_nosource;
			break;
		case evt_analogvalue:
			if ((eventInput == evt_analogscaler) || (eventInput == evt_analogvalue))
			{
//				Serial.printf("found event source %i\n", eventInput);
				return eventInput;
			}
			else
				return evt_nosource;
			break;
		default:
			if (eventInput == thisSource)
				return eventInput;
			else
				return evt_nosource;
	}
}

int8_t IoTT_LocoNetButtons::hasBtnAddr(uint16_t thisAddr)
{
	for (uint8_t i = 0; i < btnAddrListLen; i++)
		if (btnAddrList[i] == thisAddr)
		{
//			Serial.printf("check address %i\n", thisAddr);
			return i;
		}
	return -1;
}

uint16_t IoTT_LocoNetButtons::getBtnAddr(uint8_t index)
{
	return btnAddrList[index];
}

uint8_t IoTT_LocoNetButtons::getLastRecEvent()
{
	return lastRecButtonEvent;
}

uint8_t IoTT_LocoNetButtons::getLastComplEvent()
{
	return lastComplButtonEvent;
}

bool IoTT_LocoNetButtons::getEnableStatus()
{
//	Serial.printf("Check enable %i for %i \n", enableInput, enableStatus);
	switch (enableInput)
	{
		case ent_alwayson : return true; break;
		case ent_alwaysoff : return false; break;
		case ent_button: return digitraxBuffer->getButtonValue(enableAddr) == enableStatus; break;
		case ent_switch: return (((digitraxBuffer->getSwiPosition(enableAddr) >> 5) & 0x01) == enableStatus); break;
		case ent_block: 
//		Serial.println(getBDStatus(enableAddr));
		return ((digitraxBuffer->getBDStatus(enableAddr) & 0x01) == enableStatus); break;
		default: return true; break;
	}
}

void IoTT_LocoNetButtons::processAnalogEvent(uint16_t inputValue)
{
//	Serial.printf("Process Analog Event for Value %i \n", inputValue);
	uint8_t eventNr = 0xFF;
	switch (inputValue)
	{
		case 0: eventNr = 0; break;
		case 0x0FFF: eventNr = 3; break;//4095
		default:
			if (condDataListLen > 0)
				if (inputValue < condDataList[0]) 
					eventNr = 1;
				else
					eventNr = 2;
			break;
	}
	if (eventNr < eventTypeListLen)
	{
		IoTT_BtnHandler * thisEvent = eventTypeList[eventNr];
		thisEvent->processBtnEvent();
	}
}

void IoTT_LocoNetButtons::processAnalogScaler(uint16_t inputValue)
{
	if (eventTypeListLen == 2)
	{
		IoTT_BtnHandlerCmd execCmd;
		execCmd.targetType = analoginp;
		IoTT_BtnHandler * baseEvent = eventTypeList[0];
		IoTT_BtnHandler * secEvent = eventTypeList[1];
		for (uint16_t i = 0; i < baseEvent->numCmds; i++)
		{
			IoTT_BtnHandlerCmd * thisCmd = baseEvent->getCmdByIndex(i);
			if (thisCmd->targetType == analoginp)
			{
				IoTT_BtnHandlerCmd * matchingCmd = secEvent->getCmdByTypeAddr(analoginp, thisCmd->targetAddr);
				if (matchingCmd)
				{
					uint16_t loVal = (thisCmd->cmdType << 8) + thisCmd->cmdValue;
					uint16_t hiVal = (matchingCmd->cmdType << 8) + matchingCmd->cmdValue;
					float thisSlope = (float)(hiVal - loVal) / 4095;
					int scaleVal = loVal + round(inputValue * thisSlope);
//					Serial.printf("Hi %i Lo %i Fact %f Val %i\n", hiVal, loVal, thisSlope, scaleVal);
					if (scaleVal > 4095) scaleVal = 4095;
					if (scaleVal < 0) scaleVal = 0;
					execCmd.targetAddr = thisCmd->targetAddr;
					execCmd.cmdType = (scaleVal >> 8);
					execCmd.cmdValue = scaleVal & 0x00FF;
					execCmd.execDelay = 250;
					execCmd.executeBtnEvent();
//					Serial.printf("Process Analog Scaler for Value %i %i to Addr %i \n", inputValue, scaleVal, thisCmd->targetAddr);
				}
			}


		}
	}
}

void IoTT_LocoNetButtons::processSignalEvent(uint8_t inputValue)
{
	IoTT_BtnHandler * thisEvent = NULL;
//	IoTT_LocoNetButtons * secBtn = NULL;
	for (uint16_t i = 0; i < eventTypeListLen; i++)
	{
		thisEvent = eventTypeList[i];
		if (thisEvent->getCondAddr() == inputValue)
		{
			thisEvent->processBtnEvent();
//			Serial.printf("Process Signal Event for Aspect %i \n", inputValue);
			break;
		}
	}
}

void IoTT_LocoNetButtons::processSimpleEvent(uint8_t inputValue)
{
	uint16_t swiStatus = 0;
	for (int8_t i = (btnAddrListLen-1); i >= 0; i--) //check for the latest position
		swiStatus = (2 * swiStatus) + ((digitraxBuffer->getSwiPosition(btnAddrList[i]) >> 5) & 0x01);
	if ((swiStatus != lastEvent) && (swiStatus < eventTypeListLen))
	{
		IoTT_BtnHandler * thisEvent = eventTypeList[swiStatus];
		thisEvent->processBtnEvent();
		lastEvent = swiStatus;
//		Serial.printf("Updating Static Switch %i Addr  for Status %i  \n", btnAddrList[0], swiStatus);
	}
}

void IoTT_LocoNetButtons::processBlockDetEvent(uint8_t inputValue)
{
	uint16_t bdStatus = 0;
	for (int8_t i = (btnAddrListLen-1); i >= 0; i--) //check for the latest position
		bdStatus = (2 * bdStatus) + (digitraxBuffer->getBDStatus(btnAddrList[i]) & 0x01);
	if ((bdStatus != lastEvent) && (bdStatus < eventTypeListLen))
	{
		IoTT_BtnHandler * thisEvent = eventTypeList[bdStatus];
		thisEvent->processBtnEvent();
		lastEvent = bdStatus;
//		Serial.printf("Updating Block Detector %i Addr  for Status %i  \n", btnAddrList[0], bdStatus);
	}
}

void IoTT_LocoNetButtons::processDynEvent(uint8_t inputValue)
{
	uint32_t hlpAct, lastAct = 0;
	uint16_t swiStatus = 0;
	uint8_t dynSwi = 0;
	for (uint8_t i = 0; i < btnAddrListLen; i++) //check for the latest activity
	{
		hlpAct = digitraxBuffer->getLastSwiActivity(btnAddrList[i]);
		if (hlpAct > lastAct)
		{
			dynSwi = i;
			lastAct = hlpAct;
		}
	}
	if (lastAct != 0) //we have activity
	{
		uint8_t aspectNr = dynSwi * 2;
		if (((digitraxBuffer->getSwiPosition(btnAddrList[dynSwi]) >> 4) & 0x02) > 0)
			aspectNr++; //this is the final aspect #
		if ((aspectNr != lastEvent) && (aspectNr < eventTypeListLen))
		{
			IoTT_BtnHandler * thisEvent = eventTypeList[aspectNr];
			thisEvent->processBtnEvent();
			lastEvent = aspectNr;
//			Serial.printf("Updating Aspect %i  for Switch %i\n", aspectNr, btnAddrList[0]);
		}
	}
}

int16_t IoTT_LocoNetButtons::getCondDataIndex(uint16_t ofData)
{
	for (uint16_t i = 0; i < condDataListLen; i++)
		if (condDataList[i] == ofData)
			return i;
	return -1;
}

void IoTT_LocoNetButtons::processTransponderEvent(uint16_t inputValue)
{

	uint8_t eventVal = ((inputValue & 0x8000) >> 15) ^ 0x01; // val 1 (entering) calls event 0
	uint16_t locoAddr = inputValue & 0x7FFF;
	if (getCondDataIndex(locoAddr) >= 0)
	{
		IoTT_BtnHandler * thisEvent = eventTypeList[eventVal];
		thisEvent->processBtnEvent();
	}
}

void IoTT_LocoNetButtons::processPowerEvent(uint16_t inputValue)
{
	//0: Off 1: On; 2: Idle
//	Serial.printf("Power Event %i\n", inputValue);
	IoTT_BtnHandler * thisEvent = eventTypeList[inputValue];
	thisEvent->processBtnEvent();
}

void IoTT_LocoNetButtons::processBtnEvent(uint8_t inputValue)
{
	if (lastRecButtonEvent != inputValue)
		lastComplButtonEvent = noevent;
	IoTT_BtnHandler * thisEvent = NULL;
	IoTT_LocoNetButtons * secBtn = NULL;
//	Serial.println("processBtn");
//	for (uint16_t i = 0; i < eventTypeListLen; i++)
	{
		thisEvent = eventTypeList[inputValue];
//		if (thisEvent->getEventType() == inputValue)
		{
//			Serial.printf("process Event %i\n", inputValue);
			if (((inputValue == onbtndown) || (inputValue == onbtnhold))  && (condDataListLen > 0) && (condDataList[0] != 0xFFFF)) //this is 2 hand command for onhold event
			{
//				Serial.printf("2 Btn Command %i\n", condDataList[0]);
				//two hand button, check conditions 
				secBtn = parentObj->getButtonByAddress(condDataList[0]);
				if (secBtn != NULL)
				{
					if ((secBtn->lastRecButtonEvent == onbtnhold) && ((inputValue == onbtnhold) || (inputValue == onbtndown))) //2 hand buttons onhold, current button down or hold
						if ((secBtn->lastComplButtonEvent != onbtnhold) || ((lastComplButtonEvent != onbtnhold) && (lastComplButtonEvent != onbtndown))) //and at least one button cleared
						{
//							Serial.printf("Process Hold Button Event %i\n", inputValue);
							thisEvent->processBtnEvent(); //process event
							lastComplButtonEvent = inputValue;  //set flag on this button
							secBtn->lastComplButtonEvent = inputValue; //set flag on 2 hand button as well, will be cleared in other button when next non-hold event happens
//							break; //done
						}
				}
//				else
//					Serial.printf("no 2nd button found %i\n", condDataList[0]);
			}
			else //single command, so we process. onhold commands get processed repetitively
			{
//				Serial.printf("Process Direct Button Event %i\n", inputValue);
				thisEvent->processBtnEvent();
				lastComplButtonEvent = inputValue; 
			}
//			break;
		}
	}
	lastRecButtonEvent = inputValue; //last received event type
}

/*----------------------------------------------------------------------------------------------------------------------*/

IoTT_LocoNetButtonList::IoTT_LocoNetButtonList()
{
}

IoTT_LocoNetButtonList::~IoTT_LocoNetButtonList()
{
	freeObjects();
}

void IoTT_LocoNetButtonList::freeObjects()
{
	for (uint16_t i = 0; i < numBtnHandler; i++)
	{
		IoTT_LocoNetButtons * thisPointer = btnList[i];
		delete thisPointer;
		thisPointer = NULL;
	}
	numBtnHandler = 0;
	free(btnList);
}

IoTT_LocoNetButtons * IoTT_LocoNetButtonList::getButtonByAddress(uint16_t btnAddr)
{
	IoTT_LocoNetButtons * thisButton = NULL;
	for (uint16_t i = 0; i < numBtnHandler; i++)
	{
		thisButton = btnList[i];
//		Serial.printf("Checking Index %i Addr %i\n", i, thisButton->getBtnAddr());
		if ((thisButton->getBtnAddr(0) == btnAddr) && (thisButton->getEventSource() == evt_button))
		{
//			Serial.printf("Found button for Addr %i\n", btnAddr);
			return thisButton;
			break;
		}
	}
//	Serial.println("no such button");
	return NULL;
}

uint16_t IoTT_LocoNetButtonList::getButtonIndexByAddress(sourceType inputEvent, uint16_t btnAddr, uint16_t startIndex)
{
	if (startIndex < numBtnHandler)
		for (uint16_t i = startIndex; i < numBtnHandler; i++)
		{
			IoTT_LocoNetButtons * thisButton = btnList[i];
			if ((thisButton->hasBtnAddr(btnAddr) >= 0) && (thisButton->hasEventSource(inputEvent) != evt_nosource))
			{
				return i;
				break;
			}
		}
	return 0xFFFF;
}

void IoTT_LocoNetButtonList::processBtnEvent(sourceType inputEvent, uint16_t btnAddr, uint16_t eventValue)
{
//	Serial.println("Call Handler 1");
	uint16_t lastButton = getButtonIndexByAddress(inputEvent, btnAddr);
	while (lastButton != 0xFFFF) //allow for multiple  button lines with same button Addr
	{
		IoTT_LocoNetButtons * thisButton = btnList[lastButton];
//		Serial.printf("Process Index %i\n", lastButton);
		if (thisButton)
		{
			if (thisButton->getEnableStatus())
				switch (thisButton->getEventSource())
				{
					case evt_button: thisButton->processBtnEvent(eventValue); break;
					case evt_analogscaler: thisButton->processAnalogScaler(eventValue); break; 
					case evt_analogvalue: thisButton->processAnalogEvent(eventValue); break;
					case evt_trackswitch: thisButton->processSimpleEvent(eventValue); break;
					case evt_signalmastdyn : thisButton->processDynEvent(eventValue); break;
					case evt_signalmastdcc: thisButton->processSignalEvent(eventValue); break;
					case evt_blockdetector: thisButton->processBlockDetEvent(eventValue); break;
					case evt_transponder: thisButton->processTransponderEvent(eventValue); break;
					case evt_powerstat: thisButton->processPowerEvent(eventValue); break;
					default: break;
				}
		}
		lastButton = getButtonIndexByAddress(inputEvent, btnAddr, lastButton + 1);
	}
}

void IoTT_LocoNetButtonList::loadButtonCfgJSON(DynamicJsonDocument doc, bool resetList)
{
	JsonObject thisObj = doc.as<JsonObject>();
	loadButtonCfgJSONObj(thisObj, resetList);
}

void IoTT_LocoNetButtonList::loadButtonCfgJSONObj(JsonObject doc, bool resetList)
{
	if (resetList && (numBtnHandler > 0))
		freeObjects();
	if (doc.containsKey("ButtonHandler"))
    {
        JsonArray ButtonHandlers = doc["ButtonHandler"];
        uint16_t newBtnHandler = ButtonHandlers.size();
        btnList = (IoTT_LocoNetButtons**) realloc (btnList, (numBtnHandler + newBtnHandler) * sizeof(IoTT_LocoNetButtons*));
		for (uint16_t i = 0; i < newBtnHandler; i++)
		{
//			Serial.printf("Add Button %i\n", i);
			IoTT_LocoNetButtons * thisButton = new(IoTT_LocoNetButtons);
			thisButton->loadButtonCfgJSON(ButtonHandlers[i]);
			thisButton->parentObj = this;
			btnList[numBtnHandler + i] = thisButton;
		}
		numBtnHandler += newBtnHandler;
		Serial.printf("Loading %i button handlers. Total is %i\n", ButtonHandlers.size(), numBtnHandler);
	}
}

//buffer with commands for timely execution
//expects execution time and tbd flag set
void IoTT_LocoNetButtonList::processButtonHandler()
{
	int16_t execThis = -1;
	uint32_t lowestExecTime = millis() + 10000; //start with 10 secs time window
	for (int i = 0; i < cmdBufferLen; i++)
	{
		if (outBuffer.cmdOutBuffer[i].tbd)
			if (outBuffer.cmdOutBuffer[i].execTime < lowestExecTime)
			{
				lowestExecTime = outBuffer.cmdOutBuffer[i].execTime;
				execThis = i;
			}
	}
	if (execThis >= 0)
		if (outBuffer.cmdOutBuffer[execThis].execTime <= millis())
		{
			outBuffer.cmdOutBuffer[execThis].nextCommand->executeBtnEvent();
			outBuffer.cmdOutBuffer[execThis].tbd = false;
		}
}

