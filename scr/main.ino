#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <time.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Mitsubishi.h>

// WiFi credentials
const char* ssid = "-----";
const char* password = "----------";

// Static IP configuration
IPAddress local_IP(192, 168, 1, 142);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);   // Optional
IPAddress secondaryDNS(8, 8, 4, 4); // Optional

ESP8266WebServer server(80);
IRsend irsend(5); // GPIO 5 (D1) for IR LED

// Create Mitsubishi AC object
IRMitsubishiAC ac(5);  // GPIO 5 for IR LED

// System state variables
bool acIsOn = false;
bool systemEnabled = true;
unsigned long systemStartTime = 0;
unsigned long lastOnTime = 0;
unsigned long todayOnTime = 0;
unsigned long dailyResetTime = 0;
unsigned long lastDayCheck = 0;

// Schedule structure
struct Schedule {
  int weekdayOnHour1 = 8, weekdayOnMinute1 = 0;
  int weekdayOffHour1 = 12, weekdayOffMinute1 = 0;
  bool weekdayEnabled1 = true;
  int weekdayOnHour2 = 14, weekdayOnMinute2 = 0;
  int weekdayOffHour2 = 18, weekdayOffMinute2 = 0;
  bool weekdayEnabled2 = true;
  int weekendOnHour = 10, weekendOnMinute = 0;
  int weekendOffHour = 22, weekendOffMinute = 0;
  bool weekendEnabled = true;
} currentSchedule;

struct tm timeinfo;
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7 * 3600;
const int daylightOffset_sec = 0;

#include "htmlcc.h"

// Utility functions
String formatTime(int hour, int minute) {
  return String(hour < 10 ? "0" : "") + String(hour) + ":" + 
         String(minute < 10 ? "0" : "") + String(minute);
}

String getCurrentTime() {
  if (!getLocalTime(&timeinfo)) {
    return "--:--:--";
  }
  char timeString[20];
  strftime(timeString, sizeof(timeString), "%H:%M:%S", &timeinfo);
  return String(timeString);
}

String getCurrentDate() {
  if (!getLocalTime(&timeinfo)) {
    return "----/--/--";
  }
  char dateString[20];
  strftime(dateString, sizeof(dateString), "%Y/%m/%d", &timeinfo);
  return String(dateString);
}

bool isWeekend() {
  if (!getLocalTime(&timeinfo)) {
    return false;
  }
  return (timeinfo.tm_wday == 0 || timeinfo.tm_wday == 6); // Sunday = 0, Saturday = 6
}

// AC Control function with multiple methods
void controlAC(bool turnOn) {
  Serial.printf("🎯 AC Control Request: %s (Current state: %s)\n", 
                turnOn ? "ON" : "OFF", acIsOn ? "ON" : "OFF");

  if (turnOn && !acIsOn) {
    Serial.println("🟢 Sending Mitsubishi IR ON signal...");
    sendACCommand(true);
    acIsOn = true;
    lastOnTime = millis();
    Serial.println("✅ Mitsubishi AC turned ON");
  }
  else if (!turnOn && acIsOn) {
    Serial.println("🔴 Sending Mitsubishi IR OFF signal...");
    sendACCommand(false);
    acIsOn = false;
    todayOnTime += (millis() - lastOnTime) / 60000;
    Serial.println("✅ Mitsubishi AC turned OFF");
  }
  else {
    Serial.printf("⚠️ AC already in requested state (%s)\n", acIsOn ? "ON" : "OFF");
  }
}

// Force AC control (ignores current state)
void forceControlAC(bool turnOn) {
  Serial.printf("🔄 FORCE AC Control: %s\n", turnOn ? "ON" : "OFF");
  
  if (turnOn) {
    sendACCommand(true);
    acIsOn = true;
    lastOnTime = millis();
    Serial.println("✅ FORCED Mitsubishi AC ON");
  } else {
    sendACCommand(false);
    acIsOn = false;
    if (lastOnTime > 0) {
      todayOnTime += (millis() - lastOnTime) / 60000;
    }
    Serial.println("✅ FORCED Mitsubishi AC OFF");
  }
}

// Toggle AC (for ACs that use same signal for ON/OFF)
void toggleAC() {
  Serial.println("🔄 Toggling AC state...");
  
  // Method 1: Use power toggle
  ac.begin();
  // Send power toggle command
  uint64_t powerCode = 0x23CB260100; // Generic Mitsubishi power toggle (you may need to adjust)
  irsend.sendNEC(powerCode, 32);
  
  // Update our state tracking
  acIsOn = !acIsOn;
  
  if (acIsOn) {
    lastOnTime = millis();
    Serial.println("🔄 AC toggled to ON");
  } else {
    if (lastOnTime > 0) {
      todayOnTime += (millis() - lastOnTime) / 60000;
    }
    Serial.println("🔄 AC toggled to OFF");
  }
  
  delay(500); // Wait between commands
}

// Send AC command with multiple methods
void sendACCommand(bool turnOn) {
  // Method 1: Standard library approach
  Serial.println("📡 Method 1: Standard library command");
  ac.begin();
  
  if (turnOn) {
    ac.on();
    ac.setFan(kMitsubishiAcFanAuto);
    ac.setMode(kMitsubishiAcCool);
    ac.setTemp(24);
    ac.setVane(kMitsubishiAcVaneAuto);
  } else {
    ac.off();
  }
  ac.send();
  delay(1000); // Wait 1 second
  
  // Method 2: Send command twice (some ACs need this)
  Serial.println("📡 Method 2: Repeat command");
  ac.send();
  delay(500);
  
  // Method 3: Alternative approach - send raw power command
  Serial.println("📡 Method 3: Raw IR command");
  if (turnOn) {
    // You might need to capture your actual AC's ON code
    irsend.sendNEC(0x23CB260100, 32); // Example code - adjust as needed
  } else {
    // You might need to capture your actual AC's OFF code  
    irsend.sendNEC(0x23CB260180, 32); // Example code - adjust as needed
  }
  
  delay(500);
}

// WiFi Setup with Static IP
void setupWiFi() {
  // Configure static IP
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("❌ Failed to configure static IP");
  }
  
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi with static IP 192.168.1.142");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(1000);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi connected with static IP!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Gateway: ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("Subnet: ");
    Serial.println(WiFi.subnetMask());
  } else {
    Serial.println("\n❌ WiFi connection failed!");
    Serial.println("Retrying with DHCP...");
    
    // Fallback to DHCP if static IP fails
    WiFi.config(0U, 0U, 0U);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
    }
    Serial.println("\n✅ WiFi connected with DHCP");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
}

// CORS handling
void enableCORS() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS, PUT");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

void handleOptions() {
  enableCORS();
  server.send(200, "text/plain", "");
}

// Web handlers
void handleRoot() {
  enableCORS();
  server.send(200, "text/html", htmlInterface);
}

void handleStatus() {
  enableCORS();
  
  DynamicJsonDocument doc(1024);
  doc["acIsOn"] = acIsOn;
  doc["systemEnabled"] = systemEnabled;
  doc["currentTime"] = getCurrentTime();
  doc["currentDate"] = getCurrentDate();
  doc["uptimeHours"] = (millis() - systemStartTime) / 3600000.0;
  doc["todayOnTime"] = todayOnTime;
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleControl() {
  enableCORS();
  
  if (server.hasArg("plain")) {
    DynamicJsonDocument doc(512);
    deserializeJson(doc, server.arg("plain"));
    
    String action = doc["action"];
    String message = "";
    
    if (action == "turnOn") {
      if (systemEnabled) {
        controlAC(true);
        message = "AC turned ON successfully";
      } else {
        message = "System is disabled. Enable system first.";
      }
    }
    else if (action == "turnOff") {
      controlAC(false);
      message = "AC turned OFF successfully";
    }
    else if (action == "forceOn") {
      forceControlAC(true);
      message = "AC FORCED ON (ignoring state)";
    }
    else if (action == "forceOff") {
      forceControlAC(false);
      message = "AC FORCED OFF (ignoring state)";
    }
    else if (action == "toggle") {
      toggleAC();
      message = "AC toggled";
    }
    else if (action == "enableSystem") {
      systemEnabled = true;
      message = "System enabled successfully";
      Serial.println("✅ System enabled");
    }
    else if (action == "disableSystem") {
      systemEnabled = false;
      controlAC(false); // Turn off AC when disabling system
      message = "System disabled successfully";
      Serial.println("⏸️ System disabled");
    }
    else {
      message = "Unknown action";
    }
    
    DynamicJsonDocument response(256);
    response["success"] = true;
    response["message"] = message;
    
    String responseStr;
    serializeJson(response, responseStr);
    server.send(200, "application/json", responseStr);
  } else {
    server.send(400, "application/json", "{\"error\":\"No data received\"}");
  }
}

void handleGetSchedule() {
  enableCORS();
  
  DynamicJsonDocument doc(1024);
  doc["weekdayOnTime1"] = formatTime(currentSchedule.weekdayOnHour1, currentSchedule.weekdayOnMinute1);
  doc["weekdayOffTime1"] = formatTime(currentSchedule.weekdayOffHour1, currentSchedule.weekdayOffMinute1);
  doc["weekdayEnabled1"] = currentSchedule.weekdayEnabled1;
  
  doc["weekdayOnTime2"] = formatTime(currentSchedule.weekdayOnHour2, currentSchedule.weekdayOnMinute2);
  doc["weekdayOffTime2"] = formatTime(currentSchedule.weekdayOffHour2, currentSchedule.weekdayOffMinute2);
  doc["weekdayEnabled2"] = currentSchedule.weekdayEnabled2;
  
  doc["weekendOnTime"] = formatTime(currentSchedule.weekendOnHour, currentSchedule.weekendOnMinute);
  doc["weekendOffTime"] = formatTime(currentSchedule.weekendOffHour, currentSchedule.weekendOffMinute);
  doc["weekendEnabled"] = currentSchedule.weekendEnabled;
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleSetSchedule() {
  enableCORS();
  
  if (server.hasArg("plain")) {
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, server.arg("plain"));
    
    // Parse weekday schedule 1
    if (doc.containsKey("weekdayOnTime1")) {
      String time1 = doc["weekdayOnTime1"];
      currentSchedule.weekdayOnHour1 = time1.substring(0, 2).toInt();
      currentSchedule.weekdayOnMinute1 = time1.substring(3, 5).toInt();
    }
    if (doc.containsKey("weekdayOffTime1")) {
      String time1 = doc["weekdayOffTime1"];
      currentSchedule.weekdayOffHour1 = time1.substring(0, 2).toInt();
      currentSchedule.weekdayOffMinute1 = time1.substring(3, 5).toInt();
    }
    if (doc.containsKey("weekdayEnabled1")) {
      currentSchedule.weekdayEnabled1 = doc["weekdayEnabled1"];
    }
    
    // Parse weekday schedule 2
    if (doc.containsKey("weekdayOnTime2")) {
      String time2 = doc["weekdayOnTime2"];
      currentSchedule.weekdayOnHour2 = time2.substring(0, 2).toInt();
      currentSchedule.weekdayOnMinute2 = time2.substring(3, 5).toInt();
    }
    if (doc.containsKey("weekdayOffTime2")) {
      String time2 = doc["weekdayOffTime2"];
      currentSchedule.weekdayOffHour2 = time2.substring(0, 2).toInt();
      currentSchedule.weekdayOffMinute2 = time2.substring(3, 5).toInt();
    }
    if (doc.containsKey("weekdayEnabled2")) {
      currentSchedule.weekdayEnabled2 = doc["weekdayEnabled2"];
    }
    
    // Parse weekend schedule
    if (doc.containsKey("weekendOnTime")) {
      String timeW = doc["weekendOnTime"];
      currentSchedule.weekendOnHour = timeW.substring(0, 2).toInt();
      currentSchedule.weekendOnMinute = timeW.substring(3, 5).toInt();
    }
    if (doc.containsKey("weekendOffTime")) {
      String timeW = doc["weekendOffTime"];
      currentSchedule.weekendOffHour = timeW.substring(0, 2).toInt();
      currentSchedule.weekendOffMinute = timeW.substring(3, 5).toInt();
    }
    if (doc.containsKey("weekendEnabled")) {
      currentSchedule.weekendEnabled = doc["weekendEnabled"];
    }
    
    Serial.println("📅 Schedule updated successfully");
    
    DynamicJsonDocument response(256);
    response["success"] = true;
    response["message"] = "Schedule updated successfully";
    
    String responseStr;
    serializeJson(response, responseStr);
    server.send(200, "application/json", responseStr);
  } else {
    server.send(400, "application/json", "{\"error\":\"No data received\"}");
  }
}

// Schedule checking function
void checkSchedule() {
  if (!systemEnabled || !getLocalTime(&timeinfo)) {
    return;
  }
  
  int currentHour = timeinfo.tm_hour;
  int currentMinute = timeinfo.tm_min;
  bool weekend = isWeekend();
  
  Serial.printf("⏰ Checking schedule: %02d:%02d %s\n", 
                currentHour, currentMinute, weekend ? "(Weekend)" : "(Weekday)");
  
  if (weekend) {
    // Weekend schedule
    if (currentSchedule.weekendEnabled) {
      // Check ON time
      if (currentHour == currentSchedule.weekendOnHour && 
          currentMinute == currentSchedule.weekendOnMinute) {
        Serial.println("📅 Weekend ON schedule triggered");
        controlAC(true);
      }
      // Check OFF time
      else if (currentHour == currentSchedule.weekendOffHour && 
               currentMinute == currentSchedule.weekendOffMinute) {
        Serial.println("📅 Weekend OFF schedule triggered");
        controlAC(false);
      }
    }
  } else {
    // Weekday schedules
    
    // Schedule 1
    if (currentSchedule.weekdayEnabled1) {
      if (currentHour == currentSchedule.weekdayOnHour1 && 
          currentMinute == currentSchedule.weekdayOnMinute1) {
        Serial.println("📅 Weekday Schedule 1 ON triggered");
        controlAC(true);
      }
      else if (currentHour == currentSchedule.weekdayOffHour1 && 
               currentMinute == currentSchedule.weekdayOffMinute1) {
        Serial.println("📅 Weekday Schedule 1 OFF triggered");
        controlAC(false);
      }
    }
    
    // Schedule 2
    if (currentSchedule.weekdayEnabled2) {
      if (currentHour == currentSchedule.weekdayOnHour2 && 
          currentMinute == currentSchedule.weekdayOnMinute2) {
        Serial.println("📅 Weekday Schedule 2 ON triggered");
        controlAC(true);
      }
      else if (currentHour == currentSchedule.weekdayOffHour2 && 
               currentMinute == currentSchedule.weekdayOffMinute2) {
        Serial.println("📅 Weekday Schedule 2 OFF triggered");
        controlAC(false);
      }
    }
  }
}

// Reset daily statistics at midnight
void resetDailyStats() {
  if (!getLocalTime(&timeinfo)) {
    return;
  }
  
  unsigned long currentDay = timeinfo.tm_yday; // Day of year
  
  if (lastDayCheck != currentDay) {
    todayOnTime = 0;
    lastDayCheck = currentDay;
    Serial.println("🔄 Daily stats reset");
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n🌀 Smart Mitsubishi AC Controller Starting...");

  // Initialize IR sender and Mitsubishi AC
  ac.begin();
  Serial.println("📡 Mitsubishi IR sender initialized");

  setupWiFi();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  Serial.print("⏳ Waiting for time sync");
  while (!getLocalTime(&timeinfo)) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\n✅ Time synchronized");

  systemStartTime = millis();
  lastDayCheck = timeinfo.tm_yday;

  // Setup web server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/control", HTTP_POST, handleControl);
  server.on("/api/control", HTTP_OPTIONS, handleOptions);
  server.on("/api/schedule", HTTP_GET, handleGetSchedule);
  server.on("/api/schedule", HTTP_POST, handleSetSchedule);
  server.on("/api/schedule", HTTP_OPTIONS, handleOptions);

  server.begin();
  Serial.println("🌐 Web server started");
  Serial.println("🎯 Access your Mitsubishi AC controller at: http://192.168.1.142");

  // Test Mitsubishi AC
  Serial.println("\n🧪 Testing Mitsubishi AC commands...");
  delay(2000);
  controlAC(true);   // Turn ON
  delay(5000);       // Wait 5 seconds
  controlAC(false);  // Turn OFF
  Serial.println("✅ Mitsubishi AC test completed\n");
}

void loop() {
  server.handleClient();

  static unsigned long lastScheduleCheck = 0;
  if (millis() - lastScheduleCheck > 60000) { // Check every minute
    checkSchedule();
    resetDailyStats();
    lastScheduleCheck = millis();
  }

  // Update running time counter
  if (acIsOn && (millis() - lastOnTime > 60000)) {
    todayOnTime += 1;
    lastOnTime = millis();
  }

  delay(100);
}
