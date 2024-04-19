/*
*******************************************************************************
* Copyright (c) 2021 by M5Stack
*                  Equipped with M5Core sample source code
*                          配套  M5Core 示例源代码
* Visit for more information: https://docs.m5stack.com/en/unit/encoder
* 获取更多资料请访问: https://docs.m5stack.com/zh_CN/unit/encoder
*
* Describe: Encoder.  旋转编码器
* Date: 2022/7/11
*******************************************************************************
  Display of rotary encoder values and key status on the screen
  在屏幕上显示旋转编码器的值和按键状态
*/
#include <M5Core2.h>
#include <M5GFX.h>
#include "Unit_Encoder.h"

M5GFX display;
M5Canvas canvas(&display);
Unit_Encoder sensor;

void setup() {
    M5.begin(true, false, true, true);  // Init M5Core2.  初始化M5Core2
    sensor.begin();
    display.begin();
    display.setRotation(1);
    canvas.setTextSize(2);
    canvas.createSprite(160, 80);
}

signed short int last_value = 0;

void loop() {
    signed short int encoder_value = sensor.getEncoderValue();
    bool btn_stauts                = sensor.getButtonStatus();
    Serial.println(encoder_value);
    if (last_value != encoder_value) {
        if (last_value > encoder_value) {
            sensor.setLEDColor(1, 0x000011);
        } else {
            sensor.setLEDColor(2, 0x111100);
        }
        last_value = encoder_value;
    } else {
        sensor.setLEDColor(0, 0x001100);
    }
    if (!btn_stauts) {
        sensor.setLEDColor(0, 0xC800FF);
    }
    canvas.fillSprite(BLACK);
    canvas.drawString("BTN:" + String(btn_stauts), 10, 10);
    canvas.drawString("ENCODER:" + String(encoder_value), 10, 40);
    canvas.pushSprite(0, 0);
    delay(20);
}
