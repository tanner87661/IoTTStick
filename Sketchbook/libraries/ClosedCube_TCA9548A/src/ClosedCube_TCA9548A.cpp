/*

Arduino library for Arduino library for Texas Instruments TCA9548A 8-Channel I2C Switch/Multiplexer
version 2019.3.23

---

Copyright (c) 2019, ClosedCube
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, 
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, 
   this list of conditions and the following disclaimer in the documentation and/or 
   other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors 
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <Wire.h>

#include "ClosedCube_TCA9548A.h"

ClosedCube::Wired::TCA9548A::TCA9548A() {
}

ClosedCube::Wired::TCA9548A::TCA9548A(uint8_t address):_address(address) {    
}

void ClosedCube::Wired::TCA9548A::address(uint8_t address) {    
	_address = address;
}

uint8_t ClosedCube::Wired::TCA9548A::getChannel() {
	return _currentChannel;
}

uint8_t ClosedCube::Wired::TCA9548A::selectChannel(uint8_t channel) {
	uint8_t result = 0xff;
	if (channel >= 0 && channel < TCA9548A_MAX_CHANNELS) {
		Wire.beginTransmission(_address);
		Wire.write( ((uint8_t)1) << (channel+1));
		
		_currentChannel = channel;
		result = Wire.endTransmission();
	} 
	return result;
}

uint8_t ClosedCube::Wired::TCA9548A::nextChannel() {
	uint8_t nextChannel = _currentChannel + 1;
	if (nextChannel > (TCA9548A_MAX_CHANNELS-1)) {
		nextChannel = 0;	
	}

	return selectChannel(nextChannel);
}
 
