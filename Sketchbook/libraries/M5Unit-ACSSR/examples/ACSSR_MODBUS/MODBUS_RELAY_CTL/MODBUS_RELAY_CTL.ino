/*
*******************************************************************************
* Copyright (c) 2022 by M5Stack
*                  Equipped with M5Atom sample source code
*                          配套 M5Atom 示例源代码
*
* Visit for more information: https://docs.m5stack.com/en/unit/acssr
* 获取更多资料请访问: https://docs.m5stack.com/zh_CN/unit/acssr
*
* Product: ACSSR
* Date: 2022/7/5
*******************************************************************************
  Modbus relay control
  Modbus 继电器控制
  Libraries:
    - [ArduinoModbus](https://github.com/m5stack/ArduinoModbus)
    - [Arduino485](https://github.com/m5stack/ArduinoRS485)
*/

#include "M5Atom.h"
#include "M5_ACSSR.h"
#include <ArduinoModbus.h>
#include <ArduinoRS485.h>

// RS485Class RS485(Serial2, 26, 32, -1, -1);
RS485Class RS485(Serial2, 22, 19, -1, -1);

uint8_t slave_id = 0;

void setup() {
    M5.begin(true, false, true);
    delay(2000);
    M5.dis.fillpix(0x0000ff);
    ModbusRTUClient.begin(115200, SERIAL_8N1);
    slave_id = ACSSR_DEFAULT_SLAVE_ID;
}

void loop() {
    // ACSSR CTL ON
    ModbusRTUClient.coilWrite(slave_id, ACSSR_RELAY_COIL_ADDR, 0xff);
    // ACSSR LED CTL RED COLOR
    ModbusRTUClient.holdingRegisterWrite(slave_id, ACSSR_LED_HOLDING_ADDR,
                                         0xF800);

    // GET ACSSR STATUS
    if (ModbusRTUClient.requestFrom(slave_id, COILS, ACSSR_RELAY_COIL_ADDR,
                                    1)) {
        Serial.print("ACSSR STATUS: ");
        while (ModbusRTUClient.available()) {
            Serial.print(ModbusRTUClient.read(), HEX);
            Serial.print(' ');
        }
        Serial.println();
    }

    // GET ACSSR LED COLOR
    if (ModbusRTUClient.requestFrom(slave_id, HOLDING_REGISTERS,
                                    ACSSR_LED_HOLDING_ADDR, 1)) {
        Serial.print("LED COLOR: 0x");
        uint16_t color = 0;
        while (ModbusRTUClient.available()) {
            Serial.print(ModbusRTUClient.read(), HEX);
            Serial.print(' ');
        }
        if (color == 0xF800) {
            M5.dis.fillpix(0xff0000);
        }
        Serial.println();
    }
    delay(1000);
    // ACSSR CTL OFF
    ModbusRTUClient.coilWrite(slave_id, ACSSR_RELAY_COIL_ADDR, 0x00);
    // ACSSR LED CTL GREEN COLOR
    ModbusRTUClient.holdingRegisterWrite(slave_id, ACSSR_LED_HOLDING_ADDR,
                                         0x07E0);

    // GET ACSSR STATUS
    if (ModbusRTUClient.requestFrom(slave_id, COILS, ACSSR_RELAY_COIL_ADDR,
                                    1)) {
        Serial.print("ACSSR STATUS: ");
        uint16_t color = 0;
        while (ModbusRTUClient.available()) {
            Serial.print(ModbusRTUClient.read(), HEX);
            Serial.print(' ');
        }
        if (color == 0x07E0) {
            M5.dis.fillpix(0x00ff00);
        }
        Serial.println();
    }

    // GET ACSSR LED COLOR
    if (ModbusRTUClient.requestFrom(slave_id, HOLDING_REGISTERS,
                                    ACSSR_LED_HOLDING_ADDR, 1)) {
        Serial.print("LED COLOR: 0x");
        uint16_t color = 0;

        while (ModbusRTUClient.available()) {
            Serial.print(ModbusRTUClient.read(), HEX);
            Serial.print(' ');
        }
        Serial.println();
    }
    delay(1000);
    // GET ACSSR FW VERSION
    if (ModbusRTUClient.requestFrom(slave_id, HOLDING_REGISTERS,
                                    ACSSR_VERSION_HOLDING_ADDR, 1)) {
        Serial.print("Firmware Version: 0x");
        while (ModbusRTUClient.available()) {
            Serial.print(ModbusRTUClient.read(), HEX);
            Serial.print(' ');
        }
        Serial.println();
    }
}
