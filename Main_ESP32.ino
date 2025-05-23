#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <time.h>
#include <SPIFFS.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

// === WiFi credentials ===
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// === Web server ===
WebServer server(80);

// === IR Transmitter ===
#define IR_SEND_PIN 4  // Connect IR LED to GPIO 4 (via transistor if needed)
IRsend irsend(IR_SEND_PIN);

// === IR raw signals ===
// Replace with your actual IR signals
uint16_t rawOnSignal[] = {9000, 4500, 600, 550, 600, 550, /* ... */};
uint16_t rawOffSignal[] = {9000, 4500, 600, 550, 600, 550, /* ... */};

// === AC status & scheduling ===
bool acIsOn = false;
bool systemEnabled = true;
unsigned long systemStartTime = 0;
unsigned long todayOnTime = 0;
unsigned long lastOnTime = 0;

struct Schedule {
  int weekdayOnHour = 17;
  int weekdayOnMinute = 0;
  int weekdayOffHour = 23;
  int weekdayOffMinute = 0;
  int weekendOnHour = 13;
  int weekendOnMinute = 0;
  int weekendOffHour = 23;
  int weekendOffMinute = 0;
  bool weekdayEnabled = true;
  bool weekendEnabled = true;
};
Schedule currentSchedule;

struct tm timeinfo;
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7 * 3600;  // Thailand = UTC+7
const int daylightOffset_sec = 0;

// === HTML Interface ===
const char htmlInterface[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Smart AC Controller - ESP32 S3</title>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&display=swap');
        
        :root {
            --primary-gradient: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            --secondary-gradient: linear-gradient(135deg, #4facfe 0%, #00f2fe 100%);
            --success-gradient: linear-gradient(135deg, #43e97b 0%, #38f9d7 100%);
            --danger-gradient: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);
            --warning-gradient: linear-gradient(135deg, #ffecd2 0%, #fcb69f 100%);
            --info-gradient: linear-gradient(135deg, #a8edea 0%, #fed6e3 100%);
            --card-shadow: 0 20px 40px rgba(0,0,0,0.1);
            --hover-shadow: 0 25px 50px rgba(0,0,0,0.15);
            --glass-bg: rgba(255, 255, 255, 0.25);
            --glass-border: rgba(255, 255, 255, 0.18);
        }
        
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Inter', -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 30%, #f093fb 70%, #4facfe 100%);
            min-height: 100vh;
            padding: 20px;
            animation: backgroundShift 15s ease-in-out infinite alternate;
            overflow-x: hidden;
        }
        
        @keyframes backgroundShift {
            0% { 
                background: linear-gradient(135deg, #667eea 0%, #764ba2 30%, #f093fb 70%, #4facfe 100%);
            }
            25% {
                background: linear-gradient(135deg, #4facfe 0%, #00f2fe 30%, #43e97b 70%, #38f9d7 100%);
            }
            50% {
                background: linear-gradient(135deg, #fa709a 0%, #fee140 30%, #667eea 70%, #764ba2 100%);
            }
            75% {
                background: linear-gradient(135deg, #a8edea 0%, #fed6e3 30%, #ffecd2 70%, #fcb69f 100%);
            }
            100% { 
                background: linear-gradient(135deg, #667eea 0%, #764ba2 30%, #f093fb 70%, #4facfe 100%);
            }
        }
        
        .container {
            max-width: 1400px;
            margin: 0 auto;
            background: var(--glass-bg);
            backdrop-filter: blur(25px);
            border: 1px solid var(--glass-border);
            border-radius: 30px;
            box-shadow: var(--card-shadow);
            overflow: hidden;
            animation: fadeInUp 1s ease-out;
            position: relative;
            z-index: 10;
        }
        
        @keyframes fadeInUp {
            from {
                opacity: 0;
                transform: translateY(50px) scale(0.95);
            }
            to {
                opacity: 1;
                transform: translateY(0) scale(1);
            }
        }
        
        .header {
            background: var(--secondary-gradient);
            color: white;
            text-align: center;
            padding: 50px 30px;
            position: relative;
            overflow: hidden;
        }
        
        .header h1 {
            font-size: 3.5em;
            margin-bottom: 20px;
            font-weight: 700;
            position: relative;
            z-index: 1;
            text-shadow: 0 4px 20px rgba(0,0,0,0.3);
        }
        
        .header p {
            font-size: 1.3em;
            opacity: 0.95;
            position: relative;
            z-index: 1;
            font-weight: 500;
        }
        
        .status-bar {
            display: grid;
            grid-template-columns: 1fr auto 1fr;
            align-items: center;
            padding: 30px;
            background: linear-gradient(135deg, rgba(248,249,250,0.9) 0%, rgba(233,236,239,0.9) 100%);
            border-bottom: 1px solid rgba(255,255,255,0.2);
            gap: 20px;
        }
        
        .status-item {
            display: flex;
            align-items: center;
            gap: 15px;
            padding: 15px 25px;
            background: rgba(255, 255, 255, 0.9);
            backdrop-filter: blur(10px);
            border-radius: 25px;
            box-shadow: 0 8px 25px rgba(0,0,0,0.1);
            transition: all 0.4s cubic-bezier(0.175, 0.885, 0.32, 1.275);
            border: 1px solid rgba(255,255,255,0.3);
        }
        
        .status-item:hover {
            transform: translateY(-5px) scale(1.02);
            box-shadow: 0 15px 35px rgba(0,0,0,0.2);
        }
        
        .status-light {
            width: 20px;
            height: 20px;
            border-radius: 50%;
            background: var(--success-gradient);
            position: relative;
            animation: pulse 2s infinite;
            box-shadow: 0 0 20px rgba(67, 233, 123, 0.5);
        }
        
        .status-light.off {
            background: var(--danger-gradient);
            box-shadow: 0 0 20px rgba(245, 87, 108, 0.5);
        }
        
        @keyframes pulse {
            0%, 100% { opacity: 1; transform: scale(1); }
            50% { opacity: 0.8; transform: scale(1.1); }
        }
        
        .current-time {
            font-size: 1.6em;
            font-weight: 700;
            color: #667eea;
            text-align: center;
            padding: 20px 30px;
            background: rgba(255, 255, 255, 0.95);
            backdrop-filter: blur(10px);
            border-radius: 25px;
            box-shadow: 0 8px 25px rgba(102, 126, 234, 0.2);
            border: 1px solid rgba(102, 126, 234, 0.2);
        }
        
        .main-content {
            padding: 50px 40px;
        }
        
        .control-section {
            margin-bottom: 60px;
        }
        
        .section-title {
            font-size: 2.2em;
            color: #333;
            margin-bottom: 35px;
            display: flex;
            align-items: center;
            gap: 20px;
            font-weight: 700;
            position: relative;
        }
        
        .section-title::after {
            content: '';
            flex: 1;
            height: 4px;
            background: var(--primary-gradient);
            border-radius: 2px;
        }
        
        .control-buttons {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
            gap: 25px;
            margin-bottom: 40px;
        }
        
        .btn {
            padding: 25px 35px;
            border: none;
            border-radius: 20px;
            font-size: 16px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.4s cubic-bezier(0.175, 0.885, 0.32, 1.275);
            text-transform: uppercase;
            letter-spacing: 1.2px;
            position: relative;
            overflow: hidden;
            color: white;
            box-shadow: 0 10px 25px rgba(0,0,0,0.2);
            backdrop-filter: blur(10px);
        }
        
        .btn:hover {
            transform: translateY(-8px) scale(1.03);
            box-shadow: 0 20px 40px rgba(0,0,0,0.25);
        }
        
        .btn-primary {
            background: var(--primary-gradient);
        }
        
        .btn-danger {
            background: var(--danger-gradient);
        }
        
        .btn-success {
            background: var(--success-gradient);
        }
        
        .btn-warning {
            background: var(--warning-gradient);
            color: #333;
        }
        
        .btn-info {
            background: var(--info-gradient);
            color: #333;
        }
        
        .quick-stats {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 20px;
            margin-bottom: 40px;
        }
        
        .stat-card {
            background: var(--glass-bg);
            backdrop-filter: blur(15px);
            border: 1px solid var(--glass-border);
            border-radius: 20px;
            padding: 25px;
            text-align: center;
            transition: all 0.3s ease;
        }
        
        .stat-card:hover {
            transform: translateY(-5px);
            box-shadow: 0 15px 30px rgba(0,0,0,0.1);
        }
        
        .stat-number {
            font-size: 2.5em;
            font-weight: 700;
            color: #667eea;
            margin-bottom: 10px;
        }
        
        .stat-label {
            color: #666;
            font-weight: 500;
            text-transform: uppercase;
            letter-spacing: 1px;
            font-size: 0.9em;
        }
        
        .schedule-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(400px, 1fr));
            gap: 35px;
        }
        
        .schedule-card {
            background: linear-gradient(135deg, rgba(248,249,250,0.95) 0%, rgba(233,236,239,0.95) 100%);
            backdrop-filter: blur(15px);
            border-radius: 25px;
            padding: 35px;
            border: 1px solid rgba(255,255,255,0.3);
            transition: all 0.5s cubic-bezier(0.175, 0.885, 0.32, 1.275);
            position: relative;
            overflow: hidden;
        }
        
        .schedule-card:hover {
            border-color: rgba(102, 126, 234, 0.4);
            transform: translateY(-10px) scale(1.02);
            box-shadow: 0 25px 50px rgba(0,0,0,0.15);
        }
        
        .schedule-card h3 {
            color: #667eea;
            margin-bottom: 30px;
            font-size: 1.5em;
            font-weight: 700;
            display: flex;
            align-items: center;
            gap: 12px;
        }
        
        .time-input-group {
            display: flex;
            align-items: center;
            gap: 20px;
            margin-bottom: 25px;
            position: relative;
        }
        
        .time-input-group label {
            min-width: 90px;
            font-weight: 600;
            color: #495057;
            font-size: 15px;
        }
        
        .time-input {
            flex: 1;
            padding: 15px 20px;
            border: 2px solid rgba(233,236,239,0.8);
            border-radius: 15px;
            font-size: 16px;
            transition: all 0.3s ease;
            background: rgba(255,255,255,0.9);
            backdrop-filter: blur(5px);
        }
        
        .time-input:focus {
            outline: none;
            border-color: #667eea;
            box-shadow: 0 0 0 4px rgba(102, 126, 234, 0.1);
            transform: scale(1.02);
            background: rgba(255,255,255,1);
        }
        
        .toggle-switch {
            position: relative;
            display: inline-block;
            width: 80px;
            height: 45px;
        }
        
        .toggle-switch input {
            opacity: 0;
            width: 0;
            height: 0;
        }
        
        .slider {
            position: absolute;
            cursor: pointer;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background: linear-gradient(135deg, #ccc 0%, #999 100%);
            transition: .5s;
            border-radius: 45px;
            box-shadow: inset 0 2px 10px rgba(0,0,0,0.1);
        }
        
        .slider:before {
            position: absolute;
            content: "";
            height: 37px;
            width: 37px;
            left: 4px;
            bottom: 4px;
            background: linear-gradient(135deg, #fff 0%, #f8f9fa 100%);
            transition: .4s;
            border-radius: 50%;
            box-shadow: 0 4px 15px rgba(0,0,0,0.2);
        }
        
        input:checked + .slider {
            background: var(--primary-gradient);
        }
        
        input:checked + .slider:before {
            transform: translateX(35px);
        }
        
        .save-button-container {
            text-align: center;
            margin-top: 50px;
        }
        
        .btn-save {
            background: var(--primary-gradient);
            padding: 22px 50px;
            font-size: 18px;
            border-radius: 30px;
            box-shadow: 0 15px 35px rgba(102, 126, 234, 0.3);
            position: relative;
            overflow: hidden;
            min-width: 200px;
        }
        
        .notification {
            position: fixed;
            top: 30px;
            right: 30px;
            padding: 25px 35px;
            border-radius: 20px;
            color: white;
            font-weight: 600;
            z-index: 1000;
            max-width: 400px;
            box-shadow: 0 20px 40px rgba(0,0,0,0.2);
            backdrop-filter: blur(15px);
            border: 1px solid rgba(255,255,255,0.2);
            display: none;
        }
        
        .notification.success {
            background: var(--success-gradient);
        }
        
        .notification.error {
            background: var(--danger-gradient);
        }
        
        @media (max-width: 1024px) {
            .status-bar {
                grid-template-columns: 1fr;
                gap: 20px;
                text-align: center;
            }
            
            .schedule-grid {
                grid-template-columns: 1fr;
            }
        }
        
        @media (max-width: 768px) {
            .container {
                margin: 10px;
                border-radius: 25px;
            }
            
            .header {
                padding: 40px 25px;
            }
            
            .header h1 {
                font-size: 2.8em;
            }
            
            .control-buttons {
                grid-template-columns: 1fr;
            }
            
            .main-content {
                padding: 40px 25px;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🌀 Smart AC Controller</h1>
            <p>ESP32-S3 Powered Intelligent Air Conditioning Management</p>
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
                <div class="quick-stats">
                    <div class="stat-card">
                        <div class="stat-number" id="uptimeHours">0</div>
                        <div class="stat-label">Hours Online</div>
                    </div>
                    <div class="stat-card">
                        <div class="stat-number" id="todayOnTime">0</div>
                        <div class="stat-label">Minutes On Today</div>
                    </div>
                    <div class="stat-card">
                        <div class="stat-number" id="scheduleCount">2</div>
                        <div class="stat-label">Active Schedules</div>
                    </div>
                </div>
            </div>
            
            <div class="control-section">
                <h2 class="section-title">🎛️ Manual Control</h2>
                <div class="control-buttons">
                    <button class="btn btn-success" onclick="controlAC('turnOn')">
                        🟢 Turn ON AC
                    </button>
                    <button class="btn btn-danger" onclick="controlAC('turnOff')">
                        🔴 Turn OFF AC
                    </button>
                    <button class="btn btn-primary" onclick="controlAC('enableSystem')">
                        ⚡ Enable System
                    </button>
                    <button class="btn btn-warning" onclick="controlAC('disableSystem')">
                        ⏸️ Disable System
                    </button>
                    <button class="btn btn-info" onclick="refreshData()">
                        🔄 Refresh Data
                    </button>
                </div>
            </div>
            
            <div class="control-section">
                <h2 class="section-title">⏰ Schedule Settings</h2>
                <div class="schedule-grid">
                    <div class="schedule-card">
                        <h3>📅 Weekdays (Mon-Fri)</h3>
                        <div class="time-input-group">
                            <label>Turn ON:</label>
                            <input type="time" class="time-input" id="weekdayOnTime" value="17:00">
                        </div>
                        <div class="time-input-group">
                            <label>Turn OFF:</label>
                            <input type="time" class="time-input" id="weekdayOffTime" value="23:00">
                        </div>
                        <div class="time-input-group">
                            <label>Enable:</label>
                            <label class="toggle-switch">
                                <input type="checkbox" id="weekdayEnabled" checked>
                                <span class="slider"></span>
                            </label>
                        </div>
                    </div>
                    
                    <div class="schedule-card">
                        <h3>🏖️ Weekends (Sat-Sun)</h3>
                        <div class="time-input-group">
                            <label>Turn ON:</label>
                            <input type="time" class="time-input" id="weekendOnTime" value="13:00">
                        </div>
                        <div class="time-input-group">
                            <label>Turn OFF:</label>
                            <input type="time" class="time-input" id="weekendOffTime" value="23:00">
                        </div>
                        <div class="time-input-group">
                            <label>Enable:</label>
                            <label class="toggle-switch">
                                <input type="checkbox" id="weekendEnabled" checked>
                                <span class="slider"></span>
                            </label>
                        </div>
                    </div>
                </div>
                
                <div class="save-button-container">
                    <button class="btn btn-save" onclick="saveSchedule()">
                        💾 Save Schedule
                    </button>
                </div>
            </div>
        </div>
    </div>
    
    <div class="notification" id="notification"></div>
    
    <script>
        let espIP = window.location.hostname;
        
        // Initialize page
        document.addEventListener('DOMContentLoaded', function() {
            refreshData();
            loadSchedule();
            setInterval(refreshData, 5000); // Refresh every 5 seconds
            setInterval(updateUptime, 1000); // Update uptime every second
        });
        
        async function refreshData() {
            try {
                const response = await fetch(`http://${espIP}/api/status`);
                const data = await response.json();
                
                updateStatus(data);
                updateTime(data.currentTime, data.currentDate);
            } catch (error) {
                console.error('Error fetching data:', error);
                showNotification('Connection error', 'error');
            }
        }
        
        function updateStatus(data) {
            const acStatus = document.getElementById('acStatus');
            const acStatusText = document.getElementById('acStatusText');
            const systemStatus = document.getElementById('systemStatus');
            const systemStatusText = document.getElementById('systemStatusText');
            
            // AC Status
            if (data.acIsOn) {
                acStatus.classList.remove('off');
                acStatusText.textContent = 'AC Status: ON';
            } else {
                acStatus.classList.add('off');
                acStatusText.textContent = 'AC Status: OFF';
            }
            
            // System Status
            if (data.systemEnabled) {
                systemStatus.classList.remove('off');
                systemStatusText.textContent = 'System: ENABLED';
            } else {
                systemStatus.classList.add('off');
                systemStatusText.textContent = 'System: DISABLED';
            }
        }
        
        function updateTime(time, date) {
            const currentTimeElement = document.getElementById('currentTime');
            currentTimeElement.textContent = `${date} ${time}`;
        }
        
        function updateUptime() {
            const uptimeElement = document.getElementById('uptimeHours');
            const currentUptime = parseInt(uptimeElement.textContent);
            // This is a simple counter, you can enhance it with actual uptime from ESP32
        }
        
        async function controlAC(action) {
            try {
                const response = await fetch(`http://${espIP}/api/control`, {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json',
                    },
                    body: JSON.stringify({ action: action })
                });
                
                if (response.ok) {
                    showNotification(`Command executed: ${action}`, 'success');
                    setTimeout(refreshData, 500); // Refresh after 500ms
                } else {
                    showNotification('Command failed', 'error');
                }
            } catch (error) {
                console.error('Error controlling AC:', error);
                showNotification('Connection error', 'error');
            }
        }
        
        async function loadSchedule() {
            try {
                const response = await fetch(`http://${espIP}/api/schedule`);
                const schedule = await response.json();
                
                // Update weekday schedule
                document.getElementById('weekdayOnTime').value = 
                    `${schedule.weekdayOnHour.toString().padStart(2, '0')}:${schedule.weekdayOnMinute.toString().padStart(2, '0')}`;
                document.getElementById('weekdayOffTime').value = 
                    `${schedule.weekdayOffHour.toString().padStart(2, '0')}:${schedule.weekdayOffMinute.toString().padStart(2, '0')}`;
                document.getElementById('weekdayEnabled').checked = schedule.weekdayEnabled;
                
                // Update weekend schedule
                document.getElementById('weekendOnTime').value = 
                    `${schedule.weekendOnHour.toString().padStart(2, '0')}:${schedule.weekendOnMinute.toString().padStart(2, '0')}`;
                document.getElementById('weekendOffTime').value = 
                    `${schedule.weekendOffHour.toString().padStart(2, '0')}:${schedule.weekendOffMinute.toString().padStart(2, '0')}`;
                document.getElementById('weekendEnabled').checked = schedule.weekendEnabled;
                
            } catch (error) {
                console.error('Error loading schedule:', error);
                showNotification('Failed to load schedule', 'error');
            }
        }
        
        async function saveSchedule() {
            try {
                const weekdayOnTime = document.getElementById('weekdayOnTime').value.split(':');
                const weekdayOffTime = document.getElementById('weekdayOffTime').value.split(':');
                const weekendOnTime = document.getElementById('weekendOnTime').value.split(':');
                const weekendOffTime = document.getElementById('weekendOffTime').value.split(':');
                
                const scheduleData = {
                    weekdayOnHour: parseInt(weekdayOnTime[0]),
                    weekdayOnMinute: parseInt(weekdayOnTime[1]),
                    weekdayOffHour: parseInt(weekdayOffTime[0]),
                    weekdayOffMinute: parseInt(weekdayOffTime[1]),
                    weekendOnHour: parseInt(weekendOnTime[0]),
                    weekendOnMinute: parseInt(weekendOnTime[1]),
                    weekendOffHour: parseInt(weekendOffTime[0]),
                    weekendOffMinute: parseInt(weekendOffTime[1]),
                    weekdayEnabled: document.getElementById('weekdayEnabled').checked,
                    weekendEnabled: document.getElementById('weekendEnabled').checked
                };
                
                const response = await fetch(`http://${espIP}/api/schedule`, {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json',
                    },
                    body: JSON.stringify(scheduleData)
                });
                
                if (response.ok) {
                    showNotification('Schedule saved successfully!', 'success');
                } else {
                    showNotification('Failed to save schedule', 'error');
                }
            } catch (error) {
                console.error('Error saving schedule:', error);
                showNotification('Connection error', 'error');
            }
        }
        
        function showNotification(message, type) {
            const notification = document.getElementById('notification');
            notification.textContent = message;
            notification.className = `notification ${type}`;
            notification.style.display = 'block';
            
            setTimeout(() => {
                notification.style.display = 'none';
            }, 3000);
        }
    </script>
</body>
</html>
)rawliteral";

// === Function declarations ===
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
  systemStartTime = millis();

  irsend.begin();  // Init IR sender

  if (!
