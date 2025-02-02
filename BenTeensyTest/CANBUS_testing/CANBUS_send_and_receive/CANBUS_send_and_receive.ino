#include <FlexCAN_T4.h>

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;


void setup() {
 Serial.begin(9600);
 while (!Serial);

 can1.begin();
 can1.setBaudRate(500000); 
}

void loop() {
 CAN_message_t msgTransmit, msgReceive;
 
// adress of receiving device
 msgTransmit.id = 0x82; 

 // this is the length (bytes) of can message
 msgTransmit.len = 4; 

 //example hex value in first CAN message byte
 msgTransmit.buf[0] = 0x90;

// this part is from some weird online example
// this is converting int torqueValue into 2 bytes [1] and [2] using bitwise op
// Set torque command value to 50%
 int16_t torqueValue = 16380; // 50% of maximum torque
 msgTransmit.buf[1] = torqueValue >> 8; // High byte
 msgTransmit.buf[2] = torqueValue & 0xFF; // Low byte
 // example usage of last byte in message
 msgTransmit.buf[3] =  0x88; 
 
 // writes message over can
 can1.write(msgTransmit);
 // reads can
 can1.read(msgReceive);

// msgRecieve.id is the adress for the intended recipient
// this is set by sender (see msgTransmit.id) 
// this is checking to handle messages with this (teensy) device adress
 if(msgReceive.id == 0x78){
  Serial.println("This message is for me! \nData: ");
  // print message
  for (int byte : msgReceive.buf){
    Serial.print(byte);
  }
  Serial.println();
 }

delay(100);
}