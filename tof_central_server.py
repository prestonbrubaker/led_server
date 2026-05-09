#!/usr/bin/env python3
"""
Central Server for Time-of-Flight Stair Sensor
Runs on desktop (localhost:9067)
Exposed via Cloudflare Tunnel as http://willohwilloh.immenseaccumulationonline.online

- ESP32 (sensor) POSTs distance updates to /update
- ESP-01 (LED strip) GETs current distance from /distance
- Browser can view live distance at /
"""

import http.server
import socketserver
import urllib.parse

PORT = 9067
current_distance = 9999

class ThreadedTCPServer(socketserver.ThreadingMixIn, socketserver.TCPServer):
    daemon_threads = True
    allow_reuse_address = True

class Handler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/distance':
            self.send_response(200)
            self.send_header('Content-type', 'text/plain')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(str(current_distance).encode('utf-8'))

        elif self.path in ('/', '/index.html'):
            self.send_response(200)
            self.send_header('Content-type', 'text/html')
            self.end_headers()
            html = """<!DOCTYPE html>
<html>
<head>
    <title>Stair Sensor - Central</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: system-ui, sans-serif; text-align: center; padding: 40px; background: #111; color: #eee; }
        h1 { color: #0af; }
        #dist { font-size: 4em; font-weight: bold; color: #0f0; }
        .unit { font-size: 1.5em; color: #888; }
    </style>
</head>
<body>
    <h1>🏠 Stair Sensor (Central Server)</h1>
    <p>Current Distance</p>
    <div style="font-size: 6em; margin: 20px 0;">
        <span id="dist">–</span><span class="unit"> mm</span>
    </div>
    <p style="color:#666; font-size:0.9em;">Updates every 250ms • Powered by ESP32 + Cloudflare Tunnel</p>

    <script>
        function updateDistance() {
            fetch('/distance')
                .then(r => r.text())
                .then(d => {
                    document.getElementById('dist').innerText = d;
                })
                .catch(() => {
                    document.getElementById('dist').innerText = 'ERR';
                });
        }
        setInterval(updateDistance, 250);
        updateDistance(); // initial
    </script>
</body>
</html>"""
            self.wfile.write(html.encode('utf-8'))

        else:
            self.send_response(404)
            self.end_headers()
            self.wfile.write(b'Not Found')

    def do_POST(self):
        if self.path == '/update':
            try:
                content_length = int(self.headers.get('Content-Length', 0))
                post_data = self.rfile.read(content_length).decode('utf-8')
                params = urllib.parse.parse_qs(post_data)

                if 'distance' in params:
                    dist = int(params['distance'][0])
                    global current_distance
                    current_distance = max(0, min(9999, dist))  # clamp
                    self.send_response(200)
                    self.send_header('Content-type', 'text/plain')
                    self.end_headers()
                    self.wfile.write(b'OK')
                    return
            except Exception as e:
                print(f"POST error: {e}")

            self.send_response(400)
            self.end_headers()
            self.wfile.write(b'Bad Request')
        else:
            self.send_response(404)
            self.end_headers()
            self.wfile.write(b'Not Found')

    def log_message(self, format, *args):
        # Quiet logging (only errors)
        if args and '200' not in str(args[0]) and '400' not in str(args[0]):
            print(f"[{self.client_address[0]}] {format % args}")

if __name__ == '__main__':
    print("=" * 60)
    print("🚀 Central Time-of-Flight Server")
    print(f"   Listening on http://0.0.0.0:{PORT}")
    print(f"   Public URL:  http://willohwilloh.immenseaccumulationonline.online")
    print("=" * 60)
    print("Waiting for ESP32 sensor reports and ESP-01 LED driver...")

    with ThreadedTCPServer(("", PORT), Handler) as httpd:
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nShutting down server...")
