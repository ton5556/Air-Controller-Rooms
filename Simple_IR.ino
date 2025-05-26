#include <IRremote.h>

const int RECV_PIN = 4;

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("Starting minimal IR test...");
  Serial.print("Free heap: ");
  Serial.println(ESP.getFreeHeap());
  
  // Simple initialization
  IrReceiver.begin(RECV_PIN);
  Serial.println("IR receiver initialized on pin 4");
}

void loop() {
  if (IrReceiver.decode()) {
    Serial.println("Signal received!");
    Serial.print("Protocol: ");
    Serial.println(IrReceiver.decodedIRData.protocol);
    
    IrReceiver.resume();
  }
  delay(100);
}
