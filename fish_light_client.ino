// led_control.ino
// ESP8266 + 32x8 NeoMatrix (full white)
// Reports state to desktop server 192.168.1.198:9041
// Polls server for commands (ON / OFF / BRI:xxx)

#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

// ────────────────────────────────────────────────
//               CONFIGURATION
// ────────────────────────────────────────────────
const char* ssid     = "BrubakerWifi";
const char* password = "Pre$ton01";

const char* serverHost = "192.168.1.198";
const uint16_t serverPort = 9041;

#define LED_PIN 2

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(32, 8, LED_PIN,
  NEO_MATRIX_BOTTOM + NEO_MATRIX_RIGHT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
  NEO_GRB + NEO_KHZ800);

uint16_t WHITE = matrix.Color(255, 255, 255);

bool isOn = false;
uint8_t brightness = 0;           // 0–255

unsigned long lastReport = 0;
const unsigned long reportInterval = 5000;   // report state every 5 s

unsigned long lastPoll = 0;
const unsigned long pollInterval = 1000;     // ask server for commands every 1 s
// ────────────────────────────────────────────────

void updateLEDs() {
  if (isOn && brightness > 0) {
    matrix.setBrightness(brightness);
    matrix.fillScreen(WHITE);
  } else {
    matrix.setBrightness(0);
    matrix.fillScreen(0);
  }
  matrix.show();
}

void reportState() {
  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClient client;
  HTTPClient http;

  String url = "http://" + String(serverHost) + ":" + String(serverPort) + "/report";
  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");

  String json = "{"
    "\"on\":" + String(isOn ? "true" : "false") + ","
    "\"brightness\":" + String(brightness) +
  "}";

  http.POST(json);
  http.end();
}

void checkForCommands() {
  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClient client;
  HTTPClient http;

  String url = "http://" + String(serverHost) + ":" + String(serverPort) + "/command";
  http.begin(client, url);

  int code = http.GET();
  if (code == HTTP_CODE_OK) {
    String payload = http.getString();
    payload.trim();

    if (payload == "ON") {
      isOn = true;
      if (brightness == 0) brightness = 255;
      updateLEDs();
    }
    else if (payload == "OFF") {
      isOn = false;
      updateLEDs();
    }
    else if (payload.startsWith("BRI:")) {
      String valStr = payload.substring(4);
      int val = valStr.toInt();
      if (val >= 0 && val <= 255) {
        brightness = (uint8_t)val;
        isOn = (brightness > 0);
        updateLEDs();
      }
    }
  }

  http.end();
}

void setup() {
  Serial.begin(115200);
  delay(100);

  matrix.begin();
  matrix.setRotation(4);
  matrix.setBrightness(0);
  matrix.clear();
  matrix.show();

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ");
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  Serial.print("ESP IP: "); Serial.println(WiFi.localIP());

  updateLEDs();  // start off
}

void loop() {
  unsigned long now = millis();

  // Poll server for new commands
  if (now - lastPoll >= pollInterval) {
    checkForCommands();
    lastPoll = now;
  }

  // Report current state periodically
  if (now - lastReport >= reportInterval) {
    reportState();
    lastReport = now;
  }

  delay(20);  // small breathing room
}
