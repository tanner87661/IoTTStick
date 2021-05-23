# IoTTStick
This is the development repository for the IoTT Stick, port modules and hat devices with the source code for the latest release. 

2021/05/22
- Released new version 1.5.3 with some improvements in lbServer / lbClient
- adding bounce filter for sensor inputs, requiring a 500ms stable input before sending report message to LocoNet

2021/05/06
- Released new version 1.5.3 with some new features for the YellowHat (buttons now can send input report and switch state report directly, no event handler)
- major improvement of the handshake between stick and web app in case of saving and loading disk files

2021/04/29
- added support for input reports and switch reports to button inputys for YellowHat

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
