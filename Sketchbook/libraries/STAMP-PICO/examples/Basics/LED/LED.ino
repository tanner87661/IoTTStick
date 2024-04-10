/*
*******************************************************************************
* Copyright (c) 2021 by M5Stack
*                  Equipped with STAMP-PICO sample source code
*                          配套  STAMP-PICO 示例源代码
* Visit the website for more information：https://docs.m5stack.com/en/core/stamp_pico
* 获取更多资料请访问：https://docs.m5stack.com/zh_CN/core/stamp_pico
*
* describe: FastLED.
* date：2021/9/19
*******************************************************************************
*/
#include "Arduino.h"
#include <FastLED.h>

// How many leds in your strip?
#define NUM_LEDS 1
#define DATA_PIN 27

// Define the array of leds
CRGB leds[NUM_LEDS];

/* After STAMP-PICO is started or reset
  the program in the setUp () function will be run, and this part will only be run once.
  在 STAMP-PICO 启动或者复位后，即会开始执行setup()函数中的程序，该部分只会执行一次。 */
void setup() {
  FastLED.addLeds<SK6812, DATA_PIN, RGB>(leds, NUM_LEDS);  // GRB ordering is typical
}

/* After the program in setup() runs, it runs the program in loop()
The loop() function is an infinite loop in which the program runs repeatedly
在setup()函数中的程序执行完后，会接着执行loop()函数中的程序
loop()函数是一个死循环，其中的程序会不断的重复运行 */
void loop() {
  // Turn the LED on, then pause
  leds[0] = 0xf00000;
  FastLED.show();
  delay(500);
  // Now turn the LED off, then pause
  leds[0] = 0x00f000;
  FastLED.show();
  delay(500);
}
