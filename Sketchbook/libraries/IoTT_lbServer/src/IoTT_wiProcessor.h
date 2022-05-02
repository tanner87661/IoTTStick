#ifndef IoTT_wiProcessor_h
#define IoTT_wiProcessor_h

#include <stdlib.h>
#include <arduino.h>
#include <Math.h>
#include <inttypes.h>
#include <WiFi.h>
#include <IoTT_CommDef.h>
#include <IoTT_DigitraxBuffers.h>
#include <ArduinoJSON.h>
#include <AsyncTCP.h>

class IoTT_wiProcessor
{
public:
	IoTT_wiProcessor();
	~IoTT_wiProcessor();
	bool processWIMessage(AsyncClient* client, char * data);
	bool processWIClientMessage(AsyncClient* client, char * data);
	bool processWIServerMessage(AsyncClient* client, char * data);

private:

};

#endif

