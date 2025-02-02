#include <FlexCAN_T4.h>

//============================================================

// this code is slop
// gpt made, sends a single packet of bullshit to nowhere

//=============================================================



// Create a CAN1 object with a receive buffer size of 256 and a transmit buffer size of 16.
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;

void setup() {
  Serial.begin(9600);
  Serial.println("VESC Current Control via CAN");
  Serial.println("Enter desired current (in amps) and press Enter:");
  
  // Start CAN bus at 500 kbps.
  can1.begin();
  can1.setBaudRate(500000);
}

void loop() {
  // If user input is available...
  if (Serial.available() > 0) {
    // Read the float value entered.
    float desiredCurrent = Serial.parseFloat();

    // Clear any extra characters from the Serial buffer.
    while (Serial.available() > 0) {
      Serial.read();
    }
    
    // Build the CAN message.
    CAN_message_t msgTransmit;
    // VESC frame id as per the provided chart.
    msgTransmit.id = 0x114;
    // Payload is 5 bytes:
    //   Byte 0: Packet ID for SET CURRENT (3)
    //   Bytes 1-4: 32-bit float (little-endian) of the desired current.
    msgTransmit.len = 5;
    msgTransmit.buf[0] = 3;  // SET CURRENT command

    // Convert the float to 4 bytes.
    union {
      float f;
      uint8_t b[4];
    } converter;
    converter.f = desiredCurrent;
    
    // Copy the 4 bytes into the message buffer.
    for (int i = 0; i < 4; i++) {
      msgTransmit.buf[i + 1] = converter.b[i];
    }
    
    // Transmit the message over CAN.
    can1.write(msgTransmit);
    
    // Print feedback.
    Serial.print("Sent current command: ");
    Serial.print(desiredCurrent, 3);
    Serial.println(" A");
    Serial.println("Enter desired current (in amps) and press Enter:");
  }
  
  delay(100);
}
