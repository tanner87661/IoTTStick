#include <LocoNet.h>
#include <NmraDcc.h>

/*
 * Test Phases
 * 1 Init DCC for pin 2, display all incoming commands for 5 Secs 
 * 2 Init DCC for pin 3, display all incoming commands for 5 Secs 
 * 3 Reduce DCC display to switch commands, send SW_REQ to random switch every 1 sec for 5 secs
 */

NmraDcc  Dcc ;
DCC_MSG  Packet ;

#define LN_TX_PIN     6   // UNO

#define DCC_PIN_A     2
#define DCC_PIN_B     3

// Loconet turnout position definitions to make code easier to read
#define TURNOUT_NORMAL     1  // aka closed
#define TURNOUT_DIVERGING  0  // thrown
#define LN_TURNOUT   36  // change to match your layout...

bool dispAllDCC = false;
bool usePinA = true;
uint8_t testPhase = 0;
uint32_t testCtr = millis();

#define testTimeDCC 5000
#define testTimeLN 5000


lnMsg  *LnPacket;          // pointer to a received LNet packet

void initDCC(uint8_t pinNr)
{
#ifdef digitalPinToInterrupt
  Dcc.pin(pinNr, 0);
#else
  Dcc.pin(0, pinNr, 1);
#endif
}

void setup() {
  // put your setup code here, to run once:

    Serial.begin(115200);
    LocoNet.init(LN_TX_PIN);

  initDCC(usePinA ? DCC_PIN_A : DCC_PIN_B);
  // Call the main DCC Init function to enable the DCC Receiver
  Dcc.init( MAN_ID_DIY, 10, CV29_ACCESSORY_DECODER | CV29_OUTPUT_ADDRESS_MODE, 0 );

  Serial.println("Init Done");

}

// Construct a Loconet packet that requests a turnout to set/change its state
void sendOPC_SW_REQ(int address, byte dir, byte on) {
    lnMsg SendPacket ;
    
    int sw2 = 0x00;
    if (dir == TURNOUT_NORMAL) { sw2 |= B00100000; }
    if (on) { sw2 |= B00010000; }
    sw2 |= (address >> 7) & 0x0F;
    
    SendPacket.data[ 0 ] = OPC_SW_REQ ;
    SendPacket.data[ 1 ] = address & 0x7F ;
    SendPacket.data[ 2 ] = sw2 ;
    
    LocoNet.send( &SendPacket );
    Serial.println("Send Swi Req");
}

// Some turnout decoders (DS54?) can use solenoids, this code emulates the digitrax 
// throttles in toggling the "power" bit to cause a pulse
void setLNTurnout(int address, byte dir) {
    sendOPC_SW_REQ(address - 1, dir, 1);
    sendOPC_SW_REQ(address - 1, dir, 0);
}

#define NOTIFY_DCC_MSG
#ifdef  NOTIFY_DCC_MSG
void notifyDccMsg( DCC_MSG * Msg)
{
  if (testPhase > 1) return;
  Serial.print("notifyDccMsg: Pin ") ;
  Serial.print(usePinA ? DCC_PIN_A : DCC_PIN_B);
  Serial.print(" ");
  for(uint8_t i = 0; i < Msg->Size; i++)
  {
    Serial.print(Msg->Data[i], HEX);
    Serial.write(' ');
  }
  Serial.println();
}
#endif

// This function is called whenever a normal DCC Turnout Packet is received and we're in Output Addressing Mode
void notifyDccAccTurnoutOutput( uint16_t Addr, uint8_t Direction, uint8_t OutputPower )
{
  if (Addr != LN_TURNOUT+1)
    sendOPC_SW_REQ(LN_TURNOUT,1,1);
  Serial.print("notifyDccAccTurnoutOutput: ") ;
  Serial.print(Addr,DEC) ;
  Serial.print(',');
  Serial.print(Direction,DEC) ;
  Serial.print(',');
  Serial.println(OutputPower, HEX) ;
}

// This function is called whenever a normal DCC Turnout Packet is received and we're in Board Addressing Mode
void notifyDccAccTurnoutBoard( uint16_t BoardAddr, uint8_t OutputPair, uint8_t Direction, uint8_t OutputPower )
{
  Serial.print("notifyDccAccTurnoutBoard: ") ;
  Serial.print(BoardAddr,DEC) ;
  Serial.print(',');
  Serial.print(OutputPair,DEC) ;
  Serial.print(',');
  Serial.print(Direction,DEC) ;
  Serial.print(',');
  Serial.println(OutputPower, HEX) ;
}

// This call-back function is called from LocoNet.processSwitchSensorMessage
// for all Sensor messages
void notifySensor( uint16_t Address, uint8_t State ) {
  Serial.print("Sensor: ");
  Serial.print(Address, DEC);
  Serial.print(" - ");
  Serial.println( State ? "Active" : "Inactive" );
}

// This call-back function is called from LocoNet.processSwitchSensorMessage
// for all Switch Request messages
void notifySwitchRequest( uint16_t Address, uint8_t Output, uint8_t Direction ) {
  Serial.print("Switch Request: ");
  Serial.print(Address, DEC);
  Serial.print(':');
  Serial.print(Direction ? "Closed" : "Thrown");
  Serial.print(" - ");
  Serial.println(Output ? "On" : "Off");
}

// This call-back function is called from LocoNet.processSwitchSensorMessage
// for all Switch Output Report messages
void notifySwitchOutputsReport( uint16_t Address, uint8_t ClosedOutput, uint8_t ThrownOutput) {
  Serial.print("Switch Outputs Report: ");
  Serial.print(Address, DEC);
  Serial.print(": Closed - ");
  Serial.print(ClosedOutput ? "On" : "Off");
  Serial.print(": Thrown - ");
  Serial.println(ThrownOutput ? "On" : "Off");
}

// This call-back function is called from LocoNet.processSwitchSensorMessage
// for all Switch Sensor Report messages
void notifySwitchReport( uint16_t Address, uint8_t State, uint8_t Sensor ) {
  Serial.print("Switch Sensor Report: ");
  Serial.print(Address, DEC);
  Serial.print(':');
  Serial.print(Sensor ? "Switch" : "Aux");
  Serial.print(" - ");
  Serial.println( State ? "Active" : "Inactive" );
}

// This call-back function is called from LocoNet.processSwitchSensorMessage
// for all Switch State messages
void notifySwitchState( uint16_t Address, uint8_t Output, uint8_t Direction ) {
  Serial.print("Switch State: ");
  Serial.print(Address, DEC);
  Serial.print(':');
  Serial.print(Direction ? "Closed" : "Thrown");
  Serial.print(" - ");
  Serial.println(Output ? "On" : "Off");
}

// This call-back function is called from LocoNet.processSwitchSensorMessage
// for all Power messages
void notifyPower(uint8_t State) {
  Serial.print("Layout Power State: ");
  Serial.println(State ? "On" : "Off");
}

// This call-back function is called from LocoNet.processSwitchSensorMessage
// for all MultiSensePower messages
void notifyMultiSensePower(uint8_t BoardID, uint8_t Subdistrict, uint8_t Mode, uint8_t Direction) {
  Serial.print("MultiSensePower: Board ID: ");
  Serial.print(BoardID, DEC);
  Serial.print(" Sub District: ");
  Serial.print(Subdistrict, DEC);
  Serial.print(" Mode: ");
  Serial.print(Mode, DEC);
  Serial.print(" Direction: ");
  Serial.println(Direction, DEC);
}

// This call-back function is called from LocoNet.processSwitchSensorMessage
// for all notifyMultiSenseTransponder messages
void notifyMultiSenseTransponder(uint16_t Address, uint8_t Zone, uint16_t LocoAddress, uint8_t Present) {
  Serial.print("MultiSenseTransponder: Address: ");
  Serial.print(Address, DEC);
  Serial.print(" Zone: ");
  Serial.print(Zone, DEC);
  Serial.print(" Loco Address: ");
  Serial.print(LocoAddress, DEC);
  Serial.print(" Present: ");
  Serial.println(Present, DEC);
}

// This call-back function is called from LocoNet.processSwitchSensorMessage
// for all LongAck messages

void notifyLongAck(uint8_t d1, uint8_t d2) {
  Serial.print("LongACK : Data Byte 1: ");
  Serial.print(d1, DEC);
  Serial.print(" Data Byte 2: ");
  Serial.println(d2, DEC);

}

void loop() {
  // put your main code here, to run repeatedly:
  Dcc.process();

  LnPacket = LocoNet.receive() ;
  if ( LnPacket ) {
    // First print out the packet in HEX
    Serial.print("RX: ");
    uint8_t msgLen = getLnMsgSize(LnPacket);
    for (uint8_t x = 0; x < msgLen; x++)
    {
      uint8_t val = LnPacket->data[x];
      // Print a leading 0 if less than 16 to make 2 HEX digits
      if (val < 16)
        Serial.print('0');

      Serial.print(val, HEX);
      Serial.print(' ');
    }

    // If this packet was not a Switch or Sensor Message then print a new line
    Serial.println();
    }
  }
