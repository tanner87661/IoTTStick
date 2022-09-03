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
  I2C Slave Control
  I2C 从机控制
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
    Serial.print("FW Version: 0x");
    Serial.println(SSR.getVersion(), HEX);
    M5.dis.fillpix(0x0000ff);
}

void loop() {
    SSR.on();
    Serial.print("ACSSR STATUS: ");
    Serial.println(SSR.status());
    SSR.setLEDColor(0xff0000);
    Serial.print("LED COLOR: 0x");
    Serial.println(SSR.getLEDColor(), HEX);
    delay(900);
    SSR.off();
    Serial.print("ACSSR STATUS: ");
    Serial.println(SSR.status());
    SSR.setLEDColor(0x00ff00);
    Serial.print("LED COLOR: 0x");
    Serial.println(SSR.getLEDColor(), HEX);
    delay(900);
}
