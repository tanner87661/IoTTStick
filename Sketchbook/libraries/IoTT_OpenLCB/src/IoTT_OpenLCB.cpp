/*

SoftwareSerial.cpp - Implementation of the Arduino software serial for ESP8266.
Copyright (c) 2015-2016 Peter Lerup. All rights reserved.

Adaptation to LocoNet (half-duplex network with DCMA) by Hans Tanner. 
See Digitrax LocoNet PE documentation for more information

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

#include <IoTT_OpenLCB.h>


cbFct olcbCallback = NULL;

CAN_device_t CAN_cfg;               // CAN Config

IoTT_OpenLCB::IoTT_OpenLCB(gpio_num_t receivePin, gpio_num_t transmitPin)
{
   m_rxPin = receivePin;
   m_txPin = transmitPin;
   que_rdPos = que_wrPos = 0;
 }

IoTT_OpenLCB::~IoTT_OpenLCB() 
{
	
}

void IoTT_OpenLCB::begin() 
{
  CAN_cfg.speed = CAN_SPEED_125KBPS;
  CAN_cfg.tx_pin_id = m_txPin;
  CAN_cfg.rx_pin_id = m_rxPin;
  CAN_cfg.rx_queue = xQueueCreate(rx_queue_size, sizeof(CAN_frame_t));
  // Init CAN Module
  ESP32Can.CANInit();

}

uint16_t IoTT_OpenLCB::lnWriteMsg(lnTransmitMsg txData)
{
    uint8_t hlpQuePtr = (que_wrPos + 1) % queBufferSize;
//    Serial.printf("OLCB Read: %i Write: %i Hlp: %i \n", que_rdPos, que_wrPos, hlpQuePtr); 
    if (hlpQuePtr != que_rdPos) //override protection
    {
		transmitQueue[hlpQuePtr].lnMsgSize = txData.lnMsgSize;
		transmitQueue[hlpQuePtr].reqID = txData.reqID;
		transmitQueue[hlpQuePtr].reqRecTime = micros();
		memcpy(transmitQueue[hlpQuePtr].lnData, txData.lnData, lnMaxMsgSize);// txData.lnMsgSize);
		que_wrPos = hlpQuePtr;
		return txData.lnMsgSize;
	}
	else
	{
		Serial.println("OpenLCB ESP Serial Write Error. Too many messages in queue");
		return -1;
	}
}

uint16_t IoTT_OpenLCB::lnWriteMsg(lnReceiveBuffer txData)
{
    uint8_t hlpQuePtr = (que_wrPos + 1) % queBufferSize;
//    Serial.printf("OLCB Read: %i Write: %i Hlp: %i \n", que_rdPos, que_wrPos, hlpQuePtr); 
    if (hlpQuePtr != que_rdPos) //override protection
    {
		transmitQueue[hlpQuePtr].lnMsgSize = txData.lnMsgSize;
		transmitQueue[hlpQuePtr].reqID = txData.reqID;
		transmitQueue[hlpQuePtr].reqRecTime = micros();
		memcpy(transmitQueue[hlpQuePtr].lnData, txData.lnData, lnMaxMsgSize);// txData.lnMsgSize);
		que_wrPos = hlpQuePtr;
		return txData.lnMsgSize;
	}
	else
	{
		Serial.println("OpenLCB ESP Serial Write Error. Too many messages in queue");
		return -1;
	}
}

void IoTT_OpenLCB::setOlcbCallback(cbFct newCB, bool useOnOut)
{
	olcbCallback = newCB;
	useAlways = useOnOut;
}

void IoTT_OpenLCB::processLNMsg(lnReceiveBuffer * recData)
{
	updateNodes(recData);
	if (olcbCallback != NULL)
		olcbCallback(recData);
}

void IoTT_OpenLCB::updateNodes(lnReceiveBuffer * recData)
{
  olcbMsg thisMsg;
  if (gc_format_parse_olcb(&thisMsg, recData) >= 0)
  {
	Serial.printf("From %.3X ", thisMsg.srcAlias);
	if (thisMsg.dlc > 0)
	{
		Serial.printf("Data %.2X ", thisMsg.olcbData.u8[0]);
		for (int i = 1; i < thisMsg.dlc; i++)
			Serial.printf("%.2X ", thisMsg.olcbData.u8[i]);
	}
	  switch (thisMsg.MTI)
	  {
		case 0x100: Serial.println("Initialization Complete"); break;
		case 0x488: Serial.println("Verify Node ID Number Addressed"); break;
		case 0x490: Serial.println("Verify Node ID Number Global"); break;
		case 0x170: Serial.println("Verified Node ID Number"); break;
		case 0x068: Serial.println("Optional Interaction Rejected"); break;
		case 0x0A8: Serial.println("Terminate Due to Error"); break;
		case 0x828: Serial.println("Protocol Support Inquiry"); break;
		case 0x668: Serial.println("Protocol Support Reply"); break;
		case 0x8F4: Serial.println("Identify Consumer"); break;
		case 0x4A4: Serial.println("Consumer Identify Range"); break;
		case 0x4C7: Serial.println("Consumer Identified w validity unknown"); break;
		case 0x4C4: Serial.println("Consumer Identified as currently valid"); break;
		case 0x4C5: Serial.println("Consumer Identified as currently invalid"); break;
		case 0x4C6: Serial.println("Consumer Identified (reserved)"); break;
		case 0x914: Serial.println("Identify Producer"); break;
		case 0x524: Serial.println("Producer Identify Range"); break;
		case 0x547: Serial.println("Producer Identified w validity unknown"); break;
		case 0x544: Serial.println("Producer Identified as currently valid"); break;
		case 0x545: Serial.println("Producer Identified as currently invalid"); break;
		case 0x546: Serial.println("Producer Identified (reserved)"); break;
		case 0x5E8: Serial.println("Traction Control 8?"); break;
		case 0x5EB: Serial.println("Traction Control B?"); break;
		case 0x1E8: Serial.println("Traction Control Reply"); break;
		case 0x968: Serial.println("Identify Events Addressed"); break;
		case 0x970: Serial.println("Identify Events Global"); break;
		case 0x594: Serial.println("Learn Event"); break;
		case 0x5B4: Serial.println("Producer/Consumer Event Report"); break;
		case 0x820: Serial.println("Xpressnet"); break;
		case 0xDE8: Serial.println("Simple Node Ident Info Request"); break;
		case 0xA08: Serial.println("Simple Node Ident Info Reply"); break;
		case 0xA28: Serial.println("Datagram Received OK"); break;
		case 0xA48: Serial.println("Datagram Rejected"); break;
		case 0xCC8: Serial.println("Stream Initiate Request"); break;
		case 0x868: Serial.println("Stream Initiate Reply"); break;
		case 0x888: Serial.println("Stream Data Proceed"); break;
		case 0x8A8: Serial.println("Stream Data Complete"); break;
//		case 0x: Serial.println(""); break;
		
		default: Serial.printf("Unknown MTI %i\n", thisMsg.MTI); break;
	  }
  }
}

void IoTT_OpenLCB::initOpenLCBAccess()
{
}

void IoTT_OpenLCB::handleLNIn(uint8_t inData, uint8_t inFlags)
{
}

void IoTT_OpenLCB::processLoop()
{
	processLNReceive();
	processLNTransmit();
}

void IoTT_OpenLCB::processLNReceive()
{
  if (xQueueReceive(CAN_cfg.rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE) 
  {

    if (rx_frame.FIR.B.FF == CAN_frame_std) 
    {
//      Serial.printf("Invalid standard frame received");
      return;
    }

    if (rx_frame.FIR.B.RTR == CAN_RTR) 
    {
//		Serial.printf(" RTR from 0x%08X, DLC %d\r\n", rx_frame.MsgID,  rx_frame.FIR.B.DLC);
    }
    else 
    {
//		Serial.printf("from alias 0x%03X, MTI 0x%05X, DLC %d, Data ", (rx_frame.MsgID & 0xFFF), ((rx_frame.MsgID & 0xFFFFF000) >> 12), rx_frame.FIR.B.DLC);
		lnInBuffer.msgType = OpenLCB;
		if (gc_format_generate(&rx_frame, &lnInBuffer, false) >= 0)//change to gridconnect protocol
		{
			processLNMsg(&lnInBuffer); //send to USB Injector
		}
//		else
//			Serial.println("invalid message");
    }
  }
}

void IoTT_OpenLCB::processLNTransmit()
{
	CAN_frame_t tx_frame;
	lnReceiveBuffer thisBuffer;
	if (que_wrPos != que_rdPos)
	{
//		Serial.println("OLCB Tx Message");
	    uint8_t hlpQuePtr = (que_rdPos + 1) % queBufferSize;
	    //send it here
	    if (gc_format_parse_tx(&transmitQueue[hlpQuePtr], &tx_frame) >= 0)
	    {
			ESP32Can.CANWriteFrame(&tx_frame);
			if (useAlways)
			{
				lnReceiveBuffer txDataCopy;
				txDataCopy.msgType = transmitQueue[hlpQuePtr].msgType;
				txDataCopy.lnMsgSize = transmitQueue[hlpQuePtr].lnMsgSize;
				txDataCopy.reqID = transmitQueue[hlpQuePtr].reqID & 0x3FFF; //set flag to transmit to Hat
				txDataCopy.reqRecTime = micros();
				memcpy(txDataCopy.lnData, transmitQueue[hlpQuePtr].lnData, lnMaxMsgSize); //txData.lnMsgSize);
				processLNMsg(&txDataCopy); //send to App, but not USB
			}
		}
		que_rdPos = hlpQuePtr;
	}
}

	
bool IoTT_OpenLCB::canEnabled()
{
}
