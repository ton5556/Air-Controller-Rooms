#include <IRremote.h>         // Include the IRremote library

#define IR_RECEIVE_PIN 2      // Define the pin where your IR receiver is connected

IRrecv IR(IR_RECEIVE_PIN);    // Create an IRrecv object

void setup() {
  Serial.begin(9600);         // Start the Serial Monitor
  IR.enableIRIn();            // Start the receiver
}

void loop() {
  if (IR.decode()) {          // Check if an IR signal has been received
    Serial.println(IR.decodedIRData.decodedRawData, HEX);  // Print raw data in HEX format
    delay(1500);              // Optional delay (1.5 sec)
    IR.resume();              // Prepare for the next signal
  }
}
