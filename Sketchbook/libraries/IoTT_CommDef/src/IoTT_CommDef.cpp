#include <IoTT_CommDef.h>

extern std::vector<wsClientInfo> globalClients; // a list to hold all clients when in server mode

void setXORByte(uint8_t * msgData)
{
	uint8_t msgLen = ((msgData[0] & 0x60) >> 4) + ((msgData[0] & 0x80) >> 7); //-1 for indexing
	if (msgLen == 7)
		msgLen = (msgData[1] & 0x7F)-1; //-1 for indexing
	msgData[msgLen] = 0xFF;
	for (uint8_t i = 0; i < msgLen; i++)
		msgData[msgLen] ^= msgData[i];
}

bool getXORCheck(uint8_t * msgData, uint8_t targetLen)
{
	uint8_t msgLen = ((msgData[0] & 0x60) >> 4) + ((msgData[0] & 0x80) >> 7); //-1 for indexing
	if (msgLen == 7)
		msgLen = (msgData[1] & 0x7F) -1; //-1 for indexing
//	Serial.printf("Msg Len %i %i\n", targetLen, msgLen+1);
	if (targetLen > 0)
		if (targetLen != (msgLen+1))
			return false;
	uint8_t xorResult = 0;
	for (uint8_t i = 0; i < msgLen; i++)
		xorResult ^= msgData[i];
//	Serial.printf("XOR %2X %2X \n", xorResult, msgData[msgLen]);
	return ((xorResult ^ msgData[msgLen]) == 0xFF);
}

int8_t getWSClient(int8_t withID)
{
  for (int i = 0; i < globalClients.size(); i++)
  {
    if (globalClients[i].wsClient)
      if (globalClients[i].wsClient->id() == withID)
         return i;
  }
  return -1;
}

int8_t getWSClientByPage(uint8_t startFrom, char * toWebPage)
{
//	Serial.printf("look %i %s:\n", startFrom, toWebPage);
  if (startFrom >= globalClients.size()) return -1;
  for (int i = startFrom; i < globalClients.size(); i++)
  {
    if (globalClients[i].wsClient)
    {
//		Serial.printf("%s %s %i\n", toWebPage, globalClients[i].pageName, strcmp(toWebPage, globalClients[i].pageName));
		if (strcmp(toWebPage, globalClients[i].pageName) == 0)
			return i;
	}
  }
//  Serial.println("Nada");
  return -1;
}

/*
bool verifySyntax(uint8_t * msgData)
{
	if ((msgData[0] & 0x80) == 0)
		return false;
	uint8_t msgLen = ((msgData[0] & 0x60) >> 4) + ((msgData[0] & 0x80) >> 7); //-1 for indexing
	if (msgLen == 7)
		msgLen = (msgData[1] & 0x7F) -1; //-1 for indexing
	for (uint8_t i = 1; i < msgLen; i++)
		if ((msgData[i] & 0x80) > 0)
			return false;
	return getXORCheck(msgData, msgLen + 1);
}

void dispMsg(uint8_t * msgData, uint8_t targetLen)
{
	uint8_t msgLen = ((msgData[0] & 0x60) >> 4) + 2;
	if (msgLen == 8)
		msgLen = (msgData[1] & 0x7F); 
	Serial.printf("Msg %i Len %i\n", targetLen, msgLen+1);
	for (uint8_t i = 0; i < msgLen; i++)
		Serial.printf("%2X ", msgData[i]);
	Serial.println();
}

void dispSlot(uint8_t * slotBytes)
{
	
	for (uint8_t i = 0; i < 10; i++)
		Serial.printf("%2X ", slotBytes[i]);
	Serial.println();
}
*/

void untokstr(char* strList[], uint8_t listLen, char* inpStr, const char* token)
{
	byte numStr = 0;
	char* tokPos = strtok(inpStr, token);
	while ((tokPos) && (numStr < listLen))
	{
		strList[numStr] = tokPos;
		numStr++;
		tokPos = strtok(NULL, token);
	}	
	strList[numStr] = NULL;
}

rmsBuffer::rmsBuffer(uint8_t bufSize)
{
//	Serial.printf("Buffer %i\n", bufSize);
	bufferSize = bufSize;
	rmsData = (float_t*) realloc(rmsData, bufSize * sizeof(float_t));
}

void rmsBuffer::addVal(uint16_t newVal)
{
	wrIndex = (wrIndex + 1) % bufferSize; //initialized with -1
	float_t thisVal = sq((float_t)newVal);
	rmsData[wrIndex] = thisVal;
}

float_t rmsBuffer::getRMSVal()
{
	if (wrIndex < 0)
		return -1;
	float_t sumVal = 0;
	for (uint8_t i = 0; i < bufferSize; i++)
		sumVal += rmsData[i]; 
	return sqrt(sumVal / bufferSize);
}

void rmsBuffer::clrBuffer()
{
	wrIndex = -1;
	for (uint8_t i = 0; i < bufferSize; i++)
		rmsData[i] = 0;
}

bool isSameMsg(lnReceiveBuffer* msgA, lnReceiveBuffer* msgB)
{
	if (msgA->lnMsgSize != msgB->lnMsgSize)
		return false;
	for (uint8_t i = 0; i < msgA->lnMsgSize; i++)
		if (msgA->lnData[i] != msgB->lnData[i])
			return false;
	return true;
}

bool parseDCCExNew(char* inpStr, lnTransmitMsg* inpStatus, std::vector<ppElement> * paramList)
{
/*
  inpStatus variable usage
    msgType = DCCEx;
    lnMsgSize is the index of the parameter sent for processing
    lnData[lnMaxMsgSize] holds the string data of the current parameter
    requestID used as lnBufferPtr, counts the incoming bytes in a message paramter, gets reset after processing parameter
    reqRecTime used instead of bitRecStatus as parameter counter. 0xFF means invalid, awaiting new message
*/ 	
	bool returnVal = false;
//	Serial.println(*inpStr);
	switch (*inpStr)
	{
		case '<' : //start new message
//			Serial.println("BEGIN");
			if (inpStatus->reqRecTime != 0xFF) //receiving in progress, something is wrong
			{
				Serial.println("Old Command not complete");
				inpStatus->requestID = 0xFF; //invalid message
//				processLNMsg(&lnInBuffer); //get rid of previous (invalid) message
			}
			inpStatus->reqRecTime = 0; //get ready to receive opcode
			inpStatus->requestID = 0; //reset incoming byte counter
			inpStatus->msgType = DCCEx;
			inpStatus->lnMsgSize = 0; //byte ctr for lnData in param 0, temporarily used for string status
			break;
		case '>' : //terminate message
		{
			if (inpStatus->reqRecTime < 0xFF) //valid data
			{
				inpStatus->lnData[inpStatus->requestID] = '\0'; //terminate string
				inpStatus->lnMsgSize = inpStatus->reqRecTime; //set parameter number
				if (inpStatus->requestID > 0) //there is a parameter received and waiting for processing
				{
					parseDCCExParamNew(inpStatus, paramList);
					inpStatus->reqRecTime++;
				}
				paramList->at(0).numParams = inpStatus->reqRecTime; //set number of valid paramters
				returnVal = true;
//				processDCCEXMsgNew(paramList);
				inpStatus->reqRecTime = 0xFF; //set status to invalid data, awaiting start of message
				inpStatus->requestID = 0; //reset byte counter
//				Serial.println("END");
			}
		}
			break;
		default  : //must be data (max 48 bytes)
		{
			if (*inpStr == '"') 
			{
				inpStatus->lnMsgSize = (inpStatus->lnMsgSize ^ 0x01) & 0x01; //toggle string status
				return returnVal;
			}
			switch (inpStatus->reqRecTime)
			{
				case 0: //waiting for single byte command code
				{
					if ((*inpStr != ' ') && (*inpStr != '|'))
					{
						inpStatus->lnData[0] = *inpStr; //put opcode
						inpStatus->lnData[1] = '\0'; //teminate string
						inpStatus->lnMsgSize = inpStatus->reqRecTime; //param index = 0
						parseDCCExParamNew(inpStatus, paramList);
						inpStatus->requestID = 0; //get ready for next section
						inpStatus->reqRecTime++; //increment parameter number
					} //otherwise ignore
				} break;
				case 0xFF: //non protocol data, ignore
//					Serial.println("No Data");
					break;
				default:
				{
//					Serial.println("Data");
					if (inpStatus->reqRecTime != 0xFF) //message has started
					{
						if (((*inpStr == ' ') || (*inpStr == '|')) && ((inpStatus->lnMsgSize & 0x01) == 0))
						{
							if (inpStatus->requestID > 0) //this is at the end of parameter
							{
								inpStatus->lnData[inpStatus->requestID] = '\0'; //string termination
								inpStatus->lnMsgSize = inpStatus->reqRecTime; //set parameter number
								parseDCCExParamNew(inpStatus, paramList); //process parameter
								inpStatus->reqRecTime++; //increase parameter number
								inpStatus->requestID = 0; //get ready for next paramter
								inpStatus->lnMsgSize = 0; //reet string status for next param
							}
						}
						else
						{
							inpStatus->lnData[inpStatus->requestID] = *inpStr;
							if (inpStatus->requestID < lnMaxMsgSize-1)
								inpStatus->requestID++;
							else
								Serial.println("InBuffer overflow. Partial message loss");
						}
					}
				} break;
			}
			break;
		}
	}
	return returnVal;
}

int parseDCCExParamNew(lnTransmitMsg* thisEntry, std::vector<ppElement> * paramList)
{
	char* newStr = (char*)&thisEntry->lnData[0];
	ppElement * thisParam;
	while (paramList->size() < (thisEntry->lnMsgSize) + 1)
	{
		ppElement newParam;
		newParam.dataType = 0xFF;
		newParam.paramNr = thisEntry->lnMsgSize;
		newParam.payload.strPtr = NULL;
//		Serial.println("Create Param");
		paramList->push_back(newParam);
	}
	thisParam = (ppElement*)&(paramList->at(thisEntry->lnMsgSize)); //thisEntry->lnMsgSize]);

	thisParam->paramNr = thisEntry->lnMsgSize;

	if (thisParam->dataType == 0) //char*
	{
//		Serial.println("delete str");
		free (thisParam->payload.strPtr);
		thisParam->dataType = 0xFF;
		thisParam->payload.strPtr = NULL;
	}
	if (thisEntry->lnMsgSize == 0) //OpCode
	{
		thisParam->dataType = 1; //char[1]
		thisParam->payload.strVal[0] = thisEntry->lnData[0];
		thisParam->payload.strVal[1] = '\0';
	}
	else
	{
		uint8_t paramLen = strlen(newStr);
		uint8_t decPtCtr = 0;
		uint8_t digitsCtr = 0;
		for (uint8_t i = 0; i < paramLen; i++)
		{
			if (newStr[i] == '.') decPtCtr++;
			if (isDigit(newStr[i])) digitsCtr++;
		} 
		if (((decPtCtr + digitsCtr) == paramLen) && (decPtCtr <= 1)) //numerical value
		{
			if (decPtCtr == 0) //long
			{
				thisParam->payload.longVal = atoi(newStr);
				thisParam->dataType = 10; //integer
			}
			else //float
			{
				thisParam->payload.floatVal = atof(newStr);
				thisParam->dataType = 20; //float
			}
		}
		else //string
		{
			if (paramLen < 4)
			{
				strcpy(thisParam->payload.strVal, newStr);
				thisParam->dataType = strlen(newStr); //char[4]
//				Serial.printf("Short str %i bytes\n", strlen(newStr)+1);
			}
			else
			{
//				Serial.printf("assign str %i bytes\n", strlen(newStr)+1);
				thisParam->dataType = 0; //char*
				thisParam->payload.strPtr = (char *) malloc (strlen(newStr)+1); 
				strcpy(thisParam->payload.strPtr, newStr);
			}
		}
	}

//	Serial.printf("Param: %i of %i Type: %i Value: %s\n", thisParam->paramNr, paramList->size(), thisParam->dataType, newStr);
	return 0;
}

