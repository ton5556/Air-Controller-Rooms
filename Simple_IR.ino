#include <IRremote.h>

const int RECV_PIN = 2; // IR Receiver pin
IRrecv irrecv(RECV_PIN);
IRRawData rawData;      // For raw data storage

void setup() {
  Serial.begin(9600);
  Serial.println("IR Receiver ready - Press AC remote buttons");
  IrReceiver.begin(RECV_PIN, ENABLE_LED_FEEDBACK); // Start receiver
}

void loop() {
  if (IrReceiver.decode()) {
    Serial.println("=== IR Signal Received ===");
    
    // Print raw data length and content
    Serial.print("Raw data (");
    Serial.print(IrReceiver.decodedIRData.rawDataPtr->rawlen);
    Serial.println("):");
    
    // Print raw buffer
    for (uint16_t i = 1; i < IrReceiver.decodedIRData.rawDataPtr->rawlen; i++) {
      Serial.print(IrReceiver.decodedIRData.rawDataPtr->rawbuf[i] * MICROS_PER_TICK);
      Serial.print(", ");
    }
    Serial.println();
    
    // Resume receiver
    IrReceiver.resume();
  }
}
