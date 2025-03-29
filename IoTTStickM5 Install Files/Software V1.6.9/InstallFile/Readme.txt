This file contains The IoTT Stick software Version 1.6.6
To install, connect your M5 Stick C to the computer's USB port.

Use the M5Update.zip for IoTT Stick and IoTT Stick Plus
Use the M5UpdatePlus2.zip for the IoTT Stick Plus2

Use the respective Mac OS version to install the software from an Apple computer

If your PC has difficulties to find the com port, you can modify the command lines in update.bat by adding --port COMx as first paramter. x is the number of the COM port to be used.
Example for COM2:
esptool --port COM2 --chip esp32 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size detect 0xe000 boot_app0.bin 0x1000 bootloader_dio_80m.bin 0x10000 LNFP_M5Stick.ino.bin 0x8000 LNFP_M5Stick.ino.partitions.bin 
esptool --port COM2 write_flash 0x00210000 LNFP_M5Stick.spiffs.bin
 
Extract the zip file and execute (double-click) the file update.bat
If everything goes right, the update program will find the stick on the USB port and will load the program and the web pages.

Warning: Uploading a new version will overwrite your current stick configuration. To avoid, you should first save the current configuration to your computer. 
After uploading the new software version, you can write back the previous configuration to the IoTT Stick.
