// fish_light_client.ino
// ESP8266 NeoMatrix fish light
// Polls http://lovelywill.immenseaccumulationonline.online/getstate
// Response: "white,BRIGHTNESS" or "party,BRIGHTNESS"  (BRIGHTNESS = 0-255)
// Falls back to /getbrightness (plain number) if /getstate is unavailable.
// DO NOT USE HTTPS (HTTP only)
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

// ================== YOUR SETTINGS ==================
const char* ssid = "BrubakerWifi2";
const char* password = "Pre$ton01";
const char* serverHost = "lovelywill.immenseaccumulationonline.online";
const int serverPort = 80; // HTTP only
// ===================================================

#define PIN 2
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(32, 8, PIN,
                                               NEO_MATRIX_BOTTOM + NEO_MATRIX_RIGHT +
                                                   NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
                                               NEO_GRB + NEO_KHZ800);

uint8_t brightness = 0;
bool partyMode = false;
unsigned long lastPoll = 0;
const unsigned long pollInterval = 900; // poll every 0.9s
unsigned long lastPartyFrame = 0;
const unsigned long partyFrameMs = 40;  // ~25 fps rainbow
uint16_t partyHue = 0;                  // 0-65535 style wheel via 0-255 step

void setup() {
  Serial.begin(115200);
  Serial.println("\n\n=== FISH LIGHT STARTING ===");
  matrix.begin();
  matrix.setRotation(4);
  matrix.setBrightness(0);
  matrix.clear();
  matrix.show();

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("ESP IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("Polling server for state...");
  updateWhite();
}

void loop() {
  unsigned long now = millis();

  if (now - lastPoll >= pollInterval) {
    lastPoll = now;
    pollForState();
  }

  if (partyMode && brightness > 0) {
    if (now - lastPartyFrame >= partyFrameMs) {
      lastPartyFrame = now;
      drawPartyFrame();
    }
  }

  delay(5);
}

void pollForState() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi lost — reconnecting...");
    return;
  }

  WiFiClient client;
  HTTPClient http;

  String url = "http://" + String(serverHost) + ":" + String(serverPort) + "/getstate";
  Serial.print("Polling: ");
  Serial.println(url);

  http.begin(client, url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    payload.trim();
    payload.toLowerCase();
    Serial.print("State: '");
    Serial.print(payload);
    Serial.println("'");
    applyStatePayload(payload);
  } else {
    // Backward-compatible fallback: plain brightness number
    Serial.print("getstate HTTP ");
    Serial.print(httpCode);
    Serial.println(" — falling back to getbrightness");
    http.end();
    pollBrightnessOnly();
    return;
  }
  http.end();
}

void pollBrightnessOnly() {
  WiFiClient client;
  HTTPClient http;
  String url = "http://" + String(serverHost) + ":" + String(serverPort) + "/getbrightness";
  http.begin(client, url);
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    payload.trim();
    int newBri = payload.toInt();
    if (newBri >= 0 && newBri <= 255) {
      bool briChanged = (newBri != brightness);
      brightness = (uint8_t)newBri;
      if (partyMode) {
        partyMode = false;
        updateWhite();
      } else if (briChanged) {
        updateWhite();
      }
    }
  }
  http.end();
}

void applyStatePayload(const String& payload) {
  // Formats:
  //   "white,200"
  //   "party,255"
  //   "200" (fallback plain brightness)
  int comma = payload.indexOf(',');
  bool newParty = false;
  int newBri = 0;

  if (comma > 0) {
    String mode = payload.substring(0, comma);
    mode.trim();
    newBri = payload.substring(comma + 1).toInt();
    newParty = (mode == "party" || mode == "1" || mode == "rainbow");
  } else {
    newBri = payload.toInt();
    newParty = false;
  }

  if (newBri < 0) newBri = 0;
  if (newBri > 255) newBri = 255;

  bool modeChanged = (newParty != partyMode);
  bool briChanged = ((uint8_t)newBri != brightness);

  partyMode = newParty;
  brightness = (uint8_t)newBri;

  if (!partyMode && (modeChanged || briChanged)) {
    updateWhite();
  } else if (partyMode && modeChanged) {
    Serial.println("→ Party mode ON");
    // first frame immediately
    drawPartyFrame();
  } else if (partyMode && briChanged) {
    matrix.setBrightness(brightness);
  }
}

void updateWhite() {
  if (brightness > 0) {
    matrix.setBrightness(brightness);
    matrix.fillScreen(matrix.Color(255, 255, 255));
  } else {
    matrix.setBrightness(0);
    matrix.fillScreen(0);
  }
  matrix.show();
}

// HSV wheel → RGB for NeoPixel-style rainbow (h = 0..255)
uint32_t colorWheel(uint8_t pos) {
  pos = 255 - pos;
  if (pos < 85) {
    return matrix.Color(255 - pos * 3, 0, pos * 3);
  }
  if (pos < 170) {
    pos -= 85;
    return matrix.Color(0, pos * 3, 255 - pos * 3);
  }
  pos -= 170;
  return matrix.Color(pos * 3, 255 - pos * 3, 0);
}

void drawPartyFrame() {
  matrix.setBrightness(brightness > 0 ? brightness : 180);
  // Flowing rainbow across the 32x8 matrix
  for (int16_t x = 0; x < matrix.width(); x++) {
    for (int16_t y = 0; y < matrix.height(); y++) {
      uint8_t hue = (uint8_t)(partyHue + x * 8 + y * 4);
      matrix.drawPixel(x, y, colorWheel(hue));
    }
  }
  matrix.show();
  partyHue += 3; // speed of rainbow crawl
}
