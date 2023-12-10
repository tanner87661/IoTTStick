#include <IoTT_SecurityElements.h>

sigAddrType getAddrTypeOfName(String ofName)
{
  if (ofName == "SwiDyn") return swiDyn;
  if (ofName == "SwiStat") return swiStat;
  if (ofName == "SigNMRA") return sigNMRA;
  return sigNMRA;
}

uint8_t getCmdValByName(String ofName)
{
  if (ofName == "closed") return 1;
  if (ofName == "thrown") return 0;
  return ofName.toInt();
}

uint8_t getLegPosFromChar(String ofChar)
{
  if (ofChar == "A") return 0;
  if (ofChar == "B") return 1;
  if (ofChar == "C") return 2;
  return 0xFF;
}

ctrlType getCtrlModeByName(String ofName)
{
	Serial.println(ofName);
  if (ofName == "yard") return manual;
  if (ofName == "ABSS") return ABSS;
  if (ofName == "ABSD") return ABSD;
  if (ofName == "APB") return APB;
  if (ofName == "CTC") return CTC;
  return manual;
}

fallbackMode getFallbackModeByName(String ofName)
{
	if (ofName == "None") return nofallback;
	if (ofName == "OnEnter") return fbOnEnter;
	if (ofName == "OnPrevExit") return fbOnExitPrev;
	if (ofName == "OnExit") return fbOnExit;
	return nofallback;
}

/*----------------------------------------------------------------------------------------------------------------------*/
IoTT_SpeedTable::IoTT_SpeedTable()
{
}

IoTT_SpeedTable::~IoTT_SpeedTable()
{
	freeObjects();
}

void IoTT_SpeedTable::loadSpeedTableEntryJSON(JsonObject thisObj)
{
	if (thisObj.containsKey("ModelName"))
		strcpy(speedSetName, thisObj["ModelName"]);
		
	if (thisObj.containsKey("SpeedOptions"))
    {
        JsonArray speedEntries = thisObj["SpeedOptions"];
        numSpeedLines = speedEntries.size();
        tableEntries = (IoTT_SpeedTableEntry*) realloc (tableEntries, numSpeedLines * sizeof(IoTT_SpeedTableEntry));
		for (uint16_t i = 0; i < numSpeedLines; i++)
		{
			IoTT_SpeedTableEntry * thisSpeedEntry = new(IoTT_SpeedTableEntry);
			strcpy(thisSpeedEntry->entryName, speedEntries[i]["SpeedName"]);
			thisSpeedEntry->entrySpeed = speedEntries[i]["SpeedVal"];
			tableEntries[i] = (*thisSpeedEntry);
		}
	}
	
}

void IoTT_SpeedTable::freeObjects()
{
	for (uint16_t i = 0; i < numSpeedLines; i++)
	{
		delete &tableEntries[i];
	}
	free(tableEntries);
	numSpeedLines = 0;
}

/*----------------------------------------------------------------------------------------------------------------------*/

IoTT_AspectGenerator::IoTT_AspectGenerator()
{
}

IoTT_AspectGenerator::~IoTT_AspectGenerator()
{
	freeObjects();
}

void IoTT_AspectGenerator::loadAspectCfgJSON(JsonObject thisObj)
{
	if (thisObj.containsKey("AspectSetName"))
		strcpy(aspectSetName, thisObj["AspectSetName"]);
		
	if (thisObj.containsKey("Aspects"))
    {
        JsonArray aspectList = thisObj["Aspects"];
        numAspectEntries = aspectList.size();
        aspectEntries = (IoTT_AspectEntry*) realloc (aspectEntries, numAspectEntries * sizeof(IoTT_AspectEntry));
		for (uint16_t i = 0; i < numAspectEntries; i++)
		{
			IoTT_AspectEntry * thisAspectEntry = new(IoTT_AspectEntry);
			//add code to populate fields
			strcpy(thisAspectEntry->aspectName, aspectList[i]["AspectLabel"]);
			thisAspectEntry->aspectNr = aspectList[i]["AspectValue"];
			strcpy(thisAspectEntry->staticSpeedName, aspectList[i]["StaticSpeed"]);
			strcpy(thisAspectEntry->dynamicSpeedName, aspectList[i]["DynamicSpeed"]);
			aspectEntries[i] = (*thisAspectEntry);
		}
	}
}

uint8_t IoTT_AspectGenerator::getAspectFromSpeed(uint8_t dynSpeedLevel, uint8_t statSpeedLevel)
{
	return 0;
}

uint8_t IoTT_AspectGenerator::getDynLevelFromAspect(uint8_t thisAspect)
{
	return 0;
}


void IoTT_AspectGenerator::freeObjects()
{
	for (uint16_t i = 0; i < numAspectEntries; i++)
	{
		delete &aspectEntries[i];
	}
	free(aspectEntries);
	numAspectEntries = 0;
}


/*----------------------------------------------------------------------------------------------------------------------*/
IoTT_SecElLeg::IoTT_SecElLeg()
{
//	Serial.println("Create SecElLeg");
}

IoTT_SecElLeg::~IoTT_SecElLeg()
{
	freeObjects();
}

void IoTT_SecElLeg::freeObjects()
{
}

void IoTT_SecElLeg::loadSELegCfgJSON(JsonObject thisObj)
{
	if (thisObj.containsKey("Pos"))
		legPos = getLegPosFromChar(thisObj["Pos"]);
	if (thisObj.containsKey("SpeedIn"))
		legTechSpeedIn = thisObj["SpeedIn"];
	if (thisObj.containsKey("SpeedOut"))
		legTechSpeedOut = thisObj["SpeedOut"];
	else
		legTechSpeedOut =legTechSpeedIn;
	if (thisObj.containsKey("DestSE"))
	{
		JsonArray thisDest = thisObj["DestSE"];
		destSENr = thisDest[0];
		destSELeg = getLegPosFromChar(thisDest[1]);
	}
	if (thisObj.containsKey("AspGen"))
	{
		if (thisObj["AspGen"] != -1)
		{
//			String thisType = thisObj"AspCen"]["SignalType"];
			entryAspectGenerator = parentSE->parentSEL->getAspectGeneratorByName(thisObj["AspGen"]["Templ"]);
			addrType = getAddrTypeOfName(thisObj["AspGen"]["SignalType"]);
			JsonArray sigAddr = thisObj["AspGen"]["Addr"];
			numAddr = sigAddr.size();
			for (uint8_t i = 0; i < numAddr; i++)
				entrySignalAddr[i] = sigAddr[i];
		}
	}
}

/*
void IoTT_SecElLeg::processLocoNetMsg(lnReceiveBuffer * newData)
{
	switch (newData->lnData[0])
    {
        case 0xED: //OPC_IMM_PACKET
        {
			if (addrType == sigNMRA)
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
				if (sigAddress == entrySignalAddr[0])
				{
					lastSigAspect = recData[3] & 0x1F;
					//reverse calc dynSpeed from SignalAspect
					if (entryAspectGenerator) 
						setDynSpeed(entryAspectGenerator->getDynLevelFromAspect(lastSigAspect));
				}
			}
			break;
		}
        case 0xB0: 
        case 0xB1:; //OPC_SW_REP
        case 0xBD:  //OPC_SW_ACK
        {
			uint16_t swiAddr = ((newData->lnData[1] & 0x7F)) + ((newData->lnData[2] & 0x0F)<<7);
			for (int i = 0; i < numAddr; i++)
			{
				if (swiAddr == entrySignalAddr[i])
				{
					if (addrType == swiDyn)
					{
						lastDynIndex = i;
						lastSigAspect = (2 * i) + ((newData->lnData[2] & 0x10) >> 4);
					}
					else
					if (addrType == swiStat)
					{
						uint8_t aspVal = 0;
						for (int j = 0; j < numAddr; j++)
							aspVal += (getSwiPosition(entrySignalAddr[i]) >> (4-i));
						lastSigAspect = aspVal;
					}
					break;
				}
			}
			//reverse calc dynSpeed from SignalAspect
			if (entryAspectGenerator) 
				setDynSpeed(entryAspectGenerator->getDynLevelFromAspect(lastSigAspect));
          break;
        }
	}
	
}
*/

void IoTT_SecElLeg::sendAspectCommand(uint8_t aspectVal)
{
/*
	switch (addrType)
	{
		case sigNMRA: sendSignalCommand(entrySignalAddr[0], aspectVal); break;
		case swiDyn: sendSwitchCommand(entrySignalAddr[aspectVal>>1], aspectVal & 0x01, true); break;
		case swiStat:
			for (int i = 0; i < numAddr; i++)
				sendSwitchCommand(entrySignalAddr[i], (aspectVal & (0x01<<i)) > 0, true);
			break;
	}
*/
	lastSigAspect = aspectVal;
}

int IoTT_SecElLeg::calculateAspect()
{
	if (entryAspectGenerator)
		return entryAspectGenerator->getAspectFromSpeed(dynUptickCtr, legPos == 0 ? legTechSpeedOut : legTechSpeedIn);
	else
		return -1;
}


void IoTT_SecElLeg::setDynSpeed(uint16_t newSpeedLevel)
{
	//set current in speed 
//	Serial.printf("Set Dyn Speed %i\n", newSpeedLevel);
	if (dynUptickCtr != newSpeedLevel)
	{
		dynUptickCtr = newSpeedLevel;
		//calculate signal aspect
		uint8_t newAspect = calculateAspect();
		//send signal commands to command buffer for shipping out
		if ((newAspect >= 0) and (newAspect != lastSigAspect))
			sendAspectCommand(newAspect);
	}
}

uint8_t IoTT_SecElLeg::getNextSignalDynCtr()
{
//	Serial.printf("Trying SE %i leg %i: ", parentSE->secelID, legPos);
	IoTT_SecElLeg * openLeg = getOpenLeg();
	if (openLeg == this) //leading to nowhere
		return 0;
		
	IoTT_SecElLeg * pairedLeg = getConnectedLeg();

//	if (pairedLeg)
//		Serial.printf("SE %i leg %i paired with SE %i leg %i\n", parentSE->secelID, legPos, pairedLeg->parentSE->secelID, pairedLeg->legPos);
//	else
//		Serial.printf("SE %i leg %i not paired\n", parentSE->secelID, legPos);

	if (!pairedLeg) //terminal leg
	{
		Serial.printf("terminal %i\n", parentSE->isTerminal);
		return parentSE->isTerminal ? 0 : parentSE->parentSEModel->dynamicSpeedModel->numSpeedLines - 1; //not possible to enter next leg, ctr = 0 or max in case it is leg to open track
	}
	else
	{
		if (!pairedLeg->destSE)
		{
			Serial.println("not paired ret 0 ");
			return 0; //not possible to enter next leg, ctr = 0
		}
		else
			if (pairedLeg->destSE->entryAspectGenerator) //it has an entry signal pointing towards us
			{
//				Serial.println("read destSE Ctr");
				return pairedLeg->destSE->dynUptickCtr;
			}
			else
			{
				Serial.println("recursive call destSE");
				return pairedLeg->destSE->getNextSignalDynCtr();
			}
	}
}

IoTT_SecElLeg * IoTT_SecElLeg::getConnectedLeg()
{
	if (parentSE->connLeg.size() <= 1)
	{
//		Serial.printf("Only %i leg, return NULL\n", parentSE->numLegs);
		return NULL; //isTerminal leg, has no neighbor
	}
	if (legPos > 0) //calling from B or C, so the answer is A or NULL
		if (this == getOpenLeg())
		{
//			Serial.println("Open Leg, not paired");
			return NULL;
		}
		else
		{
//			Serial.println("must be Leg A, orig legPos > 0");
			return parentSE->connLeg.at(0);
		}
	else //legPos == 0
	{
//		Serial.println("Looking from A");
		if (parentSE->connLeg.size() == 2)
		{
//			Serial.println("One-legged");
			return parentSE->connLeg.at(1); //there is no C, calling from A, so answer is B
		}
		else
		{
//			Serial.println("Check SwiPos");
			if (parentSE->switchLogic) //calling from A, answer is B or C depending on switch logic and switch position
				return (digitraxBuffer->getSwiPosition(parentSE->switchAddr) > 0) ? parentSE->connLeg.at(1): parentSE->connLeg.at(2); 
			else
				return (digitraxBuffer->getSwiPosition(parentSE->switchAddr) == 0) ?  parentSE->connLeg.at(1): parentSE->connLeg.at(2);
		}
	}
}

IoTT_SecElLeg * IoTT_SecElLeg::getOpenLeg() //returns pointer to B or C or NULL
{
//	Serial.printf("SE %i with %i legs\n", parentSE->secelID, parentSE->numLegs);
	switch (parentSE->connLeg.size()) 
	{
		case 0: return NULL; break;
		case 1: return NULL; break;
		case 2: return NULL; break; //return parentSE->connLeg[1]; break;
		case 3: if (parentSE->switchLogic) //answer is B or C depending on switch logic and switch position
				{
//					Serial.printf("Open swipos %i leg is %i\n", getSwiPosition(parentSE->switchAddr), (getSwiPosition(parentSE->switchAddr) > 0) ?  1:2);
					return (digitraxBuffer->getSwiPosition(parentSE->switchAddr) == 0) ?  parentSE->connLeg.at(1): parentSE->connLeg.at(2); 
				}
				else
				{
//					Serial.printf("Open swipos %i leg is %i\n", getSwiPosition(parentSE->switchAddr), (getSwiPosition(parentSE->switchAddr) == 0) ?  1:2);
					return (digitraxBuffer->getSwiPosition(parentSE->switchAddr) > 0) ?  parentSE->connLeg.at(1): parentSE->connLeg.at(2);
				}
				break;
	}
	return NULL;
}

uint8_t IoTT_SecElLeg::getCurrentDynSpeed()
{
	return dynUptickCtr;
}

bool IoTT_SecElLeg::hasEntrySignal()
{
	return entryAspectGenerator != NULL;
}

void IoTT_SecElLeg::resolveLegConnector()
{
	Serial.printf("Resolve LegConn SecEl %i Dest %i Leg %i ...", parentSE->secelID, destSENr, destSELeg);
	destSE = parentSE->parentSEL->getLegPtr(destSENr, destSELeg);
}

uint8_t IoTT_SecElLeg::getLegPos()
{
	return legPos;
}

uint8_t IoTT_SecElLeg::updateDynSpeedFromSignal()
{
	return 0;
}


/*----------------------------------------------------------------------------------------------------------------------*/
IoTT_SecurityElement::IoTT_SecurityElement()
{
}

IoTT_SecurityElement::~IoTT_SecurityElement()
{
	freeObjects();
}

void IoTT_SecurityElement::loadSecElCfgJSON(JsonObject thisObj)
{
	if (thisObj.containsKey("SelID"))
		secelID = thisObj["SelID"];
	if (thisObj.containsKey("CtrlMode"))
		ctrlMode = getCtrlModeByName(thisObj["CtrlMode"]);
	if (thisObj.containsKey("BDAddr"))
		blockdetAddr = thisObj["BDAddr"];
	if (thisObj.containsKey("SwiAddr"))
		switchAddr = thisObj["SwiAddr"];
	if (thisObj.containsKey("SwiLogic"))
		switchLogic = thisObj["SwiLogic"];
	if (thisObj.containsKey("TargetButton"))
		targetBtn = thisObj["TargetButton"];
	if (thisObj.containsKey("DepartureButton"))
		departBtn = thisObj["DepartureButton"];
	if (thisObj.containsKey("IsTerminal"))
		isTerminal = thisObj["IsTerminal"];
	if (thisObj.containsKey("IsStation"))
		isStation = thisObj["IsStation"];
	if (thisObj.containsKey("StationName"))
		strcpy(stationName, thisObj["StationName"]);
	if (thisObj.containsKey("TrackNr"))
		stationTrackNr = thisObj["TrackNr"];
	if (thisObj.containsKey("FallbackMode"))
		fbMode = getFallbackModeByName(thisObj["FallbackMode"]);
	
	if (thisObj.containsKey("Connectors"))
	{
        JsonArray seConnectors = thisObj["Connectors"];
		for (uint16_t i = 0; i < seConnectors.size(); i++)
		{
			IoTT_SecElLeg * thisSELeg = new(IoTT_SecElLeg);
			thisSELeg->parentSE = this;
			thisSELeg->loadSELegCfgJSON(seConnectors[i]);
			thisSELeg->legPos = connLeg.size(); //0==A
			connLeg.push_back(thisSELeg);
		}
	}	
}

void IoTT_SecurityElement::resolveLegConnectors()
{
	if (connLeg.size() > 0)
		for (uint16_t i = 0; i < connLeg.size(); i++)
		{
/*
			connLeg[i]->resolveLegConnector();
			if ((connLeg[i]->destSE) == NULL)
				Serial.printf("Allocation of SE %i Leg %i failed\n", secelID, i);
			else
				Serial.println("ok");
*/
		}
}

IoTT_SecElLeg * IoTT_SecurityElement::getLegPtr(uint8_t destSELeg)
{
/*
	if (destSELeg < connLeg.size())
		return connLeg[destSELeg];
	else			
		return NULL;
*/
}

/*	
void IoTT_SecurityElement::processLocoNetMsg(lnReceiveBuffer * newData)
{
	switch (newData->lnData[0])
    {
		case 0xB2: //OPC_INPUT_REP
        {
			uint16_t inpAddr = ((newData->lnData[1] & 0x7F)<<1) + ((newData->lnData[2] & 0x0F)<<8) + ((newData->lnData[2] & 0x20)>>5);
			if (inpAddr == blockdetAddr)
				blockdetStatus = (newData->lnData[2] & 0x10)>>4;
			break;
        }
        case 0xED: //OPC_IMM_PACKET
        {
			if ((newData->lnData[1] == 0x0B) && (newData->lnData[2] == 0x7F))
				for (uint16_t i = 0; i < numLegs; i++)
					connLeg[i]->processLocoNetMsg(newData);
			break;
		}
        case 0xB0: 
			if (getBushbyWatch()) 
				if (getBushbyStatus() > 0) 
					break; //OPC_SW_REQ
        case 0xB1:; //OPC_SW_REP
        case 0xBD:  //OPC_SW_ACK
        {
			uint16_t swiAddr = ((newData->lnData[1] & 0x7F)) + ((newData->lnData[2] & 0x0F)<<7);
			if (swiAddr == switchAddr)
				switchStatus = (newData->lnData[2] & 0x10); //Direction and ON Status
			for (uint16_t i = 0; i < numLegs; i++) //could be a signal, run it through the legs
				connLeg[i]->processLocoNetMsg(newData);
          break;
        }
	}
}
*/

void IoTT_SecurityElement::processElement()
{
	uint8_t eventMask = 0;
	uint8_t hlpStat = digitraxBuffer->getBDStatus(blockdetAddr);
	if (hlpStat != lastBDStatus)
	{
		eventMask |= 0x01;
		lastBDStatus = hlpStat;
	}
	hlpStat = digitraxBuffer->getSwiPosition(switchAddr);
	if (hlpStat != lastSwiPos)
	{
		eventMask |= 0x02;
		lastSwiPos = hlpStat;
	}
	switch (ctrlMode)
	{
		case ABSS: processABSS(eventMask); break;
		case ABSD: processABSD(eventMask); break;
		case APB: processAPB(eventMask); break;
		case CTC: processCTC(eventMask); break;
		default: processManualSE(eventMask); break;
	}
}

void IoTT_SecurityElement::processABSS(uint8_t newEvents)
{
/*
	if (numLegs > 0)
		for (uint16_t i = 0; i < numLegs; i++)
			if (connLeg[i])
			{
				if (digitraxBuffer->getBDStatus(blockdetAddr))
					connLeg[i]->setDynSpeed(0); //set dynamic In speed to zero and set all signals respective aspect
				else
				{
					//get the speed of the in-leg of the next SE
					uint8_t nextLegDynSpeed = connLeg[i]->getNextSignalDynCtr();
					
					//if not highest level, set dynamic in speed to next higher level
					if (nextLegDynSpeed < parentSEModel->dynamicSpeedModel->numSpeedLines)
						nextLegDynSpeed++;
					connLeg[i]->setDynSpeed(nextLegDynSpeed);
				}

//				if (((secelID == 5) && (i==0)) || ((secelID == 8) && (i!=0)) || ((secelID == 6) && (i==0)) || ((secelID == 7) && (i==0)) || ((secelID == 4) && (i==0)) || ((secelID == 9) && (i==0)) || ((secelID == 10) && (i==0)) || ((secelID == 11) && (i==0)) || ((secelID == 12) && (i==0))  )
//					Serial.printf("SE %i Leg %i Speed Value %i\n", secelID, i, connLeg[i]->getCurrentDynSpeed());



//				int newAspect = connLeg[i]->calculateAspect();
//				if ((newAspect >= 0) && (newAspect != connLeg[i]->lastSigAspect))
//					connLeg[i]->sendAspectCommand(newAspect);
			}
*/
}

void IoTT_SecurityElement::processABSD(uint8_t newEvents)
{
//	Serial.print("D");
	
}

void IoTT_SecurityElement::processAPB(uint8_t newEvents)
{
//		Serial.print("P");

}

void IoTT_SecurityElement::processCTC(uint8_t newEvents)
{
//		Serial.print("C");

}

void IoTT_SecurityElement::processManualSE(uint8_t newEvents) //update signal positions when set externally, recalculate dynSpeed, check validity and fallback needs
{
/*	
	if (numLegs > 0)
	{
		for (uint16_t i = 0; i < numLegs; i++)
			if (connLeg[i])
			{
				uint8_t dynSpeedLevel = connLeg[i]->updateDynSpeedFromSignal();
				switch (fbMode)
				{
					case nofallback: break;
					case fbOnEnter: 
//							if (currentBDStatus)
								dynSpeedLevel = 0; //reset signal after block got occupied
							break;
					case fbOnExit: break;
					case fbOnExitPrev: break;
				}
				if (dynSpeedLevel != connLeg[i]->getCurrentDynSpeed())
				{
					if (secelID == 3)
						Serial.printf("Leg %i new Speed %i to %i\n", i, connLeg[i]->getCurrentDynSpeed(), dynSpeedLevel);
					connLeg[i]->setDynSpeed(dynSpeedLevel);
				}
					
			}
		if (autoProtect)
		{
			bool firstPos = false;
			for (uint16_t i = 0; i < numLegs; i++)
			{
				if (firstPos)
					connLeg[i]->setDynSpeed(0);
				else
					firstPos = connLeg[i]->getCurrentDynSpeed() > 0;
			}
		}
			
	}
//	if (currentBDStatus != lastBDStatus)
//		lastBDStatus = currentBDStatus;
*/
}

void IoTT_SecurityElement::clearDirectionFlags()
{
	dirIn = false;
	dirOut = false;
}

void IoTT_SecurityElement::setDirection(bool inBound)
{
/*
	bool currState = inBound ? dirIn : dirOut;
	if (!currState) //if direction is already set, do nothing. Means: Clear directions before setting it
		for (uint16_t i = 0; i < numLegs; i++)
		{
			if (connLeg[i] != NULL)
				if (((i==0) && ((connLeg[i])->destSE->legPos != 0)) || ((i!=0) && ((connLeg[i])->destSE->legPos == 0)))
					(connLeg[i])->parentSE->setDirection(inBound);
				else
					(connLeg[i])->parentSE->setDirection(!inBound);
			
		}
*/
}


void IoTT_SecurityElement::freeObjects()
{
/*
 	if (numLegs > 0)
	{
		for (uint16_t i = 0; i < numLegs; i++)
			delete connLeg[i];
		numLegs = 0;
		free(connLeg);
	}
*/
}


/*----------------------------------------------------------------------------------------------------------------------*/
IoTT_SecurityElementSection::IoTT_SecurityElementSection()
{
}

IoTT_SecurityElementSection::~IoTT_SecurityElementSection()
{
	freeObjects();
}

void IoTT_SecurityElementSection::loadSecElCfgJSON(JsonObject thisObj)
{
	if (thisObj.containsKey("ModelName"))
		strcpy(modelName, thisObj["ModelName"]);
	if (thisObj.containsKey("ActiveModel"))
		isActive = thisObj["ActiveModel"];
	if (thisObj.containsKey("DynSpeedModel"))
		dynamicSpeedModel = parentSEL->getDynamicSpeedByName(thisObj["DynSpeedModel"]);
	if (thisObj.containsKey("StaticSpeedModel"))
		staticSpeedModel = parentSEL->getStaticSpeedByName(thisObj["StaticSpeedModel"]);
	if (thisObj.containsKey("SecurityElements"))
    {
        JsonArray secElementList = thisObj["SecurityElements"];
		for (uint16_t i = 0; i < secElementList.size(); i++)
		{
			IoTT_SecurityElement * thisSecurityElement = new(IoTT_SecurityElement);
			thisSecurityElement->parentSEL = parentSEL;
			thisSecurityElement->parentSEModel = this;
			thisSecurityElement->loadSecElCfgJSON(secElementList[i]);
			secElList.push_back(thisSecurityElement);
		}
	}
	
}

/*
void IoTT_SecurityElementSection::processLocoNetMsg(lnReceiveBuffer * newData)
{
	for (uint16_t i = 0; i < numSecEl; i++)
		secElList[i]->processLocoNetMsg(newData);
}
*/

IoTT_SecElLeg * IoTT_SecurityElementSection::getLegPtr(uint16_t destSENr, uint8_t destSELeg)
{
	for (uint16_t i = 0; i < secElList.size(); i++)
	{
		IoTT_SecurityElement* thisElement = secElList.at(i);
		if (thisElement->secelID == destSENr)
		{
			IoTT_SecElLeg * retPtr = thisElement->getLegPtr(destSELeg);
			if (retPtr)
				return retPtr;
		}
	}
	return NULL;
}

void IoTT_SecurityElementSection::resolveLegConnectors()
{
	for (uint16_t i = 0; i < secElList.size(); i++)
	{
		IoTT_SecurityElement* thisElement = secElList.at(i);
		thisElement->resolveLegConnectors();
	}
}

void IoTT_SecurityElementSection::processLoop()
{
	for (int i = 0; i < secElList.size(); i++)
	{
		IoTT_SecurityElement* thisElement = secElList.at(i);
		thisElement->processElement();
	}
}

void IoTT_SecurityElementSection::clearDirectionFlags()
{
}

void IoTT_SecurityElementSection::setDirection(bool inBound)
{
}

void IoTT_SecurityElementSection::freeObjects()
{
	secElList.clear();
}

/*----------------------------------------------------------------------------------------------------------------------*/
IoTT_SecurityElementList::IoTT_SecurityElementList()
{
}

IoTT_SecurityElementList::~IoTT_SecurityElementList()
{
	freeObjects();
}

void IoTT_SecurityElementList::loadSecElCfgJSON(DynamicJsonDocument doc, bool resetList)
{

	if (resetList)
		freeObjects(); //need to change this to allow for multi file

	if (doc.containsKey("StaticSpeedModelList"))
    {
        JsonArray staticSpeed = doc["StaticSpeedModelList"];
		for (uint16_t i = 0; i < staticSpeed.size(); i++)
		{
			IoTT_SpeedTable * thisSpeedTable = new(IoTT_SpeedTable);
			thisSpeedTable->loadSpeedTableEntryJSON(staticSpeed[i]);
			staticSpeedList.push_back(thisSpeedTable);
		}
		Serial.printf("%i static speed sets loaded\n", staticSpeedList.size());
	}

	if (doc.containsKey("DynSpeedModelList"))
    {
        JsonArray dynamicSpeed = doc["DynSpeedModelList"];
		for (uint16_t i = 0; i < dynamicSpeed.size(); i++)
		{
			IoTT_SpeedTable * thisSpeedTable = new(IoTT_SpeedTable);
			thisSpeedTable->loadSpeedTableEntryJSON(dynamicSpeed[i]);
			dynamicSpeedList.push_back(thisSpeedTable);
		}
		Serial.printf("%i dynamic speed sets loaded\n", dynamicSpeedList.size());
	}

//	return;
	
	if (doc.containsKey("AspectGeneratorList"))
    {
        JsonArray aspectGenerators = doc["AspectGeneratorList"];
 		for (uint16_t i = 0; i < aspectGenerators.size(); i++)
		{
			IoTT_AspectGenerator * thisAspectGenerator = new(IoTT_AspectGenerator);
			thisAspectGenerator->loadAspectCfgJSON(aspectGenerators[i]);
			aspGenList.push_back(thisAspectGenerator);
		}
		Serial.printf("%i aspect generators loaded\n", aspGenList.size());
	}


	if (doc.containsKey("SecurityElementSections"))
    {
        JsonArray secElMods = doc["SecurityElementSections"];
        uint16_t newNumSecElModels = secElMods.size();
        secModelList = (IoTT_SecurityElementSection**) realloc (secModelList, (numSecModel + newNumSecElModels) * sizeof(IoTT_SecurityElementSection*));
		for (uint16_t i = 0; i < newNumSecElModels; i++)
		{
			IoTT_SecurityElementSection * thisSecElModel = new(IoTT_SecurityElementSection);
			thisSecElModel->parentSEL = this;
			thisSecElModel->loadSecElCfgJSON(secElMods[i]);
			secModelList[i + numSecModel] = thisSecElModel;
		}
		numSecModel += newNumSecElModels;
		resolveLinks();
	}
}

/*
void IoTT_SecurityElementList::processLocoNetMsg(lnReceiveBuffer * newData)
{
	for (uint16_t i = 0; i < numSecModel; i++)
		secModelList[i]->processLocoNetMsg(newData);
}
*/

IoTT_SecElLeg * IoTT_SecurityElementList::getLegPtr(uint16_t destSENr, uint8_t destSELeg)
{
	for (uint16_t i = 0; i < numSecModel; i++)
	{
		IoTT_SecElLeg * retPtr = secModelList[i]->getLegPtr(destSENr, destSELeg);
		if (retPtr)
			return retPtr;
	}
	return NULL;
}

IoTT_AspectGenerator* IoTT_SecurityElementList::getAspectGeneratorByName(String aspName)
{
	for (uint16_t i = 0; i < aspGenList.size(); i++)
	{
		IoTT_AspectGenerator* thisAspGen = aspGenList.at(i);
		if (String(thisAspGen->aspectSetName) == aspName)
			return thisAspGen;
	}
	return NULL;
}


IoTT_SpeedTable* IoTT_SecurityElementList::getStaticSpeedByName(String speedName)
{
	for (uint16_t i = 0; i < staticSpeedList.size(); i++)
	{
		IoTT_SpeedTable* thisSpeedSet = staticSpeedList.at(i);
		if (String(thisSpeedSet->speedSetName) == speedName)
			return thisSpeedSet;
	}
	return NULL;
}

IoTT_SpeedTable* IoTT_SecurityElementList::getDynamicSpeedByName(String speedName)
{
	Serial.print(dynamicSpeedList.size());
	Serial.println(speedName);
	for (uint16_t i = 0; i < dynamicSpeedList.size(); i++)
	{
		IoTT_SpeedTable* thisSpeedSet = dynamicSpeedList.at(i);
		if (String(thisSpeedSet->speedSetName) == speedName)
			return thisSpeedSet;
	}
	Serial.println("No Speed Model of this name");
	return NULL;
}

void IoTT_SecurityElementList::resolveLinks()
{
	for (int i = 0; i < numSecModel; i++)
	{
		IoTT_SecurityElementSection * thisModel = secModelList[i];
		thisModel->resolveLegConnectors();
	}
}

void IoTT_SecurityElementList::freeObjects()
{
	staticSpeedList.clear();
	dynamicSpeedList.clear();
	aspGenList.clear();

	if (numSecModel > 0)
	{
		for (uint16_t i = 0; i < numSecModel; i++)
			delete secModelList[i];
		numSecModel = 0;
		free(secModelList);
	}
}

void IoTT_SecurityElementList::processLoop()
{
	for (int i = 0; i < numSecModel; i++)
	{
		IoTT_SecurityElementSection * thisModel = secModelList[i];
		thisModel->processLoop();
	}
}

/*----------------------------------------------------------------------------------------------------------------------*/

