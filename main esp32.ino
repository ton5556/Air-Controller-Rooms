#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <time.h>
#include <SPIFFS.h>

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Web server on port 80
WebServer server(80);

// AC Control Pin (relay)
const int AC_RELAY_PIN = 2;

// System variables
bool acIsOn = false;
bool systemEnabled = true;

// Schedule structure
struct Schedule {
  int weekdayOnHour = 8;
  int weekdayOnMinute = 0;
  int weekdayOffHour = 18;
  int weekdayOffMinute = 0;
  int weekendOnHour = 9;
  int weekendOnMinute = 0;
  int weekendOffHour = 22;
  int weekendOffMinute = 0;
  bool weekdayEnabled = true;
  bool weekendEnabled = true;
};

Schedule currentSchedule;

// Time variables
struct tm timeinfo;
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0; // Adjust for your timezone
const int daylightOffset_sec = 0;

// Function declarations
void setupWiFi();
void setupWebServer();
void handleRoot();
void handleAPI();
void handleControl();
void handleSchedule();
void handleGetSchedule();
void loadSchedule();
void saveSchedule();
void checkSchedule();
void controlAC(bool turnOn);
String getTimeString();
String getDateString();
void enableCORS();

void setup() {
  Serial.begin(115200);
  
  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS initialization failed!");
    return;
  }
  
  // Initialize AC relay pin
  pinMode(AC_RELAY_PIN, OUTPUT);
  digitalWrite(AC_RELAY_PIN, LOW); // AC off initially
  
  // Setup WiFi
  setupWiFi();
  
  // Initialize NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  // Load saved schedule
  loadSchedule();
  
  // Setup web server
  setupWebServer();
  
  Serial.println("ESP32-S3 Smart AC Controller ready!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  server.handleClient();
  
  // Check schedule every minute
  static unsigned long lastScheduleCheck = 0;
  if (millis() - lastScheduleCheck > 60000) {
    checkSchedule();
    lastScheduleCheck = millis();
  }
  
  delay(100);
}

void setupWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setupWebServer() {
  // Enable CORS for all requests
  server.enableCORS(true);
  
  // Serve the main HTML page
  server.on("/", HTTP_GET, handleRoot);
  
  // API endpoints
  server.on("/api/status", HTTP_GET, handleAPI);
  server.on("/api/control", HTTP_POST, handleControl);
  server.on("/api/schedule", HTTP_GET, handleGetSchedule);
  server.on("/api/schedule", HTTP_POST, handleSchedule);
  
  // Handle preflight requests
  server.on("/api/status", HTTP_OPTIONS, enableCORS);
  server.on("/api/control", HTTP_OPTIONS, enableCORS);
  server.on("/api/schedule", HTTP_OPTIONS, enableCORS);
  
  server.begin();
  Serial.println("HTTP server started");
}

void enableCORS() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.send(200, "text/plain", "");
}

void handleRoot() {
  // Read HTML file from SPIFFS or serve inline HTML
  String html = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Smart AC Controller</title>
    <style>
        :root {
            --primary-gradient: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            --secondary-gradient: linear-gradient(135deg, #4facfe 0%, #00f2fe 100%);
            --success-gradient: linear-gradient(135deg, #43e97b 0%, #38f9d7 100%);
            --danger-gradient: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);
            --card-shadow: 0 20px 40px rgba(0,0,0,0.1);
            --hover-shadow: 0 25px 50px rgba(0,0,0,0.15);
        }
        
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Inter', -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 50%, #f093fb 100%);
            min-height: 100vh;
            padding: 20px;
            animation: backgroundShift 10s ease-in-out infinite alternate;
        }
        
        @keyframes backgroundShift {
            0% { background: linear-gradient(135deg, #667eea 0%, #764ba2 50%, #f093fb 100%); }
            100% { background: linear-gradient(135deg, #4facfe 0%, #00f2fe 50%, #43e97b 100%); }
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
            background: rgba(255, 255, 255, 0.95);
            backdrop-filter: blur(20px);
            border-radius: 25px;
            box-shadow: var(--card-shadow);
            overflow: hidden;
            animation: fadeInUp 0.8s ease-out;
        }
        
        @keyframes fadeInUp {
            from { opacity: 0; transform: translateY(30px); }
            to { opacity: 1; transform: translateY(0); }
        }
        
        .header {
            background: var(--secondary-gradient);
            color: white;
            text-align: center;
            padding: 40px 30px;
            position: relative;
            overflow: hidden;
        }
        
        .header h1 {
            font-size: 3em;
            margin-bottom: 15px;
            font-weight: 700;
            text-shadow: 0 2px 10px rgba(0,0,0,0.2);
        }
        
        .status-bar {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 25px 30px;
            background: linear-gradient(135deg, #f8f9fa 0%, #e9ecef 100%);
        }
        
        .status-item {
            display: flex;
            align-items: center;
            gap: 12px;
            padding: 10px 20px;
            background: white;
            border-radius: 25px;
            box-shadow: 0 5px 15px rgba(0,0,0,0.1);
        }
        
        .status-light {
            width: 16px;
            height: 16px;
            border-radius: 50%;
            background: var(--success-gradient);
            animation: pulse 2s infinite;
        }
        
        .status-light.off { background: var(--danger-gradient); }
        
        @keyframes pulse {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.7; }
        }
        
        .current-time {
            font-size: 1.4em;
            font-weight: 700;
            color: #667eea;
            text-align: center;
            padding: 15px 25px;
            background: white;
            border-radius: 20px;
            box-shadow: 0 5px 15px rgba(0,0,0,0.1);
        }
        
        .main-content { padding: 40px 30px; }
        
        .control-section { margin-bottom: 50px; }
        
        .section-title {
            font-size: 2em;
            color: #333;
            margin-bottom: 30px;
            display: flex;
            align-items: center;
            gap: 15px;
            font-weight: 600;
        }
        
        .control-buttons {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }
        
        .btn {
            padding: 20px 30px;
            border: none;
            border-radius: 15px;
            font-size: 16px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.4s ease;
            text-transform: uppercase;
            letter-spacing: 1px;
            color: white;
        }
        
        .btn-primary { background: var(--primary-gradient); }
        .btn-danger { background: var(--danger-gradient); }
        .btn-success { background: var(--success-gradient); }
        
        .btn:hover {
            transform: translateY(-5px) scale(1.02);
            box-shadow: var(--hover-shadow);
        }
        
        .schedule-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(350px, 1fr));
            gap: 30px;
        }
        
        .schedule-card {
            background: linear-gradient(135deg, #f8f9fa 0%, #e9ecef 100%);
            border-radius: 20px;
            padding: 30px;
            transition: all 0.4s ease;
        }
        
        .schedule-card:hover {
            transform: translateY(-8px);
            box-shadow: var(--hover-shadow);
        }
        
        .schedule-card h3 {
            color: #667eea;
            margin-bottom: 25px;
            font-size: 1.4em;
            font-weight: 700;
        }
        
        .time-input-group {
            display: flex;
            align-items: center;
            gap: 15px;
            margin-bottom: 20px;
        }
        
        .time-input-group label {
            min-width: 80px;
            font-weight: 600;
            color: #495057;
        }
        
        .time-input {
            flex: 1;
            padding: 12px 16px;
            border: 2px solid #e9ecef;
            border-radius: 10px;
            font-size: 16px;
            transition: all 0.3s ease;
            background: white;
        }
        
        .time-input:focus {
            outline: none;
            border-color: #667eea;
            box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1);
        }
        
        .toggle-switch {
            position: relative;
            display: inline-block;
            width: 70px;
            height: 40px;
        }
        
        .toggle-switch input { opacity: 0; width: 0; height: 0; }
        
        .slider {
            position: absolute;
            cursor: pointer;
            top: 0; left: 0; right: 0; bottom: 0;
            background: #ccc;
            transition: .4s;
            border-radius: 40px;
        }
        
        .slider:before {
            position: absolute;
            content: "";
            height: 32px; width: 32px;
            left: 4px; bottom: 4px;
            background: white;
            transition: .4s;
            border-radius: 50%;
        }
        
        input:checked + .slider { background: var(--primary-gradient); }
        input:checked + .slider:before { transform: translateX(30px); }
        
        .save-button-container {
            text-align: center;
            margin-top: 40px;
        }
        
        .btn-save {
            background: var(--primary-gradient);
            padding: 18px 40px;
            font-size: 18px;
            border-radius: 25px;
        }
        
        .notification {
            position: fixed;
            top: 30px; right: 30px;
            padding: 20px 30px;
            border-radius: 15px;
            color: white;
            font-weight: 600;
            z-index: 1000;
            max-width: 350px;
        }
        
        .notification.success { background: var(--success-gradient); }
        .notification.error { background: var(--danger-gradient); }
        
        @media (max-width: 768px) {
            .container { margin: 10px; }
            .header { padding: 30px 20px; }
            .header h1 { font-size: 2.5em; }
            .status-bar { flex-direction: column; gap: 15px; }
            .control-buttons { grid-template-columns: 1fr; }
            .schedule-grid { grid-template-columns: 1fr; }
            .main-content { padding: 30px 20px; }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🌀 Smart AC Controller</h1>
            <p>ESP32-S3 Powered AC Management</p>
        </div>
        
        <div class="status-bar">
            <div class="status-item">
                <div class="status-light" id="acStatus"></div>
                <span id="acStatusText">AC Status: Loading...</span>
            </div>
            <div class="current-time" id="currentTime">Loading...</div>
            <div class="status-item">
                <div class="status-light" id="systemStatus"></div>
                <span id="systemStatusText">System: Loading...</span>
            </div>
        </div>
        
        <div class="main-content">
            <div class="control-section">
                <h2 class="section-title">🎛️ Manual Control</h2>
                <div class="control-buttons">
                    <button class="btn btn-success" onclick="controlAC('turnOn')">Turn ON AC</button>
                    <button class="btn btn-danger" onclick="controlAC('turnOff')">Turn OFF AC</button>
                    <button class="btn btn-primary" onclick="controlAC('enableSystem')">Enable System</button>
                    <button class="btn btn-primary" onclick="controlAC('disableSystem')">Disable System</button>
                </div>
            </div>
            
            <div class="control-section">
                <h2 class="section-title">⏰ Schedule Settings</h2>
                <div class="schedule-grid">
                    <div class="schedule-card">
                        <h3>📅 Weekdays</h3>
                        <div class="time-input-group">
                            <label>Turn ON:</label>
                            <input type="time" class="time-input" id="weekdayOnTime">
                        </div>
                        <div class="time-input-group">
                            <label>Turn OFF:</label>
                            <input type="time" class="time-input" id="weekdayOffTime">
                        </div>
                        <div class="time-input-group">
                            <label>Enable:</label>
                            <label class="toggle-switch">
                                <input type="checkbox" id="weekdayEnabled">
                                <span class="slider"></span>
                            </label>
                        </div>
                    </div>
                    
                    <div class="schedule-card">
                        <h3>🏖️ Weekends</h3>
                        <div class="time-input-group">
                            <label>Turn ON:</label>
                            <input type="time" class="time-input" id="weekendOnTime">
                        </div>
                        <div class="time-input-group">
                            <label>Turn OFF:</label>
                            <input type="time" class="time-input" id="weekendOffTime">
                        </div>
                        <div class="time-input-group">
                            <label>Enable:</label>
                            <label class="toggle-switch">
                                <input type="checkbox" id="weekendEnabled">
                                <span class="slider"></span>
                            </label>
                        </div>
                    </div>
                </div>
                
                <div class="save-button-container">
                    <button class="btn btn-save" onclick="saveSchedule()">💾 Save Schedule</button>
                </div>
            </div>
        </div>
    </div>

    <script>
        let statusInterval;
        const API_BASE = window.location.origin;
        
        function init() {
            updateStatus();
            loadSchedule();
            statusInterval = setInterval(updateStatus, 5000);
        }
        
        async function updateStatus() {
            try {
                const response = await fetch(`${API_BASE}/api/status`);
                const data = await response.json();
                
                const acStatusEl = document.getElementById('acStatus');
                const acStatusTextEl = document.getElementById('acStatusText');
                acStatusEl.className = 'status-light ' + (data.acIsOn ? '' : 'off');
                acStatusTextEl.textContent = 'AC Status: ' + (data.acIsOn ? 'ON' : 'OFF');
                
                const systemStatusEl = document.getElementById('systemStatus');
                const systemStatusTextEl = document.getElementById('systemStatusText');
                systemStatusEl.className = 'status-light ' + (data.systemEnabled ? '' : 'off');
                systemStatusTextEl.textContent = 'System: ' + (data.systemEnabled ? 'Enabled' : 'Disabled');
                
                document.getElementById('currentTime').textContent = 
                    `${data.currentTime} | ${data.currentDate}`;
                
            } catch (error) {
                console.error('Error updating status:', error);
                showNotification('Connection error', 'error');
            }
        }
        
        async function controlAC(action) {
            try {
                const response = await fetch(`${API_BASE}/api/control`, {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ action: action })
                });
                
                if (response.ok) {
                    updateStatus();
                    showNotification(getActionMessage(action), 'success');
                } else {
                    throw new Error('Failed to execute command');
                }
            } catch (error) {
                console.error('Error controlling AC:', error);
                showNotification('Failed to execute command', 'error');
            }
        }
        
        function getActionMessage(action) {
            const messages = {
                'turnOn': '✅ AC turned ON',
                'turnOff': '❌ AC turned OFF',
                'enableSystem': '🟢 System enabled',
                'disableSystem': '🔴 System disabled'
            };
            return messages[action] || 'Command executed';
        }
        
        async function loadSchedule() {
            try {
                const response = await fetch(`${API_BASE}/api/schedule`);
                const data = await response.json();
                
                document.getElementById('weekdayOnTime').value = 
                    formatTime(data.weekdayOnHour, data.weekdayOnMinute);
                document.getElementById('weekdayOffTime').value = 
                    formatTime(data.weekdayOffHour, data.weekdayOffMinute);
                document.getElementById('weekendOnTime').value = 
                    formatTime(data.weekendOnHour, data.weekendOnMinute);
                document.getElementById('weekendOffTime').value = 
                    formatTime(data.weekendOffHour, data.weekendOffMinute);
                
                document.getElementById('weekdayEnabled').checked = data.weekdayEnabled;
                document.getElementById('weekendEnabled').checked = data.weekendEnabled;
                
            } catch (error) {
                console.error('Error loading schedule:', error);
                showNotification('Failed to load schedule', 'error');
            }
        }
        
        function formatTime(hour, minute) {
            return String(hour).padStart(2, '0') + ':' + String(minute).padStart(2, '0');
        }
        
        async function saveSchedule() {
            try {
                const weekdayOn = document.getElementById('weekdayOnTime').value.split(':');
                const weekdayOff = document.getElementById('weekdayOffTime').value.split(':');
                const weekendOn = document.getElementById('weekendOnTime').value.split(':');
                const weekendOff = document.getElementById('weekendOffTime').value.split(':');
                
                const schedule = {
                    weekdayOnHour: parseInt(weekdayOn[0]),
                    weekdayOnMinute: parseInt(weekdayOn[1]),
                    weekdayOffHour: parseInt(weekdayOff[0]),
                    weekdayOffMinute: parseInt(weekdayOff[1]),
                    weekendOnHour: parseInt(weekendOn[0]),
                    weekendOnMinute: parseInt(weekendOn[1]),
                    weekendOffHour: parseInt(weekendOff[0]),
                    weekendOffMinute: parseInt(weekendOff[1]),
                    weekdayEnabled: document.getElementById('weekdayEnabled').checked,
                    weekendEnabled: document.getElementById('weekendEnabled').checked
                };
                
                const response = await fetch(`${API_BASE}/api/schedule`, {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify(schedule)
                });
                
                if (response.ok) {
                    showNotification('📅 Schedule saved!', 'success');
                } else {
                    throw new Error('Failed to save schedule');
                }
            } catch (error) {
                console.error('Error saving schedule:', error);
                showNotification('❌ Failed to save schedule', 'error');
            }
        }
        
        function showNotification(message, type) {
            const existingNotifications = document.querySelectorAll('.notification');
            existingNotifications.forEach(n => n.remove());
            
            const notification = document.createElement('div');
            notification.className = `notification ${type}`;
            notification.textContent = message;
            
            document.body.appendChild(notification);
            
            setTimeout(() => {
                if (notification.parentNode) {
                    notification.remove();
                }
            }, 3000);
        }
        
        document.addEventListener('DOMContentLoaded', init);
    </script>
</body>
</html>
  )";
  
  server.send(200, "text/html", html);
}

void handleAPI() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  
  if (!getLocalTime(&timeinfo)) {
    server.send(500, "application/json", "{\"error\":\"Failed to obtain time\"}");
    return;
  }
  
  DynamicJsonDocument doc(1024);
  doc["acIsOn"] = acIsOn;
  doc["systemEnabled"] = systemEnabled;
  doc["currentTime"] = getTimeString();
  doc["currentDate"] = getDateString();
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleControl() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  
  if (server.hasArg("plain")) {
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, server.arg("plain"));
    
    String action = doc["action"];
    
    if (action == "turnOn") {
      controlAC(true);
    } else if (action == "turnOff") {
      controlAC(false);
    } else if (action == "enableSystem") {
      systemEnabled = true;
      Serial.println("System enabled");
    } else if (action == "disableSystem") {
      systemEnabled = false;
      Serial.println("System disabled");
    }
    
    server.send(200, "application/json", "{\"success\":true}");
  } else {
    server.send(400, "application/json", "{\"error\":\"Invalid request\"}");
  }
}

void handleGetSchedule() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  
  DynamicJsonDocument doc(1024);
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
  server.sendHeader("Access-Control-Allow-Origin", "*");
  
  if (server.hasArg("plain")) {
    DynamicJsonDocument doc(1024);
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
    
    saveSchedule();
    
    Serial.println("Schedule updated and saved");
    server.send(200, "application/json", "{\"success\":true}");
  } else {
    server.send(400, "application/json", "{\"error\":\"Invalid request\"}");
  }
}

void loadSchedule() {
  File file = SPIFFS.open("/schedule.json", "r");
  if (!file) {
    Serial.println("Failed to open schedule file, using defaults");
    return;
  }
  
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, file);
  
  currentSchedule.weekdayOnHour = doc["weekdayOnHour"] | 8;
  currentSchedule.weekdayOnMinute = doc["weekdayOnMinute"] | 0;
  currentSchedule.weekdayOffHour = doc["weekdayOffHour"] | 18;
  currentSchedule.weekdayOffMinute = doc["weekdayOffMinute"] | 0;
  currentSchedule.weekendOnHour = doc["weekendOnHour"] | 9;
  currentSchedule.weekendOnMinute = doc["weekendOnMinute"] | 0;
  currentSchedule.weekendOffHour = doc["weekendOffHour"] | 22;
  currentSchedule.weekendOffMinute = doc["weekendOffMinute"] | 0;
  currentSchedule.weekdayEnabled = doc["weekdayEnabled"] | true;
  currentSchedule.weekendEnabled = doc["weekendEnabled"] | true;
  
  file.close();
  Serial.println("Schedule loaded from SPIFFS");
}

void saveSchedule() {
  File file = SPIFFS.open("/schedule.json", "w");
  if (!file) {
    Serial.println("Failed to open schedule file for writing");
    return;
  }
  
  DynamicJsonDocument doc(1024);
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
  
  serializeJson(doc, file);
  file.close();
  Serial.println("Schedule saved to SPIFFS");
}

void checkSchedule() {
  if (!systemEnabled) return;
  
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time for schedule check");
    return;
  }
  
  int currentHour = timeinfo.tm_hour;
  int currentMinute = timeinfo.tm_min;
  int currentDay = timeinfo.tm_wday; // 0=Sunday, 1=Monday, ..., 6=Saturday
  
  bool isWeekend = (currentDay == 0 || currentDay == 6); // Sunday or Saturday
  
  // Check if we should turn AC on or off based on schedule
  if (isWeekend && currentSchedule.weekendEnabled) {
    // Weekend schedule
    if (currentHour == currentSchedule.weekendOnHour && 
        currentMinute == currentSchedule.weekendOnMinute) {
      controlAC(true);
      Serial.println("Weekend schedule: AC turned ON");
    } else if (currentHour == currentSchedule.weekendOffHour && 
               currentMinute == currentSchedule.weekendOffMinute) {
      controlAC(false);
      Serial.println("Weekend schedule: AC turned OFF");
    }
  } else if (!isWeekend && currentSchedule.weekdayEnabled) {
    // Weekday schedule
    if (currentHour == currentSchedule.weekdayOnHour && 
        currentMinute == currentSchedule.weekdayOnMinute) {
      controlAC(true);
      Serial.println("Weekday schedule: AC turned ON");
    } else if (currentHour == currentSchedule.weekdayOffHour && 
               currentMinute == currentSchedule.weekdayOffMinute) {
      controlAC(false);
      Serial.println("Weekday schedule: AC turned OFF");
    }
  }
}

void controlAC(bool turnOn) {
  acIsOn = turnOn;
  digitalWrite(AC_RELAY_PIN, turnOn ? HIGH : LOW);
  
  Serial.print("AC ");
  Serial.println(turnOn ? "turned ON" : "turned OFF");
}

String getTimeString() {
  if (!getLocalTime(&timeinfo)) {
    return "Time Error";
  }
  
  char timeString[9];
  strftime(timeString, sizeof(timeString), "%H:%M:%S", &timeinfo);
  return String(timeString);
}

String getDateString() {
  if (!getLocalTime(&timeinfo)) {
    return "Date Error";
  }
  
  char dateString[12];
  strftime(dateString, sizeof(dateString), "%Y-%m-%d", &timeinfo);
  return String(dateString);
}
