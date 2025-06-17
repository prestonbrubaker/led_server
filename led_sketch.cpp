#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Adafruit_NeoPixel.h>

// Wi-Fi credentials
const char* ssid = "BrubakerWifi";
const char* password = "Pre$ton01";

// Flask server URL
const char* serverUrl = "http://192.168.1.126:5000/mode";

// LED strip configuration
#define NUM_LEDS 300
#define DATA_PIN 2 // GPIO2
#define BRIGHTNESS 50 // 0-255, lowered to reduce power draw
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, DATA_PIN, NEO_GRB + NEO_KHZ800);

// Current mode
String currentMode = "off";
unsigned long lastPoll = 0;
const unsigned long pollInterval = 2000; // Poll every 2 seconds

void setup() {
  // Initialize Serial
  Serial.begin(115200);
  delay(100);

  // Initialize LED strip
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  setLedsOff(); // Start with LEDs off
  strip.show();

  // Connect to Wi-Fi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println("IP address: " + WiFi.localIP().toString());
}

void loop() {
  // Check Wi-Fi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, reconnecting...");
    WiFi.reconnect();
    delay(2000);
    return;
  }

  // Poll server periodically
  if (millis() - lastPoll >= pollInterval) {
    String newMode = getModeFromServer();
    if (newMode != "" && newMode != currentMode) {
      currentMode = newMode;
      Serial.println("New mode: " + currentMode);
    }
    lastPoll = millis();
  }

  // Update LED pattern based on mode
  if (currentMode == "rainbow-flow") {
    rainbowFlow();
  } else if (currentMode == "constant-red") {
    setLedsRed();
  } else {
    setLedsOff();
  }
  strip.show();
}

String getModeFromServer() {
  WiFiClient client;
  HTTPClient http;
  String mode = "";

  Serial.print("Attempting HTTP GET to: ");
  Serial.println(serverUrl);
  if (http.begin(client, serverUrl)) {
    int httpCode = http.GET();
    Serial.print("HTTP Code: ");
    Serial.println(httpCode);
    if (httpCode == HTTP_CODE_OK) {
      mode = http.getString();
      mode.trim(); // Remove whitespace
      Serial.println("Received mode: " + mode);
    } else {
      Serial.printf("HTTP GET failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    Serial.println("Unable to connect to server");
  }
  return mode;
}

void rainbowFlow() {
  static uint16_t hue = 0;
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.ColorHSV(hue + (i * 65536L / NUM_LEDS)));
  }
  hue += 256; // Shift hue for flowing effect
  delay(20); // ~50 FPS
}

void setLedsRed() {
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(255, 0, 0));
  }
}

void setLedsOff() {
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
  }
}
