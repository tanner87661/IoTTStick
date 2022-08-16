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
  I2C address verification
  I2C 地址验证
*/
#include "M5Atom.h"
#include "M5_ACSSR.h"

M5_ACSSR SSR;

void setup() {
    M5.begin(true, false, true);
    M5.dis.fillpix(0xff0000);
    while (!SSR.begin(&Wire, 26, 32, ACSSR_DEFAULT_ADDR)) {
        Serial.println("ACSSR I2C INIT ERROR");
        delay(1000);
    }
    M5.dis.fillpix(0x0000ff);
}

void loop() {
    if (M5.Btn.wasPressed()) {
        if (SSR.setDeviceAddr(0x66)) {
            Serial.println("I2C ADDR CONFIG: 0x66");
            delay(300);
        } else {
            Serial.println("I2C ADDR CONFIG: FAIL");
            SSR.setLEDColor(0x8700ff);
        };
    };
    M5.update();
    SSR.on();
    SSR.setLEDColor(0xff0000);
    delay(900);
    SSR.off();
    SSR.setLEDColor(0x00ff00);
    delay(900);
}
