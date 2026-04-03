# fish_light_server.py
from flask import Flask, request
import threading
import time
from datetime import datetime
from zoneinfo import ZoneInfo

app = Flask(__name__)

brightness = 0
willez_mode = True
willez_on_time = "06:15"   # default Auto ON time (Central Time)
willez_off_time = "21:00"  # default Auto OFF time (Central Time)

@app.route('/getbrightness')
def get_brightness():
    return str(brightness)

@app.route('/set', methods=['GET', 'POST'])
def set_brightness():
    global brightness
    try:
        val = int(request.args.get('value') or request.form.get('value') or 0)
        if 0 <= val <= 255:
            brightness = val
            return "OK"
    except:
        pass
    return "ERROR", 400

@app.route('/getmode')
def get_mode():
    return str(int(willez_mode))

@app.route('/setmode', methods=['GET', 'POST'])
def set_mode():
    global willez_mode
    try:
        val = int(request.args.get('value') or request.form.get('value') or 0)
        willez_mode = bool(val)
        return "OK"
    except:
        pass
    return "ERROR", 400

@app.route('/gettimes')
def get_times():
    """Return current ON and OFF times as simple CSV: HH:MM,HH:MM"""
    return f"{willez_on_time},{willez_off_time}"

@app.route('/settimes', methods=['GET', 'POST'])
def set_times():
    global willez_on_time, willez_off_time
    try:
        on = request.args.get('on') or request.form.get('on')
        off = request.args.get('off') or request.form.get('off')
        if on and off:
            # Validate that both are valid HH:MM times
            datetime.strptime(on, "%H:%M")
            datetime.strptime(off, "%H:%M")
            willez_on_time = on
            willez_off_time = off
            return "OK"
    except:
        pass
    return "ERROR", 400

def will_ez_worker():
    """Background thread: when WillEZ mode is ON, automatically set brightness
    based on the user-configurable times in Central Time."""
    global brightness
    tz = ZoneInfo("America/Chicago")
    while True:
        if willez_mode:
            try:
                now = datetime.now(tz)
                current_time = now.time()
                
                on_start = datetime.strptime(willez_on_time, "%H:%M").time()
                on_end = datetime.strptime(willez_off_time, "%H:%M").time()
                
                new_brightness = 255 if on_start <= current_time < on_end else 0
                
                if new_brightness != brightness:
                    brightness = new_brightness
                    print(f"✅ WillEZ auto-set brightness to {brightness} at {now.strftime('%H:%M:%S %Z')} (ON:{willez_on_time} OFF:{willez_off_time})")
            except Exception as e:
                print(f"WillEZ worker error: {e}")
        time.sleep(30)

@app.route('/')
def index():
    mode_status = "ON" if willez_mode else "OFF"
    mode_color = "#0f0" if willez_mode else "#f00"
    return f"""
    <!DOCTYPE html>
    <html>
    <head>
        <title>Fish Light</title>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <style>
            body {{font-family:Arial;background:#111;color:#eee;text-align:center;padding:40px;}}
            h1 {{color:#0f8;}}
            .status {{font-size:3em;margin:30px;color:{'#0f0' if brightness>0 else '#f00'};}}
            .mode-status {{font-size:1.8em;margin:20px 0;}}
            .explanation {{background:#222;padding:20px;border-radius:15px;margin:40px auto 20px;max-width:620px;text-align:left;font-size:1.05em;line-height:1.5;}}
            button {{padding:14px 35px;font-size:1.3em;margin:10px;border:none;border-radius:10px;cursor:pointer;}}
            .on {{background:#0f0;color:black;}} 
            .off {{background:#f00;color:white;}}
            input[type=range] {{width:320px;height:14px;margin:20px 0;}}
            input[type=time] {{font-size:1.2em;padding:8px 12px;border-radius:6px;border:1px solid #444;background:#222;color:#eee;}}
            .schedule {{margin:30px 0;background:#1a1a1a;padding:20px;border-radius:15px;max-width:620px;margin-left:auto;margin-right:auto;}}
        </style>
    </head>
    <body>
        <h1>🐟 Fish Light Control</h1>
        <div class="status" id="brightness-status">Brightness: {brightness}</div>
        
        <div class="mode-status" id="mode-status">
            WillEZ Mode: <span style="color:{mode_color}">{mode_status}</span>
        </div>

        <div class="schedule">
            <h3 style="margin-top:0;color:#0f8;">📅 WillEZ Schedule (Central Time)</h3>
            <label style="font-size:1.1em;">Auto ON at: </label>
            <input type="time" id="on-time" value="{willez_on_time}">
            
            <label style="margin-left:30px;font-size:1.1em;">Auto OFF at: </label>
            <input type="time" id="off-time" value="{willez_off_time}">
            
            <button onclick="saveSchedule()" style="margin-left:25px;font-size:1.1em;padding:10px 24px;">💾 Save Schedule</button>
        </div>
        
        <button class="on" onclick="set(255)">FULL ON</button>
        <button class="off" onclick="set(0)">OFF</button><br><br>
        
        <input type="range" min="0" max="255" value="{brightness}" id="slider" oninput="live(this.value)">
        <button onclick="set(document.getElementById('slider').value)">Apply</button>
        
        <br><br>
        <button id="mode-toggle" onclick="toggleMode()" style="font-size:1.3em;padding:12px 30px;">
            Toggle WillEZ Mode
        </button>
        
        <div class="explanation">
            <strong>💡 WillEZ Mode Explained:</strong><br>
            When enabled, the Fish Light automatically turns <strong>ON</strong> at your chosen "Auto ON" time and turns <strong>OFF</strong> at your chosen "Auto OFF" time — every single day.<br>
            All times use <strong>Central Time</strong> (America/Chicago) and automatically adjust for Daylight Saving Time.<br>
            Manual controls (slider / Full ON / OFF) still work at any time.
        </div>

        <p style="margin-top:50px;color:#777;">Server @ 192.168.1.198:9041 • WillEZ auto mode (Central Time)</p>
        
        <script>
            let modeEnabled = {str(willez_mode).lower()};

            function set(v) {{
                fetch('/set?value=' + v);
            }}
            
            function live(v) {{
                document.getElementById('brightness-status').innerText = 'Brightness: ' + v;
            }}
            
            function toggleMode() {{
                const newVal = modeEnabled ? 0 : 1;
                fetch('/setmode?value=' + newVal)
                    .then(() => updateAll());
            }}
            
            function saveSchedule() {{
                const onTime = document.getElementById('on-time').value;
                const offTime = document.getElementById('off-time').value;
                if (onTime && offTime) {{
                    fetch(`/settimes?on=${{onTime}}&off=${{offTime}}`)
                        .then(response => {{
                            if (response.ok) {{
                                alert("✅ Schedule saved!");
                                updateAll();
                            }} else {{
                                alert("❌ Error saving schedule");
                            }}
                        }});
                }}
            }}
            
            function updateAll() {{
                // Brightness + live UI
                fetch('/getbrightness')
                    .then(r => r.text())
                    .then(b => {{
                        const numB = parseInt(b) || 0;
                        document.getElementById('brightness-status').innerText = 'Brightness: ' + b;
                        document.getElementById('brightness-status').style.color = numB > 0 ? '#0f0' : '#f00';
                        document.getElementById('slider').value = numB;
                    }});
                
                // Mode + toggle button
                fetch('/getmode')
                    .then(r => r.text())
                    .then(m => {{
                        modeEnabled = m === '1';
                        const modeDiv = document.getElementById('mode-status');
                        const toggleBtn = document.getElementById('mode-toggle');
                        
                        if (modeEnabled) {{
                            modeDiv.innerHTML = 'WillEZ Mode: <span style="color:#0f0">ON</span>';
                            toggleBtn.textContent = 'Disable WillEZ Mode';
                            toggleBtn.className = 'off';
                        }} else {{
                            modeDiv.innerHTML = 'WillEZ Mode: <span style="color:#f00">OFF</span>';
                            toggleBtn.textContent = 'Enable WillEZ Mode';
                            toggleBtn.className = 'on';
                        }}
                    }});
                
                // Current schedule times
                fetch('/gettimes')
                    .then(r => r.text())
                    .then(data => {{
                        const [on, off] = data.split(',');
                        document.getElementById('on-time').value = on;
                        document.getElementById('off-time').value = off;
                    }});
            }}
            
            // Initial load + live polling
            window.onload = function() {{
                updateAll();
                setInterval(updateAll, 20000);
            }};
        </script>
    </body>
    </html>
    """

if __name__ == '__main__':
    # Start WillEZ background worker
    worker_thread = threading.Thread(target=will_ez_worker, daemon=True)
    worker_thread.start()
    print("✅ WillEZ worker started — configurable auto mode (Central Time) ready")
    
    app.run(host='0.0.0.0', port=9041)
