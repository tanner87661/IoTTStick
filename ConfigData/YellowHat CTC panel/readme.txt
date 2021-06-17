Watch videos #70 and #71 for details of this configuration

Make sure to upgrade your IoTT Stick to software version 1.5.5 or higher in order to use all functions provided in this example.

Load the json files to configure your IoTT Stick as follows:

CTCLEDSetup.json 
- This file has the LED setup of the CTC panel that supports track occupancy only, no route status display
- It also support LEDs for turnout position display
- The two last LEDs set the poser indicator LED and the remote mode LED

CTCLEDFullSetup.json
- This file has the full implementation of the LED chain including support for route status display
- to work, it needs to have the event handlers loaded and active

CTCBtnEvtTargetRemote.json
- This file has all 62 event handler settings. 
- See slide 66 in the documentation directory for a guide where to find what functionality

From Video #71:

CTCBtnEvtSigCTC.json
- This file has all 94 event handler settings as described in Video #71. Load it into the YellowHat driving the CTC panel

CTCBtnEvtSigCTCBlue.json
- This is the LED chain definition for the signals. Load it into the BlueHat driving the signals


To load any of the json data files, open the Node Configuration tab, and use the Load File function at the bottom of the page. 
When loading the files, the Stick is sorting the files based on the content and will not overwrite other data. Your overall settings of the Stick will not be affected, the files are only replacing LED chain and Event Handler data.
