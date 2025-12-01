#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <Adafruit_VL53L0X.h>

const char* ssid     = "BrubakerWifi";
const char* password = "Pre$ton01";

#define HTTP_PORT  9067
#define WS_PORT    9068
#define ONBOARD_LED 2

WebServer        httpServer(HTTP_PORT);
WebSocketsServer webSocket(WS_PORT);

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

unsigned long lastReading = 0;
const unsigned long readingInterval = 50;
int currentDistanceMM = 9999;

void setup() {
  Serial.begin(115200);
  pinMode(ONBOARD_LED, OUTPUT);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  if (!lox.begin()) {
    Serial.println(F("VL53L0X not found!"));
    while (1);
  }

  httpServer.on("/distance", HTTP_GET, []() {
    httpServer.send(200, "text/plain", String(currentDistanceMM));
  });

  httpServer.on("/", HTTP_GET, []() {
    String html = "<h1>ESP32 Stair Sensor</h1>"
                  "<p>Distance: <b><span id='dist'>–</span></b> mm</p>"
                  "<script>"
                  "var ws = new WebSocket('ws://' + location.hostname + ':'" + String(WS_PORT) + "/');"
                  "ws.onmessage = e => document.getElementById('dist').innerText = e.data;"
                  "ws.onclose = () => document.getElementById('dist').innerText = 'disconnected';"
                  "</script>";
    httpServer.send(200, "text/html", html);
  });

  httpServer.begin();
  webSocket.begin();
  webSocket.onEvent([](uint8_t num, WStype_t type, uint8_t*, size_t) {
    if (type == WStype_CONNECTED) Serial.printf("WS client #%u connected\n", num);
  });

  Serial.printf("HTTP → http://%s:%d/\n", WiFi.localIP().toString().c_str(), HTTP_PORT);
  Serial.printf("WS   → ws://%s:%d/\n", WiFi.localIP().toString().c_str(), WS_PORT);
}

void broadcastDistance() {
  String msg = String(currentDistanceMM);  // ← THIS FIXES THE ERROR
  webSocket.broadcastTXT(msg);
}

void loop() {
  httpServer.handleClient();
  webSocket.loop();

  unsigned long now = millis();
  if (now - lastReading < readingInterval) return;
  lastReading = now;

  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false);
  currentDistanceMM = (measure.RangeStatus != 4) ? measure.RangeMilliMeter : 9999;

  if (currentDistanceMM < 1000) {
    int t = map(constrain(currentDistanceMM, 50, 1000), 50, 1000, 50, 400);
    digitalWrite(ONBOARD_LED, !digitalRead(ONBOARD_LED));
    delay(t);
  } else {
    digitalWrite(ONBOARD_LED, LOW);
  }

  broadcastDistance();
  Serial.println(currentDistanceMM);
}
