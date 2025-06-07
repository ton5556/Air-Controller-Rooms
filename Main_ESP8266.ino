// === Final ESP32 Smart AC Controller Code ===

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <time.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

WebServer server(80);
IRsend irsend(4); // IR LED connected to GPIO 4

// IR signal examples
uint16_t rawOnSignal[] = {38, 3500, 1650, 450, 1300, 450, 450};
uint16_t rawOffSignal[] = {38, 3450, 1700, 450, 1300, 450, 400};

bool acIsOn = false;
bool systemEnabled = true;
unsigned long systemStartTime = 0;
unsigned long lastOnTime = 0;
unsigned long todayOnTime = 0;

struct Schedule {
  int weekdayOnHour = 17, weekdayOnMinute = 0;
  int weekdayOffHour = 23, weekdayOffMinute = 0;
  int weekendOnHour = 13, weekendOnMinute = 0;
  int weekendOffHour = 23, weekendOffMinute = 0;
  bool weekdayEnabled = true;
  bool weekendEnabled = true;
} currentSchedule;

struct tm timeinfo;
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7 * 3600;
const int daylightOffset_sec = 0;

#include "htmlInterface.h" // place your html string here from the earlier file

void setupWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());
}

void enableCORS() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

void handleRoot() {
  enableCORS();
  server.send_P(200, "text/html", htmlInterface);
}

void handleControl() {
  enableCORS();
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
    return;
  }

  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, server.arg("plain"));
  if (error) {
    server.send(400, "text/plain", "Bad Request");
    return;
  }

  String action = doc["action"];
  if (action == "turnOn") controlAC(true);
  else if (action == "turnOff") controlAC(false);
  else if (action == "enableSystem") systemEnabled = true;
  else if (action == "disableSystem") systemEnabled = false;

  server.send(200, "text/plain", "OK");
}

void handleGetSchedule() {
  enableCORS();
  StaticJsonDocument<256> doc;
  doc["weekdayOnHour"] = currentSchedule.weekdayOnHour;
  doc["weekdayOnMinute"] = currentSchedule.weekdayOnMinute;
  doc["weekdayOffHour"] = currentSchedule.weekdayOffHour;
  doc["weekdayOffMinute"] = currentSchedule.weekdayOffMinute;
  doc["weekendOnHour"] = currentSchedule.weekendOnHour;
  doc["weekendOnMinute"] = currentSchedule.weekendOnMinute;
  doc["weekendOffHour"] = currentSchedule.weekendOffHour;
  doc["weekendOffMinute"] = currentSchedule.weekendOffMinute;
  doc["weekdayEnabled"] = currentSchedule.weekdayEnabled;
  doc["weekendEnabled"] = currentSchedule.weekendEnabled;

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleSchedule() {
  enableCORS();
  StaticJsonDocument<256> doc;
  deserializeJson(doc, server.arg("plain"));

  currentSchedule.weekdayOnHour = doc["weekdayOnHour"];
  currentSchedule.weekdayOnMinute = doc["weekdayOnMinute"];
  currentSchedule.weekdayOffHour = doc["weekdayOffHour"];
  currentSchedule.weekdayOffMinute = doc["weekdayOffMinute"];
  currentSchedule.weekendOnHour = doc["weekendOnHour"];
  currentSchedule.weekendOnMinute = doc["weekendOnMinute"];
  currentSchedule.weekendOffHour = doc["weekendOffHour"];
  currentSchedule.weekendOffMinute = doc["weekendOffMinute"];
  currentSchedule.weekdayEnabled = doc["weekdayEnabled"];
  currentSchedule.weekendEnabled = doc["weekendEnabled"];

  server.send(200, "text/plain", "Schedule updated");
}

void handleStatus() {
  enableCORS();
  getLocalTime(&timeinfo);
  StaticJsonDocument<256> doc;
  doc["acIsOn"] = acIsOn;
  doc["systemEnabled"] = systemEnabled;
  doc["currentTime"] = getTimeString();
  doc["currentDate"] = getDateString();
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

String getTimeString() {
  char buffer[10];
  sprintf(buffer, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
  return String(buffer);
}

String getDateString() {
  char buffer[12];
  sprintf(buffer, "%04d-%02d-%02d", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday);
  return String(buffer);
}

void controlAC(bool turnOn) {
  if (turnOn && !acIsOn) {
    irsend.sendRaw(rawOnSignal, sizeof(rawOnSignal) / sizeof(rawOnSignal[0]), 38);
    acIsOn = true;
    lastOnTime = millis();
  } else if (!turnOn && acIsOn) {
    irsend.sendRaw(rawOffSignal, sizeof(rawOffSignal) / sizeof(rawOffSignal[0]), 38);
    acIsOn = false;
    todayOnTime += (millis() - lastOnTime) / 60000; // minutes
  }
}

void checkSchedule() {
  getLocalTime(&timeinfo);
  int h = timeinfo.tm_hour;
  int m = timeinfo.tm_min;
  int wd = timeinfo.tm_wday; // Sunday = 0

  if (!systemEnabled) return;

  bool on = false, off = false;
  if (wd >= 1 && wd <= 5 && currentSchedule.weekdayEnabled) {
    if (h == currentSchedule.weekdayOnHour && m == currentSchedule.weekdayOnMinute && !acIsOn) on = true;
    if (h == currentSchedule.weekdayOffHour && m == currentSchedule.weekdayOffMinute && acIsOn) off = true;
  }
  if ((wd == 0 || wd == 6) && currentSchedule.weekendEnabled) {
    if (h == currentSchedule.weekendOnHour && m == currentSchedule.weekendOnMinute && !acIsOn) on = true;
    if (h == currentSchedule.weekendOffHour && m == currentSchedule.weekendOffMinute && acIsOn) off = true;
  }

  if (on) controlAC(true);
  if (off) controlAC(false);
}

void setup() {
  Serial.begin(115200);
  irsend.begin();
  setupWiFi();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  server.on("/", handleRoot);
  server.on("/api/status", handleStatus);
  server.on("/api/control", handleControl);
  server.on("/api/schedule", HTTP_GET, handleGetSchedule);
  server.on("/api/schedule", HTTP_POST, handleSchedule);

  server.begin();
  Serial.println("Server started");
}

void loop() {
  server.handleClient();
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 60000) { // check every minute
    checkSchedule();
    lastCheck = millis();
  }
}
