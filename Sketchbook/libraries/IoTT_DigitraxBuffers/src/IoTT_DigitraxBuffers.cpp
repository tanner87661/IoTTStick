#include <IoTT_DigitraxBuffers.h>


blockDetBuffer blockDetectorBuffer;
switchBuffer switchPositionBuffer;
signalBuffer signalAspectBuffer;
analogValBuffer analogValueBuffer;
buttonValBuffer buttonValueBuffer;
powerStatusBuffer sysPowerStatus = 2; //OPC_IDLE
slotDataBuffer slotBuffer;
uint8_t dispatchSlot = 0x00;

slotData stdSlot = {0x03, 0x80, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00}; //Byte 1 0x80 for unused, newly initialized slot

IoTT_SerInjector* dccPort = NULL;
txFct lnReplyFct = NULL;
txFct lnOutFct = NULL;
//txFct dccOutFct = NULL;

uint16_t dccGeneratorCallback(lnTransmitMsg txData)
{
	digitraxBuffer->receiveDCCGeneratorFeedback(txData);
}

//LocoNet functions for preparing reply messages
void prepLACKMsg(lnTransmitMsg * msgData, uint8_t ackCode, uint8_t ackData)
{
	msgData->lnData[0] = 0xB4; //OPC_LONG_ACK 
	msgData->lnData[1] = ackCode;
	msgData->lnData[2] = ackData;
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
	msgData->lnData[0] = 0xE7; 
	msgData->lnData[1] = 0x0E; //OPC_SL_RD_DATA
	msgData->lnData[2] = slotNr;
	memcpy(&msgData->lnData[3], slotBuffer[slotNr], 10);
	msgData->lnData[3] &= 0x7F; //make sure purge bit 7 is 0
	msgData->lnData[4] &= 0x7F; //make sure slot init bit 7 is 0
	msgData->lnMsgSize = 14;
	setXORByte(&msgData->lnData[0]);
}

void prepSlotWriteMsg(lnTransmitMsg * msgData, uint8_t slotNr)
{
	msgData->lnData[0] = 0xEF; 
	msgData->lnData[1] = 0x0E; //OPC_SL_WR_DATA
	msgData->lnData[2] = slotNr;
	memcpy(&msgData->lnData[3], slotBuffer[slotNr], 10);
	msgData->lnData[3] &= 0x7F; //make sure purge bit 7 is 0
	msgData->lnData[4] &= 0x7F; //make sure slot init bit 7 is 0
	msgData->lnMsgSize = 14;
	setXORByte(&msgData->lnData[0]);
}

//DCC functions for command station mode DCC cmd generation

void setDCCSpeedCmd(uint8_t slotNr, uint8_t * unused)
{
	lnTransmitMsg txBuffer;
	txBuffer.lnData[0] = 1; //OpCode for speed commands
	txBuffer.lnData[1] = (slotBuffer[slotNr][0] & 0x20) >> 5; //add to refresh buffer
	txBuffer.lnData[2] = slotBuffer[slotNr][6];
	txBuffer.lnData[3] = slotBuffer[slotNr][1];
	txBuffer.lnData[4] = slotBuffer[slotNr][2]; //speedVal;
	txBuffer.lnData[5] = slotBuffer[slotNr][3]; 
	txBuffer.lnData[6] = slotBuffer[slotNr][7];
	txBuffer.lnMsgSize = 7;
	dccPort->lnWriteMsg(txBuffer);
}

void setDCCFuncCmd(uint8_t slotNr, uint8_t * funcData) // funcNr, funcStatus
{
	lnTransmitMsg txBuffer;
	txBuffer.lnData[0] = 2; //OpCode for function commands
	txBuffer.lnData[1] = slotBuffer[slotNr][6];
	txBuffer.lnData[2] = slotBuffer[slotNr][1];
	txBuffer.lnData[3] = funcData[0]; //function number
	txBuffer.lnData[4] = funcData[1];
	txBuffer.lnMsgSize = 5;
	dccPort->lnWriteMsg(txBuffer);
}

void setDCCProgrammingCmd(uint8_t progMode, uint8_t OpsAdrHi, uint8_t OpsAdrLo, uint8_t CVNrHi, uint8_t CVNrLo, uint8_t CVVal)
{
	lnTransmitMsg txBuffer;
	txBuffer.lnData[0] = 5; //OpCode for programming commands
	txBuffer.lnData[1] = progMode;
	txBuffer.lnData[2] = OpsAdrHi;
	txBuffer.lnData[3] = OpsAdrLo;
	txBuffer.lnData[4] = CVNrHi;
	txBuffer.lnData[5] = CVNrLo;
	txBuffer.lnData[6] = CVVal; //new val or compare val
	txBuffer.lnMsgSize = 7;
	dccPort->lnWriteMsg(txBuffer);
}

void setDCCDirCmd(uint8_t slotNr, uint8_t * unused)
{
	slotBuffer[slotNr][3] ^= 0x20;
}

void setDCCConfigCmd(uint16_t cfgVar, uint16_t cfgVarVal)
{
	lnTransmitMsg txBuffer;
	txBuffer.lnData[0] = 99; //OpCode for configuration commands
	txBuffer.lnData[1] = (cfgVar >> 7) & 0x7f;
	txBuffer.lnData[2] = cfgVar & 0x7F;
	txBuffer.lnData[3] = (cfgVarVal >> 7) & 0x7f;
	txBuffer.lnData[4] = cfgVarVal & 0x7F;
	txBuffer.lnMsgSize = 5;
	dccPort->lnWriteMsg(txBuffer);
}

/*
 * RedHat setup Codes starting at 1025
 * 1025 Device Mode: 0: Off 1: Cmd Stn 2: Booster with LocoNet 3: Booster no LocoNet 65535: Reboot 328P
 * 1026 Output Enable: Bit 0: IBT-2 Bit 1: ProgTrack Bit 2: RailSync   Value 0: disabled, 1: enabled 
 * 1027 Set current IBT-2 VarVal has current in mAmps
 * 1028 Set ack current Prog Track
 * 1029 Set max current RailSync
 * 1030 LED Brightness analog value 0-100 in percent ofull brightness
*/

IoTT_DigitraxBuffers::IoTT_DigitraxBuffers(txFct lnOut)
{
	lnOutFct = lnOut;
}

IoTT_DigitraxBuffers::~IoTT_DigitraxBuffers()
{
}

void IoTT_DigitraxBuffers::loadRHCfgJSON(DynamicJsonDocument doc)
{
	if (doc.containsKey("RxD"))
		rxPin = doc["RxD"];
	if (doc.containsKey("TxD"))
		txPin = doc["TxD"];
	if (doc.containsKey("DevSettings"))
	{
		isRedHat = (uint8_t)doc["DevSettings"]["HWMode"] == 1;
	}
	if (doc.containsKey("CmdStationSettings"))
	{
		JsonObject thisObj = doc["CmdStationSettings"];
		if (thisObj.containsKey("LEDLevel"))
			ledLevel = thisObj["LEDLevel"];
		
	}
	if (doc.containsKey("BoosterSettings"))
	{
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

void IoTT_DigitraxBuffers::clearSlotBuffer()
{
	if (!isCommandStation)
		for (int i = 0; i < numSlots; i++)
			memcpy(&slotBuffer[i], &stdSlot[0], 10);
}

void IoTT_DigitraxBuffers::setRedHatMode(txFct lnReply, DynamicJsonDocument doc)
{
	lnReplyFct = lnReply;
	loadRHCfgJSON(doc);
	isCommandStation = lnReply != NULL;
	if (dccPort == NULL)
	{
		dccPort = new IoTT_SerInjector(rxPin, txPin, false, 1);
        dccPort->loadLNCfgJSON(doc); //loads baudrate and invert information
        dccPort->setProtType(DCCEx); //DCC++Ex DCC Generator
        dccPort->setTxCallback(dccGeneratorCallback);
	}
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
	//send RedHat configuration commands
	lnTransmitMsg txBuffer;
	if (isRedHat) //send RH HW Config, not available for Arduino HW
	{
		Serial.println("RedHat Mode");  
		if (isCommandStation)
		{
			setDCCConfigCmd(1025, 1);
		}
		else
		{
			setDCCConfigCmd(1025, 2);
		}
		setDCCConfigCmd(1027, 2500);
		setDCCConfigCmd(1030, ledLevel);
	}
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
		dataFile.write(sysPowerStatus);
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
			dataFile.read(&sysPowerStatus, 1);
		minSize += (numSlots * 10);
		if (fileSize >= minSize)
//		if ((fileSize >= minSize) && isCommandStation) 
		{
			Serial.println("Load Slot buffer from file");
			for (int i = 0; i < numSlots; i++)
			{
				dataFile.read(slotBuffer[i], 10);
				slotBuffer[i][0] &= 0x7F; //clear purge bit
			}
			setPowerStatus(0x83); //track power on after startup
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
		for (int i = 1; i < maxSlots; i++)
			slotBuffer[i][4] = 0x04 + sysPowerStatus;
		Serial.println("Digitrax Buffer Data File loaded");
	}
    else
		Serial.println("Unable to read Digitrax Buffer Data File");
}

void IoTT_DigitraxBuffers::processLoop()
{
	if (dccPort)
	{
//		Serial.print(dccPort->getMsgType());
		dccPort->processLoop();
//		Serial.println();
	}

	if (rhButtons)
		rhButtons->processButtons(); 
		
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
		if (millis() - purgeSlotTimer > purgeInterval)
		{
//			purgeUnusedSlots();
			purgeSlotTimer += purgeInterval;
		}

		if (progMode)
			if ((millis() - progSent) > (uint32_t)progTimeout)
			{
//				Serial.printf("Prog timeout");
				lnTransmitMsg txBuffer;
				memcpy(&slotBuffer[0x7C][0], &progSlot[0], 10);;
				slotBuffer[0x7C][0] = 0x04;
				prepSlotReadMsg(&txBuffer, 0x7C);
				lnOutFct(txBuffer);
				progMode = false;
			}

	}
	
	if (millis() - fcRefresh > fcRefreshInterval)
	{
		//refresh fast clock slot
		fcRefresh += fcRefreshInterval;
	}

	if (millis() > nextBufferUpdate)
	{
		nextBufferUpdate += bufferUpdateInterval;
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

void IoTT_DigitraxBuffers::writeProg(uint16_t dccAddr, uint8_t progMode, uint16_t cvNr, uint8_t cvVal)
{
	lnTransmitMsg txBuffer;
	slotData * prSlot = getSlotData(0x7C);
	switch (progMode)
	{
		case 0: (*prSlot)[0] = 0x60; //Prog track, byte mode
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
}

void IoTT_DigitraxBuffers::readProg(uint16_t dccAddr, uint8_t progMode, uint16_t cvNr)
{
	lnTransmitMsg txBuffer;
	slotData * prSlot = getSlotData(0x7C);
	switch (progMode)
	{
		case 0: (*prSlot)[0] = 0x20; //Prog track, byte mode
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
}


//status buffer update and access functions 

uint8_t IoTT_DigitraxBuffers::getPowerStatus()
{
	return sysPowerStatus;
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

uint16_t IoTT_DigitraxBuffers::receiveDCCGeneratorFeedback(lnTransmitMsg txData)
{
//	Serial.printf("receiveDCCGeneratorFeedback %i\n", txData.lnData[0]);
	switch (txData.lnData[0])
	{
		case 5: //prog Answer
		{
			lnTransmitMsg txBuffer;
			memcpy(&slotBuffer[0x7C][0], &progSlot[0], 10);;
			int16_t retVal = (txData.lnData[7] << 8) + txData.lnData[8];
			if (retVal == -1)
				slotBuffer[0x7C][0] = 0x04;
			else
			{
				slotBuffer[0x7C][0] = 0x00;
				slotBuffer[0x7C][5] = (retVal & 0x38) >> 7; //high 3 of 10 bits
				slotBuffer[0x7C][6] = retVal & 0x7F; //low 7 of 10 bits
			}
			prepSlotReadMsg(&txBuffer, 0x7C);
			lnOutFct(txBuffer);
			progMode = false;
		}
		break;
		case 11: //ssnsor input
//			Serial.println("sensor input");
			rhButtons->processDigitalInputBuffer(txData.lnData[1], txData.lnData[2]);
		break;
	}
}

void IoTT_DigitraxBuffers::enableBushbyWatch(bool enableBushby)
{
	bushbyWatch = enableBushby;
}

void IoTT_DigitraxBuffers::awaitFocusSlot(int16_t dccAddr, bool simulOnly)
{
	focusNextAddr = true;
	focusSlot = -1;
	if (dccAddr > 0)
	{
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
//	Serial.println("processBufferUpdates");
	switch (newData->lnData[0])
	{
		case 0x82:; //OPC_OFF
		case 0x83:; //OPC_ON
		case 0x85: //OPC_IDLE
		{
			setPowerStatus(newData->lnData[0]); 
			if (handlePowerStatus) 
				handlePowerStatus(); //callback function to application
		}
		break;
		case 0xA0:; //OPC_LOCO_SPD
		case 0xA1:; //OPC_LOCO_DIRF
		case 0xA2: //OPC_LOCO_SND
		{
			if (slotBuffer[newData->lnData[1]][1] == 0x80) 
			{
				if (!isCommandStation) //unused slot, send slot request to command station
				{
					lnTransmitMsg thisBuffer;
					prepSlotRequestMsg(&thisBuffer, newData->lnData[1]);
					lnOutFct(thisBuffer);
				}
				else
				{
					//ignore
				}
			}
			else
				setSlotDirfSpeed(newData, isCommandStation); //update slot, send DCC commands, if command station
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
			if (bushbyWatch) 
				if (getBushbyStatus() > 0) 
					break; //ignore if Bushby bit set and Watch active
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
//			if (!isCommandStation)
				setSlotDirfSpeed(newData, isCommandStation); //update slot, sen DCC command later, if Cmd Stn
			break;
		}
		case 0xB6: //OPC_CONSIST_FUNC 
		{
//			if (!isCommandStation)
				setSlotDirfSpeed(newData, isCommandStation); //update slot, sen DCC command later, if Cmd Stn
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
					uint16_t zoneAddr = (((newData->lnData[1] & 0x1F) << 7) + (newData->lnData[2] & 0x7F))>>1;
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
						newData->reqID = ((*newSlot)[0] ^ newData->lnData[3]) & 0x007F; //identify changes in slot status
//						Serial.printf("RDWR %i %i %i\n", (*newSlot)[0], newData->lnData[3], newData->reqID);
						memcpy(newSlot[0], &newData->lnData[3], 10);
						switch (slotNr)
						{
							case 0x7B: //Fast Clock
								break;
							case 0x7C: //Programmer
								if (newData->lnData[0] == 0xE7) //final programmer reply
									if (handleProgrammerEvent)
										handleProgrammerEvent(newSlot[0]);
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
}

void IoTT_DigitraxBuffers::setPowerStatus(uint8_t newStatus)
{
	switch (newStatus)
	{
		case 0x82: ////OPC_OFF
			sysPowerStatus = 0;
			updateTrackByte(false, 0x03);
			break;
		case 0x83: ////OPC_ON
			sysPowerStatus = 1;
			updateTrackByte(true, 0x07);
			break;
		case 0x85: ////OPC_IDLE
			sysPowerStatus = 2;
			updateTrackByte(false, 0x02);
			updateTrackByte(true, 0x05);
			break;
	}
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
		updateTrackByte(true, 0x0C);
	else
		updateTrackByte(false, 0x08);
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

void IoTT_DigitraxBuffers::updateTrackByte(bool setOp, uint8_t trackBits)
{
	for (uint8_t i = 1; i < maxSlots; i++)
	{
		if (setOp)
			slotBuffer[i][4] |= trackBits;
		else
			slotBuffer[i][4] &= (!trackBits);
	}
	if (setOp)
		slotBuffer[0x7C][4] |= trackBits;
	else
		slotBuffer[0x7C][4] &= (!trackBits);
}

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
			uint8_t ofSlot = newData->lnData[1];
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
				prepSlotReadMsg(&txBuffer, newSlot);
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
					case 0x7C: //Programmer 
					{
						uint8_t PCMD = newData->lnData[3];
						if (((PCMD & 0x20) == 0) || ((PCMD & 0x0C) == 0x0C) || ((PCMD & 0x10) > 0)) //Bit Mode or OpsMode with feedback or register mode
						{
//							Serial.println("not supported");
							prepLACKMsg(&txBuffer, 0x6F, 0x7F); //mode not supported
							break;
						}
						if (progMode && ((PCMD & 0x04) > 0))
						{
//							Serial.println("prog busy");
							prepLACKMsg(&txBuffer, 0x6F, 0x00); //programmer busy
							break;
						}
						progMode = ((PCMD & 0x04) == 0) || ((PCMD & 0x0C) == 0x0C); //only wait for response if SM or Ops with feedback
						if (progMode)
						{
							memcpy(&progSlot[0], &newData->lnData[3], 10);
							progSent = millis();
						}
						if (PCMD & 0x04)
							prepLACKMsg(&txBuffer, 0x6F, 0x40); //task accepted blind
						else
							prepLACKMsg(&txBuffer, 0x6F, 0x01); //task accepted
//						Serial.println("task accepted");
						procLNSuccess = true;
					}
					break;
					default:
//						Serial.println("default");
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
			txBuffer.lnData[0] = 0; //OpCode for power status commands <0><STAT>
			txBuffer.lnData[1] = sysPowerStatus; 
			txBuffer.lnMsgSize = 2;
			dccPort->lnWriteMsg(txBuffer);
//			dccOutFct(txBuffer);
		}
		break;
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
		{
			txBuffer.lnData[0] = 3; //OpCode for power status commands <0><STAT>
			txBuffer.lnData[1] = newData->lnData[2] & 0x0F; 
			txBuffer.lnData[2] = newData->lnData[1] & 0x7F; 
			txBuffer.lnData[3] = (newData->lnData[2] & 0x20) >> 5; 
			txBuffer.lnMsgSize = 4;
			dccPort->lnWriteMsg(txBuffer);
//			dccOutFct(txBuffer);
		}
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
							break;
						case 0x7C: //Programming task
							if (newData->lnData[0] == 0xEF)
							{
//								Serial.println("Programming task");
								setDCCProgrammingCmd(newData->lnData[3], newData->lnData[5], newData->lnData[6], newData->lnData[8], newData->lnData[9], newData->lnData[10]);
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

void IoTT_DigitraxBuffers::iterateMULinks(uint8_t thisSlot, uint8_t * templData, dccFct updateFunc, dccFct procFunc)
{
	for (uint8_t i = 1; i < maxSlots; i++)
		if ((slotBuffer[i][0] & 0x40) && (slotBuffer[i][2] == thisSlot) && (i != thisSlot))
		{
//			Serial.printf("Clear purge bit slot %i\n", i);
			slotBuffer[i][0] &= 0x7F; //slot is active, clear purge bit
			if (procFunc)
				procFunc(i, templData);
			iterateMULinks(i, templData, updateFunc, procFunc);
		}
}

void IoTT_DigitraxBuffers::setSlotDirfSpeed(lnReceiveBuffer * newData, bool sendDCC)
{
	//updates main slot and downlinked slots with current data. newData->lnData[1] must be the top slot of consist except for consist func  are generated
	//if sendDCC, DCC commands
	slotData * thisSlot = getSlotData(newData->lnData[1]);
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
			else
				Serial.printf("Slot %i not initialized\n", focusSlot);
		}
		switch (newData->lnData[0])
		{
			case 0xA0://OPC_LOCO_SPD
			    (*thisSlot)[2] = newData->lnData[2]; //set speed in top slot
			    if (sendDCC) 
				break; 
			case 0xA1: //OPC_LOCO_DIRF
			{
				newData->reqID = ((*thisSlot)[3] ^ newData->lnData[2]) & 0x003F; //mark the bits that change for updating DCC
				if ((newData->reqID & 0x20) > 0) //Direction Change
					iterateMULinks(newData->lnData[1], NULL, setDCCDirCmd, setDCCSpeedCmd);
			    (*thisSlot)[3] = newData->lnData[2];
//			    Serial.printf("New Slot DIRF %2X\n", (*thisSlot)[3]);
				break; 
			}
			case 0xA2: //OPC_LOCO_SND
			{
				newData->reqID = ((*thisSlot)[7] ^ newData->lnData[2]) & 0x000F;
			    (*thisSlot)[7] = newData->lnData[2];
				break; 
			}
			case 0xB5: //OPC_SLOT_STAT1 
				newData->reqID = ((*thisSlot)[0] ^ newData->lnData[2]) & 0x007F;
			    (*thisSlot)[0] = newData->lnData[2];
				break;
			case 0xB6: //OPC_CONSIST_FUNC 
				if ((*thisSlot)[0] & 0x40) //UPLINKED
				{
					newData->reqID = ((*thisSlot)[3] ^ newData->lnData[2]) & 0x001F;
					(*thisSlot)[3] = newData->lnData[2];
				}
				break;
		}
	}
}

/*
void IoTT_DigitraxBuffers::generateSpeedCmd(uint8_t thisSlot, uint8_t topSpeed)
{
//	Serial.printf("Gen Speed %i %i\n", thisSlot, topSpeed);
	Serial.printf("Clear purge bit slot %i\n", thisSlot);
	slotBuffer[thisSlot][0] &= 0x7F; //slot is active, clear purge bit
	dccFct thisFct = setDCCSpeedCmd;
	if ((slotBuffer[thisSlot][0] & 0x08) > 0) //has others linked into
		iterateMULinks(thisSlot, topSpeed, 0, setDCCSpeedCmd);
	setDCCSpeedCmd(thisSlot, topSpeed, 0);
}

void IoTT_DigitraxBuffers::generateFunctionCmd(lnReceiveBuffer * newData)
{
	#define opCode newData->lnData[0]
	uint8_t thisSlot = newData->lnData[1];

	Serial.printf("Clear purge bit slot %i\n", thisSlot);
	slotBuffer[thisSlot][0] &= 0x7F; //slot is active, clear purge bit

	uint8_t bitFlags = opCode == 0xA2 ? slotBuffer[thisSlot][7] : slotBuffer[thisSlot][3]; //0xA1, 0xB6
	uint8_t addrOfset = opCode == 0xA2 ? 5 : 1; //0xA1, 0xB6
	uint8_t bitMask = 0x01;
	for (uint8_t i = 0; i < 4; i++)
	{
		if ((newData->reqID & bitMask) > 0)
			//send function bit command
			setDCCFuncCmd(thisSlot, addrOfset + i, (bitFlags & bitMask) >> i);
		bitMask <<= 1;
	}
	if ((opCode == 0xA1) || (opCode == 0xB6))  //OPC_DIRF, OPC_CONSIST_FUNC
	{
		if ((newData->reqID & 0x10) > 0)
			//send light bit command
			setDCCFuncCmd(txBuffer, thisSlot, 0, (bitFlags & 0x10) >> 4);
	if (opCode == 0xA1)  //OPC_DIRF
		if ((newData->reqID & 0x20) > 0)
		{
//			Serial.println("chdir");
			//send DIR bit command, include links
			uint8_t topSpeed = slotBuffer[thisSlot][2];
			iterateMULinks(thisSlot, slotBuffer[thisSlot][2], 0, setDCCSpeedCmd);
			setDCCSpeedCmd(thisSlot, slotBuffer[thisSlot][2], 0);
		}
	}
}
*/

void IoTT_DigitraxBuffers::purgeUnusedSlots()
{
	lnTransmitMsg txBuffer;
	for (uint8_t i = 1; i < maxSlots; i++)
	{
		if ((slotBuffer[i][0] & 0x80) > 0) //bit not cleared, so free slot
		{
			slotBuffer[i][0] &= (~0x90);
//			Serial.printf("Purge slot %i\n", i);
//			setDCCSpeedCmd(&txBuffer, i, 0);
		}
		else
			if ((slotBuffer[i][0] & 0x30) == 0x30)
			{
//				Serial.printf("Set Purge bit slot %i\n", i);
				slotBuffer[i][0] |= 0x80; //set purge bit. If not cleared next time, slot will be freed up
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

/*
blockDetBuffer * getBDList()
{
	return &blockDetectorBuffer;
}

switchBuffer * getSwitchList()
{
	return &switchPositionBuffer;
}

signalBuffer * getSignalList()
{
	return &signalAspectBuffer;
}

analogValBuffer * getAnalogValBuffer()
{
	return &analogValueBuffer;
}

buttonValBuffer * getButtonValBuffer()
{
	return &buttonValueBuffer;
}

void setButtonValue(uint16_t buttonNum, uint8_t buttonValue)
{
	buttonValueBuffer[buttonNum] = buttonValue;
}

slotDataBuffer * getSlotDataBuffer()
{
	return &slotBuffer;
}

void setSlotData(uint8_t slotNum, slotData thisSlot)
{
	memcpy(slotBuffer[slotNum], thisSlot, 10);
}

*/
