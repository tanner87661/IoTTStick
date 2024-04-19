void drawLogo(int x, int y, int logoScale)
{
  String fileName = "/www/iottlogo120120.jpg";
  loadJpgFile(x,y,logoScale, fileName.c_str());
}

void drawFile(String fileName,int x, int y)
{
  loadJpgFile(x,y,1, fileName.c_str());
}

void drawText(M5Canvas* targetDisp, const char * payload, int x, int y, int what)
{
  if (!targetDisp) 
    M5.Display.drawString(payload,x, y,what);
  else
    targetDisp->drawString(payload,x, y,what);
}

void drawCenterText(M5Canvas* targetDisp, const char * payload, int x, int y, int what)
{
  if (!targetDisp) 
    M5.Display.drawCenterString(payload, x, y,what);
  else
    targetDisp->drawCenterString(payload, x, y,what);
}

void drawBackground()
{
  String fileName = "/www/Nickel240240.jpg";
  loadJpgFile(0,0,1, fileName.c_str());
//  M5.Display.drawJpgFile(fs::SPIFFSFS, fileName.c_str());//,0,0,240,240,0,0);
}

void drawCenterTextAngled(M5Canvas* targetDisp, char* payload, uint16_t x, uint16_t y, float_t dispAngle, int what)
{
  uint16_t plW = (M5.Display.textWidth(payload)*what); 
  uint16_t plH = M5.Display.fontHeight()*what;
  uint16_t plX = plW>>1;
  uint16_t plY = plH>>1;
  M5Canvas dispCanvas(&M5.Display);
  M5Canvas* mySprite;
  if (!targetDisp) 
  {
    mySprite = &dispCanvas;
    targetDisp = &dispCanvas;
  }
  else
    mySprite = targetDisp;
    
  mySprite->createSprite(plW, plH); 
  mySprite->fillSprite(TFT_PINK);
  M5.Display.setPivot(x,y);
  mySprite->setTextColor(TFT_BLACK);
//  mySprite.fillRect(0,0,plW, plH, colLocoUnsel);
//  mySprite.fillScreen(TFT_TRANSPARENT);
  drawCenterText(mySprite, payload, plX, 0, what);
  mySprite->setPivot(plX, plY);
  mySprite->pushRotated(dispAngle, TFT_PINK);  
  mySprite->deleteSprite();
}

void drawFunctionSegment(uint8_t fctPos, uint8_t fctNr, bool fctState)
{
  M5Canvas mySprite(&M5.Display);

  uint16_t fillCol;
  uint16_t fillSelCol;
  String dispText = "F" + String(fctNr);
  float_t pivotAngle;
  switch (fctPos)
  {
    case 0: fillCol = colFNOff; fillSelCol = colFNOn; pivotAngle = -60; break;
    case 1: fillCol = colFNOff; fillSelCol = colFNOn; pivotAngle = -90; break;
    case 2: fillCol = colFNOff; fillSelCol = colFNOn; pivotAngle = -120; break;
    case 3: fillCol = colFNOff; fillSelCol = colFNOn; pivotAngle = -150; break;
    default: Serial.println("No angle"); break;
  }
  mySprite.createSprite(120, 120); 
  mySprite.fillSprite(TFT_PINK);
  mySprite.fillArc(120, 120, 90, 120, 180, 210, fctState ? fillSelCol : fillCol);
  uint8_t dispSize = 4;
  uint16_t plW = (M5.Display.textWidth(dispText)*dispSize); 
  uint16_t plH = M5.Display.fontHeight()*dispSize;
  uint16_t plX = plW>>1;
  uint16_t plY = plH>>1;

  M5Canvas textSprite(&mySprite);
  textSprite.createSprite(plW, plH);
  textSprite.fillSprite(TFT_PINK);
  textSprite.setTextColor(TFT_BLACK);
  textSprite.drawCenterString(dispText, plX, 0, dispSize);
  textSprite.setPivot(plX, plY);
  mySprite.setPivot(14,91);
  textSprite.pushRotated(105, TFT_PINK);  
  textSprite.deleteSprite();
  
  M5.Display.setPivot(120, 120);
  mySprite.setPivot(120, 120);
  mySprite.pushRotated(pivotAngle, TFT_PINK);  
  mySprite.deleteSprite();
}

void updateMainMenuRing(uint8_t menuNr)
{
  for (uint8_t i = 0; i < 3; i++)
    drawMainMenuSegment(i, i == menuNr);
}

void drawMainMenuSegment(uint8_t menuNr, bool selState)
{
  M5Canvas mySprite(&M5.Display);

  uint16_t fillCol;
  uint16_t fillSelCol;
  String dispText;
  float_t pivotAngle;
  switch (menuNr)
  {
    case 0: fillCol = colLocoUnsel; fillSelCol = colLocoSel; dispText = "Loco"; pivotAngle = 0; break;
    case 1: fillCol = colSwiUnsel; fillSelCol = colSwiSel; dispText = "Swi"; pivotAngle = 45; break;
    case 2: fillCol = colRteUnsel; fillSelCol = colRteSel; dispText = "Sig"; pivotAngle = 90; break;
    case 3: fillCol = colSensUnsel; fillSelCol = colSensSel; dispText = "Rte"; pivotAngle = 135; break;
    default: Serial.println("No angle"); break;
  }
  mySprite.createSprite(120, 120); 
  mySprite.fillSprite(TFT_PINK);
  mySprite.fillArc(120, 120, 80, 120, 180, 225, selState ? fillSelCol : fillCol);
  uint8_t dispSize = 4;
  uint16_t plW = (M5.Display.textWidth(dispText)*dispSize); 
  uint16_t plH = M5.Display.fontHeight()*dispSize;
  uint16_t plX = plW>>1;
  uint16_t plY = plH>>1;

  M5Canvas textSprite(&mySprite);
  textSprite.createSprite(plW, plH);
  textSprite.fillSprite(TFT_PINK);
  textSprite.setTextColor(TFT_BLACK);
  textSprite.drawCenterString(dispText, plX, 0, dispSize);
  textSprite.setPivot(plX, plY);
  mySprite.setPivot(37,86);
  textSprite.pushRotated(-67, TFT_PINK);  
  textSprite.deleteSprite();
  
  M5.Display.setPivot(120, 120);
  mySprite.setPivot(120, 120);
  mySprite.pushRotated(pivotAngle, TFT_PINK);  
  mySprite.deleteSprite();
}
