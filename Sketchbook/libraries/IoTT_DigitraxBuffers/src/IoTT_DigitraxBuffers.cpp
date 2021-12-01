#include <IoTT_DigitraxBuffers.h>

blockDetBuffer blockDetectorBuffer;
switchBuffer switchPositionBuffer;
signalBuffer signalAspectBuffer;
analogValBuffer analogValueBuffer;
buttonValBuffer buttonValueBuffer;
powerStatusBuffer sysPowerStatus = 2; //OPC_IDLE
slotDataBuffer slotBuffer;
uint8_t dispatchSlot = 0x00;

txFct replyMessage = NULL;
txFct dccCmdMessage = NULL;

uint32_t nextBufferUpdate = millis();

slotData stdSlot = {0x03, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00};

typedef struct
{
	uint32_t lastActivity;
	uint16_t devAddr;
}protocolEntry;

const uint8_t switchProtLen = 20;
protocolEntry switchProtocol[switchProtLen];
uint8_t swWrPtr = 0;
uint16_t lastSwiAddr = 0xFFFF;
bool bushbyWatch = false;

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

void setReplyFunction(txFct newFct)
{
	replyMessage = newFct;
}

void setDccCmdFunction(txFct newFct)
{
	dccCmdMessage = newFct;
}

blockDetBuffer * getBDList()
{
	return &blockDetectorBuffer;
}

void setBDStatus(uint16_t bdNum, bool bdStatus)
{
	uint16_t byteNr = bdNum>>3;  //	uint16_t byteNr = trunc(bdNum/8);
    uint8_t bitMask = 0x01<<(bdNum % 8);
    if (bdStatus)
		blockDetectorBuffer[byteNr] |= bitMask;
	else
        blockDetectorBuffer[byteNr] &= ~bitMask;
}
	
uint8_t getBDStatus(uint16_t bdNum)
{
	uint16_t byteNr = bdNum>>3;  //	uint16_t byteNr = trunc(bdNum/8);
	return ((blockDetectorBuffer[byteNr] >> (bdNum % 8)) & 0x01); //0=free, 1=occ
}

switchBuffer * getSwitchList()
{
	return &switchPositionBuffer;
}

void setSwitchStatus(uint16_t swiNum, bool swiPos, bool coilStatus)
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
uint32_t getLastSwitchActivity(uint16_t swiNum)
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

uint8_t getSwiPosition(uint16_t swiNum)
{
	return ((switchPositionBuffer[swiNum >> 2] >> (2 * (swiNum % 4))) & 0x02) << 4;
}

uint8_t getSwiCoilStatus(uint16_t swiNum)
{
	return ((switchPositionBuffer[swiNum >> 2] >> (2 * (swiNum % 4))) & 0x01) << 4;
}

uint8_t getSwiStatus(uint16_t swiNum)
{
	return ((switchPositionBuffer[swiNum >> 2] >> (2 * (swiNum % 4))) & 0x03) << 4;
}

signalBuffer * getSignalList()
{
	return &signalAspectBuffer;
}

void setSignalAspect(uint16_t sigNum, uint8_t sigAspect)
{
	signalAspectBuffer[sigNum] = (sigAspect & 0x1F);
}

uint8_t getSignalAspect(uint16_t sigNum)
{
	return signalAspectBuffer[sigNum];
}

analogValBuffer * getAnalogValBuffer()
{
	return &analogValueBuffer;
}

void setAnalogValue(uint16_t analogNum, uint16_t analogValue)
{
	analogValueBuffer[analogNum] = analogValue;
}

uint16_t getAnalogValue(uint16_t analogNum)
{
	return analogValueBuffer[analogNum];
}


buttonValBuffer * getButtonValBuffer()
{
	return &buttonValueBuffer;
}

void setButtonValue(uint16_t buttonNum, uint8_t buttonValue)
{
	buttonValueBuffer[buttonNum] = buttonValue;
}

uint8_t getButtonValue(uint16_t buttonNum)
{
	return buttonValueBuffer[buttonNum];
}

void setPowerStatus(uint8_t newStatus)
{
	switch (newStatus)
	{
		case 0x82: ////OPC_OFF
			sysPowerStatus = 0;
			break;
		case 0x83: ////OPC_ON
			sysPowerStatus = 1;
			break;
		case 0x85: ////OPC_IDLE
			sysPowerStatus = 2;
			break;
	}
}

uint8_t getPowerStatus()
{
	return sysPowerStatus;
}

slotDataBuffer * getSlotDataBuffer()
{
	return &slotBuffer;
}

void setSlotData(uint8_t slotNum, slotData thisSlot)
{
	memcpy(slotBuffer[slotNum], thisSlot, 10);
}

slotData * getSlotData(uint8_t slotNum)
{
	return &slotBuffer[slotNum];
}

void enableBushbyWatch(bool enableBushby)
{
	bushbyWatch = enableBushby;
}

bool getBushbyWatch()
{
	return bushbyWatch;
}

uint8_t getBushbyStatus()
{
	slotData * ctrlSlot = getSlotData(0x7F);
	return (((*ctrlSlot)[3] & 0x04) >> 2);
}

void processBufferUpdates()
{
	if (millis() > nextBufferUpdate)
	{
		nextBufferUpdate += bufferUpdateInterval;
		//check if system slot need update
	}
}

void prepLACKMsg(lnTransmitMsg * msgData, uint8_t ackCode, uint8_t ackData)
{
	msgData->lnData[0] = 0xB4; //OPC_LONG_ACK 
	msgData->lnData[1] = ackCode;
	msgData->lnData[2] = ackData;
	msgData->lnMsgSize = 4;
	setXORByte(&msgData->lnData[0]);
}

void prepSlotReadMsg(lnTransmitMsg * msgData, uint8_t slotNr)
{
	msgData->lnData[0] = 0xE7; 
	msgData->lnData[1] = 0x0E; //OPC_SL_RD_DATA
	msgData->lnData[2] = slotNr;
	memcpy(&msgData->lnData[3], slotBuffer[slotNr], 10);
	msgData->lnMsgSize = 14;
	setXORByte(&msgData->lnData[0]);
}

uint8_t getSlotOfAddr(uint8_t locoAddrLo, uint8_t locoAddrHi)
{
	uint8_t firstFree = 0xFF;
	for (uint8_t i = 1; i < 0x70; i++)
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

uint8_t getTopSlot(uint8_t masterSlot)
{
	if ((slotBuffer[masterSlot][0] & 0x40) == 0)
		return masterSlot;
	else
		return getTopSlot(slotBuffer[masterSlot][2]);
}

uint8_t getFirstSlave(uint8_t masterSlot)
{
	for (uint8_t i = 1; i < 0x70; i++)
		if (((slotBuffer[i][0] & 0x40) == 0x40) && (slotBuffer[i][2] == masterSlot))
			return i;
	return 0;
}

void processLocoNetReply(lnReceiveBuffer * newData)
{
//	Serial.println("processLocoNetReply");
	if (newData->lnData[0] & 0x08)
	{
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
				}
				else
					prepLACKMsg(&txBuffer, 0x3A, 0x00); //can't be uplinked (already is)
				replyMessage(txBuffer);
				
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
				}
				else
					prepLACKMsg(&txBuffer, 0x3A, 0x00); //can't be uplinked (already is)
				replyMessage(txBuffer);

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
							}
						}
				}
				replyMessage(txBuffer);
			break;
			case 0xBB: //OPC_RQ_SL_DATA -> SL_RD
			{
				uint8_t ofSlot = newData->lnData[1];
				prepSlotReadMsg(&txBuffer, ofSlot);
				replyMessage(txBuffer);
			}
			break;
			case 0xBC: //OPC_SW_STATE request, this is answered by the central unit for all switches with LACK msg
			{
				uint16_t swiAddr = (newData->lnData[1] & 0x7F) +  ((newData->lnData[2] & 0x0F) << 7);
				uint8_t swiPos = getSwiPosition(swiAddr) >> 5;
//				Serial.printf("SwiAddr %i SwiPos %i\n", swiAddr, swiPos);
				prepLACKMsg(&txBuffer, newData->lnData[0] & 0x7F, swiPos == 0? 0x50 : 0x30);
				replyMessage(txBuffer);
			}
			break;
			case 0xBD: //OPC_SW_ACK -> LACK
			{
				prepLACKMsg(&txBuffer, newData->lnData[0] & 0x7F, 0x7F);
				replyMessage(txBuffer);
			}
			break;
			case 0xBF: //OPC_LOCO_ADR -> SL_RD or LACK
			{
				uint8_t newSlot = getSlotOfAddr(newData->lnData[2], newData->lnData[1]);
				if (newSlot != 0xFF)
					prepSlotReadMsg(&txBuffer, newSlot);
				else
					prepLACKMsg(&txBuffer, 0x3F, 0); //no free slot
				replyMessage(txBuffer);
			}
			break;
			case 0xED: //OPC_IMM_PACKET -> LACK
			{
				prepLACKMsg(&txBuffer, 0x7D, 0x7F); //no free slot
				replyMessage(txBuffer);
			}
			break;
			case 0xEF:
			{
				if (newData->lnData[1] == 0x0E) //OPC_WR_SL_DATA -> LACK
				{
					switch (newData->lnData[2])
					{
						case 0x7C: //Programmer 
							prepLACKMsg(&txBuffer, 0x6F, 0x01); //task accepted
							replyMessage(txBuffer);
							slotBuffer[0x7C][1] = 0x03;
							prepSlotReadMsg(&txBuffer, 0x7C);
							break;
						default:
							prepLACKMsg(&txBuffer, 0x6F, 0x7F); //no free slot
							break;
					}
					replyMessage(txBuffer);
				}
			}
			break;
		}
	}
}

void setDirCmd(lnTransmitMsg * txBuffer, uint8_t slotNr, uint8_t speedVal)
{
	slotBuffer[slotNr][3] ^= 0x20;
}

void setFuncCmd(lnTransmitMsg * txBuffer, uint8_t slotNr, uint8_t funcNr, uint8_t funcStatus)
{
	txBuffer->lnData[0] = 2; //OpCode for function commands
	txBuffer->lnData[1] = slotBuffer[slotNr][6];
	txBuffer->lnData[2] = slotBuffer[slotNr][1];
	txBuffer->lnData[3] = funcNr; //function number
	txBuffer->lnData[4] = funcStatus;
	txBuffer->lnMsgSize = 5;
	dccCmdMessage(*txBuffer);
}

void setSpeedCmd(lnTransmitMsg * txBuffer, uint8_t slotNr, uint8_t speedVal)
{
//	Serial.printf("Sp %i\n", slotBuffer[slotNr][0]);
	txBuffer->lnData[0] = 1; //OpCode for speed commands
	txBuffer->lnData[1] = (slotBuffer[slotNr][0] & 0x20) >> 5; //add to refresh buffer
	txBuffer->lnData[2] = slotBuffer[slotNr][6];
	txBuffer->lnData[3] = slotBuffer[slotNr][1];
	txBuffer->lnData[4] = speedVal;
	txBuffer->lnData[5] = slotBuffer[slotNr][3];
	txBuffer->lnData[6] = slotBuffer[slotNr][7];
	txBuffer->lnMsgSize = 7;
	dccCmdMessage(*txBuffer);
}

void setProgrammingCmd(lnTransmitMsg * txBuffer, uint8_t progMode, uint8_t OpsAdrHi, uint8_t OpsAdrLo, uint8_t CVNrHi, uint8_t CVNrLo, uint8_t CVVal)
{
//	Serial.printf("Sp %i\n", slotBuffer[slotNr][0]);
	txBuffer->lnData[0] = 5; //OpCode for programming commands
	txBuffer->lnData[1] = progMode;
	txBuffer->lnData[2] = OpsAdrHi;
	txBuffer->lnData[3] = OpsAdrLo;
	txBuffer->lnData[4] = CVNrHi;
	txBuffer->lnData[5] = CVNrLo;
	txBuffer->lnData[6] = CVVal; //new val or compare val
	txBuffer->lnMsgSize = 7;
	dccCmdMessage(*txBuffer);
}

void iterateMULinks(lnTransmitMsg * txBuffer, uint8_t thisSlot, uint8_t templData, dccFct procFunc)
{
	for (uint8_t i = 1; i < 0x70; i++)
		if ((slotBuffer[i][0] & 0x40) && (slotBuffer[i][2] == thisSlot) && (i != thisSlot))
		{
			procFunc(txBuffer, i, templData);
			iterateMULinks(txBuffer, i, templData, procFunc);
		}
}

void generateSpeedCmd(lnTransmitMsg * txBuffer, uint8_t thisSlot, uint8_t topSpeed)
{
//	Serial.printf("Gen Speed %i %i\n", thisSlot, topSpeed);
	dccFct thisFct = setSpeedCmd;
	if ((slotBuffer[thisSlot][0] & 0x08) > 0) //has others linked into
		iterateMULinks(txBuffer, thisSlot, topSpeed, setSpeedCmd);
	setSpeedCmd(txBuffer, thisSlot, topSpeed);
}

void generateFunctionCmd(lnTransmitMsg * txBuffer, lnReceiveBuffer * newData)
{
	#define opCode newData->lnData[0]
	
	uint8_t thisSlot = newData->lnData[1];
	uint8_t bitFlags = opCode == 0xA2 ? slotBuffer[thisSlot][7] : slotBuffer[thisSlot][3]; //0xA1, 0xB6
	uint8_t addrOfset = opCode == 0xA2 ? 5 : 1; //0xA1, 0xB6
	uint8_t bitMask = 0x01;
	for (uint8_t i = 0; i < 4; i++)
	{
		if ((newData->reqID & bitMask) > 0)
			//send function bit command
			setFuncCmd(txBuffer, thisSlot, addrOfset + i, (bitFlags & bitMask) >> i);
		bitMask <<= 1;
	}
	if ((opCode == 0xA1) || (opCode == 0xB6))  //OPC_DIRF, OPC_CONSIST_FUNC
	{
		if ((newData->reqID & 0x10) > 0)
			//send light bit command
			setFuncCmd(txBuffer, thisSlot, 0, (bitFlags & 0x10) >> 4);
	if (opCode == 0xA1)  //OPC_DIRF
		if ((newData->reqID & 0x20) > 0)
		{
//			Serial.println("chdir");
			//send DIR bit command, include links
			uint8_t topSpeed = slotBuffer[thisSlot][2];
			iterateMULinks(txBuffer, thisSlot, slotBuffer[thisSlot][2], setSpeedCmd);
			setSpeedCmd(txBuffer, thisSlot, slotBuffer[thisSlot][2]);
		}
	}
}

void processCmdGenerator(lnReceiveBuffer * newData)
{
/*
	This interface is using the LN data structure but with a different coding.
*/ 
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
			dccCmdMessage(txBuffer);
		}
		break;
		case 0xA0:; //OPC_LOCO_SPD  <1><REFRESH><ADDRHI><ADDRLO><SPEED><DIRF><SND>
			generateSpeedCmd(&txBuffer, newData->lnData[1], newData->lnData[2]);
			break;
		case 0xA1:; //OPC_LOCO_DIRF
		case 0xA2:; //OPC_LOCO_SND
		case 0xB6: //OPC_CONSIST_FUNC 
			generateFunctionCmd(&txBuffer, newData);
			break;
		break;
        case 0xB5: //OPC_SLOT_STAT1 
//			Serial.printf("SS1 %i %i\n", newData->lnData[1], newData->reqID);
			if ((newData->reqID & 0x30) > 0) //change in refresh status
				generateSpeedCmd(&txBuffer, newData->lnData[1], slotBuffer[newData->lnData[1]][2]);
        break;
		case 0xBD:; //OPC_SW_REQ <2><ADDRHI><ADDRLO><POS>
		case 0xB0:  //OPC_SW_ACK
		{
			txBuffer.lnData[0] = 3; //OpCode for power status commands <0><STAT>
			txBuffer.lnData[1] = newData->lnData[2] & 0x0F; 
			txBuffer.lnData[2] = newData->lnData[1] & 0x7F; 
			txBuffer.lnData[3] = (newData->lnData[2] & 0x20) >> 5; 
			txBuffer.lnMsgSize = 4;
			dccCmdMessage(txBuffer);
		}
		break;
 		case 0xBA: //OPC_MOVE_SLOTS, handled in limited master or higher
 		{
//			Serial.printf("Move %i %i %2X %2X\n", newData->lnData[1], newData->lnData[2], slotBuffer[newData->lnData[1]][0], slotBuffer[newData->lnData[2]][0]);
			if (((slotBuffer[newData->lnData[1]][0] & 0x48) == 0) || ((slotBuffer[newData->lnData[1]][0] & 0x48) == 0x40)) //free or consist top
				generateSpeedCmd(&txBuffer, newData->lnData[1], slotBuffer[newData->lnData[1]][2]);
			if (newData->lnData[1] != newData->lnData[2])
				if (((slotBuffer[newData->lnData[2]][0] & 0x48) == 0) || ((slotBuffer[newData->lnData[2]][0] & 0x48) == 0x40)) //free or consist top
					generateSpeedCmd(&txBuffer, newData->lnData[2], slotBuffer[newData->lnData[2]][2]);
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
						case 0x7B: break; //FastClock
						case 0x7C: //Programming task
							if (newData->lnData[0] == 0xEF)
							{
//								Serial.println("Programming task");
								setProgrammingCmd(&txBuffer, newData->lnData[3], newData->lnData[5], newData->lnData[6], newData->lnData[8], newData->lnData[9], newData->lnData[10]);
							}
							break;
						default: 
							if ((newData->lnData[2] > 0) && (newData->lnData[2] < 0x70)) //Loco Slots
							{
//								Serial.printf("Ex %i %i\n", newData->lnData[2], newData->reqID);
								if ((newData->reqID & 0x30) > 0) //change in refresh status
									generateSpeedCmd(&txBuffer, newData->lnData[2], newData->lnData[5]);
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
								dccCmdMessage(txBuffer);
						}
						break;
					}
				break;
			}
		}
	}
}

void setSlotDirfSpeed(lnReceiveBuffer * newData)
{
	slotData * thisSlot = getSlotData(newData->lnData[1]);
	if (thisSlot)
		switch (newData->lnData[0])
		{
			case 0xA0://OPC_LOCO_SPD
			    (*thisSlot)[2] = newData->lnData[2];
				break; 
			case 0xA1: //OPC_LOCO_DIRF
			{
				newData->reqID = ((*thisSlot)[3] ^ newData->lnData[2]) & 0x003F; //mark the bits that change for updating DCC
				if ((newData->reqID & 0x20) > 0)
					iterateMULinks(NULL, newData->lnData[1], 0, setDirCmd);
			    (*thisSlot)[3] = newData->lnData[2];
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

void processLocoNetMsg(lnReceiveBuffer * newData)
{
	bool sendReply = ((newData->lnData[0] & 0x08) > 0);
	bool sendDccCmd = false;
	
	switch (newData->lnData[0])
	{
		case 0x82:; //OPC_OFF
		case 0x83:; //OPC_ON
		case 0x85: //OPC_IDLE
		{
			setPowerStatus(newData->lnData[0]); 
			if (handlePowerStatus) 
				handlePowerStatus(); 
			sendDccCmd = true;
		}
		break;
		case 0xA0:; //OPC_LOCO_SPD
		case 0xA1:; //OPC_LOCO_DIRF
		case 0xA2: //OPC_LOCO_SND
		{
			setSlotDirfSpeed(newData);
			sendDccCmd = true;
		}
		break;
        case 0xB1: //OPC_SW_REP
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
//          Serial.printf("Set Switch %i to %i\n", swiAddr, inpPosStat);
          setSwitchStatus(swiAddr, (inpPosStat & 0x02)>>1, (inpPosStat & 0x01));
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
//          Serial.printf("Set Switch %i to %i\n", swiAddr, inpPosStat);
			setSwitchStatus(swiAddr, (inpPosStat & 0x02)>>1, (inpPosStat & 0x01));
			if (handleSwiEvent)
				handleSwiEvent(swiAddr, (inpPosStat & 0x02)>>1, (inpPosStat & 0x01));
			sendDccCmd = true;
			break;
        }
        case 0xBC: //OPC_SW_STATE
			lastSwiAddr = ((newData->lnData[1] & 0x7F)) + ((newData->lnData[2] & 0x0F)<<7); //store switch address for coming LACK
			break;
		case 0xB4: //OPC_LONG_ACK
			switch (newData->lnData[1])
			{
				case 0x3C: //OPC_SW_STATE
					if (lastSwiAddr != 0xFFFF)
						switch (newData->lnData[2] & 0x70)
						{
							case 0x30: setSwitchStatus(lastSwiAddr, 1, 0); break;
							case 0x50: setSwitchStatus(lastSwiAddr, 0, 0); break;
						}
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
			setSlotDirfSpeed(newData);
			sendDccCmd = true; //add or remove address to/from refresh buffer
			break;
		}
		case 0xB6: //OPC_CONSIST_FUNC 
		{
			setSlotDirfSpeed(newData);
			sendDccCmd = true;
			break;
		}
		case 0xBA: //OPC_MOVE_SLOTS, handled in limited master or higher
			sendDccCmd = true;
			break;
		
/*		
		case 0xB8: //OPC_UNLINK_SLOTS, handled in limited master or higher
		case 0xB9: //OPC_LINK_SLOTS, handled in limited master or higher
		case 0xBA: //OPC_MOVE_SLOTS, handled in limited master or higher
		case 0xBB: //OPC_RQ_SL_DATA, handled in limited master or higher
		case 0xBC:  //OPC_SW_STATE request, only processed in limited master of higher
		case 0xBF:  //OPC_LOCO_ADR request, only processed in limited master of higher
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
			//add DCC decoder fro switch and locos
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
			sendDccCmd = true;
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
			switch (newData->lnData[1])
			{
				case 0x0E: //SL_RD or WR_SL
				{
					uint8_t slotNr = newData->lnData[2];
					slotData * newSlot = &slotBuffer[slotNr];
					newData->reqID = ((*newSlot)[0] ^ newData->lnData[3]) & 0x007F; //identify changes in slot status
//					Serial.printf("RDWR %i %i %i\n", (*newSlot)[0], newData->lnData[3], newData->reqID);
					memcpy(newSlot[0], &newData->lnData[3], 10);
					switch (slotNr)
					{
						case 0x7B: //Fast Clock
							break;
						case 0x7C: //Programmer
								sendDccCmd = true;
							break;
						case 0x7F: //System Configuration
							Serial.printf("Bushby Bit is %i \n", getBushbyStatus());
							break;
						default:
							if (slotNr < 0x70) //regular slot
							{
								sendDccCmd = true;
							}
					} 
					break;
				}
				default:
					break;
			}
			break;
	}
	if (sendReply )
		if (replyMessage)
			processLocoNetReply(newData);
	if (sendDccCmd)
		if (dccCmdMessage)
			processCmdGenerator(newData);
}

bool saveToFile(String fileName)
{
//	Serial.println("Save to disk");
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
//			buf[0] = (analogValueBuffer[i] >> 8);
//			buf[1] = (analogValueBuffer[i] & 0x00FF);
			dataFile.write(buf, 2);
		}
		dataFile.write(buttonValueBuffer, numButtons);
		dataFile.write(sysPowerStatus);
		for (int i = 0; i < numSlots; i++)
			dataFile.write(slotBuffer[i], 10);
		
		dataFile.close();
		Serial.println("Writing Config File complete");
		return true;
    }
    else
    {
		Serial.println("Unable to write Config File");
		return false;
	}
}

void loadFromFile(String fileName)
{
    File dataFile = SPIFFS.open(fileName, "r");
    if (dataFile)
    {
		uint32_t fileSize = dataFile.size();
		uint32_t minSize = numBDs;
		Serial.printf("Load %i bytes from disk\n", fileSize);
		if (fileSize >= minSize) 
			dataFile.read(blockDetectorBuffer, numBDs);
//		Serial.printf("load Swi %i\n", minSize);
		minSize += numSwis;
		if (fileSize >= minSize) 
			dataFile.read(switchPositionBuffer, numSwis);
		minSize += numSigs;
//		Serial.printf("load Sig %i\n", minSize);
		if (fileSize >= minSize) 
			dataFile.read(signalAspectBuffer, numSigs);
		uint8_t buf[2];
		minSize += (2 * numAnalogVals);
//		Serial.printf("load Analog %i\n", minSize);
		if (fileSize >= minSize) 
			for (int i = 0; i < numAnalogVals; i++)
			{
				dataFile.read(buf,2);
				analogValueBuffer[i] = (buf[0]<<8) + buf[1];
			}
		minSize += numButtons;
//		Serial.printf("load Button %i\n", minSize);
		if (fileSize >= minSize) 
			dataFile.read(buttonValueBuffer, numButtons);
		minSize += 1;
//		Serial.printf("load Power %i\n", minSize);
		if (fileSize >= minSize) 
			dataFile.read(&sysPowerStatus, 1);
		minSize += (numSlots * 10);
//		Serial.printf("load Slots %i\n", minSize);
		if (fileSize >= minSize) 
			for (int i = 0; i < numSlots; i++)
				dataFile.read(slotBuffer[i], 10);
		else
			for (int i = 0; i < numSlots; i++)
				memcpy(&slotBuffer[i], &stdSlot[0], 10);
		dataFile.close();
		for (int i = 1; i < 0x70; i++)
			slotBuffer[i][4] = 0x04 + sysPowerStatus;
		Serial.println("Config File loaded");
	}
    else
		Serial.println("Unable to read Config File");
}

