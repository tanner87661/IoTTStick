
/*
*******************************************************************************
* Copyright (c) 2022 by M5Stack
*                  Equipped with ATOM DTU CAT1 Modbus sample source code
* Visit the website for more
information：https://docs.m5stack.com/en/atom/atom_dtu_cat1
* describe: ATOM DTU CAT1 Modbus Slave Example.
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

const int numCoils            = 10;
const int numDiscreteInputs   = 10;
const int numHoldingRegisters = 10;
const int numInputRegisters   = 10;

RS485Class RS485(Serial2, ATOM_DTU_RS485_RX, ATOM_DTU_RS485_TX, -1, -1);

void setup() {
    M5.begin();
    // start the Modbus RTU server, with (slave) id 42
    if (!ModbusRTUServer.begin(42, 9600)) {
        Serial.println("Failed to start Modbus RTU Server!");
        while (1)
            ;
    }
    // configure coils at address 0x00
    ModbusRTUServer.configureCoils(0x00, numCoils);
    // configure discrete inputs at address 0x00
    ModbusRTUServer.configureDiscreteInputs(0x00, numDiscreteInputs);
    // configure holding registers at address 0x00
    ModbusRTUServer.configureHoldingRegisters(0x00, numHoldingRegisters);
    // configure input registers at address 0x00
    ModbusRTUServer.configureInputRegisters(0x00, numInputRegisters);
}

void loop() {
    // poll for Modbus RTU requests
    ModbusRTUServer.poll();

    // map the coil values to the discrete input values
    for (int i = 0; i < numCoils; i++) {
        int coilValue = ModbusRTUServer.coilRead(i);

        ModbusRTUServer.discreteInputWrite(i, coilValue);
    }

    // map the holding register values to the input register values
    for (int i = 0; i < numHoldingRegisters; i++) {
        long holdingRegisterValue = ModbusRTUServer.holdingRegisterRead(i);

        ModbusRTUServer.inputRegisterWrite(i, holdingRegisterValue);
    }
}