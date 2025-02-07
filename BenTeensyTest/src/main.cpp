#include <FlexCAN_T4.h>

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;

 int countInit = 0;
 int count = 0;

void setup() {
 Serial.begin(9600);
 while (!Serial);

 can1.begin();
 can1.setBaudRate(500000); 

}

void loop() {
CAN_message_t msgTransmit, msgReceive;
 
// adress of receiving device
 count ++;
  msgTransmit.id = count;
  Serial.print(count);
  Serial.println();



  
 // this is the length (bytes) of can message
 msgTransmit.len = 4; 

 //example hex value in first CAN message byte
 msgTransmit.buf[0] = 0xFF;
 msgTransmit.buf[1] = 0xFF; 
 msgTransmit.buf[2] = 0xFF; 
 msgTransmit.buf[3] = 0xFF; 
 
 // writes message over can
 can1.write(msgTransmit);
 Serial.println("teensy transmited data  --------------------------------------------------------------");
 
 // reads can
 can1.read(msgReceive);

// msgRecieve.id is the adress for the intended recipient
// this is set by sender (see msgTransmit.id) 
// this is checking to handle messages with this (teensy) device adress
if ((msgReceive.id == 0x1B71 || msgReceive.id == 0x971) && msgReceive.flags.extended) {
  Serial.println("=====================================================");
  Serial.println("This message is for me! \nData: ");
  // print message
  for (int byte : msgReceive.buf){
    Serial.print(byte);
  }
  Serial.println();
  Serial.println("=====================================================");
 }else if (msgReceive.id != 0){
    Serial.print("Unexpected message: ");
    Serial.print(msgReceive.id,HEX);

    Serial.println();
 }


 delay(100);
}