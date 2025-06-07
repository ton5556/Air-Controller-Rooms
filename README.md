# Air-Controller-Rooms

# Smart Mitsubishi AC Controller (ESP8266)

A comprehensive IoT solution for controlling Mitsubishi air conditioners using ESP8266 microcontroller with IR transmission, web interface, and automated scheduling capabilities.

## 🌟 Features

- **Remote Control**: Control your Mitsubishi AC via web browser from anywhere on your network
- **Automated Scheduling**: Set up weekday and weekend schedules with dual time slots
- **Real-time Monitoring**: Track AC status, runtime, and system statistics
- **Multiple Control Methods**: Standard commands, force control, and toggle functionality
- **Static IP Configuration**: Fixed IP address (192.168.1.142) for consistent access
- **Web API**: RESTful API endpoints for integration with other systems
- **CORS Support**: Cross-origin requests enabled for web applications

## 📋 Hardware Requirements

### Components
- **ESP8266 Development Board** (NodeMCU, Wemos D1 Mini, etc.)
- **IR LED** (940nm recommended)
- **220Ω Resistor** (for IR LED current limiting)
- **Breadboard or PCB** for connections
- **Power Supply** (5V USB or 3.3V)

### Wiring Diagram
```
ESP8266 (NodeMCU)     IR LED
     3.3V ----------- Anode (+)
     D1 (GPIO5) ----- Cathode (-) [through 220Ω resistor]
     GND ------------ Ground
```

## 📚 Software Dependencies

### Arduino Libraries Required
```cpp
#include <ESP8266WiFi.h>      // ESP8266 WiFi functionality
#include <ESP8266WebServer.h> // HTTP web server
#include <ArduinoJson.h>      // JSON parsing and generation
#include <time.h>             // Time management
#include <IRremoteESP8266.h>  // IR communication base
#include <IRsend.h>           // IR transmission
#include <ir_Mitsubishi.h>    // Mitsubishi AC specific codes
```

### Installation
1. Install Arduino IDE
2. Add ESP8266 board package: `http://arduino.esp8266.com/stable/package_esp8266com_index.json`
3. Install required libraries through Library Manager

## ⚙️ Configuration

### Network Settings
```cpp
// WiFi credentials
const char* ssid = "-------";
const char* password = "-------";

// Static IP configuration
IPAddress local_IP(------);    // Fixed device IP
IPAddress gateway(192, 168, 1, 1);       // Router IP
IPAddress subnet(255, 255, 255, 0);      // Subnet mask
```

### Hardware Pins
```cpp
IRsend irsend(5);        // GPIO 5 (D1) for IR LED
IRMitsubishiAC ac(5);    // Same pin for AC control object
```

## 🚀 How It Works

### 1. System Architecture

```
[Web Browser] ←→ [ESP8266 Web Server] ←→ [IR Transmitter] ←→ [Mitsubishi AC]
                       ↕
                 [Schedule System]
                       ↕
                 [NTP Time Sync]
```

### 2. Core Components

#### **WiFi Connection with Static IP**
- Configures ESP8266 with fixed IP address (192.168.1.142)
- Fallback to DHCP if static configuration fails
- Automatic reconnection handling

#### **IR Communication**
- Uses IRremoteESP8266 library for Mitsubishi AC protocol
- Multiple transmission methods for reliability:
  - Standard Mitsubishi AC library commands
  - Duplicate command transmission
  - Raw IR code transmission
- Supports various AC functions: ON/OFF, temperature, fan speed, mode

#### **Web Server**
- Hosts control interface on port 80
- RESTful API endpoints for programmatic control
- CORS headers for cross-origin requests
- JSON-based request/response format

#### **Scheduling System**
- Dual weekday schedules (morning and evening)
- Separate weekend schedule
- Minute-by-minute schedule checking
- Individual schedule enable/disable controls

#### **Time Management**
- NTP synchronization for accurate timekeeping
- GMT+7 timezone configuration (Bangkok time)
- Daily statistics reset at midnight
- Runtime tracking and monitoring

### 3. Control Methods

#### **Standard Control**
```cpp
controlAC(true);   // Turn ON (only if currently OFF)
controlAC(false);  // Turn OFF (only if currently ON)
```

#### **Force Control**
```cpp
forceControlAC(true);   // Force ON (ignore current state)
forceControlAC(false);  // Force OFF (ignore current state)
```

#### **Toggle Control**
```cpp
toggleAC();  // Switch current state (ON→OFF or OFF→ON)
```

## 🌐 Web Interface

### Access Points
- **Main Interface**: `http://192.168.1.142/`
- **Status API**: `http://192.168.1.142/api/status`
- **Control API**: `http://192.168.1.142/api/control`
- **Schedule API**: `http://192.168.1.142/api/schedule`

### API Endpoints

#### GET `/api/status`
Returns current system status:
```json
{
  "acIsOn": true,
  "systemEnabled": true,
  "currentTime": "14:30:25",
  "currentDate": "2025/06/07",
  "uptimeHours": 12.5,
  "todayOnTime": 180
}
```

#### POST `/api/control`
Control AC operations:
```json
{
  "action": "turnOn"     // turnOn, turnOff, forceOn, forceOff, toggle
}
```

#### GET/POST `/api/schedule`
Manage scheduling:
```json
{
  "weekdayOnTime1": "08:00",
  "weekdayOffTime1": "12:00",
  "weekdayEnabled1": true,
  "weekdayOnTime2": "14:00",
  "weekdayOffTime2": "18:00",
  "weekdayEnabled2": true,
  "weekendOnTime": "10:00",
  "weekendOffTime": "22:00",
  "weekendEnabled": true
}
```

## 📊 System States and Variables

### State Management
```cpp
bool acIsOn = false;           // Current AC power state
bool systemEnabled = true;     // Master system enable/disable
unsigned long lastOnTime = 0;  // Timestamp when AC was turned on
unsigned long todayOnTime = 0; // Total runtime today (minutes)
```

### Schedule Structure
```cpp
struct Schedule {
  // Weekday Schedule 1 (e.g., morning)
  int weekdayOnHour1, weekdayOnMinute1;
  int weekdayOffHour1, weekdayOffMinute1;
  bool weekdayEnabled1;
  
  // Weekday Schedule 2 (e.g., evening)
  int weekdayOnHour2, weekdayOnMinute2;
  int weekdayOffHour2, weekdayOffMinute2;
  bool weekdayEnabled2;
  
  // Weekend Schedule
  int weekendOnHour, weekendOnMinute;
  int weekendOffHour, weekendOffMinute;
  bool weekendEnabled;
};
```

## 🔧 Setup Instructions

### 1. Hardware Assembly
1. Connect IR LED to GPIO 5 (D1) through 220Ω resistor
2. Ensure proper power supply to ESP8266
3. Position IR LED to face the AC unit (optimal range: 1-3 meters)

### 2. Software Configuration
1. Update WiFi credentials in the code
2. Adjust IP configuration for your network
3. Modify timezone settings if needed (currently GMT+7)
4. Upload code to ESP8266

### 3. AC Integration
1. Test IR transmission with your specific AC model
2. Adjust IR codes if necessary (capture with IR receiver)
3. Verify AC responds to commands
4. Fine-tune temperature and mode settings

### 4. Network Setup
1. Ensure IP 192.168.1.142 is available on your network
2. Configure router to allow the static IP (if needed)
3. Access web interface at `http://192.168.1.142`

   🛠️ Troubleshooting

### Common Issues

**AC Not Responding**
- Check IR LED connection and orientation
- Verify IR codes match your AC model
- Ensure adequate power supply
- Test with closer proximity to AC

**WiFi Connection Issues**
- Verify WiFi credentials
- Check if static IP conflicts with DHCP range
- Monitor serial output for connection status
- Use DHCP fallback if static IP fails

**Schedule Not Working**
- Confirm time synchronization (NTP)
- Check timezone configuration
- Verify schedule times are set correctly
- Ensure system is enabled

**Web Interface Inaccessible**
- Ping 192.168.1.142 to verify connectivity
- Check if web server started successfully
- Verify firewall settings
- Try DHCP mode for initial setup

Debug Information
Monitor serial output (115200 baud) for detailed system information:
- WiFi connection status
- IR transmission attempts
- Schedule trigger events
- API request handling
- Error messages and warnings

Security Considerations

- **Network Access**: Device accessible to anyone on local network
- **No Authentication**: Web interface has no password protection
- **HTTP Only**: Communications not encrypted
- **Consider**: Adding authentication for production use

Future Enhancements

- **Mobile App**: Dedicated smartphone application
- **MQTT Integration**: Home automation system compatibility
- **Temperature Monitoring**: Add temperature sensor feedback
- **Energy Monitoring**: Power consumption tracking
- **Voice Control**: Integration with Alexa/Google Assistant
- **Remote Access**: VPN or cloud connectivity options
- 
📄 License
  Dev. https://github.com/crankyoldgit
  Sincerely, David Conrad Library
