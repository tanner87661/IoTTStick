# IoTTStick
This is the code repository for the IoTT Stick, port modules and hat devices with the source code for the latest release. 
2024/04/19 1.6.7
- Version 1.6.7
- Some problems in 1.6.6 with Loconet, DCC and MQTT communication fixed

2024/04/12 1.6.6
- Version 1.6.6
- adding support for M5 Stick C Plus2 hardware
- Several bug fixes

2023/11/19 1.6.5
- Version 1.6.5
- Improvement of PurpleHat SpeedMagic tab as described in Video #131 https://youtu.be/hhmCFm9BiHA
- Withrottle bug fixes (MU assignment, DCC EX support for changed <t> command
- upgrade to latest library versions of several libraries
- some internal restructuring to reduce main loop cycle time (shorter response times)

2023/5/20 1.6.0
- Version 1.6.0
- WiThrottle server can now read turnouts, routes and roster from DCC EX
- Loconet bridge allows to bridge into Loconet subnet

2023/5/20 1.6.0
- Version 1.6.0
- WiThrottle server can now read turnouts, routes and roster from DCC EX
- Loconet bridge allows to bridge into Loconet subnet

2023/1/5 1.5.19
- Version 1.5.19
- added client report to WiThrottle and Loconet TCP servers
- fixed setup bugs in LED config page

2022/11/22 1.5.18
- Pin change for BrownHat: Now using pin 36 for Rx
- Adjustments to the setup of RedHat current measuring

2022/10/12 1.5.17
- Version 1.5.17
- Added Current gauges to RedHat to display track current (requires special version of DCC EX)

2022/09/18 1.5.16
- Version 1.5.16
- Improved webpage loading (bugfix for timeout problem)
- Added buttons in LED chain configuration to increment and decrement LED numbers when adding/removing Neopixels

2022/09/03 1.5.15
- Version 1.5.15
- switch to No OTA memory model due to code size
- switch to 2.0.4
- bug fix for PurpleHat SpeedMagic when using imperial dimensions
- adding grade and superelevation data to PurpleHat from Stick IMU

2022/07/31
- Version 1.5.13
- Official support for WiThrottle and RedHat++ Shield
- WiThrottle now with mDNS support

2022/05/10
- Version 1.5.12
- Improvements to the speed measurement algorithm, resulting in smoother display curve

2022/05/05
- Version 1.5.11 
- bug fixes for automated speed matching function of PurpleHat
- fixed a bug with upgrading the node configuration to the latest version
- added Plain English translation to LocoNet viewer
- corrected a mistake in the assignment of BDL16 zones

2022/04/09
- Version 1.5.10 with support for PurpleHat and automated speed matching
- added WiThrottle client
- improved LocoNet library
- minor bug fixes

2021/12/17
- Version 1.5.9 with improved support for RedHat
- minor bug fixes

2021/10/23
- Version 1.5.8 Experimental with VoiceWatcher Support
- Watch Video#80 for more information https://youtu.be/dP01Lq9poZw

2022/09/03 1.5.14
- Version 1.5.14
- switch to No OTA memory model due to code size
- switch to 2.0.4
- bug fix for PurpleHat SpeedMagic when using imperial dimensions
- adding grade and superelevation data to PurpleHat from Stick IMU

2022/07/31
- Version 1.5.13
- Official support for WiThrottle and RedHat++ Shield
- WiThrottle now with mDNS support

2022/05/10
Version 1.5.12
Improvements to the speed measurement algorithm, resulting in smoother display curve

2022/05/05
Version 1.5.11
bug fixes for automated speed matching function of PurpleHat
fixed a bug with upgrading the node configuration to the latest version
added Plain English translation to LocoNet viewer
corrected a mistake in the assignment of BDL16 zones

2022/04/09
Version 1.5.10 with support for PurpleHat and automated speed matching
added WiThrottle client
improved LocoNet library
minor bug fixes

2021/10/09
- Version 1.5.8 with support for RedHat++
- improved lbServer/lbClient
- improved LocoNet library
- minor bug fixes

2021/09/06
- Version 1.5.7 Maintenance release
- replaced NMRADcc library with latest version, enhances stability of DCC decoding
- fix an error in the routine that stores the GreenHat configuration data to the flash drive

2021/07/24
- Ready to release version 1.5.6 with new setup screen for GreenHat, including possibility to connect and drive relays and coil driver board
- some minor bug fixes

2021/06/04
- Ready to release new version 1.5.5 adding enabler function to event handler
- adding analog scaler function to event handler to allow for changing ranges and slope of analog data input to adapt sensor input to the levels needed
- adding swi ack command to event handler and GreenHat button command lists
- some minor bug fixes

2021/05/22
- Released new version 1.5.4 with some improvements in lbServer / lbClient
- adding bounce filter for sensor inputs, requiring a 500ms stable input before sending report message to LocoNet

2021/05/06
- Released new version 1.5.3 with some new features for the YellowHat (buttons now can send input report and switch state report directly, no event handler)
- major improvement of the handshake between stick and web app in case of saving and loading disk files

2021/04/29
- added support for input reports and switch reports to button inputs for YellowHat

2021/4/20
- Released new version 1.5.2 to fix a problem with saving event handler data to the Stick. Plus some minor bug fixes and improvements to the GreenHat web app and library

2021/4/12
- Released new version 1.5.1 with finalized GreenHat support and improved (more stable) storage for large configuration files

2021/4/10
- Merged in Prerelease in preparation of new version 1.5.1

2021/3/27
- Releases as 1.5.0 with support for GreenHat, BrownHat and new interfaces (lbServer, lbClient, loopback)

2021/3/20
- added a LocoNet Loopback mode to the communication options. It allows for using devices with input and output functions without physical LocoNet

2021/3/7
- created repository with preview of version 1.5 supporting TCP over LocoNet and GreenHat
