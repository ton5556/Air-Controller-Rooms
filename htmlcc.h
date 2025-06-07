const char* htmlInterface = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Smart AC Controller</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
            color: #333;
        }

        .container {
            max-width: 800px;
            margin: 0 auto;
            background: rgba(255, 255, 255, 0.95);
            border-radius: 20px;
            padding: 30px;
            box-shadow: 0 20px 40px rgba(0, 0, 0, 0.1);
            backdrop-filter: blur(10px);
        }

        .header {
            text-align: center;
            margin-bottom: 30px;
            padding-bottom: 20px;
            border-bottom: 2px solid #eee;
        }

        .header h1 {
            color: #2c3e50;
            font-size: 2.5em;
            margin-bottom: 10px;
            font-weight: 700;
        }

        .status-indicator {
            display: inline-block;
            padding: 8px 16px;
            border-radius: 20px;
            font-weight: 600;
            font-size: 0.9em;
            text-transform: uppercase;
            letter-spacing: 1px;
        }

        .status-on {
            background: #d4edda;
            color: #155724;
            border: 2px solid #c3e6cb;
        }

        .status-off {
            background: #f8d7da;
            color: #721c24;
            border: 2px solid #f5c6cb;
        }

        .card {
            background: white;
            border-radius: 15px;
            padding: 25px;
            margin-bottom: 20px;
            box-shadow: 0 8px 16px rgba(0, 0, 0, 0.1);
            border: 1px solid #e9ecef;
        }

        .card h2 {
            color: #2c3e50;
            margin-bottom: 20px;
            font-size: 1.4em;
            display: flex;
            align-items: center;
            gap: 10px;
        }

        .control-buttons {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 15px;
            margin-bottom: 20px;
        }

        .btn {
            padding: 15px 25px;
            border: none;
            border-radius: 10px;
            font-size: 1em;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s ease;
            text-transform: uppercase;
            letter-spacing: 1px;
            position: relative;
            overflow: hidden;
        }

        .btn:hover {
            transform: translateY(-2px);
            box-shadow: 0 8px 16px rgba(0, 0, 0, 0.2);
        }

        .btn:active {
            transform: translateY(0);
        }

        .btn-success {
            background: linear-gradient(45deg, #28a745, #20c997);
            color: white;
        }

        .btn-danger {
            background: linear-gradient(45deg, #dc3545, #e83e8c);
            color: white;
        }

        .btn-primary {
            background: linear-gradient(45deg, #007bff, #6610f2);
            color: white;
        }

        .btn-secondary {
            background: linear-gradient(45deg, #6c757d, #495057);
            color: white;
        }

        .schedule-section {
            margin-bottom: 30px;
        }

        .schedule-group {
            background: #f8f9fa;
            border-radius: 10px;
            padding: 20px;
            margin-bottom: 15px;
            border-left: 4px solid #007bff;
        }

        .schedule-group h3 {
            color: #495057;
            margin-bottom: 15px;
            font-size: 1.1em;
        }

        .schedule-row {
            display: grid;
            grid-template-columns: auto 1fr 1fr auto;
            gap: 15px;
            align-items: center;
            margin-bottom: 10px;
        }

        .time-input {
            padding: 10px;
            border: 2px solid #dee2e6;
            border-radius: 8px;
            font-size: 1em;
            transition: border-color 0.3s ease;
        }

        .time-input:focus {
            outline: none;
            border-color: #007bff;
            box-shadow: 0 0 0 3px rgba(0, 123, 255, 0.1);
        }

        .toggle-switch {
            position: relative;
            display: inline-block;
            width: 60px;
            height: 34px;
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
            background-color: #ccc;
            transition: .4s;
            border-radius: 34px;
        }

        .slider:before {
            position: absolute;
            content: "";
            height: 26px;
            width: 26px;
            left: 4px;
            bottom: 4px;
            background-color: white;
            transition: .4s;
            border-radius: 50%;
        }

        input:checked + .slider {
            background-color: #007bff;
        }

        input:checked + .slider:before {
            transform: translateX(26px);
        }

        .stats-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 15px;
        }

        .stat-item {
            background: #f8f9fa;
            padding: 20px;
            border-radius: 10px;
            text-align: center;
            border: 1px solid #dee2e6;
        }

        .stat-value {
            font-size: 2em;
            font-weight: bold;
            color: #007bff;
            margin-bottom: 5px;
        }

        .stat-label {
            color: #6c757d;
            font-size: 0.9em;
            text-transform: uppercase;
            letter-spacing: 1px;
        }

        .message {
            padding: 15px;
            border-radius: 8px;
            margin: 10px 0;
            font-weight: 500;
            display: none;
            animation: slideIn 0.3s ease;
        }

        .message.success {
            background: #d4edda;
            color: #155724;
            border: 1px solid #c3e6cb;
        }

        .message.error {
            background: #f8d7da;
            color: #721c24;
            border: 1px solid #f5c6cb;
        }

        @keyframes slideIn {
            from { opacity: 0; transform: translateY(-10px); }
            to { opacity: 1; transform: translateY(0); }
        }

        .loading {
            display: inline-block;
            width: 20px;
            height: 20px;
            border: 2px solid #f3f3f3;
            border-top: 2px solid #007bff;
            border-radius: 50%;
            animation: spin 1s linear infinite;
            margin-left: 10px;
        }

        @keyframes spin {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
        }

        .system-status {
            display: flex;
            align-items: center;
            gap: 10px;
            margin-bottom: 20px;
            padding: 15px;
            background: #f8f9fa;
            border-radius: 10px;
            border-left: 4px solid #28a745;
        }

        .status-dot {
            width: 12px;
            height: 12px;
            border-radius: 50%;
            background: #28a745;
            animation: pulse 2s infinite;
        }

        @keyframes pulse {
            0% { opacity: 1; }
            50% { opacity: 0.5; }
            100% { opacity: 1; }
        }

        @media (max-width: 768px) {
            .container {
                padding: 20px;
                margin: 10px;
            }

            .schedule-row {
                grid-template-columns: 1fr;
                gap: 10px;
            }

            .control-buttons {
                grid-template-columns: 1fr;
            }

            .header h1 {
                font-size: 2em;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🌀 Smart AC Controller</h1>
            <div id="acStatus" class="status-indicator">Loading...</div>
        </div>

        <div id="messageArea"></div>

        <!-- System Status -->
        <div class="card">
            <h2>📊 System Status</h2>
            <div class="system-status">
                <div class="status-dot" id="systemDot"></div>
                <span id="systemStatusText">System Active</span>
            </div>
            <div class="stats-grid">
                <div class="stat-item">
                    <div class="stat-value" id="currentTime">--:--:--</div>
                    <div class="stat-label">Current Time</div>
                </div>
                <div class="stat-item">
                    <div class="stat-value" id="currentDate">----------</div>
                    <div class="stat-label">Date</div>
                </div>
                <div class="stat-item">
                    <div class="stat-value" id="uptimeHours">0</div>
                    <div class="stat-label">Uptime (Hours)</div>
                </div>
                <div class="stat-item">
                    <div class="stat-value" id="todayOnTime">0</div>
                    <div class="stat-label">Today On Time (Min)</div>
                </div>
            </div>
        </div>

        <!-- Manual Controls -->
        <div class="card">
            <h2>🎮 Manual Controls</h2>
            <div class="control-buttons">
                <button class="btn btn-success" onclick="controlAC('turnOn')">
                    🟢 Turn AC ON
                </button>
                <button class="btn btn-danger" onclick="controlAC('turnOff')">
                    🔴 Turn AC OFF
                </button>
                <button class="btn btn-success" onclick="controlAC('forceOn')" style="background: linear-gradient(45deg, #17a2b8, #138496);">
                    🔄 FORCE ON
                </button>
                <button class="btn btn-danger" onclick="controlAC('forceOff')" style="background: linear-gradient(45deg, #fd7e14, #e8590c);">
                    🔄 FORCE OFF
                </button>
                <button class="btn btn-secondary" onclick="controlAC('toggle')">
                    🔄 TOGGLE AC
                </button>
                <button class="btn btn-primary" onclick="controlAC('enableSystem')">
                    ✅ Enable System
                </button>
                <button class="btn btn-secondary" onclick="controlAC('disableSystem')">
                    ⏸️ Disable System
                </button>
            </div>
        </div>

        <!-- Schedule Settings -->
        <div class="card">
            <h2>📅 Schedule Settings</h2>
            
            <!-- Weekday Schedules -->
            <div class="schedule-section">
                <div class="schedule-group">
                    <h3>💼 Weekday Schedule 1</h3>
                    <div class="schedule-row">
                        <label class="toggle-switch">
                            <input type="checkbox" id="weekdayEnabled1" onchange="updateSchedule()">
                            <span class="slider"></span>
                        </label>
                        <div>
                            <label>ON Time:</label>
                            <input type="time" id="weekdayOnTime1" class="time-input" onchange="updateSchedule()">
                        </div>
                        <div>
                            <label>OFF Time:</label>
                            <input type="time" id="weekdayOffTime1" class="time-input" onchange="updateSchedule()">
                        </div>
                    </div>
                </div>

                <div class="schedule-group">
                    <h3>💼 Weekday Schedule 2</h3>
                    <div class="schedule-row">
                        <label class="toggle-switch">
                            <input type="checkbox" id="weekdayEnabled2" onchange="updateSchedule()">
                            <span class="slider"></span>
                        </label>
                        <div>
                            <label>ON Time:</label>
                            <input type="time" id="weekdayOnTime2" class="time-input" onchange="updateSchedule()">
                        </div>
                        <div>
                            <label>OFF Time:</label>
                            <input type="time" id="weekdayOffTime2" class="time-input" onchange="updateSchedule()">
                        </div>
                    </div>
                </div>

                <div class="schedule-group">
                    <h3>🏖️ Weekend Schedule</h3>
                    <div class="schedule-row">
                        <label class="toggle-switch">
                            <input type="checkbox" id="weekendEnabled" onchange="updateSchedule()">
                            <span class="slider"></span>
                        </label>
                        <div>
                            <label>ON Time:</label>
                            <input type="time" id="weekendOnTime" class="time-input" onchange="updateSchedule()">
                        </div>
                        <div>
                            <label>OFF Time:</label>
                            <input type="time" id="weekendOffTime" class="time-input" onchange="updateSchedule()">
                        </div>
                    </div>
                </div>
            </div>

            <button class="btn btn-primary" onclick="saveSchedule()">
                💾 Save All Schedules
            </button>
        </div>
    </div>

    <script>
        let updateInterval;
        let isUpdating = false;

        // Initialize the interface
        document.addEventListener('DOMContentLoaded', function() {
            updateStatus();
            loadSchedule();
            startAutoUpdate();
        });

        function startAutoUpdate() {
            updateInterval = setInterval(updateStatus, 5000); // Update every 5 seconds
        }

        function showMessage(message, type = 'success') {
            const messageArea = document.getElementById('messageArea');
            const messageDiv = document.createElement('div');
            messageDiv.className = `message ${type}`;
            messageDiv.textContent = message;
            messageDiv.style.display = 'block';
            
            messageArea.innerHTML = ''; // Clear previous messages
            messageArea.appendChild(messageDiv);
            
            setTimeout(() => {
                messageDiv.style.display = 'none';
            }, 5000);
        }

        async function updateStatus() {
            if (isUpdating) return;
            isUpdating = true;

            try {
                const response = await fetch('/api/status');
                if (!response.ok) throw new Error('Failed to fetch status');
                
                const data = await response.json();
                
                // Update AC status
                const statusElement = document.getElementById('acStatus');
                if (data.acIsOn) {
                    statusElement.textContent = 'AC IS ON';
                    statusElement.className = 'status-indicator status-on';
                } else {
                    statusElement.textContent = 'AC IS OFF';
                    statusElement.className = 'status-indicator status-off';
                }

                // Update system status
                const systemDot = document.getElementById('systemDot');
                const systemText = document.getElementById('systemStatusText');
                if (data.systemEnabled) {
                    systemDot.style.background = '#28a745';
                    systemText.textContent = 'System Active';
                } else {
                    systemDot.style.background = '#dc3545';
                    systemText.textContent = 'System Disabled';
                }

                // Update stats
                document.getElementById('currentTime').textContent = data.currentTime || '--:--:--';
                document.getElementById('currentDate').textContent = data.currentDate || '----/--/--';
                document.getElementById('uptimeHours').textContent = Math.floor(data.uptimeHours || 0);
                document.getElementById('todayOnTime').textContent = data.todayOnTime || 0;

            } catch (error) {
                console.error('Error updating status:', error);
                showMessage('Failed to update status', 'error');
            } finally {
                isUpdating = false;
            }
        }

        async function controlAC(action) {
            const button = event.target;
            const originalText = button.innerHTML;
            button.innerHTML = originalText + '<span class="loading"></span>';
            button.disabled = true;

            try {
                const response = await fetch('/api/control', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json',
                    },
                    body: JSON.stringify({ action: action })
                });

                if (!response.ok) throw new Error('Failed to control AC');
                
                const data = await response.json();
                showMessage(data.message, 'success');
                
                // Update status immediately
                setTimeout(updateStatus, 500);
                
            } catch (error) {
                console.error('Error controlling AC:', error);
                showMessage('Failed to control AC: ' + error.message, 'error');
            } finally {
                button.innerHTML = originalText;
                button.disabled = false;
            }
        }

        async function loadSchedule() {
            try {
                const response = await fetch('/api/schedule');
                if (!response.ok) throw new Error('Failed to load schedule');
                
                const data = await response.json();
                
                // Load weekday schedule 1
                document.getElementById('weekdayOnTime1').value = data.weekdayOnTime1 || '08:00';
                document.getElementById('weekdayOffTime1').value = data.weekdayOffTime1 || '12:00';
                document.getElementById('weekdayEnabled1').checked = data.weekdayEnabled1 !== false;

                // Load weekday schedule 2
                document.getElementById('weekdayOnTime2').value = data.weekdayOnTime2 || '14:00';
                document.getElementById('weekdayOffTime2').value = data.weekdayOffTime2 || '18:00';
                document.getElementById('weekdayEnabled2').checked = data.weekdayEnabled2 !== false;

                // Load weekend schedule
                document.getElementById('weekendOnTime').value = data.weekendOnTime || '10:00';
                document.getElementById('weekendOffTime').value = data.weekendOffTime || '22:00';
                document.getElementById('weekendEnabled').checked = data.weekendEnabled !== false;

            } catch (error) {
                console.error('Error loading schedule:', error);
                showMessage('Failed to load schedule', 'error');
            }
        }

        async function updateSchedule() {
            // Debounce the update to avoid too many requests
            clearTimeout(window.scheduleUpdateTimeout);
            window.scheduleUpdateTimeout = setTimeout(saveSchedule, 1000);
        }

        async function saveSchedule() {
            try {
                const scheduleData = {
                    weekdayOnTime1: document.getElementById('weekdayOnTime1').value,
                    weekdayOffTime1: document.getElementById('weekdayOffTime1').value,
                    weekdayEnabled1: document.getElementById('weekdayEnabled1').checked,
                    
                    weekdayOnTime2: document.getElementById('weekdayOnTime2').value,
                    weekdayOffTime2: document.getElementById('weekdayOffTime2').value,
                    weekdayEnabled2: document.getElementById('weekdayEnabled2').checked,
                    
                    weekendOnTime: document.getElementById('weekendOnTime').value,
                    weekendOffTime: document.getElementById('weekendOffTime').value,
                    weekendEnabled: document.getElementById('weekendEnabled').checked
                };

                const response = await fetch('/api/schedule', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json',
                    },
                    body: JSON.stringify(scheduleData)
                });

                if (!response.ok) throw new Error('Failed to save schedule');
                
                const data = await response.json();
                showMessage('Schedule saved successfully!', 'success');

            } catch (error) {
                console.error('Error saving schedule:', error);
                showMessage('Failed to save schedule: ' + error.message, 'error');
            }
        }

        // Cleanup on page unload
        window.addEventListener('beforeunload', function() {
            clearInterval(updateInterval);
        });
    </script>
</body>
</html>
)rawliteral";
