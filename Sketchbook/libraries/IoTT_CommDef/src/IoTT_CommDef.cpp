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

