/*
 * DFPlayerMini by JonnieZG - A reliable, responsive driver for DFPlayer Mini sound module for Arduino
 *
 * - Requires both TX and RX pins to be connected, and optionally the BUSY pin (only if you plan to use isBusy() from outside).
 * - Supports optional input key for aborting playAndWait (active-low)
 * - Supports optional whileBusy callback that is executed while the module waits for something, or just performing a delay
 *
 * Use the optional whileBusy callback method to perform fast operations in order to achieve responsiveness of your system
 * while the driver is wait states (waiting for a response or a timeout). These operations should be used for detecting
 * non-interrupt-backed events, or driving displays and/or other devices, as long as you keep them FAST! Do NOT attempt to
 * call a DFPlayerMini driver method from it, or you will trigger a recursion!
 */

#ifndef DFPLAYERMINI_H_
#define DFPLAYERMINI_H_
#include "Arduino.h"
#include <SoftwareSerial.h>

// Basic debug-mode switch
#define DFPLAYER_DEBUG

// Heavy debug-mode switch
//#define DFPLAYER_DEBUG_HEAVY

// Maximum allowed time for playAndWait - YOU ARE FREE TO MODIFY THIS ONE TO YOUR NEEDS
#define DFPLAYER_WAIT_TIMEOUT		6000

// ---- DO NOT MESS AROUND WITH THESE! ----

// Maximum allowed delay for receiving response
#define DFPLAYER_RESPONSE_TIMEOUT	200
// Minimum delay between two sequential commands
#define DFPLAYER_MIN_FROM_LAST		100
// Maximum allowed delay for waking from a reset
#define DFPLAYER_RESET_TIMEOUT		3000

#define DFPLAYER_MIN_VOLUME		0
#define DFPLAYER_MAX_VOLUME		30

// Response codes
#define DFPLAYER_CODE_OK		0x41
#define DFPLAYER_CODE_ERROR		0x40
#define DFPLAYER_CODE_DONE		0x3D
#define DFPLAYER_CODE_READY		0x3F

extern "C" {
typedef void (*CallbackMethod)(void);
}

class DFPlayerMini {
public:
	DFPlayerMini();
	~DFPlayerMini();

	// Initialize sound player.
	// pinReceived - input from player's TX, pinTransmit = output to player's RX
	// CallbackMethod - optional pointer to a method that will be kept called in wait cycles
	void init(int pinBusy, int pinReceive, int pinTransmit, CallbackMethod whileBusyMethod = NULL);

	// Reset the module
	void reset();

	// Play a specified file in specified folder number. Folders are optional, and discouraged if you need a gapless play!
	void playFile(int fileNumber, int folderNumber = 0);

	// Play a specified file and wait for it to be completed.
	// folderNumber - optional (see above)
	// abortTriggerPin - optional, specifies a pin connected to a button (active-low) that can be used to abort the waiting
	// timeout - optional timeout (defaults to DFPLAYER_WAIT_TIMEOUT)
	bool playFileAndWait(int fileNumber, int folderNumber = 0, int abortTriggerPin = 0, unsigned long timeout =
	DFPLAYER_WAIT_TIMEOUT);

	// Play a specified file in a loop
	void loopFile(byte fileNumber, int folderNumber = 0);

	// Set the volume
	void setVolume(int volume);

	// Stop playing any current sound
	void stop();

	// Returns true if a sound is currently being played (inverted state of the BUSY signal)
	bool isBusy();

	// Wait for a "DONE" response. Internally used by playFileAndWait.
	bool wait(int abortTriggerPin = 0, unsigned long timeout = DFPLAYER_WAIT_TIMEOUT);

	// Enables or disables looping of the current sample.
	void loop(bool repeat = true);

private:
	// Waits for a response with given code. Returns true if OK, false if not OK in given timeout
	bool waitResponse(long unsigned timeout, uint8_t code, int abortTriggerPin = 0);
	SoftwareSerial *serial = NULL;

	bool send(uint8_t cmd, uint16_t argument = 0);
	int pinBusy = 0;

	bool isPlaying = false;
	unsigned long lastSent = 0;

	CallbackMethod whileBusyMethod;
};

#endif
