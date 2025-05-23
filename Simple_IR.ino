#include <IRremote.h>
#include <RTClib.h>

#define IR_SEND_PIN 3

RTC_DS3231 rtc;

unsigned int rawOnSignal[] = {
  /* Replace with your real "turn ON" raw signal */
  9000, 4500, 600, 550, 600, 550, /* ... */
};

unsigned int rawOffSignal[] = {
  /* Replace with your real "turn OFF" raw signal */
  9000, 4500, 600, 550, 600, 550, /* ... */
};

bool acIsOn = false;

void setup() {
  Serial.begin(9600);

  // IR Transmitter setup
  IrSender.begin(IR_SEND_PIN);

  // RTC setup
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  Serial.println("System ready.");
}

void loop() {
  DateTime now = rtc.now();
  int currentHour = now.hour();
  int currentMinute = now.minute();
  int currentDay = now.dayOfTheWeek(); // Sunday = 0

  bool shouldTurnOn = false;
  bool shouldTurnOff = false;

  // Monday (1) to Friday (5)
  if (currentDay >= 1 && currentDay <= 5) {
    if (currentHour == 17 && currentMinute == 0 && !acIsOn) shouldTurnOn = true;
    if (currentHour == 23 && currentMinute == 0 && acIsOn) shouldTurnOff = true;
  }

  // Saturday (6) and Sunday (0)
  if (currentDay == 6 || currentDay == 0) {
    if (currentHour == 13 && currentMinute == 0 && !acIsOn) shouldTurnOn = true;
    if (currentHour == 23 && currentMinute == 0 && acIsOn) shouldTurnOff = true;
  }

  if (shouldTurnOn) {
    Serial.println("Turning ON AC...");
    IrSender.sendRaw(rawOnSignal, sizeof(rawOnSignal) / sizeof(rawOnSignal[0]), 38);
    acIsOn = true;
    delay(60000); // wait 1 min to avoid retrigger
  }

  if (shouldTurnOff) {
    Serial.println("Turning OFF AC...");
    IrSender.sendRaw(rawOffSignal, sizeof(rawOffSignal) / sizeof(rawOffSignal[0]), 38);
    acIsOn = false;
    delay(60000); // wait 1 min to avoid retrigger
  }

  delay(1000);
}
