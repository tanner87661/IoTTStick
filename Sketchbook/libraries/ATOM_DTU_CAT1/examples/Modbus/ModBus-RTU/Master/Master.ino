/*
*******************************************************************************
* Copyright (c) 2022 by M5Stack
*                  Equipped with ATOM DTU CAT1 Modbus sample source code
* Visit the website for more
information：https://docs.m5stack.com/en/atom/atom_dtu_cat1
* describe: ATOM DTU CAT1 Modbus Master Example.
* Libraries:
    - [ArduinoModbus](https://github.com/m5stack/ArduinoModbus)
    - [Arduino485](https://github.com/m5stack/ArduinoRS485)
* date：2022/1/25
*******************************************************************************
*/

#include <M5Atom.h>
#include <ArduinoModbus.h>
#include <ArduinoRS485.h>
#include "ATOM_DTU_CAT1.h"

RS485Class RS485(Serial2, ATOM_DTU_RS485_RX, ATOM_DTU_RS485_TX, -1, -1);

void setup() {
    M5.begin(true, false, true);
    if (!ModbusRTUClient.begin(9600, SERIAL_8N1)) {
        Serial.println("Failed to start Modbus RTU Client!");
        while (1)
            ;
    }
}

void loop() {
    // for (slave) id 1: write the value of 0x01, to the coil at address 0x00
    if (!ModbusRTUClient.coilWrite(1, 0x00, 0xff)) {
        Serial.print("Failed to write coil! ");
        Serial.println(ModbusRTUClient.lastError());
    }

    // wait for 1 second
    delay(2000);
    // for (slave) id 1: write the value of 0x00, to the coil at address 0x00
    if (!ModbusRTUClient.coilWrite(1, 0x00, 0x00)) {
        Serial.print("Failed to write coil! ");
        Serial.println(ModbusRTUClient.lastError());
    }
    // wait for 1 second
    delay(2000);
}
