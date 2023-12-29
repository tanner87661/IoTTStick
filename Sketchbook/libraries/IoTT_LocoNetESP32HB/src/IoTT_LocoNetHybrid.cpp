/*

Target CPU: ESP32
UART Emulation with special features for LocoNet (half-duplex network with DCMA) by Hans Tanner. 
See Digitrax LocoNet PE documentation for more information

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (arxpint your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

#include <IoTT_LocoNetHybrid.h>


volatile uint8_t pinRx;
volatile uint8_t pinTx;

#define defaultTransmitAttempts 25 //as per LocoNet standard
#define defaultNetworkAccessAttempts 1000 // = 15ms as per LocoNet standard
#define defaultCDBackoffPriority 80 //20 bit times

#define uartrxBufferSize 50
#define uarttxBufferSize 50 //longest possible LocoNet message is 48 bytes, buffer must be more than that

#define maxBusy 100

//volatile uartRxStateType uartRxState = waitStartBit;
volatile uartModeType uartMode = uart_idle;
volatile uartTxStateType uartTxState = sendStartBit;


volatile uint8_t currentCDBackoffPriority = defaultCDBackoffPriority; //the current priority, maybe decreased after unsuccessful transmit attempts
volatile uint8_t cdBackoffCounter = defaultCDBackoffPriority;
volatile uint16_t transmitAttemptCounter;
volatile uint16_t networkAccessAttemptCounter;

volatile uint8_t bitCounter = 0;
volatile uint16_t bitMask;

/*
volatile uint8_t uartrdPointer = 0;
volatile uint8_t uartwrPointer = 0;
volatile uint16_t uartBuffer[uartrxBufferSize];

HardwareSerial * uartPort = NULL;
*/

//volatile uint8_t tmpPointer;	

typedef struct {  
	uint8_t txrdPointer = 0;
	uint8_t txtmprdPointer = 0;
	uint8_t txwrPointer = 0;
	uint8_t txBuffer[uarttxBufferSize];
	uint8_t rxrdPointer = 0;
	uint8_t rxwrPointer = 0;
	uint16_t rxBuffer[uartrxBufferSize];
	uint16_t commError = 0;
} hybridtxbuffer;

volatile hybridtxbuffer txBuf;
volatile hybridtxbuffer busyBuf;
volatile hybridtxbuffer * thisBuffer = &txBuf;

volatile uint8_t busyCtr = maxBusy;

volatile uint8_t lastByte;

volatile uint16_t lastBitTime;
volatile uint8_t  timerSelector;

volatile uint32_t lastCarrierOKTicker = carrierLossNotification;

volatile uint32_t lastRxChangeTime;

volatile bool inverseLogicRx;
volatile bool inverseLogicTx;

volatile bool timerHighSpeed;

volatile int8_t busyLED = -1;
volatile bool   busyLevel = false;

volatile bool insertBusyOPC = false;

nodeType networkType = standardMode;

volatile uint8_t opcBusy[] = {0x81, 0x7E};

hw_timer_t * timer = NULL;
//portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
//portMUX_TYPE inputMux = portMUX_INITIALIZER_UNLOCKED;

/* the timer interrupt handles LocoNet access, transmit and receiving of data
 * in Idle state, it checks if data needs to be sent. If so, it does a prioritized network access based on CD. Prioritiy is increased one bit time (starting at 20) every 15ms as per LocoNet standard
 * in receiving mode, data is sampled 30 microseconds into the bit (in the middle of the bit). This ensures correct sampling over the entire byte even in case of slight difference in bittiming
 * in transmit mode, the line level is set and then a collision check is performed after 15, 30, 45 and 60 microseconds. This ensures any collision is properly detected (as it will be longer than 15 micros)
 * and gives a good protection against random spikes and instabilities after initially setting the bit level (LocoNet raise/fall times)
 * the service also handles the CD backoff timing and switching off the LocoNet BUSY LED (if defined)
 * in case of a collision, a 15 bit break is sent
*/

static void IRAM_ATTR hybrid_driver() //timer interrupt occurs every 15 micros.
{
	uint8_t tmpPointer;
/*	
	if (uartPort)
		while (uartPort->available())
		{
			uartwrPointer = (uartwrPointer + 1) % uartrxBufferSize;
			uartBuffer[uartwrPointer] = uartPort->read();
		}
*/	
  switch (uartMode)
  {
    case uart_idle: //if transmitData is available and is a valid LocoNet message, access network and start sending
	{
      if (thisBuffer->txrdPointer != thisBuffer->txwrPointer) //we have a need to send a message
      {
        if (thisBuffer->txrdPointer == thisBuffer->txtmprdPointer) //
        {
          thisBuffer->txtmprdPointer = (thisBuffer->txrdPointer + 1) % uarttxBufferSize; //set temporary pointer to data to be sent
          transmitAttemptCounter = defaultTransmitAttempts;
          networkAccessAttemptCounter = defaultNetworkAccessAttempts;
          thisBuffer->commError = 0;
        }
        if (hybrid_LocoNetAvailable() == lnNetAvailable)
//        if (cdBackoffCounter == 0) //network is available
        {
//			Serial.print("s");
//			Serial.println(txBuffer[txtmprdPointer],16);
          if (digitalRead(pinRx) == (inverseLogicRx ? 0:1))
          {
            digitalWrite(pinTx, (inverseLogicTx ? 1:0)); //start sending. This must come within 2us after last check
//  portENTER_CRITICAL_ISR(&inputMux);
            uartMode = uart_transmit;
//  portEXIT_CRITICAL_ISR(&inputMux);
            uartTxState = sendStartBit;
            timerSelector = 0; 
          }
        }
        else
          if (networkAccessAttemptCounter > 0)
            networkAccessAttemptCounter--;
          else
            if (transmitAttemptCounter > 0)
            {
              transmitAttemptCounter--;
              networkAccessAttemptCounter = defaultNetworkAccessAttempts;
              if (currentCDBackoffPriority > 40) //10 bittimes, slower than throttle
                currentCDBackoffPriority -= 4; //increase CDBackoff Priority by 1 bittime
            }
            else //now we are out of options, no network access achieved
            {
              //no network access failure, permanently giving up, data is lost
              thisBuffer->txwrPointer = thisBuffer->txrdPointer;
              thisBuffer->commError |= errorTimeout;
            }
      }      
      else 
      {
	    if (cdBackoffCounter == currentCDBackoffPriority) //this means we have carrier loss
        {
		  lastCarrierOKTicker--;
		  if (lastCarrierOKTicker == 0)
		  {
			lastCarrierOKTicker = carrierLossNotification;
			thisBuffer->commError |= errorCarrierLoss;
            tmpPointer = (thisBuffer->rxwrPointer + 1) % uartrxBufferSize;
            thisBuffer->rxBuffer[tmpPointer] = (thisBuffer->commError<<8);
            thisBuffer->commError = 0;
            thisBuffer->rxwrPointer = tmpPointer;
		  }
	    }
	    else
		  lastCarrierOKTicker = carrierLossNotification;
	  }
      break;  
	}
    case uart_transmit:
      //check for collisions, stop sending if collision is detected and send break. Not needed for MARK=1, but we do it anyway as it can not create a false collision detection

      if (digitalRead(pinRx) != digitalRead(pinTx)) //collision detected
      {
		Serial.print("coll");
        digitalWrite(pinTx, (inverseLogicTx ? 1:0)); //send BREAK
        uartMode = uart_collision;
        bitCounter = 15; //15 bit times
        thisBuffer->commError |= errorCollision;
        if (uartTxState == sendStopBit)
          thisBuffer->commError |= errorFrame;
        tmpPointer = (thisBuffer->rxwrPointer + 1) % uartrxBufferSize;
        thisBuffer->rxBuffer[tmpPointer] = thisBuffer->txBuffer[thisBuffer->txrdPointer] + (thisBuffer->commError<<8);
        thisBuffer->commError = 0;
        thisBuffer->rxwrPointer = tmpPointer;
      }
      else
      {
//			noInterrupts();
      if ((timerSelector & 0x03) == 0)
      {
        //load next bit, send startbit, data bits, stopbit, load next bit, then load next byte until entire message is sent
        switch (uartTxState)
        {
          case sendStartBit: //if this is called, startBit is out, so we go to databits
            bitMask = 0x0001;
            digitalWrite(pinTx, ((bitMask & thisBuffer->txBuffer[thisBuffer->txtmprdPointer]) == (inverseLogicTx ? 0:1))); //send first databit, lsb first
            uartTxState = sendDataBit;
            break;
          case sendDataBit:
            bitMask = (bitMask << 1);
            if (bitMask == 0x0100)
            {
              digitalWrite(pinTx, (inverseLogicTx ? 0:1)); //send stop bit
              uartTxState = sendStopBit;
            }
            else
              digitalWrite(pinTx, ((bitMask & thisBuffer->txBuffer[thisBuffer->txtmprdPointer]) == (inverseLogicTx ? 0:1))); //send next databit
            break;
          case sendStopBit: //if this is called, stopBit is out, we just add additional break to make sure timing errors do not add up
//            uartTxState = sendBreakBit; //come back next time
//            break;
//          case sendBreakBit: //if this is called, stopBit is out, we just add additional break to make sure timing errors do not add up
//            uartTxState = sendBreakBit2; //come back next time
//            break;
          
//          case sendBreakBit: //if this is called, stopBit is out, so we start next byte, if there is one
            thisBuffer->txrdPointer = thisBuffer->txtmprdPointer;
            tmpPointer = (thisBuffer->rxwrPointer + 1) % uartrxBufferSize;
            thisBuffer->commError |= msgEcho;
            thisBuffer->rxBuffer[tmpPointer] = thisBuffer->txBuffer[thisBuffer->txrdPointer] + (thisBuffer->commError<<8);
            thisBuffer->commError = 0;
            thisBuffer->rxwrPointer = tmpPointer;
            if (thisBuffer->txrdPointer == thisBuffer->txwrPointer)// && (!insertBusyOPC)) //no more data in this message and no OPC_BUSY
            {
				if ((insertBusyOPC) && (busyCtr > 0))
				{
					busyCtr--;
					thisBuffer = &busyBuf;
					thisBuffer->txrdPointer = 0;
					thisBuffer->txtmprdPointer = 0;
//					digitalWrite(pinTx, (inverseLogicTx ? 1:0)); //start sending start bit of next byte
//					uartTxState = sendStartBit;
					currentCDBackoffPriority = defaultCDBackoffPriority; //reset network priority in case it was reduced in this transmit attempt
					uartMode = uart_idle;
				}
				else //return to standard buffer pointer and idle mode
				{
					if (thisBuffer == &busyBuf)
					{
						thisBuffer = &txBuf;
//						hybrid_flush();
					}
					currentCDBackoffPriority = defaultCDBackoffPriority; //reset network priority in case it was reduced in this transmit attempt
					uartMode = uart_idle;
				}
			}
            else
            {
				thisBuffer->txtmprdPointer = (thisBuffer->txrdPointer + 1) % uarttxBufferSize;
				digitalWrite(pinTx, (inverseLogicTx ? 1:0)); //start sending start bit of next byte
				uartTxState = sendStartBit;
            }
            break;
        }
      }
//      interrupts();
	}
      break;
    case uart_collision: //if collision has been detected, send BREAK (15 bit times SPACE=0, creates framing error on all listening devices)
      switch (timerSelector & 0x03)
      {
        case 0: //called every 60 microseconds (1 bit time) decrement bit counter, until zero. Then, break is done and we go back to idle status
        {
          bitCounter--;
          if (bitCounter == 0)
          {
            digitalWrite(pinTx, (inverseLogicTx ? 0:1));
            thisBuffer->txwrPointer = thisBuffer->txrdPointer;
            uartMode = uart_idle;
            thisBuffer->commError = 0;
          }
          break;
        }
      }
      break;
  }
  timerSelector++; //increment counter. last 2 bits are used to determine sub-state
  if (digitalRead(pinRx) == (inverseLogicRx ? 1:0))
  {
    cdBackoffCounter = currentCDBackoffPriority;
	if (busyLED >= 0)
		digitalWrite(busyLED, busyLevel);
  }
  else
    if (cdBackoffCounter > 0) 
      if (timerHighSpeed)
		cdBackoffCounter--;
	  else
	    if (cdBackoffCounter > 60)
			cdBackoffCounter -=60;
		else
			cdBackoffCounter =0;
    else
		if (busyLED >= 0)
			digitalWrite(busyLED, !busyLevel);
//	portEXIT_CRITICAL_ISR(&timerMux);
}

void hybrid_highSpeed(bool goFast)
{
	bool changeSpeed = true;
	if (timerHighSpeed != goFast)
	{
		if (goFast)
			timerAlarmWrite(timer, 240, true); //every 15 microseconds for Tx
		else
			if (uartMode == uart_idle) //do not change mode if uart is busy transmitting
				timerAlarmWrite(timer, 16000, true); //every 1 milliseconds for Tx
			else
				changeSpeed = false;
		if (changeSpeed)
		{		
//			Serial.println(goFast ? "fst":"slw");
			timerWrite(timer, 0);
			timerAlarmEnable(timer);
			timerHighSpeed = goFast;
		}
	}
}

void hybrid_begin(uint8_t pinRxNum, uint8_t pinTxNum, bool invLogic)
{
	hybrid_begin(pinRxNum, pinTxNum, invLogic, invLogic);
}

void hybrid_begin(uint8_t pinRxNum, uint8_t pinTxNum, bool invLogicRx, bool invLogicTx)
{
	pinRx = pinRxNum;
	pinTx = pinTxNum;
	inverseLogicTx = invLogicTx;
	inverseLogicRx = invLogicRx;
	pinMode(pinRx, INPUT_PULLUP);
	pinMode(pinTx, OUTPUT);
	digitalWrite(pinTx, inverseLogicTx ? 0:1);
	lastCarrierOKTicker = carrierLossNotification;

    timer = timerBegin(0, 5, true); //prescale to 16MHz, counting up
    timerAttachInterrupt(timer, &hybrid_driver, false); 
    timerHighSpeed = true;
    hybrid_highSpeed(false);
}

void hybrid_end()
{
	Serial.println("Stop Interrupts");
    timerAlarmDisable(timer);
}

void hybrid_setBusyLED(int8_t ledNr, bool logLevel)
{
	if (busyLED != -1)
		pinMode(busyLED, INPUT);
	busyLevel = logLevel;	
	busyLED = ledNr;
	if (busyLED >= 0)
		pinMode(busyLED, OUTPUT);
}

bool hybrid_availableForWrite()
{
	return (txBuf.txwrPointer == txBuf.txrdPointer);// && (thisBuffer == &txBuf));
}

void hybrid_setNetworkType(nodeType newNwType)
{
	networkType = newNwType;
}

nodeType hybrid_getNetworkType()
{
	return networkType;
}

void hybrid_setBusyMode(bool newMode)
{
	if (newMode != insertBusyOPC)
		if ((networkType != standardMode) && (uartMode == uart_idle))
		{
			insertBusyOPC = newMode; //only changeable for limited or full master
			if (insertBusyOPC)
			{
			//set busyBuf data
				busyBuf.txrdPointer = 0;
				busyBuf.txtmprdPointer = 0;
				busyBuf.txwrPointer = 2;
				busyBuf.txBuffer[1] = opcBusy[0];
				busyBuf.txBuffer[2] = opcBusy[1];
				thisBuffer = &busyBuf;
				busyCtr = maxBusy;
//				hybrid_flush();
				hybrid_highSpeed(true);
			}
//			Serial.println(newMode ? "set Busy":"clear Busy");
		}
		else
			insertBusyOPC = false; 
}

bool hybrid_getBusyMode()
{
	return insertBusyOPC;
}

uint16_t hybrid_available()
{
	return (txBuf.rxwrPointer + uartrxBufferSize - txBuf.rxrdPointer) % uartrxBufferSize;
}

bool hybrid_carrierOK()
{
  if (uartMode == uart_idle)
    return (cdBackoffCounter < currentCDBackoffPriority);
  else
    return true;
}

void hybrid_LineBreak(uint16_t numBits)
{
//	Serial.print("Line break");
	hybrid_flush();
    digitalWrite(pinTx, (inverseLogicTx ? 1:0)); //send BREAK
    uartMode = uart_collision;
    bitCounter = numBits; //numBits bit times
}

uint16_t hybrid_read() //always check if data is available before calling this function
{
	if (txBuf.rxrdPointer != txBuf.rxwrPointer)
	{
		uint8_t temprdPointer = (txBuf.rxrdPointer + 1) % uartrxBufferSize;	
		uint16_t thisData = txBuf.rxBuffer[temprdPointer];
//		portENTER_CRITICAL_ISR(&timerMux);
		txBuf.rxrdPointer = temprdPointer;
//		portEXIT_CRITICAL_ISR(&timerMux);
		return thisData;
	}
	else
		return 0;
}

uint8_t hybrid_write(uint8_t * dataByte, uint8_t numBytes)
{
//	portENTER_CRITICAL_ISR(&inputMux);
	uint8_t tempwrPointer = txBuf.txwrPointer;
	uint8_t i;
//	Serial.printf("hybrid_write %i Data byte %i \n", numBytes, dataByte[0]);
	for (i=0; i < numBytes; i++)
	{
		tempwrPointer = (tempwrPointer + 1) % uarttxBufferSize;
		if (tempwrPointer == txBuf.txrdPointer) //buffer overflow protection
			break;
//		Serial.printf("%2X ", dataByte[i]);
		txBuf.txBuffer[tempwrPointer] = dataByte[i];
	}
//	Serial.println();
	if (i == numBytes)
	{
		txBuf.txwrPointer = tempwrPointer;
//		return i;		
	}
	else
	{
//		Serial.println("Hybrid Error");
		i = 0;
//		return 0;
	}
//	portEXIT_CRITICAL_ISR(&inputMux);
	return i;
}

void hybrid_flush()
{
	txBuf.rxrdPointer = txBuf.rxwrPointer;
	txBuf.txrdPointer = txBuf.txwrPointer;
    txBuf.commError = 0;
}

uint8_t  hybrid_LocoNetAvailable()
{
	if (digitalRead(pinRx) == (inverseLogicRx ? 1 : 0))
		return lnBusy;
	else
		if (((cdBackoffCounter == 0) || (networkType != standardMode)) )// && (cdBackoffCounter < (defaultCDBackoffPriority-20)))
		{
			return lnNetAvailable;
		}
		else
			return lnAwaitBackoff;
}

/*
uint8_t uart_available()
{
	if (uartwrPointer >= uartrdPointer)
		return uartwrPointer - uartrdPointer;
	else
		return uartwrPointer + uartrxBufferSize - uartrdPointer;
}

uint8_t uart_read()
{
	uartrdPointer = (uartrdPointer + 1) % uartrxBufferSize;
	return uartBuffer[uartrdPointer];
}

void setUartPort(HardwareSerial * newPort)
{
	uartPort = newPort;
}
*/
