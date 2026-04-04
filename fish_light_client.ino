// led_control.ino
// SIMPLE & RELIABLE VERSION — this one WILL work
// ESP polls http://lovelywill.immenseaccumulationonline.online/ every ~0.9 seconds for the brightness level
// Returns a plain number (0-255) — no JSON, no commands, nothing complicated
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
const char* serverHost = "lovelywill.immenseaccumulationonline.online"; // ← your new domain
const int serverPort = 80;                                            // HTTP port (do NOT use 443)
// ===================================================

#define PIN 2
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(32, 8, PIN,
                                               NEO_MATRIX_BOTTOM + NEO_MATRIX_RIGHT +
                                                   NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
                                               NEO_GRB + NEO_KHZ800);

uint16_t WHITE = matrix.Color(255, 255, 255);
uint8_t brightness = 0; // current brightness 0-255
unsigned long lastPoll = 0;
const unsigned long pollInterval = 900; // poll every 0.9 seconds

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
  Serial.println("Now polling lovelywill.immenseaccumulationonline.online for brightness...");
  updateLEDs(); // start off
}

void loop() {
  if (millis() - lastPoll >= pollInterval) {
    lastPoll = millis();
    pollForBrightness();
  }
  delay(10);
}

void pollForBrightness() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi lost — reconnecting...");
    return;
  }

  WiFiClient client;
  HTTPClient http;

  // HTTP only — no HTTPS
  String url = "http://" + String(serverHost) + ":" + String(serverPort) + "/getbrightness";
  Serial.print("Polling: ");
  Serial.println(url);

  http.begin(client, url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    payload.trim();
    Serial.print("Server returned: '");
    Serial.print(payload);
    Serial.println("'");

    int newBri = payload.toInt();
    if (newBri >= 0 && newBri <= 255) {
      if (newBri != brightness) {
        brightness = newBri;
        Serial.print("→ Setting brightness to ");
        Serial.println(brightness);
        updateLEDs();
      }
    } else {
      Serial.println("Invalid number from server");
    }
  } else {
    Serial.print("HTTP error: ");
    Serial.println(httpCode);
  }
  http.end();
}

void updateLEDs() {
  if (brightness > 0) {
    matrix.setBrightness(brightness);
    matrix.fillScreen(WHITE);
  } else {
    matrix.setBrightness(0);
    matrix.fillScreen(0);
  }
  matrix.show();
}
