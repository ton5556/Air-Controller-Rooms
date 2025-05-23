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
            background: linear-gradient(120deg, #e0eafc 0%, #cfdef3 100%);
            min-height: 100vh;
            padding: 20px;
            overflow-x: hidden;
        }
        
        .floating-particles {
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            pointer-events: none;
            z-index: 1;
        }
        
        .particle {
            position: absolute;
            background: rgba(255, 255, 255, 0.3);
            border-radius: 50%;
            animation: float 20s infinite linear;
        }
        
        @keyframes float {
            0% {
                transform: translateY(100vh) rotate(0deg);
                opacity: 0;
            }
            10% {
                opacity: 1;
            }
            90% {
                opacity: 1;
            }
            100% {
                transform: translateY(-100px) rotate(360deg);
                opacity: 0;
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
        
        .header::before {
            content: '';
            position: absolute;
            top: -100%;
            left: -100%;
            width: 300%;
            height: 300%;
            background: radial-gradient(circle, rgba(255,255,255,0.1) 0%, transparent 70%);
            animation: headerPulse 8s ease-in-out infinite;
        }
        
        @keyframes headerPulse {
            0%, 100% { 
                transform: scale(1) rotate(0deg);
                opacity: 0.5;
            }
            50% { 
                transform: scale(1.2) rotate(180deg);
                opacity: 0.8;
            }
        }
        
        .header h1 {
            font-size: 3.5em;
            margin-bottom: 20px;
            font-weight: 700;
            position: relative;
            z-index: 1;
            text-shadow: 0 4px 20px rgba(0,0,0,0.3);
            animation: titleGlow 3s ease-in-out infinite alternate;
        }
        
        @keyframes titleGlow {
            0% { text-shadow: 0 4px 20px rgba(0,0,0,0.3); }
            100% { text-shadow: 0 4px 30px rgba(255,255,255,0.5); }
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
        
        .status-light::after {
            content: '';
            position: absolute;
            top: 50%;
            left: 50%;
            width: 30px;
            height: 30px;
            border-radius: 50%;
            background: inherit;
            opacity: 0.3;
            transform: translate(-50%, -50%);
            animation: ripple 2s infinite;
        }
        
        .status-light.off {
            background: var(--danger-gradient);
            box-shadow: 0 0 20px rgba(245, 87, 108, 0.5);
        }
        
        @keyframes pulse {
            0%, 100% { opacity: 1; transform: scale(1); }
            50% { opacity: 0.8; transform: scale(1.1); }
        }
        
        @keyframes ripple {
            0% { transform: translate(-50%, -50%) scale(1); opacity: 0.3; }
            100% { transform: translate(-50%, -50%) scale(2.5); opacity: 0; }
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
            animation: timeGlow 4s ease-in-out infinite alternate;
            border: 1px solid rgba(102, 126, 234, 0.2);
        }
        
        @keyframes timeGlow {
            0% { 
                box-shadow: 0 8px 25px rgba(102, 126, 234, 0.2);
                transform: scale(1);
            }
            100% { 
                box-shadow: 0 12px 35px rgba(102, 126, 234, 0.4);
                transform: scale(1.02);
            }
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
            animation: lineExpand 2s ease-out;
        }
        
        @keyframes lineExpand {
            0% { width: 0; }
            100% { width: 100%; }
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
        
        .btn::before {
            content: '';
            position: absolute;
            top: 0;
            left: -100%;
            width: 100%;
            height: 100%;
            background: linear-gradient(90deg, transparent, rgba(255,255,255,0.3), transparent);
            transition: left 0.6s;
        }
        
        .btn:hover::before {
            left: 100%;
        }
        
        .btn::after {
            content: '';
            position: absolute;
            top: 50%;
            left: 50%;
            width: 0;
            height: 0;
            background: rgba(255,255,255,0.2);
            border-radius: 50%;
            transform: translate(-50%, -50%);
            transition: all 0.6s ease;
        }
        
        .btn:active::after {
            width: 300px;
            height: 300px;
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
        
        .btn:hover {
            transform: translateY(-8px) scale(1.03);
            box-shadow: 0 20px 40px rgba(0,0,0,0.25);
        }
        
        .btn:active {
            transform: translateY(-3px) scale(1.01);
        }
        
        .btn:disabled {
            opacity: 0.6;
            cursor: not-allowed;
            transform: none;
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
        
        .schedule-card::before {
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
            height: 5px;
            background: var(--primary-gradient);
            transform: scaleX(0);
            transition: transform 0.4s ease;
        }
        
        .schedule-card:hover::before {
            transform: scaleX(1);
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
        
        .btn-save:hover {
            box-shadow: 0 20px 45px rgba(102, 126, 234, 0.4);
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
        }
        
        .notification.success {
            background: var(--success-gradient);
        }
        
        .notification.error {
            background: var(--danger-gradient);
        }
        
        .notification.info {
            background: var(--info-gradient);
            color: #333;
        }
        
        .notification.slide-in {
            animation: slideIn 0.6s cubic-bezier(0.175, 0.885, 0.32, 1.275);
        }
        
        .notification.slide-out {
            animation: slideOut 0.4s ease-in;
        }
        
        @keyframes slideIn {
            from {
                transform: translateX(120%) scale(0.8) rotate(10deg);
                opacity: 0;
            }
            to {
                transform: translateX(0) scale(1) rotate(0deg);
                opacity: 1;
            }
        }
        
        @keyframes slideOut {
            from {
                transform: translateX(0) scale(1) rotate(0deg);
                opacity: 1;
            }
            to {
                transform: translateX(120%) scale(0.8) rotate(-10deg);
                opacity: 0;
            }
        }
        
        .connection-status {
            position: fixed;
            bottom: 30px;
            left: 30px;
            padding: 15px 25px;
            background: var(--glass-bg);
            backdrop-filter: blur(15px);
            border-radius: 25px;
            border: 1px solid var(--glass-border);
            display: flex;
            align-items: center;
            gap: 12px;
            font-weight: 600;
            z-index: 1000;
            transition: all 0.3s ease;
        }
        
        .connection-dot {
            width: 12px;
            height: 12px;
            border-radius: 50%;
            background: var(--success-gradient);
            animation: pulse 2s infinite;
        }
        
        .connection-dot.offline {
            background: var(--danger-gradient);
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
            
            .status-bar {
                padding: 25px 20px;
            }
            
            .control-buttons {
                grid-template-columns: 1fr;
            }
            
            .main-content {
                padding: 40px 25px;
            }
            
            .notification {
                right: 15px;
                left: 15px;
                max-width: none;
            }
            
            .connection-status {
                bottom: 15px;
                left: 15px;
            }
        }
        
        @media (max-width: 480px) {
            .time-input-group {
                flex-direction: column;
                align-items: stretch;
                gap: 15px;
            }
            
            .time-input-group label {
                min-width: auto;
                text-align: center;
            }
            
            .header h1 {
                font-size: 2.2em;
            }
            
            .section-title {
                font-size: 1.8em;
            }
        }
    </style>
</head>
<body>
    <div class="floating-particles" id="particles"></div>
    
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
                            <input type="time" class="time-input" id="weekdayOnTime" value="08:00">
                        </div>
                        <div class="time-input-group">
                            <label>Turn OFF:</label>
                            <input type="time" class="time-input" id="weekdayOffTime" value="18:00">
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
                                                        <input type="time" class="time-input" id="weekendOnTime" value="10:00">
                        </div>
                        <div class="time-input-group">
                            <label>Turn OFF:</label>
                            <input type="time" class="time-input" id="weekendOffTime" value="22:00">
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
                    <button class="btn btn-save" onclick="saveSchedule()">💾 Save Schedule</button>
                </div>
            </div>
        </div>
    </div>

    <div class="notification success slide-in" id="successNotification" style="display:none;">✅ Schedule saved successfully!</div>
    <div class="notification error slide-in" id="errorNotification" style="display:none;">❌ Failed to save schedule!</div>
    <div class="connection-status" id="connectionStatus">
        <div class="connection-dot" id="connectionDot"></div>
        <span id="connectionText">Connected</span>
    </div>

    <script>
        function controlAC(action) {
            console.log("Sending control command:", action);
            // Implementation needed to send command to server or device
        }

        function refreshData() {
            console.log("Refreshing data...");
            // Implementation to refresh data from backend
        }

        function saveSchedule() {
            console.log("Saving schedule...");
            document.getElementById("successNotification").style.display = "block";
            setTimeout(() => {
                document.getElementById("successNotification").classList.add("slide-out");
                setTimeout(() => {
                    document.getElementById("successNotification").style.display = "none";
                    document.getElementById("successNotification").classList.remove("slide-out");
                }, 400);
            }, 2500);
        }

        function updateTime() {
            const now = new Date();
            const timeString = now.toLocaleTimeString();
            document.getElementById("currentTime").innerText = timeString;
        }

        function simulateStatus() {
            document.getElementById("acStatus").classList.toggle("off");
            document.getElementById("systemStatus").classList.toggle("off");
            document.getElementById("acStatusText").innerText = "AC Status: " + 
                (document.getElementById("acStatus").classList.contains("off") ? "OFF" : "ON");
            document.getElementById("systemStatusText").innerText = "System: " + 
                (document.getElementById("systemStatus").classList.contains("off") ? "DISABLED" : "ENABLED");
        }

        setInterval(updateTime, 1000);
        setInterval(simulateStatus, 5000);
        updateTime();
        simulateStatus();

        // --- Add this block for floating particles ---
        function createParticles(count = 30) {
            const container = document.getElementById('particles');
            for (let i = 0; i < count; i++) {
                const p = document.createElement('div');
                p.className = 'particle';
                const size = Math.random() * 18 + 8;
                p.style.width = `${size}px`;
                p.style.height = `${size}px`;
                p.style.left = `${Math.random() * 100}%`;
                p.style.animationDuration = `${14 + Math.random() * 10}s`;
                p.style.animationDelay = `${Math.random() * 10}s`;
                p.style.background = `rgba(255,255,255,${0.15 + Math.random() * 0.25})`;
                container.appendChild(p);
            }
        }
        createParticles(32);
    </script>
</body>
</html>

)rawliteral";
