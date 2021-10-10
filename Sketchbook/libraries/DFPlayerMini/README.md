# DFPlayerMini

## Overview
This is a reliable, responsive driver for **`DFPlayer Mini`** sound module for **`Arduino`** . It guarantees stability 
and responsiveness, due to respecting the two-way communication protocol with the module, taking care of required wait cycles, 
and letting you do your own stuff while waiting.

It does **not** require interrupts or multithreading, because if you use the `whileBusyMethod` callback wisely, you will be able
to achieve great responsiveness.

## Using the Driver
### Non-Idle Waits
The driver provides you with an optional callback method support, allowing you to do your own stuff while the driver is
waiting for a timeout or performing a delay. This way you can achieve great responsiveness of your system, near to the one achieved
with multi-threaded systems. 

These operations should be used for detecting non-interrupt-backed events, or driving displays and/or other devices, as long as
you keep them **fast**! Do **not** attempt to call a DFPlayerMini driver method from it, or you will trigger a recursion!

```C++
// Do your time-critical stuff here - KEEP IT SHORT AND FAST!
void readSensorsAndDoStuff() {
   actionDetected = readSensorMagic();
   updateDisplayMagic();
}
...

// Initialize the driver, passing the busy-wait method
DFPlayerMini player;
player.init(pinBusy, pinReceive, pinTransmit, readSensorsAndDoStuff);
...

// The readSensorsAndDoStuff() will be called from within the player, even when playing or waiting
player.playFileAndWait(MY_SOUND);

```

### Addressing a Sound and Achieving a Gapless Play
All the sounds should be stored on a FAT formatted SD-card. The files can be organized in folders, sorted in a natural
(sort-by-name) order. You address the file by its number (starting from 1) and its folder (also starting from 1).

However, if you need to achieve a **gapless play**, you should use only **WAV format**, and write all the files to the **root** 
of the SD-card, rather than using folders. Furthermore, it might be a good idea to **format the SD-card** each time before copying 
the files, and to **copy the files in their order of appearence**. Using Windows' Copy/Paste will not guarantee you the proper order,
so you might want to use the [SDCardRecorder Utility](https://github.com/jonnieZG/SDCardRecorder), a small utility written in Java,
that will also generate `#define` entries for each sound.

The module requires some time to respond to a PLAY command, so if you rely just on `playAndWait` to achieve a gapless play, you might
still notice a short gap. The only way to solve that issue with the DFPlayer Mini, is to use `play` combined with your own timing, and
trigger the next sample slightly before the previous is done.

If you did all as described, and there is still a pause between playing two files, then try with a **faster SD-card**.

### Supported Sound Formats
The DFPlayerMini supports both WAV and MP3 formats. When using WAV files, you should make sure to remove any metadata
from the WAV file, since the player will interpret it as noise.

I have found the MP3[44100 Hz, Mono, 32-bit float, VBR] and WAV[44100 Hz, Mono, 16-bit] to be working great, while MP3[22050 CBR]
to cause erratic behavior in form of garbled response messages. So, if you notice that the module stars acting funky with certain
sound files, turn on the DFPLAYER_DEBUG_HEAVY mode and see if the module is returning proper responses.

## Wiring the Module

### Signal Pins
The driver uses two-way communication with the module. Therefore, you must connect module's `RX` pin (2) with a `pinTransmit` on
the microcontroller, and module's `TX` pin (3) with a `pinReceive` on the controller.

In cases where the Arduino is driven at 5V, you **must** put a 1k resistor between data-out on Arduino and `RX` of the player. `BUSY`
and `DX` must be connected to their counterparts on the Arduino **directly**, even if you drive the module at 5V! This is because the
module *always* uses 3.3V levels for its signal lines.

`BUSY` pin (16) on the module is optional, and is used only by the `isBusy()` method. For the `playAndWait` method, the driver waits
for a `DFPLAYER_CODE_DONE` response, instead of reading the `BUSY` signal.

> **NOTE:** If you plan to use the `BUSY` signal from the outside to figure out if a sample is currently being played, keep in mind 
> that there is a non-derterministic delay between sending a play command, and the `BUSY` signal being actually activated. 

### Powering the Module
According to the module specifications, it requires input voltage between 3.2V and 5.0V. However, if you use a breadboard, the
contact resistance of power lines will be around 0.2 Ohm on each contact, and since the module can draw well above 0.5A at full 
power, the voltage drop will cause erratic behavior, when the source voltage is not high enough.

Therefore, ensure a **stable power source** and **keep the contact resistance as low as possible**, especially in cases when power
leads are not soldered to the module.

------------------------

## Library Reference

### `DFPlayerMini` class

- `DFPlayerMini::DFPlayerMini()` - Constructor.

- `DFPlayerMini::~DFPlayerMini()` - Destructor.

- `void` `DFPlayerMini::init(int pinBusy, int pinReceive, int pinTransmit, CallbackMethod whileBusyMethod = NULL)` -
   Initializes the driver. 
   `pinReceived` - input from player's TX;
   `pinTransmit` - output to player's RX; 
   `whileBusyMethod` - optional pointer to a method that will be kept called in wait cycles

- `void` `DFPlayerMini::reset()` - Reset the module.

- `void` `DFPlayerMini::playFile(int fileNumber, int folderNumber = 0)` - Play a specified file in specified folder number.
   Folders are optional, and discouraged if you need a gapless play!

- `bool` `DFPlayerMini::playFileplayFileAndWait(int fileNumber, int folderNumber = 0, int abortTriggerPin = 0, unsigned long timeout =
	DFPLAYER_WAIT_TIMEOUT)` -
   Play a specified file and wait for it to be completed. 
   `folderNumber` - optional (see above);
   `abortTriggerPin` - optional, specifies a pin connected to a button (active-low) that can be used to abort the waiting;
	`timeout` - optional timeout (defaults to DFPLAYER_WAIT_TIMEOUT);
   returns `true` if completed successfuly, or `false` if interrupted or failed
   
- `void` `DFPlayerMini::loopFile(byte fileNumber, int folderNumber = 0)` - Play a specified file in a loop. Non-blocking operation.

- `void` `DFPlayerMini::loop(bool repeat = true)` - Enables or disables looping of the current sample.

- `void` `DFPlayerMini::setVolume(int volume)` - Set the volume.

- `void` `DFPlayerMini::stop()` - Stop playing any current sound.

- `bool` `DFPlayerMini::isBusy()` - Returns `true` if a sound is currently being played (inverted state of the BUSY signal).

- `bool` `DFPlayerMini::wait(int abortTriggerPin = 0, unsigned long timeout = DFPLAYER_WAIT_TIMEOUT)` - 
   Wait for a `DONE` response. Internally used by `playFileAndWait`.

------------------------

## External References
* [DFPlayer Module Documentation](https://www.dfrobot.com/wiki/index.php/DFPlayer_Mini_SKU:DFR0299)

------------------------

## Version History

* `1.0.0 (2017-02-17)`: Original release

