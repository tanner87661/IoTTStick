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
  Modbus ID verification
  Modbus ID 确认
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
    // slave_id = 0x0066;
}

void loop() {
    if (M5.Btn.wasPressed()) {
        if (ModbusRTUClient.holdingRegisterWrite(
                slave_id, ACSSR_ID_HOLDING_ADDR, 0x0066)) {
            Serial.println("SLAVE ID CONFIG: 0x0066");
            slave_id = 0x0066;
            delay(300);
        } else {
            Serial.println("SLAVE ID  CONFIG: FAIL");
        };
    };
    M5.update();
    // ACSSR CTL ON
    ModbusRTUClient.coilWrite(slave_id, ACSSR_RELAY_COIL_ADDR, 0xff);
    // ACSSR LED CTL RED COLOR
    ModbusRTUClient.holdingRegisterWrite(slave_id, ACSSR_LED_HOLDING_ADDR,
                                         0xF800);
    delay(1000);
    // ACSSR CTL OFF
    ModbusRTUClient.coilWrite(slave_id, ACSSR_RELAY_COIL_ADDR, 0x00);
    // ACSSR LED CTL GREEN COLOR
    ModbusRTUClient.holdingRegisterWrite(slave_id, ACSSR_LED_HOLDING_ADDR,
                                         0x07E0);
    delay(1000);
}
