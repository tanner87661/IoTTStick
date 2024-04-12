This file contains The IoTT Stick software Version 1.6.6

Installation on Linux and MacOs.  
This distribution includes an installer script, call update.mac.  

Use the M5Update.zip for IoTT Stick and IoTT Stick Plus
Use the M5UpdatePlus2.zip for the IoTT Stick Plus2

Use the respective Mac OS version to install the software from an Apple computer

This uses esptool.py, which is a downloader for the ESP32 and uses the Python language.   
It is included here, but is also available from Espressif at https://github.com/espressif/esptool.  

esptool.py requires Python to run. 
Python is included with MacOS, but one can follow the directions here to upgrade or install it for a Mac: https://legacy.python.org/download/mac/  For other OSes, follow these instructions: https://legacy.python.org/download/other/

To run the installer MacOS script, update.mac in a Terminal window:
  (1)  type "chmod 777 update.mac"
  (2)  type “sh update.mac”
Terminal.app is available in the Applications/Utilities directory in MacOS.  

Warning: Uploading a new version will overwrite your current stick configuration. To avoid, you should first save the current configuration to your computer. 
After uploading the new software version, you can write back the previous configuration to the IoTT Stick.
