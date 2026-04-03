# fish_light_server.py
from flask import Flask, request
import threading
import time
from datetime import datetime
from zoneinfo import ZoneInfo

app = Flask(__name__)

brightness = 0  # global brightness 0-255
willez_mode = False  # WillEZ mode (auto on 6:15am–9:00pm CT)

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

def will_ez_worker():
    """Background thread: when WillEZ mode is ON, automatically set brightness
    based on Central Time (America/Chicago). Runs every 30 seconds."""
    global brightness
    tz = ZoneInfo("America/Chicago")
    while True:
        if willez_mode:
            try:
                now = datetime.now(tz)
                current_time = now.time()
                
                # 6:15 AM to 9:00 PM Central Time (off at exactly 9:00 PM)
                on_start = datetime.strptime("06:15", "%H:%M").time()
                on_end = datetime.strptime("21:00", "%H:%M").time()
                
                new_brightness = 255 if on_start <= current_time < on_end else 0
                
                if new_brightness != brightness:
                    brightness = new_brightness
                    print(f"WillEZ mode auto-set brightness to {brightness} at {now.strftime('%Y-%m-%d %H:%M:%S %Z')}")
            except Exception as e:
                print(f"Error in WillEZ worker: {e}")
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
            button {{padding:16px 40px;font-size:1.4em;margin:12px;border:none;border-radius:10px;cursor:pointer;}}
            .on {{background:#0f0;color:black;}} 
            .off {{background:#f00;color:white;}}
            input[type=range] {{width:320px;height:14px;margin:20px 0;}}
        </style>
    </head>
    <body>
        <h1>🐟 Fish Light Control</h1>
        <div class="status" id="brightness-status">Brightness: {brightness}</div>
        
        <div class="mode-status" id="mode-status">
            WillEZ Mode: <span style="color:{mode_color}">{mode_status}</span>
        </div>
        
        <button class="on" onclick="set(255)">FULL ON</button>
        <button class="off" onclick="set(0)">OFF</button><br><br>
        
        <input type="range" min="0" max="255" value="{brightness}" id="slider" oninput="live(this.value)">
        <button onclick="set(document.getElementById('slider').value)">Apply</button>
        
        <br><br>
        <button id="mode-toggle" onclick="toggleMode()" style="font-size:1.3em;padding:12px 30px;">
            Toggle WillEZ Mode
        </button>
        
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
                    .then(() => updateStatus());
            }}
            
            function updateStatus() {{
                // Update brightness
                fetch('/getbrightness')
                    .then(r => r.text())
                    .then(b => {{
                        const numB = parseInt(b);
                        document.getElementById('brightness-status').innerText = 'Brightness: ' + b;
                        document.getElementById('brightness-status').style.color = numB > 0 ? '#0f0' : '#f00';
                        document.getElementById('slider').value = b;
                    }});
                
                // Update mode
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
            }}
            
            // Initial update + live polling (every 20 seconds)
            window.onload = function() {{
                updateStatus();
                setInterval(updateStatus, 20000);
            }};
        </script>
    </body>
    </html>
    """

if __name__ == '__main__':
    # Start WillEZ background worker (daemon so it shuts down with the server)
    worker_thread = threading.Thread(target=will_ez_worker, daemon=True)
    worker_thread.start()
    print("✅ WillEZ worker thread started (checks Central Time every 30s)")
    
    app.run(host='0.0.0.0', port=9041)
