#include <arduino.h>
#include <IoTT_LocoNetHBESP32.h> 

/////////USER CONFIGURATION//////////////////////////////////////////
#define pinRx    22  //pin used to receive LocoNet signals
#define pinTx    23  //pin used to transmit LocoNet signals
#define pinNop   30  //unused pin to set for the hardware Serial port as transmit pin
#define InverseLogic true
#define showPerformance
/////////END OF USER CONFIGURATION//////////////////////////////////////////


LocoNetESPSerial lnSerial(pinRx, pinTx, InverseLogic); //true is inverted signals

//used for performance analysis
#ifdef showPerformance
  uint16_t loopCtr = 0;
  uint32_t myTimer = millis() + 1000;
#endif

void onLocoNetMessage(lnReceiveBuffer * newData)
{
   Serial.printf("LN Msg Error Flags %2X ReqID %i with %i bytes requested %i response time %i echo time %i: ", newData->errorFlags, newData->reqID, newData->lnMsgSize, newData->reqRecTime, newData->reqRespTime, newData->echoTime); 
   for (int i=0; i<newData->lnMsgSize; i++)
   {
     Serial.print(newData->lnData[i],16);
     Serial.print(" ");
   }
   Serial.println();
}
 
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1000);
  lnSerial.setBusyLED(LED_BUILTIN);
  Serial.println("Init Done");
  randomSeed((uint32_t)ESP.getEfuseMac()); //initialize random generator with MAC
}

void sendBDInput(uint16_t bdNum, bool bdStat)
{
  lnTransmitMsg myMsg;
  myMsg.lnMsgSize = 4;
  myMsg.reqID = random(2000);
  myMsg.lnData[0] = 0xB2; //B2 1A 50 7
  myMsg.lnData[1] = (bdNum & 0x7E) >> 1;
  myMsg.lnData[2] = ((uint8_t)bdStat<<5) | ((bdNum & 0x01)<<4);
  myMsg.lnData[3] = myMsg.lnData[0] ^ myMsg.lnData[1] ^ myMsg.lnData[2] ^ 0xFF;
  if (lnSerial.carrierOK())
  {
    int numBytes = lnSerial.lnWriteMsg(myMsg);
    Serial.printf("Write %i bytes for ID %i Target %i\n", numBytes, myMsg.reqID, myMsg.lnMsgSize);
  }
  else
    Serial.println("LocoNet not connected");
}

void sendLocoReq(uint8_t LocoNum)
{
  lnTransmitMsg myMsg;
  myMsg.lnMsgSize = 4;
  myMsg.reqID = random(2000);
  myMsg.lnData[0] = 0xBF; //B2 1A 50 7
  myMsg.lnData[1] = 0;
  myMsg.lnData[2] = LocoNum;
  myMsg.lnData[3] = myMsg.lnData[0] ^ myMsg.lnData[1] ^ myMsg.lnData[2] ^ 0xFF;
  if (lnSerial.carrierOK())
  {
    int numBytes = lnSerial.lnWriteMsg(myMsg);
    Serial.printf("Write %i bytes for ID %i Target %i\n", numBytes, myMsg.reqID, myMsg.lnMsgSize);
  }
  else
    Serial.println("LocoNet not connected");
}
void loop() {
  // put your main code here, to run repeatedly:

#ifdef showPerformance
  loopCtr++;
  if (millis() > myTimer)
  {
    Serial.printf("Timer Loop: %i\n", loopCtr);
    loopCtr = 0;
    myTimer += 1000;
  }
#endif
  lnSerial.processLoop();

  if (Serial.available())
  {
    char c;
    while (Serial.available())
      c = Serial.read();
    Serial.println("Sending LN Messages:");
    for (int i = 0; i < 8; i++)
    {
      sendLocoReq(50+i);
      sendBDInput(i, false);
      sendBDInput(i, true);
    }
  }
  yield();
}
