# IoTTStick
This is the development repository for the IoTT Stick, port modules and hat devices with the source code for the latest release. 

2022/04/09

Version 1.5.11 with support for PurpleHat and automated speed matching
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
- Ready to release new vesion 1.5.5 adding enabler function to event handler
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
