#include <IoTT_DigitraxBuffers.h>


//blockDetBuffer blockDetectorBuffer;
//switchBuffer switchPositionBuffer;
//signalBuffer signalAspectBuffer;
//analogValBuffer analogValueBuffer;
//buttonValBuffer buttonValueBuffer;
//powerStatusBuffer sysPowerStatus = 2; //OPC_IDLE
//slotDataBuffer slotBuffer;
//uint8_t dispatchSlot = 0x00;

//slotData stdSlot = {0x03, 0x80, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00}; //Byte 1 0x80 for unused, newly initialized slot

IoTT_SerInjector* dccPort = NULL;
txFct lnReplyFct = NULL;
txFct lnOutFct = NULL;
//txFct dccOutFct = NULL;

void dccGeneratorCallbackNew(std::vector<ppElement> * txData)
{
	digitraxBuffer->receiveDCCGeneratorFeedbackNew(txData);
}

/*
uint16_t dccGeneratorCallback(lnTransmitMsg txData)
{
	return digitraxBuffer->receiveDCCGeneratorFeedback(txData);
}
*/

//LocoNet functions for preparing reply messages
void prepLACKMsg(lnTransmitMsg * msgData, uint8_t ackCode, uint8_t ackData)
{
	msgData->lnData[0] = 0xB4; //OPC_LONG_ACK 
	msgData->lnData[1] = ackCode;
	msgData->lnData[2] = ackData;
	msgData->lnMsgSize = 4;
	setXORByte(&msgData->lnData[0]);
}

void prepTurnoutMsg(lnTransmitMsg * msgData, bool useACK, uint16_t swiAddr, uint8_t swiPos)
{
	if (swiPos == 2)
		swiPos = (digitraxBuffer->getSwiPosition(swiAddr) >> 5) ^ 0x01;
	msgData->lnData[0] = useACK ? 0xBD : 0xB0; //OPC_SW_ACK : OPC_SW_REQ
	msgData->lnData[1] = swiAddr & 0x7F;
	msgData->lnData[2] = ((swiAddr >> 7) & 0x0F) + (swiPos << 5);
	msgData->lnMsgSize = 4;
	setXORByte(&msgData->lnData[0]);
}

void prepTurnoutReportMsg(lnTransmitMsg * msgData, uint16_t swiAddr, uint8_t swiPos)
{
	if (swiPos == 2)
		swiPos = (digitraxBuffer->getSwiPosition(swiAddr) >> 5) ^ 0x01;
	msgData->lnData[0] = 0xBC; //OPC_SW_STATE
	msgData->lnData[1] = swiAddr & 0x7F;
	msgData->lnData[2] = ((swiAddr >> 7) & 0x0F) + (swiPos << 5);
	msgData->lnMsgSize = 4;
	setXORByte(&msgData->lnData[0]);
}

void prepSlotStat1Msg(lnTransmitMsg * msgData, uint8_t slotNr, uint8_t stat)
{
	msgData->lnData[0] = 0xB5; //OPC_SLOT_STAT1
	msgData->lnData[1] = slotNr;
	msgData->lnData[2] = stat;
	msgData->lnMsgSize = 4;
	setXORByte(&msgData->lnData[0]);
}

void prepSlotSpeedMsg(lnTransmitMsg * msgData, uint8_t slotNr, uint8_t speed)
{
	msgData->lnData[0] = 0xA0; //OPC_LOCO_SPD
	msgData->lnData[1] = slotNr;
	msgData->lnData[2] = speed;
	msgData->lnMsgSize = 4;
	setXORByte(&msgData->lnData[0]);
}

void prepSlotDirFMsg(lnTransmitMsg * msgData, uint8_t slotNr, uint8_t dirfdata)
{
	msgData->lnData[0] = 0xA1; //OPC_LOCO_DIRF
	msgData->lnData[1] = slotNr;
	msgData->lnData[2] = dirfdata;
	msgData->lnMsgSize = 4;
	setXORByte(&msgData->lnData[0]);
}

void prepSlotSndMsg(lnTransmitMsg * msgData, uint8_t slotNr, uint8_t snddata)
{
	msgData->lnData[0] = 0xA2; //OPC_LOCO_SND
	msgData->lnData[1] = slotNr;
	msgData->lnData[2] = snddata;
	msgData->lnMsgSize = 4;
	setXORByte(&msgData->lnData[0]);
}

void prepSlotRequestMsg(lnTransmitMsg * msgData, uint8_t slotNr)
{
	msgData->lnData[0] = 0xBB; //OPC_SL_RQ_DATA
	msgData->lnData[1] = slotNr;
	msgData->lnData[2] = 0;
	msgData->lnMsgSize = 4;
	setXORByte(&msgData->lnData[0]);
}

void prepLocoAddrReqMsg(lnTransmitMsg * msgData, uint16_t dccAddr)
{
//	Serial.println(dccAddr);
	msgData->lnData[0] = 0xBF; //OPC_LOCO_ADR
	msgData->lnData[1] = ((dccAddr >> 7) & 0x7F); //Addr Hi
	msgData->lnData[2] = dccAddr & 0x7F; //Addr Lo
	msgData->lnMsgSize = 4;
	setXORByte(&msgData->lnData[0]);
}

void prepSlotReadMsg(lnTransmitMsg * msgData, uint8_t slotNr)
{
//	Serial.println("Prep SL RD");
//	dispSlot(&digitraxBuffer->slotBuffer[slotNr][0]);
	msgData->lnData[0] = 0xE7; 
	msgData->lnData[1] = 0x0E; //OPC_SL_RD_DATA
	msgData->lnData[2] = slotNr;
	memcpy(&msgData->lnData[3], digitraxBuffer->slotBuffer[slotNr], 10);
	msgData->lnData[3] &= 0x7F; //make sure purge bit 7 is 0
	msgData->lnData[4] &= 0x7F; //make sure slot init bit 7 is 0
	msgData->lnData[7] = digitraxBuffer->trackByte; //insert global TrackByte
	msgData->lnMsgSize = 14;
	setXORByte(&msgData->lnData[0]);
//	dispMsg(&msgData->lnData[0], 14);
}

void prepSlotWriteMsg(lnTransmitMsg * msgData, uint8_t slotNr)
{
//	Serial.println("Prep SL WR");
	msgData->lnData[0] = 0xEF; 
	msgData->lnData[1] = 0x0E; //OPC_SL_WR_DATA
	msgData->lnData[2] = slotNr;
	memcpy(&msgData->lnData[3], digitraxBuffer->slotBuffer[slotNr], 10);
	msgData->lnData[3] &= 0x7F; //make sure purge bit 7 is 0
	msgData->lnData[4] &= 0x7F; //make sure slot init bit 7 is 0
	msgData->lnData[7] = digitraxBuffer->trackByte; //insert global TrackByte
	msgData->lnMsgSize = 14;
	setXORByte(&msgData->lnData[0]);
//	dispMsg(&msgData->lnData[0], 14);
}

void prepSlotMoveMsg(lnTransmitMsg * msgData, uint8_t slotSRC, uint8_t slotDEST)
{
	msgData->lnData[0] = 0xBA; //OPC_MOVE_SLOTS
	msgData->lnData[1] = slotSRC;
	msgData->lnData[2] = slotDEST;
	msgData->lnMsgSize = 4;
	setXORByte(&msgData->lnData[0]);
}

void prepTrackPowerMsg(lnTransmitMsg * msgData, uint8_t pwrStatus)
{
	msgData->lnData[0] = pwrStatus; //OPC_ON, OFF, IDLE
	msgData->lnMsgSize = 2;
	setXORByte(&msgData->lnData[0]);
}

//version meeting https://wiki.rocrail.net/doku.php?id=loconet:ln-pe-en
//this version based on OPC_MULTI_SENSE_LONG
void prepLissyMsg(lnReceiveBuffer * srcData, lnTransmitMsg * msgData)
{
	msgData->lnData[0] = 0xE4;
	msgData->lnData[1] = 0x08;

	uint8_t zoneAddr[2] = {0,0};
	uint8_t locoAddr[2] = {0,0};

	zoneAddr[0] = srcData->lnData[2] & 0x1F;
	zoneAddr[1] = srcData->lnData[3];
	
	bool locoDetect = (srcData->lnData[2] & 0x20);
	
//    if (srcData->lnData[3] == 0x7E)
//		locoAddr[1] = srcData->lnData[4];
//   else
//    {
		locoAddr[0] = srcData->lnData[4];
		locoAddr[1] = srcData->lnData[5];
//	}
	bool trackDir = (srcData->lnData[6] & 0x40);
	
	if (locoDetect)
	{
		msgData->lnData[2]=0x0f;
		msgData->lnData[5]= locoAddr[0];
		msgData->lnData[6]= locoAddr[0];
//		Serial.println("belegt");
	}
	else 
	{
		msgData->lnData[2]=0x01;
		msgData->lnData[3]|= 0x20;
		msgData->lnData[5]=0x00;
		msgData->lnData[6]=0x02;
//		Serial.println("frei");
	}
	msgData->lnData[7] = 0;
	msgData->lnMsgSize = 8;
	setXORByte(&msgData->lnData[0]);
}

//-----------------------------DCC functions for command station mode DCC cmd generation----------------------------------------------------------------------------------------------------------------------

void setDCCSpeedCmd(uint8_t slotNr, uint8_t * speedVal)
{
	lnTransmitMsg txBuffer;
	char* outStr = (char*)&txBuffer.lnData[0];
	uint16_t cabAddr = (digitraxBuffer->slotBuffer[slotNr][ADR2] << 7) + (digitraxBuffer->slotBuffer[slotNr][ADR] & 0x7F);
	if (digitraxBuffer->getRefreshStatus(slotNr) & slotCommon) //common or active, slot is refreshed
		sprintf(outStr, "t 1 %i %i %i", cabAddr, speedVal ? ((*speedVal) & 0x7F) : digitraxBuffer->slotBuffer[slotNr][SPD], ((digitraxBuffer->slotBuffer[slotNr][DIRF] & 0x20)>>5) ^ 0x01); //[4]: SPD, [5]:DIRF Dir bit change from LocoNet to DCC++
	else
		sprintf(outStr, "- %i", cabAddr); //idle or free, not refreshed
	txBuffer.lnMsgSize = strlen(outStr);
	dccPort->lnWriteMsg(txBuffer);
//	Serial.println(outStr);

}

void setDCCFuncCmd(uint8_t slotNr, uint8_t * funcData) // funcNr, funcStatus
{
	lnTransmitMsg txBuffer;
	char* outStr = (char*)&txBuffer.lnData[0];
	uint16_t cabAddr = (digitraxBuffer->slotBuffer[slotNr][6] << 7) + (digitraxBuffer->slotBuffer[slotNr][1] & 0x7F);
	uint8_t funcCode = 0; 
	if ((*funcData == 0xA1) || (*funcData == 0xB6))
		funcCode = 0x80 + (digitraxBuffer->slotBuffer[slotNr][3] & 0x1F); //function 0-4
	if (*funcData == 0xA2)
		funcCode = 0xB0 + (digitraxBuffer->slotBuffer[slotNr][7] & 0x0F); //function 5-8
	if (*funcData == 0xA3)
		funcCode = 0xA0 + (digitraxBuffer->slotBuffer[slotNr][7] & 0x0F); //function 9-12  //WRONG Current State data. Where to store F9-F12??
	sprintf(outStr, "<f %i %i>", cabAddr, funcCode);
	txBuffer.lnMsgSize = strlen(outStr);
	dccPort->lnWriteMsg(txBuffer);
//	Serial.println(outStr);

}

//Prog byte on main:  <w CAB CV VALUE>
//Prog bit on main: <b CAB CV BIT VALUE>

//Read Addr on Prog: <R>
//Write/Verify Addr on Prog: <W ADDRESS>
//Write CV byte on Prog: <W CV VALUE> (no paged/direct selector)
//Write CV bit on Prog:  <B CV BIT VALUE CALLBACKNUM CALLBACKSUB>
//Read CV byte on Prog: <R CV CALLBACKNUM CALLBACKSUB>
//Verify byte on Prog: <V CV BYTEVALUE>
//Verify bit on Prog:  <V CV BIT BITVALUE>


void setDCCProgrammingCmd(uint8_t progCmd, uint16_t OpsAddr, uint16_t CVNr, uint8_t CVVal, uint8_t id0, uint8_t id1)
{
	lnTransmitMsg txBuffer;
	char* outStr = (char*)&txBuffer.lnData[0];
	if (progCmd == 0xFF)
		sprintf(outStr, "R");
	else
		switch (progCmd & 0x64)
		{
//			case 0x00: //Read bit on Prog
//			case 0x04: //Read bit on main
			case 0x10: //read full Address
				sprintf(outStr, "R");
				break;
			case 0x20: //read byte on Prog
				sprintf(outStr, "R %i %i %i", CVNr+1, id0, id1);
				break;
//			case 0x24: //read byte on main
//			case 0x40: //write bit on Prog
//			case 0x44: //write bit on main
			case 0x60: //write byte on Prog
				sprintf(outStr, "W %i %i", CVNr+1, CVVal);
				break;
			case 0x64: //write byte on main
				sprintf(outStr, "w %i %i %i", OpsAddr, CVNr+1, CVVal);
				break;
			default: return;
		}
	txBuffer.lnMsgSize = strlen(outStr);
	dccPort->lnWriteMsg(txBuffer);
//	Serial.println(outStr);
}

void setDCCSwitchCmd(uint16_t swiAddr, uint8_t swiPos, uint8_t coilStatus)
{
	lnTransmitMsg txBuffer;
	char* outStr = (char*)&txBuffer.lnData[0];
	sprintf(outStr, "a %i %i", swiAddr+1, swiPos ^ 0x01);
	txBuffer.lnMsgSize = strlen(outStr);
	dccPort->lnWriteMsg(txBuffer);
//	Serial.println(outStr);
}

void setDCCSwitchById(uint16_t swiId, uint8_t swiPos, uint8_t coilStatus)
{
	lnTransmitMsg txBuffer;
	char* outStr = (char*)&txBuffer.lnData[0];
	sprintf(outStr, "T %i %i", swiId, swiPos ^ 0x01);
	txBuffer.lnMsgSize = strlen(outStr);
	dccPort->lnWriteMsg(txBuffer);
//	Serial.println(outStr);
}

void reqSwitchInfo(uint16_t swiId)
{
	lnTransmitMsg txBuffer;
	char* outStr = (char*)&txBuffer.lnData[0];
	sprintf(outStr, "T %i X", swiId);
	txBuffer.lnMsgSize = strlen(outStr);
	dccPort->lnWriteMsg(txBuffer);
	sprintf(outStr, "JT %i", swiId);
	txBuffer.lnMsgSize = strlen(outStr);
	dccPort->lnWriteMsg(txBuffer);
}

void reqRouteInfo(uint16_t rteId)
{
	lnTransmitMsg txBuffer;
	char* outStr = (char*)&txBuffer.lnData[0];
	sprintf(outStr, "JA %i", rteId);
	txBuffer.lnMsgSize = strlen(outStr);
	dccPort->lnWriteMsg(txBuffer);
}

void reqRosterInfo(uint16_t locoId)
{
	lnTransmitMsg txBuffer;
	char* outStr = (char*)&txBuffer.lnData[0];
	sprintf(outStr, "JR %i", locoId);
	txBuffer.lnMsgSize = strlen(outStr);
	dccPort->lnWriteMsg(txBuffer);
}

void setDCCPinById(uint16_t pinId, uint8_t pinState)
{
	lnTransmitMsg txBuffer;
	char* outStr = (char*)&txBuffer.lnData[0];
	sprintf(outStr, "Z %i %i", pinId, pinState ^ 0x01);
	txBuffer.lnMsgSize = strlen(outStr);
	dccPort->lnWriteMsg(txBuffer);
//	Serial.println(outStr);
}

void setDCCPowerOutMsg(uint8_t trStatus, uint8_t trType)
{
	lnTransmitMsg txBuffer;
	char* outStr = (char*)&txBuffer.lnData[0];
	strcpy(outStr, "");
	switch (trStatus)
	{
		case 0x82 : strcpy(outStr, "0"); break;
		case 0x83 : strcpy(outStr, "1"); break;
		case 0x85 : strcpy(outStr, "!"); break;
	}
	switch (trType)
	{
		case 0x00 : break;
		case 0x01 : strcat(outStr, " MAIN"); break;
		case 0x02 : strcat(outStr, " PROG"); break;
		case 0x03 : strcat(outStr, " JOIN"); break;
	}
	txBuffer.lnMsgSize = strlen(outStr);
	txBuffer.lnData[txBuffer.lnMsgSize] = 0;
//	Serial.printf("try: %s\n", outStr);
	dccPort->lnWriteMsg(txBuffer);

	strcpy(outStr, "= C DC 55");
	txBuffer.lnMsgSize = strlen(outStr);
	txBuffer.lnData[txBuffer.lnMsgSize] = 0;
//	Serial.printf("try: %s\n", outStr);
	dccPort->lnWriteMsg(txBuffer);

}

void setCurrReportMode(uint8_t mainMode, uint8_t progMode)
{
	lnTransmitMsg txBuffer;
	char* outStr = (char*)&txBuffer.lnData[0];
	sprintf(outStr, "A %i %i", mainMode, progMode);
	txBuffer.lnMsgSize = strlen(outStr);
	txBuffer.lnData[txBuffer.lnMsgSize] = 0;
	dccPort->lnWriteMsg(txBuffer);
}

void setDCCSpeedSteps(uint8_t speedStepMode)
{
	lnTransmitMsg txBuffer;
	char* outStr = (char*)&txBuffer.lnData[0];
	if (speedStepMode == 0)
		strcpy(outStr, "D SPEED128");
	else
		strcpy(outStr, "D SPEED28");
	
	txBuffer.lnMsgSize = strlen(outStr);
	txBuffer.lnData[txBuffer.lnMsgSize] = 0;
	dccPort->lnWriteMsg(txBuffer);
}

void reqDCCPeripheralList(const char* cmdChar)
{
	lnTransmitMsg txBuffer;
	char* outStr = (char*)&txBuffer.lnData[0];
	strcpy(outStr, cmdChar);
	txBuffer.lnMsgSize = strlen(cmdChar);
	txBuffer.lnData[txBuffer.lnMsgSize] = 0;
//	Serial.println(outStr);
	dccPort->lnWriteMsg(txBuffer);
}

void setDCCSensorPin(arduinoPins * thisSensor)
{
	lnTransmitMsg txBuffer;
	char* outStr = (char*)&txBuffer.lnData[0];
	sprintf(outStr, "S %i %i %i", thisSensor->ardID, thisSensor->ardPin, thisSensor->posLogic);
	txBuffer.lnMsgSize = strlen(outStr);
	dccPort->lnWriteMsg(txBuffer);
}

void setDCCDefTurnout(arduinoPins * thisTurnout)
{
	lnTransmitMsg txBuffer;
	char* outStr = (char*)&txBuffer.lnData[0];
	switch (thisTurnout->ardType)
	{
		case 0:	sprintf(outStr, "Z %i %i %i", thisTurnout->ardID, thisTurnout->ardPin, thisTurnout->posLogic); //mke it an output pin
			break;
		case 1:	sprintf(outStr, "T %i %s %i", thisTurnout->ardID, "DCC", thisTurnout->lnAddr); //DCC decoder
			break;
		case 2:	sprintf(outStr, "T %i %s %i %i %i %i", thisTurnout->ardID, "SERVO", thisTurnout->ardPin, 410, 205, 2); //drive a servo
			break;
		default:	sprintf(outStr, "T %i %s %i", thisTurnout->ardID, "VPIN", thisTurnout->ardPin); //VPIN output, the preferred option compared to pin output
			break;
	}
	txBuffer.lnMsgSize = strlen(outStr);
	dccPort->lnWriteMsg(txBuffer);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------

IoTT_DigitraxBuffers::IoTT_DigitraxBuffers(txFct lnOut)
{
	lnOutFct = lnOut;
}

IoTT_DigitraxBuffers::~IoTT_DigitraxBuffers()
{
}

void IoTT_DigitraxBuffers::loadRHCfgJSON(DynamicJsonDocument doc)
{
//	Serial.println("Load JSON");
	if (doc.containsKey("RxD"))
		rxPin = doc["RxD"];
	if (doc.containsKey("TxD"))
		txPin = doc["TxD"];
	if (doc.containsKey("DevSettings"))
	{
		JsonObject thisObj = doc["DevSettings"];
		if (thisObj.containsKey("FCRate"))
			slotBuffer[0x7B][0] = doc["DevSettings"]["FCRate"];

		if (thisObj.containsKey("LEDLevel"))
			ledLevel = thisObj["LEDLevel"];
		if (thisObj.containsKey("CfgSlot"))
		{
			JsonArray cfgArray = doc["DevSettings"]["CfgSlot"];
			for (int i = 0; i < 10; i++)
				if (i != 4)
					slotBuffer[0x7F][i] = cfgArray[i];
		}
		configPeripheralsPwrUp = !doc["DevSettings"]["ConfigToEEPROM"];
		if (thisObj.containsKey("ProgTrack"))
		{
			progLimit = thisObj["ProgTrack"]["CurrLimit"];
			progPulseMin = thisObj["ProgTrack"]["MinAckP"];
			progPulseMax = thisObj["ProgTrack"]["MaxAckP"];
			progNumTry = thisObj["ProgTrack"]["NumTry"];
			progBoost = thisObj["ProgTrack"]["Boost"];
		}
	}
	if (doc.containsKey("InputSettings"))
	{
		JsonArray pinArray = doc["InputSettings"]["InpPins"];
		sensorInputLen = pinArray.size();
        sensorInputs = (arduinoPins*) realloc (sensorInputs, sensorInputLen * sizeof(arduinoPins));
		for (int i=0; i<sensorInputLen;i++)
		{
			sensorInputs[i].ardID = pinArray[i]["Id"];
			sensorInputs[i].ardPin = pinArray[i]["PinNr"];
			sensorInputs[i].ardType = pinArray[i]["PNType"];
			sensorInputs[i].lnType = pinArray[i]["LNType"];
			sensorInputs[i].posLogic = pinArray[i]["Logic"];
			sensorInputs[i].lnAddr = pinArray[i]["LNAddr"];
		}
		
	}
	if (doc.containsKey("TurnoutSettings"))
	{
		JsonArray btnArray = doc["TurnoutSettings"]["OutPins"];
		turnoutOutputLen = btnArray.size();
        turnoutOutputs = (arduinoPins*) realloc (turnoutOutputs, turnoutOutputLen * sizeof(arduinoPins));
		for (int i=0; i<turnoutOutputLen;i++)
		{
			turnoutOutputs[i].ardID = btnArray[i]["Id"];
			turnoutOutputs[i].ardPin = btnArray[i]["PinNr"];
			turnoutOutputs[i].ardType = btnArray[i]["TOType"];
			turnoutOutputs[i].lnType = btnArray[i]["LNType"];
			turnoutOutputs[i].posLogic = btnArray[i]["Logic"];
			turnoutOutputs[i].lnAddr = btnArray[i]["LNAddr"];
			strcpy(turnoutOutputs[i].Descr, btnArray[i]["Descr"]);
//			if (btnArray[i]["Descr"])
//				turnoutOutputs[i].Descr = btnArray[i]["Descr"];
//			else
//				turnoutOutputs[i].Descr = "IoTT";
		}
	}
	if (doc.containsKey("AutomationSettings"))
	{
		JsonArray rteArray = doc["AutomationSettings"]["Routes"];
		routeOutputLen = rteArray.size();
        automationList = (automationEntry*) realloc (automationList, routeOutputLen * sizeof(automationEntry));
		for (int i=0; i<routeOutputLen;i++)
		{
			automationList[i].ID = rteArray[i]["ID"];
			strcpy(automationList[i].automationType, rteArray[i]["Type"]);
			strcpy(automationList[i].Descr, rteArray[i]["Descr"]);
		}
	}
	if (doc.containsKey("RosterSettings"))
	{
		JsonArray locoArray = doc["RosterSettings"]["Locos"];
		for (int i=0; i<locoArray.size();i++)
		{
			rosterEntry newLoco;
			newLoco.ID = locoArray[i]["DCCAddr"];
			strcpy(newLoco.Description, locoArray[i]["Descr"]);
			strcpy(newLoco.FunctionMap, locoArray[i]["FNMap"]);
			rosterList.push_back(newLoco);
		}
	}

	if (doc.containsKey("CurrentTracker"))
	{
		JsonArray trackerArray = doc["CurrentTracker"];
		if (trackerArray.isNull())
		{
			trackGaugesLen = 2;
			trackGauges = (trackerData*) realloc (trackGauges, trackGaugesLen * sizeof(trackerData));
			uint8_t repMode = doc["CurrentTracker"]["ReportMode"];
			for (int i=0; i<2;i++)
			{
				trackGauges[i].currUsePin = i;
				if (repMode & (0x01 << i))
					trackGauges[i].currUsePin |= 0x10;
				trackGauges[i].currOffset = 0;
				trackGauges[i].currBuffSize = doc["CurrentTracker"]["SampleSize"];
				trackGauges[i].currMultiplier = doc["CurrentTracker"]["Multiplier"];
			}
		}
		else
		{
			trackGaugesLen = trackerArray.size();
			trackGauges = (trackerData*) realloc (trackGauges, trackGaugesLen * sizeof(trackerData));
			for (int i=0; i<trackGaugesLen;i++)
			{
				trackGauges[i].currUsePin = trackerArray[i]["PinNr"];
				if (trackerArray[i]["ShowGauge"])
					trackGauges[i].currUsePin |= 0x10;
				trackGauges[i].currOffset = trackerArray[i]["Offset"];
				trackGauges[i].currBuffSize = trackerArray[i]["SampleSize"];
				trackGauges[i].currMultiplier = trackerArray[i]["Multiplier"];
			}
		}
	}
}

bool IoTT_DigitraxBuffers::getLocoNetMode()
{
	return isLocoNet;
}

void IoTT_DigitraxBuffers::setLocoNetMode(bool newMode)
{
	isLocoNet = newMode;
}

void IoTT_DigitraxBuffers::clearSlotBuffer(bool hardReset)
{
//	Serial.println("Clear Slots");
	for (int i = 0; i < numSlots; i++)
	{
		if (hardReset)
			memcpy(&slotBuffer[i], &stdSlot[0], 10);
		else
		{
			for (uint8_t j = 0; j < 10; j++)
				slotBuffer[i][j] &= 0x7F;
			slotBuffer[i][TRK] = 0; //clear purge counter
			if (getConsistStatus(i) != cnFree)
				setConsistStatus(i, cnFree); //for the moment, just clear consists. Later full verification
		}
		
	}
}

bool IoTT_DigitraxBuffers::cnTreeValid(uint8_t ofSlot, uint8_t cnLevel)
{
	bool upLink = (slotBuffer[ofSlot][STAT] & 0x40); 
	bool downLink = (slotBuffer[ofSlot][STAT] & 0x08);
	
	if (!(upLink || downLink)) return true; //unlinked slot
	
	if (downLink) //has others linked into
	{
		for (uint8_t i = 1; i < maxSlots; i++)
			if ((slotBuffer[i][STAT] & 0x40) && (slotBuffer[i][SPD] == ofSlot) && (i != ofSlot)) //found a node linked into this one
				if (!cnTreeValid(i, cnLevel + 1)) return false;
	}
	else //uplink only
		if (cnLevel == 0)	return false; //first level can't be uplinked
	
	return true;
}

void IoTT_DigitraxBuffers::setRedHatMode(txFct lnReply, DynamicJsonDocument doc)
{
	memcpy(&slotBuffer[0x7B], &stdSlot[0], 10);
	memcpy(&slotBuffer[0x7C], &stdSlot[0], 10);
	lnReplyFct = lnReply;
	loadRHCfgJSON(doc);
	isCommandStation = lnReply != NULL;
	if (dccPort == NULL)
	{
		dccPort = new IoTT_SerInjector(rxPin, txPin, false, 1);
        dccPort->loadLNCfgJSON(doc); //loads baudrate and invert information
        dccPort->setProtType(DCCEx); //DCC++Ex DCC Generator
//        dccPort->setTxCallback(dccGeneratorCallback);
        dccPort->setDCCCallback(dccGeneratorCallbackNew);
//		Serial.printf("Track Gauge %i\n", trackGaugesLen);
		if (trackGaugesLen > 0)
		{
			trackData = (rmsBuffer**) realloc (trackData, trackGaugesLen * sizeof(rmsBuffer*));
			for (int i=0; i < trackGaugesLen; i++)
			{
				if (trackGauges[i].currUsePin & 0x10)
				{
//					Serial.printf("Track Data %i\n", i);
					trackData[i] = new	rmsBuffer(trackGauges[i].currBuffSize);
				}
			}
		}
	}
/**
	if (isRedHat && (rhButtons == NULL)) //buttons are only available if RedHat hardware is used
	{
        DynamicJsonDocument* jsonDataObj = getDocPtr("/configdata/btn.cfg", false);
        if (jsonDataObj != NULL)
        {
			Serial.println("Load Button Data");  
			rhButtons = new IoTT_Mux64Buttons();
			rhButtons->initButtonsDirect(true); //poll mode
			rhButtons->loadButtonCfgDirectJSON(*jsonDataObj);
			delete(jsonDataObj);
		}
	}
*/
	//send RedHat configuration commands
	lnTransmitMsg txBuffer;
	if (isCommandStation)
	{
		enableFCRefresh(true, 60);
		initArduinoBoard();
	}
}

void IoTT_DigitraxBuffers::initArduinoBoard()
{
	switch (getOpSw(opSwTrackPwrRestore, 2))
	{
		case 0: //Serial.println("Track Power Init ON");
				localPowerStatusChange(0x83);
				break;
		case 1: //Serial.println("Track Power Init Idle");
				localPowerStatusChange(0x83);
				localPowerStatusChange(0x85);
				break;
		case 2: //Serial.println("Track Power Init OFF");
				localPowerStatusChange(0x82);
				break;
		case 3: //Serial.println("Track Power Init Previous");
				switch (trackByte & 0x03)
				{
					case 0: localPowerStatusChange(0x82); //off
							break;
					case 3: localPowerStatusChange(0x83); //on
							break;
					case 1: localPowerStatusChange(0x83); //idle
							localPowerStatusChange(0x85); 
							break;
					case 2: localPowerStatusChange(0x85); //idle with power off
							break;
				}
				break;
	}
	char outStr[30];
	setDCCSpeedSteps(getOpSw(opSwSpeedStepDefault, 1));
	if (configPeripheralsPwrUp)
	{
		for (uint8_t i = 0; i < sensorInputLen; i++)
			setDCCSensorPin(&sensorInputs[i]);
		for (uint8_t i = 0; i < turnoutOutputLen; i++)
			setDCCDefTurnout(&turnoutOutputs[i]);
	}
	sprintf(outStr, "D ACK LIMIT %i", progLimit);
	reqDCCPeripheralList(outStr);
	sprintf(outStr, "D ACK MIN %i", progPulseMin);
	reqDCCPeripheralList(outStr);
	sprintf(outStr, "D ACK MAX %i", progPulseMax);
	reqDCCPeripheralList(outStr);
	sprintf(outStr, "D ACK RETRY %i", progNumTry);
	reqDCCPeripheralList(outStr);
	if (progBoost)
		reqDCCPeripheralList("D PROGBOOST");
	getRedHatConfig(0);
	if (getOpSw(opSwPurgeTimeExt, 1))
		purgeLimit = purgeLimitLong;
	else
		purgeLimit = purgeLimitShort;
	if (trackGaugesLen > 0)
		setCurrReportMode((trackGauges[0].currUsePin & 0x10) >> 4, (trackGauges[1].currUsePin & 0x10) >> 4);
}

bool IoTT_DigitraxBuffers::saveToFile(String fileName)
{
	Serial.println("Save Digitrax Data to disk");
    File dataFile = SPIFFS.open(fileName, "w");
    if (dataFile)
    {
		dataFile.write(blockDetectorBuffer, numBDs);
		dataFile.write(switchPositionBuffer, numSwis);
		dataFile.write(signalAspectBuffer, numSigs);
		uint8_t buf[2];
		for (int i = 0; i < numAnalogVals; i++)
		{
			memcpy(&buf, &analogValueBuffer[i], 2);
			dataFile.write(buf, 2);
		}
		dataFile.write(buttonValueBuffer, numButtons);
		dataFile.write(trackByte);
		for (int i = 0; i < numSlots; i++)
			dataFile.write(slotBuffer[i], 10);
		
		dataFile.close();
		Serial.println("Writing Digitrax Buffer Data  File complete");
		return true;
    }
    else
    {
		Serial.println("Unable to write Config File");
		return false;
	}
}

void IoTT_DigitraxBuffers::loadFromFile(String fileName)
{
    File dataFile = SPIFFS.open(fileName, "r");
    if (dataFile)
    {
		uint32_t fileSize = dataFile.size();
		uint32_t minSize = numBDs;
		Serial.printf("Load %i bytes from disk\n", fileSize);
		if (fileSize >= minSize) 
			dataFile.read(blockDetectorBuffer, numBDs);
		minSize += numSwis;
		if (fileSize >= minSize) 
			dataFile.read(switchPositionBuffer, numSwis);
		minSize += numSigs;
		if (fileSize >= minSize) 
			dataFile.read(signalAspectBuffer, numSigs);
		uint8_t buf[2];
		minSize += (2 * numAnalogVals);
		if (fileSize >= minSize) 
			for (int i = 0; i < numAnalogVals; i++)
			{
				dataFile.read(buf,2);
//				analogValueBuffer[i] = (buf[0]<<8) + buf[1];
				memcpy(&analogValueBuffer[i], &buf, 2);
			}
		minSize += numButtons;
		if (fileSize >= minSize) 
			dataFile.read(buttonValueBuffer, numButtons);
		minSize += 1;
		if (fileSize >= minSize) 
			dataFile.read(&trackByte, 1);
		trackByte |= 0x04; //LocoNet 1.1 support
		trackByte &= ~0x08; //Programmer OFF
		minSize += (numSlots * sizeof(slotData));
		if (fileSize >= minSize)
//		if ((fileSize >= minSize) && isCommandStation) 
		{
			Serial.println("Load Slot buffer from file");
			for (int i = 0; i < numSlots; i++)
			{
				dataFile.read(slotBuffer[i], 10);
			}

//			setPowerStatus(0x83); //track power on after startup
		}
//		else
//		{
//			Serial.println("Clear Slot buffer from file");
//			clearSlotBuffer(true);
//			resetComm();
//		}

//clear MU links
//		for (int i = 0; i < maxSlots; i++)
		{
//			slotBuffer[i][0] &= 0xB7; //clear links
//			slotBuffer[i][TRK] = 0; //clear purge counter
//			Serial.print(i);
//			Serial.print(" ");
//			dispSlot(&slotBuffer[i][0]);
		}

/*		
		else
		{
			Serial.println("Reset Slot buffer");
			for (int i = 0; i < numSlots; i++)
				memcpy(&slotBuffer[i], &stdSlot[0], 10);
		}
*/
		dataFile.close();
		Serial.println("Digitrax Buffer Data File loaded");
	}
    else
		Serial.println("Unable to read Digitrax Buffer Data File");
}

void IoTT_DigitraxBuffers::processLoop()
{
	lnTransmitMsg txBuffer;
	if (dccPort)
	{
//		Serial.print(dccPort->getMsgType());
		dccPort->processLoop();
//		Serial.println();
	}

	if (millis() > nextSlotUpdate)
	{

		if ((!isCommandStation) && isLocoNet)
		{
//			Serial.println("request slot");
//			if (!focusNextAddr) //pause process if waiting for a focus slot for purplehat
//				requestNextSlotUpdate();
		}
		nextSlotUpdate += slotRequestInterval + random(500);
	}
	if (isCommandStation)
	{
		if (millis() > queryDelay)
		{
			if (inpQuery != 0xFF)
			{
				queryDelay += queryInterval;
//				Serial.printf("Set Switch %i to %i\n", 1020 - (inpQuery & 0x03), (inpQuery & 0x04) >> 2);
				sendSwitchCommand(0xB0, 1020 - (inpQuery & 0x03) - 1, (inpQuery & 0x04) >> 2, false);
				inpQuery--;
			}
		}
/*
		if (millis() - getStatusTimer > statusInterval) //runs every 5 secs
		{
			reqDCCCurrent();
			getStatusTimer += statusInterval;
		}
*/		
		if (millis() - purgeSlotTimer > purgeInterval) //runs every 10 secs
		{
//			Serial.println("check purging");
			purgeUnusedSlots();
			purgeSlotTimer += purgeInterval;
		}

		if (progModeActive) //prog command set, waiting for reply
		{
			if ((millis() - progSent) > (uint32_t)progTimeout)
			{
//				Serial.printf("Prog timeout");
				memcpy(&slotBuffer[0x7C][0], &progSlot[0], 10);
				slotBuffer[0x7C][1] = 0x04;
				prepSlotReadMsg(&txBuffer, 0x7C);
				lnOutFct(txBuffer);
				progModeActive = false;
			}
		}
	}
	else
	{
		if (!progModeActive) //prog track available
			if (readFullAddr)
			{
				std::vector<ppElement> ppList;
				ppElement newData;
				newData.dataType = 1;
				newData.paramNr = 0;
				newData.numParams = 2;
				strcpy(&newData.payload.strVal[0], "R");
				ppList.push_back(newData);
				newData.dataType = 10;
				newData.paramNr = 1;
				switch (cvBuffer.size())
				{
					case 0: break;
					case 1: if (cvBuffer.at(0) & 0x0020) 
								readProg(0,0,0,17); 
							else 
								readProg(0,0,0,1); 
							break;
					case 2: if (cvBuffer.at(0) & 0x0020)
								readProg(0,0,0,18); 
							else 
							{
//								Serial.println("OK"); 
								if (wiAddrCallback !=  NULL)
								{
									newData.payload.longVal = cvBuffer.at(1) & 0x00FF;
									ppList.push_back(newData);
									wiAddrCallback(&ppList);
								}
								readFullAddr = false; 
							}
							break;
					case 3: 
							{
//								Serial.println("OK"); 
								char outBuf[10];
								if (wiAddrCallback !=  NULL)
								{
									newData.payload.longVal = ((cvBuffer.at(1) & 0x003F) << 8) + (cvBuffer.at(2) & 0x00FF);
									ppList.push_back(newData);
									wiAddrCallback(&ppList);
									
								}
								readFullAddr = false;
							}
							break;
					default: Serial.printf("Reset %i\n", cvBuffer.size()); readFullAddr = false; break;
				}
			}
	}

	if ((millis() - fcRefresh) > fcRefreshInterval)
	{
		//refresh fast clock slot
		setFCTime(getFCTime() + slotBuffer[0x7B][0], false); //call every second, so just add the fc rate
		fcRefresh += fcRefreshInterval;
	}
	
	if ((millis() - fcLastBroadCast) > fcBroadcastInterval)
	{
		prepSlotWriteMsg(&txBuffer, 0x7B);
		if (broadcastFC)
			lnOutFct(txBuffer);
		fcLastBroadCast = millis(); //+= fcBroadcastInterval;
//		Serial.printf(" Broadcast FC %i:%i -> %i\n", trunc(intFastClock/3600), trunc((intFastClock % 3600)/60), intFastClock);
	}

	if (millis() > nextBufferUpdate)
	{
		nextBufferUpdate += bufferUpdateInterval;
	}
	if (initPhase) 
		if (millis() > 20000)
		{
			initPhase = false;
//			Serial.println("Init report");
			requestInpStatusUpdate = 0xFF;
		}
}

//all incoming Loconet messages, update buffers and handle if command station mode
void IoTT_DigitraxBuffers::processLocoNetMsg(lnReceiveBuffer * newData) 
{
	if (isCommandStation)
		processSlotManager(newData); //includes DCC generator and updating the buffers if message was processes
	else
		processBufferUpdates(newData); //update the buffers
}

void IoTT_DigitraxBuffers::writeProg(uint16_t dccAddr, uint8_t progMode, uint8_t progMethod, uint16_t cvNr, uint8_t cvVal)
{
	lnTransmitMsg txBuffer;
	slotData * prSlot = getSlotData(0x7C);
	switch (progMode)
	{
		case 0: (*prSlot)[0] = 0x60; //Prog track, byte mode, paged
			if (progMethod == 0) (*prSlot)[0] |= 0x08; //direct mode
			break;
		case 1: (*prSlot)[0] = 0x64; //OpsMode, byte mode, no feedback
			break;
		case 2: (*prSlot)[0] = 0x6C; //OpsMode, byte mode, feedback
			break;
		//add options for bitmode
	}
	(*prSlot)[2] = (dccAddr & 0x3F80) >> 7; //HOPSA
	(*prSlot)[3] = (dccAddr & 0x007F);//LOPSA
	(*prSlot)[5] = (((cvNr-1) & 0x0080) >> 7) + (((cvNr-1) & 0x0300)>>4) + ((cvVal & 0x80)>>6); //CVH + Data 7
	(*prSlot)[6] = ((cvNr-1) & 0x007F);//CVLO
	(*prSlot)[7] = cvVal & 0x7F;
	prepSlotWriteMsg(&txBuffer, 0x7C);
	lnOutFct(txBuffer);
	progModeActive = true;
}

void IoTT_DigitraxBuffers::readProg(uint16_t dccAddr, uint8_t progMode, uint8_t progMethod, uint16_t cvNr)
{
	lnTransmitMsg txBuffer;
	slotData * prSlot = getSlotData(0x7C);
	switch (progMode)
	{
		case 0: (*prSlot)[0] = 0x20; //Prog track, byte mode
			if (progMethod == 0) (*prSlot)[0] |= 0x08; //direct mode
			break;
		case 1: (*prSlot)[0] = 0x24; //OpsMode, no feedback
			break;
		case 2: (*prSlot)[0] = 0x2C; //OpsMode, feedback
			break;
	}
	(*prSlot)[1] = 0;
	(*prSlot)[2] = (dccAddr & 0x3F80) >> 7; //HOPSA
	(*prSlot)[3] = (dccAddr & 0x007F);//LOPSA
	(*prSlot)[5] = (((cvNr-1) & 0x0080) >> 7) + (((cvNr-1) & 0x0300)>>4); //CVH 
	(*prSlot)[6] = ((cvNr-1) & 0x007F);//CVLO
	(*prSlot)[7] = 0;
	prepSlotWriteMsg(&txBuffer, 0x7C);
	lnOutFct(txBuffer);
	progModeActive = true;
}

void IoTT_DigitraxBuffers::readAddrOnly()
{
	cvBuffer.clear();
	readFullAddr = true;
	if (isCommandStation)
		setDCCProgrammingCmd(0xFF, 0, 0, 0, 0, 0);
	else
		readProg(0,0, 0, 29);
}


//status buffer update and access functions 

uint8_t IoTT_DigitraxBuffers::getPowerStatus()
{
	switch (trackByte & 0x03)
	{
		case 1: return 2; //IDLE
		case 3: return 1; //ON
		default: return 0; //OFF
	}
}

uint8_t IoTT_DigitraxBuffers::getButtonValue(uint16_t buttonNum)
{
	return buttonValueBuffer[buttonNum];
}

uint8_t IoTT_DigitraxBuffers::getBDStatus(uint16_t bdNum)
{
	uint16_t byteNr = bdNum>>3;  //	uint16_t byteNr = trunc(bdNum/8);
	return ((blockDetectorBuffer[byteNr] >> (bdNum % 8)) & 0x01); //0=free, 1=occ
}

uint8_t IoTT_DigitraxBuffers::getSwiPosition(uint16_t swiNum)
{
	return ((switchPositionBuffer[swiNum >> 2] >> (2 * (swiNum % 4))) & 0x02) << 4;
}

uint8_t IoTT_DigitraxBuffers::getSwiCoilStatus(uint16_t swiNum)
{
	return ((switchPositionBuffer[swiNum >> 2] >> (2 * (swiNum % 4))) & 0x01) << 4;
}

uint8_t IoTT_DigitraxBuffers::getSwiStatus(uint16_t swiNum)
{
	return ((switchPositionBuffer[swiNum >> 2] >> (2 * (swiNum % 4))) & 0x03) << 4;
}

void IoTT_DigitraxBuffers::setSwiStatus(uint16_t swiNum, bool swiPos, bool coilStatus)
{
	uint16_t byteNr = trunc(swiNum/4);
	uint8_t inpPosStat = 0;
	swWrPtr = (swWrPtr + 1) % switchProtLen;
	switchProtocol[swWrPtr].devAddr = swiNum;
	switchProtocol[swWrPtr].lastActivity = millis();
	if (swiPos)
		inpPosStat |= 0x02;
	if (coilStatus)
		inpPosStat |= 0x01;
    switchPositionBuffer[byteNr] &= ~(0x03<<(2*(swiNum % 4))); //clear bits
    switchPositionBuffer[byteNr] |= inpPosStat<<(2*(swiNum % 4)); //set status bits
}

//get the time when switch received last command. Used for retriggering while active
uint32_t IoTT_DigitraxBuffers::getLastSwiActivity(uint16_t swiNum)
{
	uint8_t thisOfs = swWrPtr + switchProtLen;
	for (uint8_t i = 0; i < switchProtLen; i++)
	{
		uint8_t thisEntry = (thisOfs - i) % switchProtLen;
		if (switchProtocol[thisEntry].devAddr == swiNum)
			return switchProtocol[thisEntry].lastActivity;
	}
	return 0;
}

uint8_t IoTT_DigitraxBuffers::getSignalAspect(uint16_t sigNum)
{
	return signalAspectBuffer[sigNum];
}

void IoTT_DigitraxBuffers::setSignalAspect(uint16_t sigNum, uint8_t sigAspect)
{
	signalAspectBuffer[sigNum] = (sigAspect & 0x1F);
}

uint16_t IoTT_DigitraxBuffers::getAnalogValue(uint16_t analogNum)
{
//	Serial.printf("Get analog %i %i\n", analogNum, analogValueBuffer[analogNum]);
	return analogValueBuffer[analogNum];
}

void IoTT_DigitraxBuffers::setAnalogValue(uint16_t analogNum, uint16_t analogValue)
{
//	Serial.printf("Set Analog %i %i \n", analogNum, analogValue);
	analogValueBuffer[analogNum] = analogValue;
}

bool IoTT_DigitraxBuffers::getBushbyWatch()
{
	return bushbyWatch;
}

/*
void IoTT_DigitraxBuffers::addActor(uint16_t Id, uint8_t pinType, uint8_t pinNr, uint8_t flags)
{
	
}
*/

void IoTT_DigitraxBuffers::sendFCCmdToWeb()
{
	int8_t currClient = getWSClientByPage(0, "pgRedHatCfg");
	if (currClient >= 0)
	{
		DynamicJsonDocument doc(200);
		char myMqttMsg[100];
		doc["Cmd"] = "FC";
		JsonObject Data = doc.createNestedObject("Data");
		Data["Time"] = getFCTime();
		Data["Rate"] = getFCRate();
		serializeJson(doc, myMqttMsg);
		while (currClient >= 0)
		{
			globalClients[currClient].wsClient->text(myMqttMsg);
			currClient = getWSClientByPage(currClient + 1, "pgRedHatCfg");
		}
	}
}

void IoTT_DigitraxBuffers::sendTrackCurrent(uint8_t trackId)
{
	int8_t currClient = getWSClientByPage(0, "pgRedHatCfg");
	if (currClient >= 0)
	{
		DynamicJsonDocument doc(200);
		char myMqttMsg[100];
		doc["Cmd"] = "DCCAmp";
		JsonObject Data = doc.createNestedObject("Data");
		Data["Track"] = trackId;		
		Data["Value"] = (trackGauges[trackId].currUsePin & 0x10) > 0 ? trackData[trackId]->getRMSVal() : 0;
//		Data["Value"] = (trackGauges[trackId].currUsePin & 0x10) > 0 ? trackData[trackId]->getRMSVal() * trackGauges[trackId].currMultiplier : 0;
		serializeJson(doc, myMqttMsg);
		while (currClient >= 0)
		{
			globalClients[currClient].wsClient->text(myMqttMsg);
			currClient = getWSClientByPage(currClient + 1, "pgRedHatCfg");
		}
//		Serial.println(myMqttMsg);
	}
}

//void IoTT_DigitraxBuffers::sendDCCCmdToWeb(ppElement * myParams)
void IoTT_DigitraxBuffers::sendDCCCmdToWeb(std::vector<ppElement> * myParams)
{
	int8_t currClient = getWSClientByPage(0, "pgRedHatCfg");
	if (currClient >= 0)
	{
		uint8_t paramCount = myParams->at(0).numParams;
		DynamicJsonDocument doc(600);
		char myMqttMsg[300];
		doc["Cmd"] = "DCCPP";
		JsonObject Data = doc.createNestedObject("Data");
		JsonArray msgArray = Data.createNestedArray("Msg");
		for (uint8_t i = 0; i < paramCount; i++)
		{
			char nodeName[5];
			msgArray[i]["t"] = myParams->at(i).dataType;
			switch (myParams->at(i).dataType)
			{
				case 10: msgArray[i]["d"] = myParams->at(i).payload.longVal ; break;
				case 20: msgArray[i]["d"] = myParams->at(i).payload.floatVal; break;
				default: if (myParams->at(i).dataType == 0)
							msgArray[i]["d"] = myParams->at(i).payload.strPtr;
						 else
							msgArray[i]["d"] = myParams->at(i).payload.strVal;
//		  				 memcpy(&nodeName[0], &myParams[i].payload.strVal[0], myParams[i].dataType);
//						 nodeName[myParams->at(i).dataType] = '\0';
//						 nodeName[myParams[i].dataType] = 0; 
//						 msgArray[i]["d"] = nodeName;
						 break;
			}
		}
		serializeJson(doc, myMqttMsg);
		while (currClient >= 0)
		{
			globalClients[currClient].wsClient->text(myMqttMsg);
			currClient = getWSClientByPage(currClient + 1, "pgRedHatCfg");
		}
//		Serial.println(myMqttMsg);
	}
}

uint16_t IoTT_DigitraxBuffers::receiveDCCGeneratorFeedbackNew(std::vector<ppElement> * txData)
{
	lnTransmitMsg txBuffer;
	bool sendToWeb = false;
	uint8_t paramCount = txData->at(0).numParams;
	char opCode = txData->at(0).payload.strVal[0];

/*	
	for (uint8_t i = 0; i < paramCount; i++)
	{
		switch (txData->at(i).dataType)
		{
			case 0:  Serial.printf("Param %i %i String %s \n", i, txData->at(i).paramNr, txData->at(i).payload.strPtr); break;
			case 10: Serial.printf("Param %i %i Int = %i\n", i, txData->at(i).paramNr, txData->at(i).payload.longVal); break;
			case 20: Serial.printf("Param %i %i Float = %.2f\n", i, txData->at(i).paramNr, txData->at(i).payload.floatVal); break;
			default: Serial.printf("Param %i %i Short String %s\n", i, txData->at(i).paramNr, txData->at(i).payload.strVal); break;
		}
	}
*/
	switch (opCode)
	{
		case 'a': //current reporting
			if (paramCount == 3) // <a TRACK VALUE>
			{
				uint8_t trackID = txData->at(1).payload.longVal;
				if ((trackGauges[trackID].currUsePin & 0x10) > 0) 
				{
					uint16_t newVal = round(trackGauges[trackID].currMultiplier * txData->at(2).payload.longVal);
					if (newVal > trackGauges[trackID].currOffset)
						newVal = round(newVal - trackGauges[trackID].currOffset);
					else
						newVal = 0;
					trackData[trackID]->addVal(newVal); 
					break;
				}
			}
			break;
		case 'c': 
//			Serial.println(txData->at(2).payload.longVal);
			break;
		case 'H': //turnout info
			if (paramCount == 3) // <H ID STATE> for each defined DCC Accessory Turnout in return to <T>
			{
				arduinoPins * thisActor = findPeripherialItemById(turnoutOutputs, turnoutOutputLen, 0x80, txData->at(1).payload.longVal); //looking for ardID
				if (thisActor)
				{
					if (thisActor->ardType > 0)
						thisActor->confOK = true;
//					Serial.printf("Defined Turnout %i Addr %i Position %i\n", myParams[1].payload.longVal, thisActor->lnAddr, myParams[2].payload.longVal);
					setSwiStatus(thisActor->lnAddr, txData->at(2).payload.longVal == 0 ? 1 : 0, 0); //DCC EX uses reverse status than Loconet
//					if ()
					{
						lnTransmitMsg thisBuffer;
						prepTurnoutReportMsg(&thisBuffer, thisActor->lnAddr, txData->at(2).payload.longVal == 0? 1: 0);
						lnOutFct(thisBuffer);
					}
				}
//				else
//					Serial.printf("Undefined Turnout %i Position %i\n", myParams[1].payload.longVal, myParams[2].payload.longVal);
			}
			else
				if (txData->at(2).dataType < 5) //string type
				{
					arduinoPins * thisActor = findPeripherialItemById(turnoutOutputs, turnoutOutputLen, 0x80, txData->at(1).payload.longVal); //looking for ardID
//					char tType[5] = {0,0,0,0,0};
//					memcpy(&tType, &txData->at(2).payload.strVal, txData->at(2).dataType);
					if (thisActor)
					{
						if (thisActor->ardType > 0)
							thisActor->confOK = true;
//						Serial.printf("Defined Turnout %i Type %s on Pin %i Position %i\n", myParams[1].payload.longVal, tType, myParams[3].payload.longVal, myParams[4].payload.longVal);
					}
//					else
//					{
//						Serial.printf("Undefined Turnout %i Type %s on Pin %i Position %i\n", myParams[1].payload.longVal, tType, myParams[3].payload.longVal, myParams[4].payload.longVal);
//						addActor(txData->at(1).payload.longVal, 0, txData->at(3).payload.longVal, txData->at(4).payload.longVal);
//					}	
					sendToWeb = true;
				}
			break;
		case 'j': 
			switch (txData->at(1).payload.strVal[0])
			{
				case 'T': 
					if (paramCount > 2)
					{
						bool isList = (paramCount < 4) ? true : txData->at(3).dataType == 10; //integer
						if (isList)
						{
//							Serial.printf("Turnout List of %i\n", paramCount-2);
							for (uint16_t i = 2; i < paramCount; i++)
								reqSwitchInfo(txData->at(i).payload.longVal);
						}
						else
						{
							sendToWeb = true;
//							Serial.printf("Process Turnout %i\n", txData->at(2).payload.longVal);
						}
					}
					break;
				case 'A': 
					if (paramCount > 2)
					{
						bool isList = (paramCount < 4) ? true : txData->at(3).dataType == 10; //integer
						if (isList)
						{
//							Serial.printf("Route List of %i\n", paramCount-2);
							for (uint16_t i = 2; i < paramCount; i++)
								reqRouteInfo(txData->at(i).payload.longVal);
						}
						else
						{
//							Serial.printf("Route Entry of %i\n", txData->at(2).payload.longVal);
							//process JA info
							sendToWeb = true;
						}
					}
					break;
				case 'R': 
					if (paramCount > 2)
					{
						bool isList = (paramCount < 4) ? true : txData->at(3).dataType == 10; //integer
						if (isList)
						{
//							Serial.printf("Roster List of %i\n", paramCount-2);
							for (uint16_t i = 2; i < paramCount; i++)
								reqRosterInfo(txData->at(i).payload.longVal);
						}
						else
						{
//							Serial.printf("Roster Entry of %i\n", txData->at(2).payload.longVal);
							//process JR info
							sendToWeb = true;
						}
					}
					break;
			}
			break;
		case 'l': //loco info  <l CAB SLOT SPEED/DIR FUNC>
		{
			uint16_t dccAddr = txData->at(1).payload.longVal;
			uint8_t thisSlot = getSlotOfAddr(dccAddr & 0x7F, dccAddr >> 7);
//			Serial.printf("<l> Loco %i Speed %i Dir %i Functions %2X\n", myParams[1].payload.longVal, myParams[3].payload.longVal & 0x7F, (myParams[3].payload.longVal & 0x80)>>7, myParams[4].payload.longVal);
		}
		break;
		case 'p': //Power Report
		{
//			Serial.printf("Power Report %i\n", myParams[1].payload.longVal);
			switch (txData->at(1).payload.longVal)
			{ 
				case 0: if ((trackByte &0x01) != 0) localPowerStatusChange(0x82); //OFF
					break;
				case 1: if ((trackByte &0x01) != 1) localPowerStatusChange(0x83); //ON
					break;
			}
		}
		break;
		case 'O': //slot # report
			Serial.println("Command successfully executed");
		break;

		case 'q':;
		case 'Q': //sensor input
		{
			arduinoPins * thisSensor = findPeripherialItemById(sensorInputs, sensorInputLen, 0x80, txData->at(1).payload.longVal); //looking for ardID
			if (thisSensor)
			{
				if (paramCount == 4) //<Q ID PIN PULLUP> for each defined sensor as return to <S>
				{
					thisSensor->confOK = true;
//					Serial.printf("Defined Sensor %i on Pin %i Pullup %i\n", myParams[1].payload.longVal, myParams[2].payload.longVal, myParams[3].payload.longVal);
				}
				if (paramCount == 2) //<Q ID > for sensor event report
					processDCCInput(txData->at(1).payload.longVal, (opCode == 'Q') ? true: false);
			}
			if (paramCount == 4) //<Q ID PIN PULLUP> for each defined sensor as return to <S>
				sendToWeb = true;
			break;
		}
		case 'r': //programmer callback on read/write cv bit/byte on prog
		{
//<r Value> //Address Read return. Can be long address
//<r CV Value> //write CV value return
//<r CALLBACKNUM|CALLBACKSUB|CV Value>
//<r CALLBACKNUM|CALLBACKSUB|CV BIT VALUE> not used
//<r CALLBACKNUM|CALLBACKSUB|CV VALUE> //read cv value

			switch (paramCount)
			{
				case 2: if (wiAddrCallback !=  NULL) wiAddrCallback(txData); return 0; break;
				case 6: break; //bit mode not used
				default:
				{
					uint16_t cvNr = txData->at(paramCount - 2).payload.longVal - 1;
					int16_t cvVal = txData->at(paramCount - 1).payload.longVal;
					memcpy(&slotBuffer[0x7C][0], &progSlot[0], 10);
					if (cvVal < 0)
						slotBuffer[0x7C][1] = 0x07;
					else
					{
						slotBuffer[0x7C][1] = 0x00;
						slotBuffer[0x7C][5] = ((cvNr >> 7) & 0x01) + ((cvNr >> 4) & 0x30) + ((cvVal & 0x80)>>6); //high 3 of 10 bits + MSB of data
						slotBuffer[0x7C][6] = cvNr & 0x7F; //low 7 of 10 bits
						slotBuffer[0x7C][7] = cvVal & 0x7F;
					}
				}
				break;
			}
			prepSlotReadMsg(&txBuffer, 0x7C);
			lnOutFct(txBuffer);
			progModeActive = false;
//			Serial.printf("Send Programmer final task reply CV %i Val %i \n", cvNr+1, cvVal);
		}
		break;
		case 'X': //slot # report
//			Serial.println("Command execution failed");
		break;
		
		case 'Y': //<Y ID IN IFLAG STATE> for each defined output pin in return to <Z>
			if (paramCount == 5) // <Y ID PIN IFLAG STATE> for each defined pin as return to <Z>
			{
				arduinoPins * thisActor = findPeripherialItemById(turnoutOutputs, turnoutOutputLen, 0x80, txData->at(1).payload.longVal); //looking for ardID
				if (thisActor)
				{
					if (thisActor->ardType == 0)
						thisActor->confOK = true;
//					Serial.printf("Defined ZPin %i on Pin %i IFlag %i\n", myParams[1].payload.longVal, myParams[2].payload.longVal, myParams[3].payload.longVal, myParams[4].payload.longVal);
				}
//				else
//					Serial.printf("Undefined ZPin %i on Pin %i IFlag %i\n", myParams[1].payload.longVal, myParams[2].payload.longVal, myParams[3].payload.longVal, myParams[4].payload.longVal);
//					addActor(myParams[1].payload.longVal, 0, myParams[3].payload.longVal, myParams[4].payload.longVal);
			}
			sendToWeb = true;
			break;
		break;
		case 'Z': //output pin report
		break;
	
		case '#': //slot # report
			DCCActiveSlots = txData->at(1).payload.longVal;
			sendToWeb = true;
//			Serial.printf("DCC++EX has %i refresh slots\n", myParams[1].payload.longVal);
		break;
		default: 	// Serial.printf("Unprocessed DCC++ Msg OpCode %c with %i params\n", myParams[0].payload.strVal[0], myParams[0].numParams);
		break;
	}
	if (sendToWeb)
		if (millis() < webTimeout)
			sendDCCCmdToWeb(txData);
	return 0; //not really needed
}

void IoTT_DigitraxBuffers::sendRedHatCmd(char * cmdStr)
{
//	Serial.println(cmdStr);
	reqDCCPeripheralList(cmdStr);
	webTimeout = millis() + 3000; //time window to process reply to web page 
}


void IoTT_DigitraxBuffers::getRedHatConfig(uint16_t filterMask)
{
	reqDCCPeripheralList("T");
	reqDCCPeripheralList("S");
	reqDCCPeripheralList("Z");
	reqDCCPeripheralList("#");
	reqDCCPeripheralList("JT");
	reqDCCPeripheralList("JA");
	reqDCCPeripheralList("JR");
	webTimeout = millis() + 3000;
}

arduinoPins * IoTT_DigitraxBuffers::findPeripherialItemById(arduinoPins * itemList, uint16_t listLen, uint8_t devType, uint16_t itemID) //devType bits 0-6 reserved for device types. 
																																		//Right now all is digital input and switch commands
{																																		//Bit 0 indicates in or out messages
	for (int i=0; i < listLen; i++)
		if (devType & 0x80)
		{
			if (itemList[i].ardID == itemID)
				return &itemList[i];
		}
		else
		{
			if (itemList[i].lnAddr == itemID)
				return &itemList[i];
		}
	return NULL;
}

void IoTT_DigitraxBuffers::processDCCSwitch(uint16_t swiAddr, uint8_t swiPos, uint8_t coilStatus)
{
	if (turnoutOutputs)
	{
		arduinoPins * thisActor = findPeripherialItemById(turnoutOutputs, turnoutOutputLen, 0x00, swiAddr); //incoming, find lnAddr
		if (thisActor)
		{
			if (thisActor->confOK)
			{
				if (thisActor->ardType == 0)
					setDCCPinById(thisActor->ardID, swiPos);
				else
					setDCCSwitchById(thisActor->ardID, swiPos, coilStatus);
				return;
			} //else Serial.println("not ok");
		} //else Serial.println("not actor");
	}
	setDCCSwitchCmd(swiAddr, swiPos, coilStatus);
}

void IoTT_DigitraxBuffers::processDCCInput(uint16_t sensID, bool sensStatus)
{
	lnTransmitMsg txBuffer;
	if (sensorInputs)
	{
		arduinoPins * thisSensor = findPeripherialItemById(sensorInputs, sensorInputLen, 0x80, sensID); //incoming, find ardID
		if (thisSensor)
			if (thisSensor->confOK)
			{
//				Serial.printf("found %i LN Addr %i Type %i\n",thisSensor->ardID, thisSensor->lnAddr, thisSensor->lnType);
				switch (thisSensor->lnType)
				{
					case blockdet: //0 Block detector
						if (getOpSw(opEchoInp, 1))
							sendBlockDetectorCommand(thisSensor->lnAddr, sensStatus);
						else
							setBDStatus(thisSensor->lnAddr, sensStatus);
					break;
					case dccswitchrep: //9
						if (getOpSw(opEchoInp, 1))
							sendSwiReportMessage(thisSensor->lnAddr, sensStatus);
						else
							setSwiStatus(thisSensor->lnAddr, sensStatus, false);
						break;
					case svbutton: //5
						if (getOpSw(opEchoInp, 1))
							sendButtonCommand(thisSensor->lnAddr, sensStatus);
						else
							setButtonValue(thisSensor->lnAddr, sensStatus);
						break;
					default: return;
						break;
			}
		}
	}
}

uint8_t IoTT_DigitraxBuffers::getUpdateReqStatus()
{
	return requestInpStatusUpdate;
}

void IoTT_DigitraxBuffers::clearUpdateReqFlag(uint8_t clrFlagMask)
{
	requestInpStatusUpdate &= (~clrFlagMask);
}

void IoTT_DigitraxBuffers::enableBushbyWatch(bool enableBushby)
{
	bushbyWatch = enableBushby;
}

uint32_t IoTT_DigitraxBuffers::getFCTime()
{
	return intFastClock;
}

uint32_t IoTT_DigitraxBuffers::getFCRate()
{
	return slotBuffer[0x7B][0];
}

void IoTT_DigitraxBuffers::setFCTime(uint32_t newTime, bool updateLN)
{
//	Serial.printf("New time: %i\n", newTime);
	intFastClock = newTime;
	intFastClock %= 86400;
	uint16_t sysHour = trunc(intFastClock / 3600);
	sysHour %= 24;
	slotBuffer[0x7B][5] = 0x68 + sysHour;
		
	uint16_t sysMin = trunc(intFastClock / 60);
	sysMin %= 60;
	slotBuffer[0x7B][3] = 0x44 + sysMin;
	if (updateLN)
		if (millis() > fcBroadcastInterval)
			fcLastBroadCast = millis() - fcBroadcastInterval;
		else
			fcLastBroadCast = millis();
}

void IoTT_DigitraxBuffers::setFCRate(uint8_t newRate, bool updateLN)
{
//	Serial.printf("New rate: %i\n", newRate);
	slotBuffer[0x7B][0] = newRate;
	if (updateLN)
		if (millis() > fcBroadcastInterval)
			fcLastBroadCast = millis() - fcBroadcastInterval;
		else
			fcLastBroadCast = millis();
}

void IoTT_DigitraxBuffers::enableLissyMod(bool enableLissy)
{
	translateLissy = enableLissy;
}

void IoTT_DigitraxBuffers::enableFCRefresh(bool useFC, uint16_t fcRefreshRate)
{
	broadcastFC = useFC;
	fcBroadcastInterval = 1000 * fcRefreshRate; //secs to msecs
}

void IoTT_DigitraxBuffers::awaitFocusSlot(int16_t dccAddr, bool simulOnly)
{
	focusNextAddr = true;
	focusSlot = -1;
//	Serial.println("Await");
	if (dccAddr > 0)
	{
//		Serial.println("Transmit");
		lnTransmitMsg txBuffer;
		prepLocoAddrReqMsg(&txBuffer, dccAddr);
		lnOutFct(txBuffer);
	}
}

int8_t IoTT_DigitraxBuffers::getFocusSlotNr()
{
	return focusSlot;
} 

void IoTT_DigitraxBuffers::processBufferUpdates(lnReceiveBuffer * newData) //process incoming Loconet messages to the buffer
{
//  this is the main procedure for handling LN if not command station
//  if cmd stn mode buffer is updated after slot manager and before DCC command generator 
//	Serial.printf("processBufferUpdates %2X\n", newData->lnData[0]);
	switch (newData->lnData[0])
	{
		case 0x82:; //OPC_OFF
		case 0x83:; //OPC_ON
		case 0x85: //OPC_IDLE
		{
			setPowerStatus(newData->lnData[0]); 
//			if (handlePowerStatus) 
//				handlePowerStatus(); //callback function to application
		}
		break;
		case 0xA0:; //OPC_LOCO_SPD
		case 0xA1:; //OPC_LOCO_DIRF
		case 0xA2:; //OPC_LOCO_SND
		case 0xA3:  //OPCLOCO_F912
		{
			if (slotBuffer[newData->lnData[1]][1] == 0x80) 
			{
				if (!isCommandStation) //unused slot, send slot request to command station
				{
//					Serial.println("Get Slot Info");
					lnTransmitMsg thisBuffer;
					prepSlotRequestMsg(&thisBuffer, newData->lnData[1]);
					lnOutFct(thisBuffer);
				}
				else
				{
//					Serial.println("Ignore");
					//ignore
				}
			}
			else
			{
//				Serial.println("Update");
				setSlotDirfSpeed(newData); //update slot, send DCC commands, if command station
			}
		}
		break;
        case 0xB1: //OPC_SW_REP from e.g. switch decoder
        {
//			if (!isCommandStation)
			{
				uint16_t swiAddr = ((newData->lnData[1] & 0x7F)) + ((newData->lnData[2] & 0x0F)<<7);
				uint8_t inpPosStat = 0;
				if (newData->lnData[2] & 0x40) //AUX/SWI fromat
				{
					if (newData->lnData[2] & 0x20) //SWI input
						inpPosStat = (newData->lnData[2] & 0x10) >> 3; //Coil always 0 
					else
						return; //ignore AUX input report
				
				}
				else //COIL format
				{
					switch (newData->lnData[2] & 0x30)
					{
						case 0x30:; //invalid both ON, ignore
						case 0x00: return;  break;//all off
						case 0x10:
							inpPosStat = 0x01; //thrown, coil on
							break;
						case 0x20:
							inpPosStat = 0x03; //closed, coil on
							break;
					}
				}
//          	Serial.printf("Set Switch %i to %i\n", swiAddr, inpPosStat);
				setSwiStatus(swiAddr, (inpPosStat & 0x02)>>1, (inpPosStat & 0x01));
			}
			break;
		}
        case 0xB0: //OPC_SW_REQ
        {
			if (bushbyWatch) 
				if (getBushbyStatus() > 0) 
					break; //ignore if Bushby bit set and Watch active
			uint16_t swiAddr = ((newData->lnData[1] & 0x7F)) + ((newData->lnData[2] & 0x0F)<<7);
//			Serial.println(swiAddr & 0x3FFC, 16);
			if (((swiAddr & 0x3FFC) == 0x03F8) && ((newData->lnData[2] & 0x10) == 0))
			{
				requestInpStatusUpdate |= 0x01 << ((2 * (swiAddr & 0x0003)) + ((newData->lnData[2] & 0x20) >> 5));
//				Serial.println(requestInpStatusUpdate, 16);
			}
		}
        case 0xBD:  //OPC_SW_ACK
        {
			uint16_t swiAddr = ((newData->lnData[1] & 0x7F)) + ((newData->lnData[2] & 0x0F)<<7);
			uint8_t inpPosStat = (newData->lnData[2] & 0x30)>>4; //Direction and ON Status
 //           Serial.printf("Set Switch %i to %i\n", swiAddr, inpPosStat);
			setSwiStatus(swiAddr, (inpPosStat & 0x02)>>1, (inpPosStat & 0x01));
			if (handleSwiEvent) //app callback
				handleSwiEvent(swiAddr, (inpPosStat & 0x02)>>1, (inpPosStat & 0x01));
			break;
        }
        case 0xBC: //OPC_SW_STATE
			lastSwiAddr = ((newData->lnData[1] & 0x7F)) + ((newData->lnData[2] & 0x0F)<<7); //store switch address for coming LACK
			break;
		case 0xB4: //OPC_LONG_ACK
			switch (newData->lnData[1])
			{
				case 0x3C: //OPC_SW_STATE
					if (!isCommandStation)
					{
						if (lastSwiAddr != 0xFFFF)
						{
							switch (newData->lnData[2] & 0x70)
							{
								case 0x30: if ((getSwiPosition(lastSwiAddr) >> 5) == 0) setSwiStatus(lastSwiAddr, 1, 0); break; //only change in case of deviation
								case 0x50: if ((getSwiPosition(lastSwiAddr) >> 5) == 1) setSwiStatus(lastSwiAddr, 0, 0); break; //to avoid setting of dyn signals
							}
						}
					}
					break;
				case 0x6F: //slot write
					if (!isCommandStation)
						progBusy =  (newData->lnData[2] == 1);
					break;
			}
			lastSwiAddr = 0xFFFF;
			break;
        case 0xB2: //OPC_INPUT_REP
        {
			uint16_t inpAddr = ((newData->lnData[1] & 0x7F)<<1) + ((newData->lnData[2] & 0x0F)<<8) + ((newData->lnData[2] & 0x20)>>5);
			uint8_t inpStatus = (newData->lnData[2] & 0x10)>>4;
			setBDStatus(inpAddr, inpStatus > 0);
			if (handleInputEvent)
				handleInputEvent(inpAddr, inpStatus);
			break;
        }
        case 0xB5: //OPC_SLOT_STAT1 
		{
			setSlotDirfSpeed(newData); //update slot, sen DCC command later, if Cmd Stn
			break;
		}
		case 0xB6: //OPC_CONSIST_FUNC 
		{
			setSlotDirfSpeed(newData); //update slot, sen DCC command later, if Cmd Stn
			break;
		}
/*		
		case 0xB8: //OPC_UNLINK_SLOTS, handled in limited master or higher
		case 0xB9: //OPC_LINK_SLOTS, handled in limitd master or higher
		case 0xBA: //OPC_MOVE_SLOTS, handled in limited master or higher
		case 0xBB: //OPC_RQ_SL_DATA, handled in limited master or higher
		case 0xBC:  //OPC_SW_STATE request, only processed in limited master of higher
		case 0xBF:  //OPC_LOCO_ADR request, only processed in limited master of higher
		* 	break;
*/

        case 0xD0: //OPC_MULTI_SENSE
        {
			switch (newData->lnData[1] & 0x60)
			{
				case 0x00:
				case 0x20: 
				{
					uint16_t zoneAddr = (((newData->lnData[1] & 0x1F) << 7) + (newData->lnData[2] & 0x7F));
					uint16_t locoAddr;
					if (newData->lnData[3] == 0x7E)
						locoAddr = newData->lnData[4] & 0x7F;
					else
						locoAddr = (newData->lnData[3] << 7) + (newData->lnData[4] & 0x7F);
					if (handleTranspondingEvent)
						handleTranspondingEvent(zoneAddr, locoAddr, (newData->lnData[1] & 0x20)>>5);
					break;
				}
				case 0x60:
				{
					switch ((newData->lnData[1] & 0X0E) >> 1)
					{
						case 0x01:
						{
							uint16_t pmAddr = ((newData->lnData[1] & 0x01) << 7) + newData->lnData[2];
//							return "Power Manager " + pmAddr.toString() + " Message ";
							break;
						}
						case 0x02:
						{
//							return "Barcode reader message";
							break;
						}
						default:
						{
//							return "Invalid OPC_MULTI_SENSE Device Message";
							break;
						}
					}
					break;
				}
				default:
				{
//					return "unspecified OPC_MULTI_SENSE message";
					break;
				}
			}
			break;
		}
		case 0xE0: //OPC_MULTI_SENSE_LONG as used by Digikeijs railcom detector
			if ((translateLissy) & (newData->lnData[1] == 0x09))
			{
				lnTransmitMsg thisBuffer;
				prepLissyMsg(newData, &thisBuffer);
				lnOutFct(thisBuffer);
			}
			break;
        case 0xED: //OPC_IMM_PACKET
        {
			//add DCC decoder from switch and locos
			if ((newData->lnData[1] == 0x0B) && (newData->lnData[2] == 0x7F))
			{
				byte recData[6];
				for (int i=0; i < 5; i++)
				{
					recData[i] = newData->lnData[i+4]; //get all the IMM bytes
					if (i > 0)
						recData[i] |= ((recData[0] & (0x01<<(i-1)))<<(8-i)); //distribute 8th  its to data bytes
				}
				word boardAddress = (((~recData[2]) & 0x70) << 2) | (recData[1] & 0x3F) ;
				byte turnoutPairIndex = (recData[2] & 0x06) >> 1;
				word sigAddress = (((boardAddress-1)<<2) | turnoutPairIndex) + 1;
				word sigAspect = recData[3] & 0x1F;
				setSignalAspect(sigAddress, sigAspect);
				if (handleSignalEvent)
					handleSignalEvent(sigAddress, sigAspect);
			}
			break;
        }
        case 0xE5: 
        {
			switch (newData->lnData[1])
			{
				case 0x10: //OPC_PEER_XFER (button input, sv subgroup)
				{
					//add code to interpret button and analog commands
					uint16_t btnAddr = (newData->lnData[8] & 0x3F) + ((newData->lnData[9] & 0x3F) << 6);
					uint8_t  svCmd = newData->lnData[3];
					uint8_t  svType = newData->lnData[4];
					uint8_t  inputType = newData->lnData[11];
					uint16_t inputValue = (newData->lnData[12] & 0x3F) + ((newData->lnData[13] & 0x3F) << 6);
					if ((svCmd == 0x71) && (svType == 0x02))
					{
						switch (inputType)
						{
							case 0: setAnalogValue(btnAddr, inputValue); 
								if (handleAnalogValue)	
									handleAnalogValue(btnAddr, inputValue); 
								break;
							case 1: setButtonValue(btnAddr, inputValue & 0xFF); 
								if (handleButtonValue)	
									handleButtonValue(btnAddr, inputValue); 
								break;
						}
					}
					break;
				}
//				case 0x14: break;//OPC_RFID
				default: break;
			}
			break;
		}
        case 0xEF:; //OPC_WR_SL
        case 0xE7: //OPC_SL_RD
			if (!isCommandStation) //for command station, this is handled in slotmanager
			{
				nextSlotUpdate = millis() + slotRequestInterval + random(500); //prevent periodic update in case
				switch (newData->lnData[1])
				{
					case 0x0E: //SL_RD or WR_SL
					{
						uint8_t slotNr = newData->lnData[2];
						slotData * newSlot = &slotBuffer[slotNr];
						newData->requestID = ((*newSlot)[0] ^ newData->lnData[3]) & 0x007F; //identify changes in slot status
//						Serial.printf("RDWR %i %i %i %i\n", slotNr, (*newSlot)[0], newData->lnData[3], newData->reqID);
						memcpy(newSlot[0], &newData->lnData[3], 10);
						switch (slotNr)
						{
							case 0x7B: //Fast Clock
								{
									uint8_t sysHour = (digitraxBuffer->slotBuffer[0x7B][5] - 104) % 24;
									uint8_t sysMin = (digitraxBuffer->slotBuffer[0x7B][3] - 68) % 60;
									intFastClock = 3600 * sysHour + 60 * sysMin; //seconds per day, roll over
									fcLastBroadCast = millis();
//									sendFCCmdToWeb();
//									Serial.printf("Update internal FC %i:%i -> %i\n", sysHour, sysMin, intFastClock);
//									Serial.println("FC Counter Reset");

								}
								break;
							case 0x7C: //Programmer
								if (newData->lnData[0] == 0xE7) //final programmer reply
								{
									if (readFullAddr)
									{
										uint8_t * progSlot = newSlot[0];
										if (progSlot[1] == 0)
										{
											uint16_t cvNr = ((progSlot[5] & 0x30)<<4) + ((progSlot[5] & 0x01)<<7) + (progSlot[6] & 0x7F);
											uint8_t cvVal = ((progSlot[5] & 0x02)<<6) +  progSlot[7];
											uint32_t progData = (cvNr << 16) + cvVal;
											cvBuffer.push_back(progData);
											progModeActive = false;
//  Serial.printf("Prog Stat: %i CV: %i Val: %i Long: %u\n", progSlot[1], cvNr+1, cvVal, progData);
										}
										else
											readFullAddr = false;



									}
									if (handleProgrammerEvent)
										handleProgrammerEvent(newSlot[0]);
								}
								break;
							case 0x7F: //System Configuration
//								Serial.printf("Bushby Bit is %i \n", getBushbyStatus());
								break;
							default:
								if (slotNr < maxSlots) //regular slot
								{
//									Serial.printf("Msg Slot %i %i %i %i\n", slotNr, (*newSlot)[0],(*newSlot)[2],(*newSlot)[3]);
									if (focusNextAddr)
									{
										focusSlot = newData->lnData[2];
										focusNextAddr = false;
//										Serial.printf("Set focus Slot %i\n", focusSlot);
									}
//									else
//									Serial.println("no focusNextAddr");
								}
						} 
						break;
					}
					default:
						break;
				}
			}
			break;
	}
//	Serial.println("done");
}

/*
void IoTT_DigitraxBuffers::sendSwiReq(bool useAck, uint16_t swiAddr, uint8_t newPos)
{
	lnTransmitMsg thisBuffer;
	prepSwiReqMsg(&thisBuffer, useAck, swiAddr, newPos); //nnew pos b0 = pos b1 = pwr
	lnOutFct(thisBuffer);
}
*/

void IoTT_DigitraxBuffers::localPowerStatusChange(uint8_t newStatus)
{
	lnTransmitMsg thisBuffer;
	if (getOpSw(opSwDisablePwrUpdate, 1) != 0)
	{
//		Serial.println("localPowerStatusChange to LocoNet");
		sendPowerCommand(onVal, newStatus);
	}
	else
	{
		setPowerStatus(newStatus);
		uint8_t destTrack = 0;
		if (newStatus == 0x83)
			destTrack = getOpSw(opSwProgIsMain, 1) ? 3 : 0;
		if (trackGaugesLen > 0)
			setCurrReportMode((trackGauges[0].currUsePin & 0x10) >> 4, (trackGauges[1].currUsePin & 0x10) >> 4);
		setDCCPowerOutMsg(newStatus, destTrack);
	}
}

void IoTT_DigitraxBuffers::setPowerStatus(uint8_t newStatus)
{
//	Serial.println("setPowerStatus");
	switch (newStatus)
	{
		case 0x82: ////OPC_OFF
			trackByte &= ~0x03; //clear both bits
			break;
		case 0x83: ////OPC_ON
			trackByte |= 0x03; //set both bits
			break;
		case 0x85: ////OPC_IDLE
			trackByte &= ~0x02; //clear em break bits
			trackByte |= 0x01; //set power bits
			break;
	}
	if (handlePowerStatus) 
		handlePowerStatus(); //callback function to application
	if (isCommandStation)
	{
		if ((newStatus == 0x83) && (getOpSw(opSwDisableInterrogate, 1) == 0))
		{
			inpQuery = 7;
			queryDelay = millis() + queryInterval;
		}
		else
			inpQuery = 0xFF; //kill any query operation going on
	}
}

uint8_t IoTT_DigitraxBuffers::getOpSw(uint8_t opSwNr, uint8_t numBits)
{
	uint8_t byteNr = opSwNr >> 3;
	if (byteNr > 3)
		byteNr++; //byte for is track power
	uint8_t bitNr = opSwNr & 0x07;
	uint8_t bitMask = pow(2, numBits) - 1;
//	Serial.println("getOpSw");
//	dispSlot(&slotBuffer[0x7f][0]);
//	Serial.printf("%i %i %i %i\n", opSwNr, byteNr, bitNr, slotBuffer[0x7F][byteNr]);
	return ((slotBuffer[0x7F][byteNr] >> bitNr) & bitMask); //config slot
}

void IoTT_DigitraxBuffers::setOpSw(uint8_t opSwNr, uint8_t numBits, uint8_t newVal)
{
	uint8_t byteNr = opSwNr >> 3;
	if (byteNr > 3)
		byteNr++; //byte for is track power
	uint8_t bitNr = opSwNr & 0x07;
	uint8_t bitMask = pow(2, numBits) - 1;
}


void IoTT_DigitraxBuffers::setButtonValue(uint16_t buttonNum, uint8_t buttonValue)
{
	buttonValueBuffer[buttonNum] = buttonValue;
}

void IoTT_DigitraxBuffers::setBDStatus(uint16_t bdNum, bool bdStatus)
{
	uint16_t byteNr = bdNum>>3;  //	uint16_t byteNr = trunc(bdNum/8);
    uint8_t bitMask = 0x01<<(bdNum % 8);
    if (bdStatus)
		blockDetectorBuffer[byteNr] |= bitMask;
	else
        blockDetectorBuffer[byteNr] &= ~bitMask;
}

void IoTT_DigitraxBuffers::setProgStatus(bool progBusy)
{
	if (progBusy)
		trackByte |= 0x08; //set prog bit
	else
		trackByte &= ~0x08; //clear prog bit
}

void IoTT_DigitraxBuffers::requestNextSlotUpdate()
{
	for (uint8_t i = 1; i < maxSlots; i++)
		if (slotBuffer[i][1] == 0x80) //unused slot, send slot request
		{
			lnTransmitMsg thisBuffer;
			prepSlotRequestMsg(&thisBuffer, i);
			lnOutFct(thisBuffer);
			return;
		}
}

uint8_t IoTT_DigitraxBuffers::getBushbyStatus()
{
	slotData * ctrlSlot = getSlotData(0x7F);
	return (((*ctrlSlot)[3] & 0x04) >> 2);
}

slotData * IoTT_DigitraxBuffers::getSlotData(uint8_t slotNum)
{
	return &slotBuffer[slotNum];
}

uint8_t IoTT_DigitraxBuffers::getSlotStatus(uint8_t slotNr)
{
	if (slotNr < maxSlots)
		return slotBuffer[slotNr][0];
	else
		return 0;
}

void IoTT_DigitraxBuffers::updateSlotStatus(uint8_t slotNr, uint8_t newStatus)
{
	slotBuffer[slotNr][0] = newStatus;
}

uint8_t IoTT_DigitraxBuffers::getSlotOfAddr(uint8_t locoAddrLo, uint8_t locoAddrHi)
{
	uint8_t firstFree = 0xFF;
	for (uint8_t i = 1; i < maxSlots; i++)
	{
		if (firstFree == 0xFF)
			if ((slotBuffer[i][0] &0x30) == 0x00)
				firstFree = i;
		if ((slotBuffer[i][1] == locoAddrLo) && (slotBuffer[i][6] == locoAddrHi))
			return i;
	}
	if (firstFree != 0xFF)
	{
		slotBuffer[firstFree][1] = locoAddrLo;
		slotBuffer[firstFree][6] = locoAddrHi;
		return firstFree;
	}
	return 0xFF; //no slot available
}

String IoTT_DigitraxBuffers::getTurnoutInfo(uint16_t id, bool exFormat)
{
	char tempBuf[10];
	char outBuf[150];
	if (exFormat)
	{
		strcpy(outBuf, "jT");
		if (id == 0)
		{
			for (uint16_t i = 0; i < turnoutOutputLen; i++)
			{
				sprintf(tempBuf, " %i", turnoutOutputs[i].lnAddr);
				strcat(outBuf, tempBuf);
			}
		}
		else
		{
			for (uint16_t i = 0; i < turnoutOutputLen; i++)
				if (turnoutOutputs[i].lnAddr == id)
				{
					sprintf(outBuf, "jT %i T \"Turnout %i\"", turnoutOutputs[i].lnAddr, turnoutOutputs[i].ardID);
					break;
				}
		}
		return String(outBuf);
	}
	else
	{
		String retStr = "";
		if (turnoutOutputLen > 0)
		{
			retStr = "PTL";
			for (uint16_t i = 0; i < turnoutOutputLen; i++)
				retStr += "]\\[" + String(turnoutOutputs[i].lnAddr) + "}|{" + String(turnoutOutputs[i].Descr) + "}|{" + ((digitraxBuffer->getSwiPosition(turnoutOutputs[i].lnAddr) > 0) ? "4" : "2");
//				retStr += "]\\[" + String(turnoutOutputs[i].lnAddr) + "}|{" + ((turnoutOutputs[i].Descr != NULL) ? String(turnoutOutputs[i].Descr) : "IoTT") + "}|{" + ((digitraxBuffer->getSwiPosition(turnoutOutputs[i].lnAddr) > 0) ? "2" : "4");
		}
		return retStr;
	}
}

String IoTT_DigitraxBuffers::getRouteInfo(uint16_t id, bool exFormat)
{
	char tempBuf[10];
	char outBuf[150];
	if (exFormat)
	{
		if (id == 0)
		{
			strcpy(outBuf, "jA");
			for (uint16_t i = 0; i < routeOutputLen; i++)
			{
				sprintf(tempBuf, " %i", automationList[i].ID);
				strcat(outBuf, tempBuf);
			}
		}
		else
		{
			sprintf(outBuf, "jA %i X \"\"", id);
			for (uint16_t i = 0; i < routeOutputLen; i++)
				if (automationList[i].ID == id)
				{
					sprintf(outBuf, "jA %i %s %s", automationList[i].ID, automationList[i].automationType, automationList[i].Descr);
					break;
				}
		return String(outBuf);
		}
	}
	else
	{
		String retStr = "";
		if (routeOutputLen > 0)
		{
			retStr = "PRL" + String(routeOutputLen);
			for (uint16_t i = 0; i < routeOutputLen; i++)
			{
				sprintf(outBuf, "]\\[%i}|{%s}|{1", automationList[i].ID, automationList[i].Descr);
				retStr += String(outBuf);
			}
		}
		return retStr;
	}
}

String IoTT_DigitraxBuffers::getRosterInfo(uint16_t id, bool exFormat)
{
	char tempBuf[10];
	char outBuf[150];
	if (exFormat)
	{
		if (id == 0)
		{
			strcpy(outBuf, "jR");
			for (uint16_t i = 0; i < rosterList.size(); i++)
			{
				sprintf(tempBuf, " %i", rosterList[i].ID);
				strcat(outBuf, tempBuf);
			}
		}
		else
		{
//			sprintf(outBuf, "jR %i \"unknown\" \"loco\"", id);
			for (uint16_t i = 0; i < rosterList.size(); i++)
				if (rosterList[i].ID == id)
				{
					sprintf(outBuf, "jR %i %s %s", rosterList[i].ID, rosterList[i].Description, rosterList[i].FunctionMap);
					break;
				}
		}
		return String(outBuf);
	}
	else
	{
		String retStr = "RL" + String(rosterList.size());
		if (rosterList.size() > 0)
			for (uint16_t i = 0; i < rosterList.size(); i++)
			{
				sprintf(outBuf, "]\\[%s}|{%i}|{%c", rosterList[i].Description, rosterList[i].ID, rosterList[i].ID > 127 ? 'L' : 'S');
				retStr += String(outBuf);
			}
		return retStr;
	}
}

String IoTT_DigitraxBuffers::getSensorInfo(uint16_t id, bool exFormat)
{
	char outBuf[50];
	if (id == 0xFFFF)
	{
		return String(sensorInputLen);
	}
	else
	{
		sprintf(outBuf, "Q %i %i %i", sensorInputs[id].ardID, sensorInputs[id].ardPin, sensorInputs[id].posLogic);
		return String(outBuf);
	}
}

uint16_t IoTT_DigitraxBuffers::getAddrOfSlot(uint8_t slotNr)
{
	if ((slotNr > 0) && (slotNr <maxSlots))
		return (((slotBuffer[slotNr][6] & 0x7F) << 7) + (slotBuffer[slotNr][1] & 0x7F));
	else
		return 0;
}

uint8_t IoTT_DigitraxBuffers::getTopSlot(uint8_t masterSlot)
{
	if ((slotBuffer[masterSlot][0] & 0x40) == 0)
		return masterSlot;
	else
		return getTopSlot(slotBuffer[masterSlot][2]);
}

uint8_t IoTT_DigitraxBuffers::getFirstSlave(uint8_t masterSlot)
{
	for (uint8_t i = 1; i < maxSlots; i++)
		if (((slotBuffer[i][0] & 0x40) == 0x40) && (slotBuffer[i][2] == masterSlot))
			return i;
	return 0;
}

/*
void IoTT_DigitraxBuffers::updateTrackByte(bool setOp, uint8_t trackBits)
{
	if (setOp)
		trackByte |= trackBits;
	else
		trackByte &= (!trackBits);
}
*/
void IoTT_DigitraxBuffers::processSlotManager(lnReceiveBuffer * newData) //process incoming Loconet messages to the buffer
{
//	Serial.println("processSlotManager");
	bool procLNSuccess = false;
	lnTransmitMsg txBuffer;
	switch (newData->lnData[0])
	{
		case 0xB8: //OPC_UNLINK_SLOTS -> SL_RD
		{
			uint8_t slaveSlot = newData->lnData[1];
			uint8_t masterSlot = newData->lnData[2];
			uint8_t topSlot = getTopSlot(masterSlot);
			if (((slotBuffer[slaveSlot][0] &0x40) == 0x40) && (slotBuffer[slaveSlot][2] == masterSlot)) //can be unlinked from master
			{
				slotBuffer[slaveSlot][0] &= ~0x40; //clear uplink bit
				slotBuffer[slaveSlot][2] = slotBuffer[topSlot][2]; //set speed 
				if (getFirstSlave(masterSlot) == 0)
					slotBuffer[masterSlot][0] &= ~0x08; //clear downlink bit
				prepSlotReadMsg(&txBuffer, slaveSlot);
				procLNSuccess = true;
			}
			else
				prepLACKMsg(&txBuffer, 0x3A, 0x00); //can't be uplinked (already is)
			lnReplyFct(txBuffer);
		}
		break;
		case 0xB9: //OPC_LINK_SLOTS -> SL_RD
		{
			uint8_t slaveSlot = newData->lnData[1];
			uint8_t masterSlot = newData->lnData[2];
			if ((slotBuffer[slaveSlot][0] &0x40) == 0) //can be uplinked
			{
				slotBuffer[masterSlot][0] |= 0x08; //downlink bit
				slotBuffer[slaveSlot][0] |= 0x40; //uplink bit
				slotBuffer[slaveSlot][2] = masterSlot; //uplink slot #
				prepSlotReadMsg(&txBuffer, slaveSlot);
				procLNSuccess = true;
			}
			else
				prepLACKMsg(&txBuffer, 0x3A, 0x00); //can't be uplinked (already is)
			lnReplyFct(txBuffer);
			}
		break;
		case 0xBA: //OPC_MOVE_SLOTS -> SL_RD
		{
			uint8_t srcSlot = newData->lnData[1];
			uint8_t dstSlot = newData->lnData[2];
			if (srcSlot == 0) //Dispatch GET
			{
				if (dispatchSlot != 0)
				{
					slotBuffer[dispatchSlot][0] |= 0x30;
					prepSlotReadMsg(&txBuffer, dispatchSlot);
					dispatchSlot = 0;
					procLNSuccess = true;
				}
				else
					prepLACKMsg(&txBuffer, 0x3A, 0x00); //Dispatch buffer empty
			}
			else
				if (dstSlot == 0) //Dispatch PUT
				{
					if ((slotBuffer[srcSlot][0] & 0x40) == 0) //not a consist member
					{
						dispatchSlot = srcSlot;
						slotBuffer[srcSlot][0] &= ~0x10;
						slotBuffer[srcSlot][0] |= 0x20;
						prepSlotReadMsg(&txBuffer, srcSlot);
						procLNSuccess = true;
					}
					else
					{
						prepLACKMsg(&txBuffer, 0x3A, 0x00); //illegal, this is a consist
					}
				}
				else
					if (dstSlot == srcSlot) //NULL move
					{
						if ((slotBuffer[dstSlot][0] & 0x30) == 0x30)
							prepLACKMsg(&txBuffer, 0x3A, 0x00); //in use. Steal it?
						else
						{							
							slotBuffer[dstSlot][0] |= 0x30;
							prepSlotReadMsg(&txBuffer, srcSlot);
						}
						procLNSuccess = true;
					}
					else //regular slot move
					{
						if ((slotBuffer[dstSlot][0] & 0x30) == 0x30)
						{
							prepLACKMsg(&txBuffer, 0x3A, 0x00); //illegal move
						}
						else
						{
							memcpy(&slotBuffer[dstSlot][0], &slotBuffer[srcSlot][0], 10);
							slotBuffer[srcSlot][0] &= ~0x30; //set to FREE
							prepSlotReadMsg(&txBuffer, dstSlot);
							procLNSuccess = true;
						}
					}
			}
			lnReplyFct(txBuffer);
		break;
		case 0xBB: //OPC_RQ_SL_DATA -> SL_RD
		{
//			Serial.println("RQ_SL_DATA");
			uint8_t ofSlot = newData->lnData[1];
			slotBuffer[ofSlot][TRK] = 0; //slot is active, reset purge counter
			prepSlotReadMsg(&txBuffer, ofSlot);
			lnReplyFct(txBuffer);
			procLNSuccess = true;
		}
		break;
		case 0xBC: //OPC_SW_STATE request, this is answered by the central unit for all switches with LACK msg
		{
			uint16_t swiAddr = (newData->lnData[1] & 0x7F) +  ((newData->lnData[2] & 0x0F) << 7);
			uint8_t swiPos = getSwiPosition(swiAddr) >> 5;
//				Serial.printf("SwiAddr %i SwiPos %i\n", swiAddr, swiPos);
			prepLACKMsg(&txBuffer, newData->lnData[0] & 0x7F, swiPos == 0? 0x50 : 0x30);
			lnReplyFct(txBuffer);
			procLNSuccess = true;
		}
		break;
		case 0xBD: //OPC_SW_ACK -> LACK
		{
			prepLACKMsg(&txBuffer, newData->lnData[0] & 0x7F, 0x7F);
			lnReplyFct(txBuffer);
			procLNSuccess = true;
		}
		break;
		case 0xBF: //OPC_LOCO_ADR -> SL_RD or LACK
		{
			uint8_t newSlot = getSlotOfAddr(newData->lnData[2], newData->lnData[1]);
			if (newSlot != 0xFF)
			{
				prepSlotReadMsg(&txBuffer, newSlot);
			}
			else
				prepLACKMsg(&txBuffer, 0x3F, 0); //no free slot
			lnReplyFct(txBuffer);
			procLNSuccess = true;
		}
		break;
		case 0xED: //OPC_IMM_PACKET -> LACK
		{
			prepLACKMsg(&txBuffer, 0x7D, 0x7F); //no free slot
			lnReplyFct(txBuffer);
			procLNSuccess = true;
		}
		break;
		case 0xEF:
		{
			if (newData->lnData[1] == 0x0E) //OPC_WR_SL_DATA -> LACK
			{
				switch (newData->lnData[2])
				{
					case 0x7B:
					{
						memcpy(&slotBuffer[0x7B][0], &newData->lnData[3], 10);
						uint8_t sysHour = (digitraxBuffer->slotBuffer[0x7B][5] - 104) % 24;
						uint8_t sysMin = (digitraxBuffer->slotBuffer[0x7B][3] - 68) % 60;
						intFastClock = 3600 * sysHour + 60 * sysMin; //seconds per day, roll over
						fcLastBroadCast = millis();
//						sendFCCmdToWeb();
//						Serial.printf(" UpdateFC, Counter Reset, LACK %i:%i -> %i\n", sysHour, sysMin, intFastClock);
						prepLACKMsg(&txBuffer, 0x6F, 0x7F); //ok
					}
					break;
					case 0x7C: //Programmer 
					{
						uint8_t PCMD = newData->lnData[3];
						if (((PCMD & 0x20) == 0) || ((PCMD & 0x0C) == 0x0C) || ((PCMD & 0x10) > 0)) //Bit Mode or OpsMode with feedback or register mode
						{
//							Serial.println("not supported");
							prepLACKMsg(&txBuffer, 0x6F, 0x7F); //mode not supported
							break;
						}
						if (progModeActive && ((PCMD & 0x04) == 0))
						{
//							Serial.println("prog busy");
							prepLACKMsg(&txBuffer, 0x6F, 0x00); //programmer track busy
							break;
						}
						progModeActive = ((PCMD & 0x04) == 0) || ((PCMD & 0x0C) == 0x0C); //only wait for response if SM or Ops with feedback
						if (progModeActive)
						{
							memcpy(&progSlot[0], &newData->lnData[3], 10);
							progSent = millis();
						}
						if (PCMD & 0x04)
							prepLACKMsg(&txBuffer, 0x6F, 0x40); //task accepted blind, no E7 reply
						else
							prepLACKMsg(&txBuffer, 0x6F, 0x01); //task accepted, await E7 reply
//						Serial.println("task accepted");
						procLNSuccess = true;
					}
					break;
					default:
//						Serial.println("default slot write");
						memcpy(&slotBuffer[newData->lnData[2]][0], &newData->lnData[3], 10);
						slotBuffer[newData->lnData[2]][TRK] = 0; //slot is active, reset purge counter
						prepLACKMsg(&txBuffer, 0x6F, 0x7F); //no free slot
						break;
				}
				lnReplyFct(txBuffer);
//				Serial.println("WR_SL reply");
				
			}
		}
		break;
		default: 
			procLNSuccess = true;
		break;
		
	}
	if (procLNSuccess)
	{
		processBufferUpdates(newData);
		processDCCGenerator(newData);
	}
}

bool IoTT_DigitraxBuffers::processDCCGenerator(lnReceiveBuffer * newData)
{
//	function creates the DCC commands body sent to ser_injector
//	at this time, all slot data updated
//	This interface is using the LN data structure but with a different coding.
//	Serial.println("processDCCGenerator");
	lnTransmitMsg txBuffer;
	switch (newData->lnData[0])
	{
		case 0x82:; //OPC_OFF
		case 0x83:; //OPC_ON
		case 0x85: //OPC_IDLE
		{
			uint8_t destTrack = 0;
			if (newData->lnData[0] == 0x83)
				destTrack = getOpSw(opSwProgIsMain, 1) ? 3 : 0;
			if (trackGaugesLen > 0)
				setCurrReportMode((trackGauges[0].currUsePin & 0x10) >> 4, (trackGauges[1].currUsePin & 0x10) >> 4);
			setDCCPowerOutMsg(newData->lnData[0], destTrack);
//		Serial.println("2051");
			break;
		}
		case 0xA0: //OPC_LOCO_SPD  <1><REFRESH><ADDRHI><ADDRLO><SPEED><DIRF><SND>
//			generateSpeedCmd(&txBuffer, newData->lnData[1], newData->lnData[2]);
			break;
		case 0xA1:; //OPC_LOCO_DIRF
		case 0xA2:; //OPC_LOCO_SND
		case 0xB6: //OPC_CONSIST_FUNC 
//			generateFunctionCmd(&txBuffer, newData);
			break;
		break;
        case 0xB5: //OPC_SLOT_STAT1 
//			Serial.printf("SS1 %i %i\n", newData->lnData[1], newData->reqID);
//			if ((newData->reqID & 0x30) > 0) //change in refresh status
//				generateSpeedCmd(&txBuffer, newData->lnData[1], slotBuffer[newData->lnData[1]][2]);
        break;
		case 0xBD: //OPC_SW_REQ <2><ADDRHI><ADDRLO><POS>
			if (bushbyWatch) 
				if (getBushbyStatus() > 0) 
					break; //ignore if Bushby bit set and Watch active
		case 0xB0:  //OPC_SW_ACK
			processDCCSwitch(((newData->lnData[2] & 0x0F) << 7) + (newData->lnData[1] & 0x7F), (newData->lnData[2] & 0x20) >> 5, (newData->lnData[2] & 0x10) >> 4);
			break;
 		case 0xBA: //OPC_MOVE_SLOTS, handled in limited master or higher
 		{
//			Serial.printf("Move %i %i %2X %2X\n", newData->lnData[1], newData->lnData[2], slotBuffer[newData->lnData[1]][0], slotBuffer[newData->lnData[2]][0]);
//			if (((slotBuffer[newData->lnData[1]][0] & 0x48) == 0) || ((slotBuffer[newData->lnData[1]][0] & 0x48) == 0x40)) //free or consist top
//				generateSpeedCmd(&txBuffer, newData->lnData[1], slotBuffer[newData->lnData[1]][2]);
//			if (newData->lnData[1] != newData->lnData[2])
//				if (((slotBuffer[newData->lnData[2]][0] & 0x48) == 0) || ((slotBuffer[newData->lnData[2]][0] & 0x48) == 0x40)) //free or consist top
//					generateSpeedCmd(&txBuffer, newData->lnData[2], slotBuffer[newData->lnData[2]][2]);
			break;
		}
        case 0xEF:; //OPC_WR_SL
        case 0xE7: //OPC_SL_RD
		{
			switch (newData->lnData[1])
			{
				case 0x0E:
				{
					switch (newData->lnData[2])
					{
						case 0x7B: //FastClock
//							Serial.println("FC no action");
							break;
						case 0x7C: //Programming task, data is in progSlot at this time
							if (newData->lnData[0] == 0xEF) //OPC_WR_SL
							{
//								Serial.printf("Programming task %2X %2X\n", newData->lnData[0],newData->lnData[1]);
								setDCCProgrammingCmd(newData->lnData[3], ((newData->lnData[5]<<7) + newData->lnData[6]), ((newData->lnData[8] & 0x30)<<4) + ((newData->lnData[8] & 0x01)<< 7) + newData->lnData[9], ((newData->lnData[8] & 0x02)<< 6) + newData->lnData[10], newData->lnData[11], newData->lnData[12]);
//								progModeActive = true;
							}
							break;
						default: 
							if ((newData->lnData[2] > 0) && (newData->lnData[2] < maxSlots)) //Loco Slots
							{
//								Serial.printf("Ex %i %i\n", newData->lnData[2], newData->reqID);
//								if ((newData->reqID & 0x30) > 0) //change in refresh status
//									generateSpeedCmd(&txBuffer, newData->lnData[2], newData->lnData[5]);
							}
							break;
					}
				}
				break;
			}
			break;
		}
        case 0xED: 
        {
			switch (newData->lnData[1])
			{
				case 0x0B: 
					switch (newData->lnData[2])
					{
						case 0x7F: //OPC_IMM_PACKET
						{
							uint8_t numBytes = (newData->lnData[3] & 0x70) >> 4;
//							Serial.println(numBytes);
							txBuffer.lnData[0] = 4; //OpCode for direct DCC command <
							for (uint8_t i = 0; i < numBytes; i++)
								txBuffer.lnData[1+i] = newData->lnData[i+5] + ((newData->lnData[4] & (0x01 << i)) << (7 - i));
							txBuffer.lnMsgSize = numBytes + 1;
							if (dccPort)
								for (uint8_t i = 0; i < (newData->lnData[3] & 0x07); i++)
									dccPort->lnWriteMsg(txBuffer);
								//dccOutFct(txBuffer);
						}
						break;
					}
				break;
			}
		}
	}
	return true;
}

//void IoTT_DigitraxBuffers::processDCCGeneratorFeedback(lnTransmitMsg txData)
//{
	
//}

void IoTT_DigitraxBuffers::iterateMULinks(uint8_t thisSlot, uint8_t dirSpeedData)
{
//	Serial.printf("Clear purge counter slot %i\n", thisSlot);
	slotBuffer[thisSlot][TRK] = 0; //slot is active, reset purge counter
	if (dirSpeedData & 0x80)
	{
		slotBuffer[thisSlot][DIRF] ^= 0x20;
	}
	if (isCommandStation)
		setDCCSpeedCmd(thisSlot, &dirSpeedData);

	if (getConsistStatus(thisSlot) & cnDownlink) //has sub slots 
		for (uint8_t i = 1; i < maxSlots; i++)
			if ((getConsistStatus(i) & cnUplink) && (slotBuffer[i][SPD] == thisSlot) && (i != thisSlot))
				iterateMULinks(i, dirSpeedData);
}

void IoTT_DigitraxBuffers::setSlotDirfSpeed(lnReceiveBuffer * newData)
{
	//updates main slot and downlinked slots with current data. newData->lnData[1] must be the top slot of consist except for consist func  are generated
	//if sendDCC, DCC commands are sent to cmmand station
	slotData * thisSlot = getSlotData(newData->lnData[1]);
	if (isCommandStation)
		setRefreshStatus(newData->lnData[1], slotActive);
	if (thisSlot) 
	{
		if (focusNextAddr) //for Train side Sensor to select current slot
		{
			if (thisSlot)
			{
				focusSlot = newData->lnData[1];
				focusNextAddr = false;
//				Serial.printf("Set focus Slot %i\n", focusSlot);
			}
//			else
//				Serial.printf("Slot %i not initialized\n", focusSlot);
		}
		switch (newData->lnData[0])
		{
			case 0xA0://OPC_LOCO_SPD
				(*thisSlot)[SPD] = newData->lnData[2]; 
//				if (isCommandStation)
				iterateMULinks(newData->lnData[1], newData->lnData[2]);
				break;
			case 0xA1: //OPC_LOCO_DIRF
			{
				newData->requestID = ((*thisSlot)[DIRF] ^ newData->lnData[2]) & 0x003F; //mark the bits that change for updating DCC
				(*thisSlot)[DIRF] = (newData->lnData[2] & 0x3F); //update with new data
				if (isCommandStation)
				{
					if ((newData->requestID & 0x20) > 0) //Direction Change
					{
						(*thisSlot)[DIRF] ^= 0x20; //turn back, so it can be turned again
						iterateMULinks(newData->lnData[1], (*thisSlot)[SPD] | 0x80);
					}
					if ((newData->requestID & 0x1F) > 0) //Function Change
						if (isCommandStation)
							setDCCFuncCmd(newData->lnData[1], &newData->lnData[0]); //light and func 1-4
				}	
//			    Serial.printf("New Slot DIRF %2X\n", (*thisSlot)[3]);
				break; 
			}
			case 0xA2: //OPC_LOCO_SND
			{
				newData->requestID = ((*thisSlot)[7] ^ newData->lnData[2]) & 0x000F;
			    (*thisSlot)[7] = newData->lnData[2] & 0x0F;
				if (isCommandStation)
					if ((newData->requestID & 0x0F) > 0) //Function Change
						setDCCFuncCmd(newData->lnData[1], &newData->lnData[0]); //light and func 1-4
				break; 
			}
			case 0xA3: //OPC_LOCO_F912 
			{
				break;
			}
			case 0xB5: //OPC_SLOT_STAT1 
//				Serial.println("slot stat1");
				newData->requestID = ((*thisSlot)[0] ^ newData->lnData[2]) & 0x007F;
			    (*thisSlot)[0] = newData->lnData[2];
				break;
			case 0xB6: //OPC_CONSIST_FUNC 
				if ((*thisSlot)[0] & 0x40) //UPLINKED
				{
					newData->requestID = ((*thisSlot)[3] ^ newData->lnData[2]) & 0x000F;
					(*thisSlot)[3] = newData->lnData[2];
					if (isCommandStation)
						if ((newData->requestID & 0x0F) > 0) //Function Change
							setDCCFuncCmd(newData->lnData[1], &newData->lnData[0]); //light and func 1-4
				}
				break;
		}
	}
}

uint8_t IoTT_DigitraxBuffers::getConsistStatus(uint8_t ofSlot)
{
	return (((slotBuffer[ofSlot][STAT] & 0x40) >> 6) + ((slotBuffer[ofSlot][STAT] & 0x08) >> 2));
}

void IoTT_DigitraxBuffers::setConsistStatus(uint8_t forSlot, uint8_t newStatus)
{
	slotBuffer[forSlot][STAT] &= ~0x48; //clear bits
	slotBuffer[forSlot][STAT] |= ((newStatus & 0x01) << 6);
	slotBuffer[forSlot][STAT] |= ((newStatus & 0x02) << 2);
}

uint8_t IoTT_DigitraxBuffers::getRefreshStatus(uint8_t ofSlot)
{
	return ((slotBuffer[ofSlot][STAT] & 0x30) >> 4);
}

void IoTT_DigitraxBuffers::setRefreshStatus(uint8_t forSlot, uint8_t newStatus)
{
	slotBuffer[forSlot][STAT] &= ~0x30; //clear bits
	slotBuffer[forSlot][STAT] |= ((newStatus & 0x03) << 4);
	if (getConsistStatus(forSlot) & cnDownlink) //has sub slots that need update
		for (uint8_t i = 1; i < maxSlots; i++)
			if ((getConsistStatus(i) & cnUplink) && (slotBuffer[i][SPD] == forSlot) && (i != forSlot))
				setRefreshStatus(i, newStatus);
}

uint8_t IoTT_DigitraxBuffers::getConsistTopSlot(uint8_t ofSlot)
{
	switch (getConsistStatus(ofSlot))
	{
		case cnFree: return 0; //not a consist
		case cnUplink : ;
		case cnMiddle : return getConsistTopSlot(slotBuffer[ofSlot][SPD]); //return iterative top
		case cnDownlink: return ofSlot; //return self
	}
	return 0; 
}


void IoTT_DigitraxBuffers::purgeUnusedSlots()
{
	for (uint8_t i = 1; i < maxSlots; i++)
	{
		if ((getConsistStatus(i) & cnUplink) == 0) //not a consist or consist top
			if (getRefreshStatus(i) & slotCommon) //loco is refreshed, (common or active)
				if ((slotBuffer[i][TRK]) > purgeLimit) //limit is reached, so free slot
				{
//					Serial.printf("Purge slot %i\n", i);
					uint8_t newSpeed = getOpSw(opSwPurgeForce0, 1) ? 0 : slotBuffer[i][SPD];
					if (newSpeed == 0)
						setDCCSpeedCmd(i, &newSpeed);
					setRefreshStatus(i, slotIdle);
					setDCCSpeedCmd(i, &newSpeed);
				}
				else
				{
//					Serial.printf("Incr Purge bit slot %i to %i of %i \n", i, slotBuffer[i][TRK] + 1, purgeLimit);
//					dispSlot(&slotBuffer[i][0]);
					slotBuffer[i][TRK]++; //increment purge counter. If limit is reached next time, slot will be freed up
			}
	}
}

uint32_t millisElapsed(uint32_t since)
{
	uint32_t redMask = 0x0001FFFF;
	uint32_t now = millis() & redMask;
	if (now > (since & redMask))
		return (now - (since & redMask));
	else
		return ((redMask - (since & redMask)) + now);
}

uint32_t microsElapsed(uint32_t since)
{
	uint32_t redMask = 0x00FFFFFF;
	uint32_t now = micros() & redMask;
	if (now > (since & redMask))
		return (now - (since & redMask));
	else
		return ((redMask - (since & redMask)) + now);
}

