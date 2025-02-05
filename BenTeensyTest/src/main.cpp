#include <FlexCAN_T4.h>

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;

int countInit = 0;
int count = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  can1.begin();
  can1.setBaudRate(500000);
  Serial.println("CAN Bus Initialized.");
}

void loop() {
  CAN_message_t msgTransmit, msgReceive;
  
  // Increment count for message ID
  count++;
  msgTransmit.id = count;
  
  // Print message ID that will be transmitted
  Serial.print("Transmitting message with ID: ");
  Serial.println(count, HEX);

  // Set length of the message
  msgTransmit.len = 4;

  // Example payload: 0x99 repeated in every byte
  msgTransmit.buf[0] = 0x99;
  msgTransmit.buf[1] = 0x99;
  msgTransmit.buf[2] = 0x99;
  msgTransmit.buf[3] = 0x99;

  // Attempt to write message to the CAN bus and check if it succeeds.
  bool success = can1.write(msgTransmit);
  if (success) {
    Serial.println("Teensy transmitted data successfully.");
  } else {
    Serial.println("Transmission failed! Check CAN bus status and wiring.");
  }

  // Attempt to read an incoming CAN message.
  bool received = can1.read(msgReceive);
  if (received) {
    // Check if the received message is intended for this Teensy.
    if (msgReceive.id == 0x871 || msgReceive.id == 0x872) {
      Serial.println("=====================================================");
      Serial.println("This message is for me! Data:");
      for (int i = 0; i < msgReceive.len; i++) {
        Serial.print(msgReceive.buf[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
      Serial.println("=====================================================");
    } else if (msgReceive.id != 0) {
      Serial.print("Unexpected message ID: ");
      Serial.println(msgReceive.id, HEX);
    }
  } else {
    Serial.println("No CAN message received in this cycle.");
  }

  delay(100);
}
