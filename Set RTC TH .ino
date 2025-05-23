#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;

void setup () {
  Serial.begin(9600);

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power or first time use.");
  }

  // Automatically sets RTC to computer's current time (assumed Thailand time)
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  Serial.println("RTC time has been set to Thailand time.");
}

void loop () {
  DateTime now = rtc.now();
  Serial.print("Current RTC Time: ");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.println(now.second(), DEC);
  delay(1000);
}
