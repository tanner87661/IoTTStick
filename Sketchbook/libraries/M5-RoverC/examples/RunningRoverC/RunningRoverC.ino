#include <M5StickC.h>
// #include <M5StickCPlus.h>

#include "M5_RoverC.h"

TFT_eSprite canvas = TFT_eSprite(&M5.Lcd);

M5_ROVERC roverc;

void setup() {
  M5.begin();
  roverc.begin();

  M5.Lcd.setRotation(1);
  canvas.createSprite(160, 80);
  canvas.setTextColor(ORANGE);
  roverc.setSpeed(0, 0, 0);

  canvas.setTextDatum(MC_DATUM);
  canvas.drawString("RoverC TEST", 80, 40, 4);
  canvas.pushSprite(0, 0);
}

void loop(){
  //roverc.setServoAngle(uint8_t pos, uint8_t angle)
  roverc.setServoAngle(0, 30);
  roverc.setServoAngle(1, 30);
  delay(1000);
  roverc.setServoAngle(0, 10);
  roverc.setServoAngle(1, 10);
  delay(1000);
  //roverc.setSpeed(int8_t x, int8_t y, int8_t z)
  roverc.setSpeed(100, 0, 0);
  delay(1000);
  roverc.setSpeed(0, 100, 0);
  delay(1000);
  roverc.setSpeed(0, 0, 100);
  delay(1000);
  roverc.setSpeed(0, 0, 0);
  delay(1000); 
}
