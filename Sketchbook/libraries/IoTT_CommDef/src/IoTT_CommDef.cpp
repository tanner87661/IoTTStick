#include <IoTT_CommDef.h>

void setXORByte(uint8_t * msgData)
{
	uint8_t msgLen = ((msgData[0] & 0x60) >> 4) + ((msgData[0] & 0x80) >> 7); //-1 for indexing
	if (msgLen == 7)
		msgLen = (msgData[1] & 0x7F) -1; //-1 for indexing
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

