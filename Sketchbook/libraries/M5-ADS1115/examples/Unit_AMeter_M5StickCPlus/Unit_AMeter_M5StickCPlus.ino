/*
*******************************************************************************
* Copyright (c) 2021 by M5Stack
*                  Equipped with M5StickCPlus sample source code
*                          配套  M5StickCPlus 示例源代码
* Visit for more information：https://docs.m5stack.com/en/unit/ameter
* 获取更多资料请访问：https://docs.m5stack.com/zh_CN/unit/ameter
*
* Product:  Ameter_ADS1115.  电流计
* Date: 2021/8/19
*******************************************************************************
  Please connect to Port A,Measure current and display.
  请连接端口A,测量电流并显示到屏幕上
  Pay attention: EEPROM (0x51) has built-in calibration parameters when leaving
  the factory. Please do not write to the EEPROM, otherwise the calibration data
  will be overwritten and the measurement results will be inaccurate.
  注意:EEPROM(0x51)在出厂时具有内置的校准参数。请不要写入EEPROM，否则校准数据会被覆盖，测量结果会不准确。
*/

#include <M5StickCPlus.h>
#include <Wire.h>
#include "M5_ADS1115.h"

ADS1115 Ammeter(AMETER, AMETER_ADDR, AMETER_EEPROM_ADDR);

float page512_volt = 2000.0F;

int16_t volt_raw_list[10];
uint8_t raw_now_ptr = 0;
int16_t adc_raw     = 0;

int16_t hope = 0.0;

ADS1115Gain_t now_gain = PAG_512;

void setup(void) {
    M5.begin();
    Wire.begin();

    Ammeter.setMode(SINGLESHOT);
    Ammeter.setRate(RATE_8);
    Ammeter.setGain(PAG_512);
    hope = page512_volt / Ammeter.resolution;
    // | PAG      | Max Input Voltage(V) |
    // | PAG_6144 |        128           |
    // | PAG_4096 |        64            |
    // | PAG_2048 |        32            |
    // | PAG_512  |        16            |
    // | PAG_256  |        8             |

    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextFont(2);

    M5.Lcd.setCursor(51, 225);
    M5.Lcd.printf("2A");

    //   M5.Lcd.setCursor(118, 90);
    //   M5.Lcd.printf("SAVE");

    // bool result1 = Ammeter.saveCalibration2EEPROM(PAG_256, 1024, 1024);
    // delay(10);
}

void loop(void) {
    M5.update();
    if (M5.BtnA.wasPressed()) {
        Ammeter.setMode(SINGLESHOT);
        Ammeter.setRate(RATE_8);
        Ammeter.setGain(PAG_512);
        now_gain = PAG_512;
        hope     = page512_volt / Ammeter.resolution;

        for (uint8_t i = 0; i < 10; i++) {
            volt_raw_list[i] = 0;
        }
    }

    //   if (M5.BtnB.wasPressed()) {
    //     bool success = Ammeter.saveCalibration2EEPROM(now_gain, hope,
    //     adc_raw); M5.Lcd.setCursor(118, 90);
    //
    //     if (success) {
    //       M5.Lcd.setTextColor(GREEN, BLACK);
    //     } else {
    //       M5.Lcd.setTextColor(RED, BLACK);
    //     }
    //
    //     M5.Lcd.printf("SAVE");
    //
    //     delay(300);
    //     M5.Lcd.setCursor(118, 90);
    //     M5.Lcd.setTextColor(WHITE, BLACK);
    //     M5.Lcd.printf("SAVE");
    //
    //     Ammeter.setGain(now_gain);
    //  }

    float current = Ammeter.getValue();

    volt_raw_list[raw_now_ptr] = Ammeter.adc_raw;
    raw_now_ptr                = (raw_now_ptr == 9) ? 0 : (raw_now_ptr + 1);

    int count = 0;
    int total = 0;

    for (uint8_t i = 0; i < 10; i++) {
        if (volt_raw_list[i] == 0) {
            continue;
        }
        total += volt_raw_list[i];
        count += 1;
    }

    if (count == 0) {
        adc_raw = 0;
    } else {
        adc_raw = total / count;
    }

    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.setCursor(3, 0);
    M5.Lcd.printf("Hope volt:");
    M5.Lcd.setCursor(9, 15);
    M5.Lcd.printf("%.2f mAn", page512_volt);

    M5.Lcd.setCursor(3, 37);
    M5.Lcd.printf("Hope ADC:");
    M5.Lcd.setCursor(9, 52);
    M5.Lcd.printf("%d", hope);

    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.setCursor(3, 75);
    M5.Lcd.printf("Cal volt:");
    M5.Lcd.fillRect(65, 90, 40, 15, BLACK);
    M5.Lcd.setCursor(9, 90);
    M5.Lcd.printf("%.2f mA", current);

    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.setCursor(3, 112);
    M5.Lcd.printf("Cal ADC:");
    M5.Lcd.fillRect(15, 127, 40, 15, BLACK);
    M5.Lcd.setCursor(9, 127);
    M5.Lcd.printf("%.0f", adc_raw * Ammeter.calibration_factor);

    M5.Lcd.setCursor(3, 150);
    if (abs(adc_raw) <= hope * 1.005 && abs(adc_raw) >= hope * 0.995) {
        M5.Lcd.setTextColor(GREEN, BLACK);
    } else {
        M5.Lcd.setTextColor(RED, BLACK);
    }
    M5.Lcd.printf("RAW ADC:");
    M5.Lcd.fillRect(15, 165, 40, 15, BLACK);
    M5.Lcd.setCursor(9, 165);
    M5.Lcd.printf("%d", adc_raw);
}
