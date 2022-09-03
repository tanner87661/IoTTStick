/*
*******************************************************************************
* Copyright (c) 2021 by M5Stack
*                  Equipped with M5Core sample source code
*                          配套  M5Core 示例源代码
* Visit more information: https://docs.m5stack.com/en/unit/dds
* 获取更多资料请访问: https://docs.m5stack.com/zh_CN/unit/dds
*
* Product: Unit DDS.
* Date: 2022/7/8
*******************************************************************************
*/
#include "M5Stack.h"
#include "Unit_DDS.h"
#include "WaveIcon.c"

Unit_DDS dds;

int phase     = 0;
int freq      = 10000;
int modeIndex = 0;

String modeName[] = {"Sine", "Square", "Triangle", "Sawtooth"};

const unsigned char *waveIcon[] = {sine, square, triangle, sawtooth};

void displayInfo() {
    M5.Lcd.fillRect(0, 120, 250, 120, TFT_BLACK);
    M5.Lcd.drawJpg(waveIcon[modeIndex], sizeof(waveIcon[modeIndex]));
    M5.Lcd.setTextColor(TFT_GREEN);
    M5.Lcd.drawString("Wave:  " + modeName[modeIndex], 10, 140);
    M5.Lcd.setTextColor(TFT_BLUE);
    M5.Lcd.drawString("Phase:  " + String(phase), 10, 170);
    M5.Lcd.setTextColor(TFT_YELLOW);
    M5.Lcd.drawString("Freq:  " + String(freq), 10, 200);
}

void changeWave(int expression) {
    switch (expression) {
        case 0:
            dds.quickOUT(Unit_DDS::kSINUSMode, freq, phase);
            break;
        case 1:
            dds.quickOUT(Unit_DDS::kSQUAREMode, freq, phase);
            break;
        case 2:
            dds.quickOUT(Unit_DDS::kTRIANGLEMode, freq, phase);
            break;
        case 3:
            // SAWTOOTH WAVE Only support 13.6Khz
            freq = 13600;
            dds.quickOUT(Unit_DDS::kSAWTOOTHMode, freq, phase);
            break;
        default:
            break;
    }
    displayInfo();
}

void uiInit() {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.fillTriangle(250, 200, 250, 230, 270, 215, TFT_GREEN);
    M5.Lcd.setTextFont(4);
    M5.Lcd.setTextDatum(CC_DATUM);
}

void setup() {
    M5.begin(true, true, true, true);
    uiInit();
    dds.begin(&Wire);
    changeWave(modeIndex);
}

void loop() {
    M5.update();
    if (M5.BtnA.wasPressed()) {
        freq += 10000;
        changeWave(modeIndex);
    }
    if (M5.BtnB.wasPressed()) {
        phase += 10;
        changeWave(modeIndex);
    }
    if (M5.BtnC.wasPressed()) {
        if (modeIndex == 3) {
            freq = 10000;
        }
        if (modeIndex < 3) {
            modeIndex++;
        } else {
            modeIndex = 0;
        }
        changeWave(modeIndex);
    }
}