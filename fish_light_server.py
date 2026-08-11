# fish_light_server.py
"""
Fish light control server.

API (query params, plain text responses — ESP-friendly):
  GET  /getbrightness          → "0".."255"
  GET|POST /set?value=0..255   → "OK"  (starts timed override if WillEZ is on)
  GET  /getmode                → "0" or "1"  (WillEZ auto schedule)
  GET|POST /setmode?value=0|1  → "OK"
  GET  /gettimes               → "HH:MM,HH:MM"
  GET|POST /settimes?on=HH:MM&off=HH:MM → "OK"
  GET  /getparty               → "0" or "1"
  GET|POST /setparty?value=0|1 → "OK"
  GET  /getoverride            → remaining override seconds, or "0"
  GET|POST /clearoverride      → "OK"  (resume schedule immediately)
  GET  /getstate               → "white,BRIGHTNESS" or "party,BRIGHTNESS"
  GET  /                       → control web UI
"""
from flask import Flask, request
import threading
import time
from datetime import datetime, timedelta
from zoneinfo import ZoneInfo

app = Flask(__name__)

TZ = ZoneInfo("America/Chicago")

brightness = 0
willez_mode = True
willez_on_time = "06:15"   # default Auto ON time (Central Time)
willez_off_time = "21:30"  # default Auto OFF time (Central Time)
party_mode = False
# When WillEZ is on and the user sets brightness manually, auto schedule
# is paused until this timestamp (Central Time). None = no override.
override_until = None
# How long a manual change pauses WillEZ (late feedings, etc.)
OVERRIDE_MINUTES = 60

_state_lock = threading.Lock()


def _now():
    return datetime.now(TZ)


def _parse_hhmm(s: str):
    return datetime.strptime(s, "%H:%M").time()


def _schedule_brightness(now=None):
    """Brightness the WillEZ schedule wants right now (0 or 255)."""
    now = now or _now()
    current_time = now.time()
    on_start = _parse_hhmm(willez_on_time)
    on_end = _parse_hhmm(willez_off_time)
    return 255 if on_start <= current_time < on_end else 0


def _override_remaining_seconds(now=None):
    global override_until
    now = now or _now()
    if override_until is None:
        return 0
    remaining = (override_until - now).total_seconds()
    if remaining <= 0:
        override_until = None
        return 0
    return int(remaining)


def _start_override(minutes=None):
    """Pause WillEZ auto-control for a temporary window."""
    global override_until
    mins = OVERRIDE_MINUTES if minutes is None else minutes
    override_until = _now() + timedelta(minutes=mins)


def _clear_override():
    global override_until
    override_until = None


def _apply_schedule_if_due(force=False):
    """Apply WillEZ brightness when not overridden / not in party mode."""
    global brightness
    if not willez_mode or party_mode:
        return False
    now = _now()
    if not force and _override_remaining_seconds(now) > 0:
        return False
    if force:
        _clear_override()
    target = _schedule_brightness(now)
    if target != brightness:
        brightness = target
        print(
            f"✅ WillEZ auto-set brightness to {brightness} at "
            f"{now.strftime('%H:%M:%S %Z')} (ON:{willez_on_time} OFF:{willez_off_time})"
        )
        return True
    return False


@app.route('/getbrightness')
def get_brightness():
    with _state_lock:
        return str(brightness)


@app.route('/set', methods=['GET', 'POST'])
def set_brightness():
    global brightness
    try:
        val = int(request.args.get('value') or request.form.get('value') or 0)
        if 0 <= val <= 255:
            with _state_lock:
                brightness = val
                # Manual brightness while WillEZ is on → temporary override
                # so the 30s worker does not immediately fight the user.
                if willez_mode and not party_mode:
                    _start_override()
                    rem = _override_remaining_seconds()
                    print(
                        f"🖐 Manual brightness {brightness}; "
                        f"WillEZ overridden for ~{rem // 60}m"
                    )
            return "OK"
    except Exception:
        pass
    return "ERROR", 400


@app.route('/getmode')
def get_mode():
    with _state_lock:
        return str(int(willez_mode))


@app.route('/setmode', methods=['GET', 'POST'])
def set_mode():
    global willez_mode
    try:
        val = int(request.args.get('value') or request.form.get('value') or 0)
        with _state_lock:
            willez_mode = bool(val)
            if willez_mode:
                # Re-enabling schedule: drop any stale override and snap to schedule
                _clear_override()
                if not party_mode:
                    _apply_schedule_if_due(force=True)
            else:
                _clear_override()
        return "OK"
    except Exception:
        pass
    return "ERROR", 400


@app.route('/gettimes')
def get_times():
    """Return current ON and OFF times as simple CSV: HH:MM,HH:MM"""
    with _state_lock:
        return f"{willez_on_time},{willez_off_time}"


@app.route('/settimes', methods=['GET', 'POST'])
def set_times():
    global willez_on_time, willez_off_time
    try:
        on = request.args.get('on') or request.form.get('on')
        off = request.args.get('off') or request.form.get('off')
        if on and off:
            _parse_hhmm(on)
            _parse_hhmm(off)
            with _state_lock:
                willez_on_time = on
                willez_off_time = off
                # If schedule is active and not overridden, apply immediately
                if willez_mode and not party_mode and _override_remaining_seconds() == 0:
                    _apply_schedule_if_due(force=True)
            return "OK"
    except Exception:
        pass
    return "ERROR", 400


@app.route('/getparty')
def get_party():
    with _state_lock:
        return str(int(party_mode))


@app.route('/setparty', methods=['GET', 'POST'])
def set_party():
    global party_mode, brightness
    try:
        val = int(request.args.get('value') or request.form.get('value') or 0)
        with _state_lock:
            party_mode = bool(val)
            if party_mode:
                # Ensure the strip is lit enough for the rainbow to show
                if brightness < 80:
                    brightness = 200
                print("🎉 Party mode ON")
            else:
                print("🎉 Party mode OFF — resuming normal control")
                if willez_mode and _override_remaining_seconds() == 0:
                    _apply_schedule_if_due(force=True)
        return "OK"
    except Exception:
        pass
    return "ERROR", 400


@app.route('/getoverride')
def get_override():
    with _state_lock:
        return str(_override_remaining_seconds())


@app.route('/clearoverride', methods=['GET', 'POST'])
def clear_override_route():
    with _state_lock:
        _clear_override()
        if willez_mode and not party_mode:
            _apply_schedule_if_due(force=True)
    return "OK"


@app.route('/getstate')
def get_state():
    """ESP-friendly combined state: mode,brightness  (mode = white|party)."""
    with _state_lock:
        mode = "party" if party_mode else "white"
        return f"{mode},{brightness}"


def will_ez_worker():
    """Background thread: when WillEZ mode is ON (and not overridden / party),
    automatically set brightness from the Central-Time schedule."""
    while True:
        try:
            with _state_lock:
                _apply_schedule_if_due(force=False)
        except Exception as e:
            print(f"WillEZ worker error: {e}")
        time.sleep(30)


@app.route('/')
def index():
    with _state_lock:
        b = brightness
        mode_on = willez_mode
        on_t = willez_on_time
        off_t = willez_off_time
        party = party_mode
        ov_secs = _override_remaining_seconds()

    mode_status = "ON" if mode_on else "OFF"
    mode_color = "#0f0" if mode_on else "#f00"
    party_status = "ON" if party else "OFF"
    party_color = "#f0f" if party else "#888"
    override_label = (
        f"Override active (~{ov_secs // 60}m {ov_secs % 60}s left)"
        if ov_secs > 0
        else "No override — schedule rules"
    )

    return f"""
    <!DOCTYPE html>
    <html>
    <head>
        <title>Fish Light</title>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <style>
            body {{font-family:Arial;background:#111;color:#eee;text-align:center;padding:40px;}}
            h1 {{color:#0f8;}}
            .status {{font-size:3em;margin:30px;color:{'#0f0' if b > 0 else '#f00'};}}
            .mode-status {{font-size:1.8em;margin:20px 0;}}
            .override {{font-size:1.1em;margin:10px 0;color:#fa0;}}
            .explanation {{background:#222;padding:20px;border-radius:15px;margin:40px auto 20px;max-width:620px;text-align:left;font-size:1.05em;line-height:1.5;}}
            button {{padding:14px 35px;font-size:1.3em;margin:10px;border:none;border-radius:10px;cursor:pointer;}}
            .on {{background:#0f0;color:black;}}
            .off {{background:#f00;color:white;}}
            .party {{background:linear-gradient(90deg,#f0f,#0ff,#ff0,#f0f);color:#111;font-weight:bold;}}
            input[type=range] {{width:320px;height:14px;margin:20px 0;}}
            input[type=time] {{font-size:1.2em;padding:8px 12px;border-radius:6px;border:1px solid #444;background:#222;color:#eee;}}
            .schedule {{margin:30px 0;background:#1a1a1a;padding:20px;border-radius:15px;max-width:620px;margin-left:auto;margin-right:auto;}}
        </style>
    </head>
    <body>
        <h1>🐟 Fish Light Control</h1>
        <div class="status" id="brightness-status">Brightness: {b}</div>

        <div class="mode-status" id="mode-status">
            WillEZ Mode: <span style="color:{mode_color}">{mode_status}</span>
        </div>
        <div class="mode-status" id="party-status">
            Party Mode: <span style="color:{party_color}">{party_status}</span>
        </div>
        <div class="override" id="override-status">{override_label}</div>

        <div class="schedule">
            <h3 style="margin-top:0;color:#0f8;">📅 WillEZ Schedule (Central Time)</h3>
            <label style="font-size:1.1em;">Auto ON at: </label>
            <input type="time" id="on-time" value="{on_t}">

            <label style="margin-left:30px;font-size:1.1em;">Auto OFF at: </label>
            <input type="time" id="off-time" value="{off_t}">

            <button onclick="saveSchedule()" style="margin-left:25px;font-size:1.1em;padding:10px 24px;">💾 Save Schedule</button>
        </div>

        <button class="on" onclick="set(255)">FULL ON</button>
        <button class="off" onclick="set(0)">OFF</button>
        <button class="party" id="party-toggle" onclick="toggleParty()">🎉 Toggle Party Mode</button>
        <br><br>

        <input type="range" min="0" max="255" value="{b}" id="slider" oninput="live(this.value)">
        <button onclick="set(document.getElementById('slider').value)">Apply</button>

        <br><br>
        <button id="mode-toggle" onclick="toggleMode()" style="font-size:1.3em;padding:12px 30px;">
            Toggle WillEZ Mode
        </button>
        <button onclick="clearOverride()" style="font-size:1.1em;padding:10px 20px;background:#555;color:#eee;">
            Resume Schedule Now
        </button>

        <div class="explanation">
            <strong>💡 WillEZ Mode:</strong><br>
            When enabled, the Fish Light automatically turns <strong>ON</strong> at your "Auto ON" time and
            <strong>OFF</strong> at "Auto OFF" — every day, Central Time (DST-aware).<br><br>
            <strong>Temporary override:</strong> Full ON / OFF / slider still work anytime. While WillEZ is on,
            a manual change pauses the schedule for <strong>{OVERRIDE_MINUTES} minutes</strong> (late feedings, etc.),
            then WillEZ resumes. Use <em>Resume Schedule Now</em> to end the override early.<br><br>
            <strong>🎉 Party Mode:</strong> Rainbow animation on the matrix (ESP firmware that supports
            <code>/getstate</code>). Disables schedule fighting until you turn party off.
        </div>

        <p style="margin-top:50px;color:#777;">Server @ lovelywill • WillEZ + party + timed override</p>

        <script>
            let modeEnabled = {str(mode_on).lower()};
            let partyEnabled = {str(party).lower()};

            function set(v) {{
                fetch('/set?value=' + v).then(() => updateAll());
            }}

            function live(v) {{
                document.getElementById('brightness-status').innerText = 'Brightness: ' + v;
            }}

            function toggleMode() {{
                const newVal = modeEnabled ? 0 : 1;
                fetch('/setmode?value=' + newVal).then(() => updateAll());
            }}

            function toggleParty() {{
                const newVal = partyEnabled ? 0 : 1;
                fetch('/setparty?value=' + newVal).then(() => updateAll());
            }}

            function clearOverride() {{
                fetch('/clearoverride').then(() => updateAll());
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

            function fmtOverride(secs) {{
                const n = parseInt(secs) || 0;
                if (n <= 0) return 'No override — schedule rules';
                const m = Math.floor(n / 60);
                const s = n % 60;
                return `Override active (~${{m}}m ${{s}}s left)`;
            }}

            function updateAll() {{
                fetch('/getbrightness')
                    .then(r => r.text())
                    .then(b => {{
                        const numB = parseInt(b) || 0;
                        document.getElementById('brightness-status').innerText = 'Brightness: ' + b;
                        document.getElementById('brightness-status').style.color = numB > 0 ? '#0f0' : '#f00';
                        document.getElementById('slider').value = numB;
                    }});

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

                fetch('/getparty')
                    .then(r => r.text())
                    .then(p => {{
                        partyEnabled = p === '1';
                        const el = document.getElementById('party-status');
                        if (partyEnabled) {{
                            el.innerHTML = 'Party Mode: <span style="color:#f0f">ON</span>';
                        }} else {{
                            el.innerHTML = 'Party Mode: <span style="color:#888">OFF</span>';
                        }}
                    }});

                fetch('/getoverride')
                    .then(r => r.text())
                    .then(s => {{
                        document.getElementById('override-status').innerText = fmtOverride(s);
                    }});

                fetch('/gettimes')
                    .then(r => r.text())
                    .then(data => {{
                        const [on, off] = data.split(',');
                        document.getElementById('on-time').value = on;
                        document.getElementById('off-time').value = off;
                    }});
            }}

            window.onload = function() {{
                updateAll();
                setInterval(updateAll, 15000);
            }};
        </script>
    </body>
    </html>
    """


if __name__ == '__main__':
    worker_thread = threading.Thread(target=will_ez_worker, daemon=True)
    worker_thread.start()
    print("✅ WillEZ worker started — schedule + timed override + party mode")
    app.run(host='0.0.0.0', port=9041)
