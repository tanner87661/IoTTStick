/*
IoTT_lbServer.h

MQTT interface to send and receive Loconet messages to and from an MQTT broker

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

#ifndef IoTT_VoiceControl_h
#define IoTT_VoiceControl_h

#include <stdlib.h>
#include <arduino.h>
#include <Math.h>
#include <inttypes.h>
#include <IoTT_CommDef.h>
#include <ArduinoJSON.h>
//#include <M5Unified.h>


/** Audio buffers, pointers and selectors */
typedef struct {
    signed short *buffers[2];
    unsigned char buf_select;
    unsigned char buf_ready;
    unsigned int buf_count;
    unsigned int n_samples;
} inference_t;

class IoTT_VoiceControl
{
public:
	IoTT_VoiceControl();
	~IoTT_VoiceControl();
	void setTxCallback(txFct newCB);
	void beginKeywordRecognition();
	void processKeywordRecognition();
	void loadKeywordCfgJSON(DynamicJsonDocument doc);
private:
   // Member functions
	bool sendStopCmd = true;
	bool sendGoCmd = false;
};

//this is the callback function. Provide a function of this name and parameter in your application and it will be called when a new message is received
//extern void onLocoNetMessage(lnReceiveBuffer * recData) __attribute__ ((weak));

#endif
