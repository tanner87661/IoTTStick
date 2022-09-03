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
  RGB LED control
  RGB LED 灯控制
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
    canvas.setPaletteColor(1, GREEN);  // set palette color. 设置色调颜色
    while (!extio.begin(&Wire, 21, 22, 0x45)) {
        Serial.println("extio Connect Error");
        M5.Lcd.print("extio Connect Error");
        delay(100);
    }
    // extio.setAllPinMode(DIGITAL_INPUT_MODE);
    // extio.setAllPinMode(DIGITAL_OUTPUT_MODE);
    // extio.setAllPinMode(ADC_INPUT_MODE);
    // extio.setAllPinMode(SERVO_CTL_MODE);
    extio.setAllPinMode(RGB_LED_MODE);  // Set all pins to RGB LED mode.
                                        // 设置所有引脚为RGB LED 模式
}

void loop() {
    for (uint8_t m = 0; m < 3; m++) {
        canvas.fillSprite(0);
        canvas.setTextSize(2);
        canvas.drawString("RGB LED MODE", 10, 10);
        canvas.drawString("FW VERSION: " + String(extio.getVersion()), 10, 40);
        for (uint8_t i = 0; i < 8; i++) {
            extio.setLEDColor(i, 0xff << (m * 8));
        }
        canvas.setCursor(10, 120);
        canvas.printf("COLOR: 0x%X", 0xff << (m * 8));
        canvas.pushSprite(0, 0);
        vTaskDelay(800);
    }
}
