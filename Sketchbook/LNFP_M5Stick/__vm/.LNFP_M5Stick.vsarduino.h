/* 
	Editor: https://www.visualmicro.com/
			This file is for intellisense purpose only.
			Visual micro (and the arduino ide) ignore this code during compilation. This code is automatically maintained by visualmicro, manual changes to this file will be overwritten
			The contents of the _vm sub folder can be deleted prior to publishing a project
			All non-arduino files created by visual micro and all visual studio project or solution files can be freely deleted and are not required to compile a sketch (do not delete your own code!).
			Note: debugger breakpoints are stored in '.sln' or '.asln' files, knowledge of last uploaded breakpoints is stored in the upload.vmps.xml file. Both files are required to continue a previous debug session without needing to compile and upload again
	
	Hardware: M5StickC (m5stack_m5stack_stickc), Platform=esp32, Package=m5stack
*/

#if defined(_VMICRO_INTELLISENSE)

#ifndef _VSARDUINO_H_
#define _VSARDUINO_H_
#include <arduino.h>
#include <pins_arduino.h> 
#include "..\LNFP_M5Stick.ino"
#include "..\ConfigLoader.ino"
#include "..\DCC_In_Handler.ino"
#include "..\LocoNet_In_Handler.ino"
#include "..\LocoNet_Out_Handler.ino"
#include "..\M5Functions.ino"
#include "..\MQTTGateway.ino"
#include "..\MQTT_In_Handler.ino"
#include "..\OpenLCB_In_Handler.ino"
#include "..\WebServer.ino"
#include "..\WifiAccess.ino"
#endif
#endif
