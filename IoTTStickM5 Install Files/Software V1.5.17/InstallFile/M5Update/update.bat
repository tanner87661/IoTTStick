esptool --chip esp32 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size detect 0xe000 boot_app0.bin 0x1000 bootloader_dio_80m.bin 0x10000 LNFP_M5Stick.ino.bin 0x8000 LNFP_M5Stick.ino.partitions.bin 
esptool write_flash 0x00210000 LNFP_M5Stick.spiffs.bin


