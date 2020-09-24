#!/bin/sh
esptool.py write_flash 0x00010000 M5.bin
esptool.py write_flash 0x00290000 SPIFFS.bin
