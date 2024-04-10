/*
*******************************************************************************
* Copyright (c) 2021 by M5Stack
*                  Equipped with M5StickC / M5StickCPlus sample source code
*                          配套  M5StickC / M5StickCPlus 示例源代码
* Visit for more information: https://docs.m5stack.com/en/hat/hat-joyc
* 获取更多资料请访问: https://docs.m5stack.com/zh_CN/hat/hat-joyc
*
* Product: JoyC.
* Date: 2022/9/17
*******************************************************************************
  Please connect to Port,
  Read JOYSTICK Unit X, Y axis offset data and button status
  请连接端口,读取操纵杆单位X, Y轴偏移数据和按钮状态
*/
#include <M5StickC.h>
// #include <M5StickCPlus.h>
#include "Hat_JoyC.h"

TFT_eSprite canvas = TFT_eSprite(&M5.Lcd);
JoyC joyc;

void setup() {
    M5.begin();             // Initialize host device. 初始化主机设备
    M5.Lcd.setRotation(1);  // Rotation screen. 旋转屏幕
    canvas.createSprite(160,
                        80);  // Create a 160*80 canvas. 创建一块160*80的画布
    canvas.setTextColor(
        ORANGE);   // Set font colour to orange. 设置字体颜色为橙色
    joyc.begin();  // Initialize JoyC. 初始化 JoyC
}

char info[50];

void loop() {
    joyc.update();             // Update JoyC's data 更新JoyC的数据
    canvas.fillSprite(BLACK);  // Fill the canvas with black 填充画布为黑色
    canvas.setCursor(0, 10);  // Set the cursor at (0,10) 设置光标在(0,10)
    canvas.println("JoyC TEST");

    sprintf(info, "X0: %d Y0: %d", joyc.x0, joyc.y0);
    canvas.println(info);
    Serial.println(info);
    sprintf(info, "X1: %d Y1: %d", joyc.x1, joyc.y1);
    canvas.println(info);
    Serial.println(info);
    sprintf(info, "Angle0: %d Angle1: %d", joyc.angle0, joyc.angle1);
    canvas.println(info);
    Serial.println(info);
    sprintf(info, "D0: %d D1: %d", joyc.distance0, joyc.distance1);
    canvas.println(info);
    Serial.println(info);
    sprintf(info, "Btn0: %d Btn1: %d", joyc.btn0, joyc.btn1);
    canvas.println(info);
    Serial.println(info);
    canvas.pushSprite(10, 0);
    if (joyc.btn0 &&
        joyc.btn1) {  // If the buttons are all pressed. 如果按键都被按下
        joyc.setLEDColor(0x00ffe8);
    } else if (joyc.btn0) {
        joyc.setLEDColor(0xff0000);
    } else if (joyc.btn1) {
        joyc.setLEDColor(0x0000ff);
    } else {
        joyc.setLEDColor(0x00ff00);
    }
}
