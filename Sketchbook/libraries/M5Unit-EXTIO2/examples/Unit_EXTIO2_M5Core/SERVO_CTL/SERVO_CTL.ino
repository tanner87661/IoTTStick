/*
*******************************************************************************
* Copyright (c) 2022 by M5Stack
*                  Equipped with M5Core sample source code
*                          配套 M5Core 示例源代码
*
* Visit for more information: https://docs.m5stack.com/en/unit/extio2
* 获取更多资料请访问: https://docs.m5stack.com/zh_CN/unit/extio2
*
* Product: EXT.IO2.
* Date: 2022/7/4
*******************************************************************************
  Servo control
  舵机控制
*/
#include <M5Stack.h>
#include <M5GFX.h>
#include "M5_EXTIO2.h"

M5GFX display;
M5Canvas canvas(&display);
M5_EXTIO2 extio;

void setup() {
    M5.begin();
    display.begin();
    canvas.setColorDepth(1);  // mono color
    canvas.setFont(&fonts::efontCN_14);
    canvas.createSprite(display.width(), display.height());
    canvas.setPaletteColor(1, GREEN);
    while (!extio.begin(&Wire, 21, 22, 0x45)) {
        Serial.println("extio Connect Error");
        M5.Lcd.print("extio Connect Error");
        delay(100);
    }
    // extio.setAllPinMode(DIGITAL_INPUT_MODE);
    // extio.setAllPinMode(DIGITAL_OUTPUT_MODE);
    // extio.setAllPinMode(ADC_INPUT_MODE);
    extio.setAllPinMode(SERVO_CTL_MODE);
    // extio.setAllPinMode(RGB_LED_MODE);
}

char info[50];

void loop() {
    canvas.fillSprite(0);
    canvas.setTextSize(2);
    canvas.drawString("SERVO CTL MODE", 10, 10);
    canvas.drawString("FW VERSION: " + String(extio.getVersion()), 10, 40);
    for (uint8_t deg = 0; deg <= 180; deg += 45) {
        for (uint8_t i = 0; i < 8; i++) {
            extio.setServoAngle(i, deg);
            Serial.printf("CH:%d DEG: %d", i, deg);
            canvas.drawRect(0, i * 20 + 75, 200, 15, 1);
            canvas.fillRect(0, i * 20 + 75, map(deg, 0, 180, 0, 200), 15, 1);
            canvas.setCursor(220, i * 28 + 10);
            canvas.setTextSize(1);
            canvas.printf("CH:%d DEG: %d", i, deg);
        }
        canvas.pushSprite(0, 0);
        vTaskDelay(500);
    }
    canvas.setTextSize(2);
    canvas.drawString("SERVO CTL MODE", 10, 10);
    canvas.drawString("FW VERSION: " + String(extio.getVersion()), 10, 40);
    for (int pulse = 500; pulse <= 2500; pulse += 100) {
        for (uint8_t i = 0; i < 8; i++) {
            extio.setServoPulse(i, pulse);
            Serial.printf("CH:%d P: %d", i, pulse);
            canvas.drawRect(0, i * 20 + 75, 200, 15, 1);
            canvas.fillRect(0, i * 20 + 75, map(pulse, 500, 2500, 0, 200), 15,
                            1);
            canvas.setCursor(220, i * 28 + 10);
            canvas.setTextSize(1);
            canvas.printf("CH:%d P: %d", i, pulse);
        }
        canvas.pushSprite(0, 0);
    }
    delay(100);
    canvas.pushSprite(0, 0);
}
