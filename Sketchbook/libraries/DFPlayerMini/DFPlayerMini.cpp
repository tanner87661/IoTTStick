#include "Arduino.h"

#include "DFPlayerMini.h"

DFPlayerMini::DFPlayerMini() {
}

DFPlayerMini::~DFPlayerMini() {
	if (serial != NULL) {
		delete serial;
	}
}

void DFPlayerMini::init(int pinBusy, int pinReceive, int pinTransmit, CallbackMethod whileBusyMethod) {
	if (serial != NULL) {
		delete serial;
	}
	serial = new SoftwareSerial(pinReceive, pinTransmit);
	serial->begin(9600);
	this->pinBusy = pinBusy;
	pinMode(pinBusy, INPUT_PULLUP);
	this->whileBusyMethod = whileBusyMethod;
	reset();
}

void DFPlayerMini::reset() {
#ifdef DEBUG
	Serial.println(F("DFPlayer:reset()"));
#endif

	send(0x0C);
	if (!waitResponse(DFPLAYER_RESET_TIMEOUT, DFPLAYER_CODE_READY)) {
		while (true) {
			Serial.println(F("DFPlayer not responding - REBOOT"));
			delay(5000);
		}
	}
}

void DFPlayerMini::playFile(int fileNumber, int folderNumber) {
#ifdef DFPLAYER_DEBUG
	Serial.print(F("Sound "));
	Serial.print(fileNumber);
	Serial.print(F(" in "));
	Serial.println(folderNumber);
#endif
	if (folderNumber == 0) {
		send(0x03, fileNumber);
	} else {
		send(0x0F, (folderNumber << 8) | fileNumber);
	}
}

bool DFPlayerMini::playFileAndWait(int fileNumber, int folderNumber, int abortTriggerPin, unsigned long timeout) {
	playFile(fileNumber, folderNumber);
	return wait(abortTriggerPin, timeout);
}

void DFPlayerMini::loopFile(byte fileNumber, int folderNumber) {
	playFile(fileNumber, folderNumber);
	loop(true);
}

void DFPlayerMini::loop(bool repeat) {
	send(0x19, !repeat);
}

bool DFPlayerMini::wait(int abortTriggerPin, unsigned long timeout) {
	// Wait for Playing Done code up to timeout
	return waitResponse(timeout, DFPLAYER_CODE_DONE, abortTriggerPin);
}

bool DFPlayerMini::isBusy() {
	return digitalRead(pinBusy) == LOW;
}

void DFPlayerMini::setVolume(int volume) {
#ifdef DFPLAYER_DEBUG
	Serial.print(F("volume="));
	Serial.println(volume);
#endif
	send(0x06, constrain(volume, DFPLAYER_MIN_VOLUME, DFPLAYER_MAX_VOLUME));
}

void DFPlayerMini::stop() {
	send(0x16);
}

bool DFPlayerMini::send(uint8_t cmd, uint16_t argument) {
	// Wait until enough delay from last write (otherwise the latest one it will be ignored)
	while (((unsigned long) (millis() - lastSent)) < DFPLAYER_MIN_FROM_LAST) {
		if (whileBusyMethod != NULL)
			whileBusyMethod();
	}
	uint8_t message[10] =
			{ 0x7E, 0xFF, 0x06, cmd, 0x01, (uint8_t) (argument >> 8), (uint8_t) argument, 0x00, 0x00, 0xEF };

#ifdef DFPLAYER_DEBUG_HEAVY
	Serial.print(millis());
	Serial.print(F(" - Req: "));
#endif

	uint16_t sum = 0;
	for (int i = 1; i < 7; i++) {
		sum -= message[i];
	}
	message[7] = (uint8_t) (sum >> 8);
	message[8] = (uint8_t) sum;

	for (byte i = 0; i < 10; i++) {
#ifdef DFPLAYER_DEBUG_HEAVY
		Serial.print(message[i], 16);
		Serial.print(F(" "));
#endif
		serial->write(message[i]);
		if (whileBusyMethod != NULL)
			whileBusyMethod();
	}
#ifdef DFPLAYER_DEBUG_HEAVY
	Serial.println();
	Serial.print(millis());
	Serial.print(F(" - Res: "));
#endif
	bool completed = waitResponse(DFPLAYER_RESPONSE_TIMEOUT, DFPLAYER_CODE_OK);
	lastSent = millis();
	return completed;
}

bool DFPlayerMini::waitResponse(unsigned long timeout, uint8_t code, int abortTriggerPin) {
	unsigned long startTime = millis();
	// Expected response: byte 0 means "any value"
	uint8_t expectedResponse[10] = { 0x7E, 0xFF, 06, code, 0, 0, 0, 0, 0, 0xEF };

#ifdef DFPLAYER_DEBUG_HEAVY
	Serial.print(F("Expected resp: "));
	for (int i = 0; i < 10; i++) {
		Serial.print(expectedResponse[i], 16);
		Serial.print(F(" "));
	}
	Serial.println();
#endif

	int idx = 0;
	// Repeat until timeout
	while ((unsigned long) (millis() - startTime) <= timeout) {
		if (whileBusyMethod != NULL)
			whileBusyMethod();
		// Read a byte (-1 if nothing found)
		int b = serial->read();
		if (b >= 0) {
			// There was a byte read!
#ifdef DFPLAYER_DEBUG_HEAVY
			Serial.print(b, 16);
			Serial.print(F(" "));
#endif
			if (expectedResponse[idx] == 0 || b == expectedResponse[idx]) {
				// If byte matches expected message (0 is ignored), then go to next byte
				idx++;
			} else {
				// Else reset index
				idx = 0;
			}
			if (idx == sizeof(expectedResponse)) {
#ifdef DFPLAYER_DEBUG_HEAVY
				Serial.print(F("Res in "));
				Serial.println((unsigned long) (millis() - startTime));
#endif
				// If all bytes read - return true
				return true;
			}
		}
		if (abortTriggerPin > 0 && digitalRead(abortTriggerPin) == LOW)
			return false;
	}
#ifdef DFPLAYER_DEBUG_HEAVY
	Serial.print(F("No expected res in "));
	Serial.println((unsigned long) (millis() - startTime));
#endif
	return false;
}
