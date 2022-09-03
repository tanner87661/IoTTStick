/*
*******************************************************************************
* Copyright (c) 2021 by M5Stack
*                  Equipped with M5Devices sample source code
*                          配套  M5Devices 示例源代码
* Visit for more information: https://docs.m5stack.com/en/unit/kmeter
* 获取更多资料请访问: https://docs.m5stack.com/zh_CN/unit/kmeter
*
* Product: KMeter.  热电偶传感器
* date: 2022/7/6
*******************************************************************************
  Thermocouple and internal temperature output on screen and serial port
  在屏幕和串口输出热电偶温度及内部温度
*/
#include <M5GFX.h>
#include <M5_KMeter.h>
#include <esp_log.h>

M5GFX display;
M5_KMeter sensor;

void setup(void) {
    display.begin();

    Wire.begin(SDA, SCL, 400000L);
    // Wire.begin(21, 22, 400000L);

    sensor.begin();
    // sensor.begin(&Wire, 0x66);

    display.setTextSize(2);
    if (display.width() < display.height()) {
        display.setRotation(display.getRotation() ^ 1);
    }
}

void loop(void) {
    delay(100);
    float temperature = sensor.getTemperature();
    display.setCursor(display.width() / 2 - 64,
                      (display.height() - display.fontHeight() * 3) / 2);
    display.printf("Tmp:%7.2f ", temperature);

    float internaltemp = sensor.getInternalTemp();
    display.setCursor(display.width() / 2 - 64,
                      (display.height() - display.fontHeight()) / 2);
    display.printf("In:%8.4f ", internaltemp);

    uint8_t raw[4];
    display.setCursor(display.width() / 2 - 64,
                      (display.height() + display.fontHeight()) / 2);
    if (sensor.getRawData(raw)) {
        display.printf("%02x %02x %02x %02x", raw[0], raw[1], raw[2], raw[3]);
    } else {
        display.print(" I2C error ");
    }

    ESP_LOGI("loop", "%4.2f , %4.4f  %02x %02x %02x %02x", temperature,
             internaltemp, raw[0], raw[1], raw[2], raw[3]);
}
