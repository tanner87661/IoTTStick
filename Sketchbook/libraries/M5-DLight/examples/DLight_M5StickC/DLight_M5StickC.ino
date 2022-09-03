/*
*******************************************************************************
* Copyright (c) 2022 by M5Stack
*                  Equipped with M5StickC sample source code
*                          配套  M5StickC 示例源代码
* Visit for more information: http://docs.m5stack.com/en/hat/hat_dlight
* 获取更多资料请访问: http://docs.m5stack.com/zh_CN/hat/hat_dlight
*
* Product: DLight.
* Date: 2022/07/7
*******************************************************************************
  Measure and output light intensity on screen and serial port
  测量并在屏幕和串口输出光照强度
*/

#include <M5StickC.h>
#include <M5_DLight.h>

TFT_eSprite canvas(&M5.Lcd);

M5_DLight sensor;
uint16_t lux;

void setup() {
    M5.begin();
    M5.Lcd.setRotation(1);
    Wire.begin(0, 26);  // HAT DLight
    // Wire.begin(21, 22);  // Unit DLight
    canvas.setTextColor(GREEN);
    canvas.setTextDatum(MC_DATUM);
    canvas.setTextSize(2);
    canvas.createSprite(160, 80);
    Serial.println("Sensor begin.....");
    sensor.begin();

    // CONTINUOUSLY_H_RESOLUTION_MODE
    // CONTINUOUSLY_H_RESOLUTION_MODE2
    // CONTINUOUSLY_L_RESOLUTION_MODE
    // ONE_TIME_H_RESOLUTION_MODE
    // ONE_TIME_H_RESOLUTION_MODE2
    // ONE_TIME_L_RESOLUTION_MODE
    sensor.setMode(CONTINUOUSLY_H_RESOLUTION_MODE);
}

char info[40];

void loop() {
    lux = sensor.getLUX();
    sprintf(info, "lux: %d", lux);
    canvas.fillSprite(BLACK);
    canvas.drawString(info, 80, 40);
    canvas.pushSprite(0, 0);
    Serial.println(info);
    delay(100);
}