#include <IRremote.h>

const int RECV_PIN = 5; // IR Receiver pin (GPIO5 on ESP32-S3)
const int LED_PIN = 2;  // Built-in LED on ESP32-S3

void setup() {
  Serial.begin(115200); // Higher baud rate for ESP32
  pinMode(LED_PIN, OUTPUT);
  
  Serial.println("ESP32-S3 IR Receiver ready - Press AC remote buttons");
  Serial.println("========================================");
  
  // Start the receiver
  IrReceiver.begin(RECV_PIN, ENABLE_LED_FEEDBACK);
  
  // Print some info
  Serial.print("Ready to receive IR signals @ Pin ");
  Serial.println(RECV_PIN);
}

void loop() {
  if (IrReceiver.decode()) {
    // Visual feedback
    digitalWrite(LED_PIN, HIGH);
    
    Serial.println("\n=== IR Signal Received ===");
    
    // Print protocol information
    Serial.print("Protocol: ");
    Serial.println(getProtocolString(IrReceiver.decodedIRData.protocol));
    
    // Print address and command if available
    if (IrReceiver.decodedIRData.protocol != UNKNOWN) {
      Serial.print("Address: 0x");
      Serial.println(IrReceiver.decodedIRData.address, HEX);
      Serial.print("Command: 0x");
      Serial.println(IrReceiver.decodedIRData.command, HEX);
    }
    
    // Print raw data timing
    Serial.print("Raw data length: ");
    Serial.println(IrReceiver.decodedIRData.rawDataPtr->rawlen);
    
    Serial.print("Raw timing data (microseconds): ");
    for (uint16_t i = 1; i < IrReceiver.decodedIRData.rawDataPtr->rawlen; i++) {
      if (i > 1) Serial.print(", ");
      Serial.print(IrReceiver.decodedIRData.rawDataPtr->rawbuf[i] * MICROS_PER_TICK);
      
      // Line break every 10 values for readability
      if (i % 10 == 0) {
        Serial.println();
        Serial.print("  ");
      }
    }
    Serial.println();
    
    // Print hex dump format (useful for sending)
    Serial.print("sendRaw({");
    for (uint16_t i = 1; i < IrReceiver.decodedIRData.rawDataPtr->rawlen; i++) {
      if (i > 1) Serial.print(", ");
      Serial.print(IrReceiver.decodedIRData.rawDataPtr->rawbuf[i] * MICROS_PER_TICK);
    }
    Serial.print("}, ");
    Serial.print(IrReceiver.decodedIRData.rawDataPtr->rawlen - 1);
    Serial.println(", 38);");
    
    Serial.println("========================================");
    
    // Turn off LED
    digitalWrite(LED_PIN, LOW);
    
    // Resume receiver for next signal
    IrReceiver.resume();
  }
  
  delay(100); // Small delay to prevent overwhelming serial output
}
