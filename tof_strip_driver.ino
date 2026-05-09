/******************* ESP-01 — Smart Stair Light with 20s Hold + Rainbow Color *******************/
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_NeoPixel.h>

const char* ssid = "BrubakerWifi2";
const char* password = "Pre$ton01";

// NEW: Central server via Cloudflare Tunnel (your desktop)
const char* sensorUrl = "http://willohwilloh.immenseaccumulationonline.online/distance";

#define NUM_LEDS 300
#define DATA_PIN 2

Adafruit_NeoPixel strip(NUM_LEDS, DATA_PIN, NEO_GRB + NEO_KHZ800);

WiFiClient client;
HTTPClient http;

unsigned long lastCheck = 0;
const unsigned long checkInterval = 250; // check 4× per second

unsigned long lastTriggerTime = 0;
const unsigned long holdTimeMs = 20000; // 20 seconds keep-alive

int currentDistance = 9999; // last valid reading

void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.setBrightness(140); // nice and bright but safe
  strip.clear();
  strip.show();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());
}

void loop() {
  if (millis() - lastCheck < checkInterval) return;
  lastCheck = millis();

  int dist = getDistance();

  if (dist > 0 && dist < 900) {
    // Person detected — update color immediately and reset timer
    currentDistance = dist;
    lastTriggerTime = millis();
    setRainbowColor(dist);
    Serial.printf("TRIP → %d mm → ", dist);
    printColorName(dist);
  }
  else {
    // No one close — check if we are still in the 20-second hold window
    if (millis() - lastTriggerTime < holdTimeMs) {
      // Still in grace period → keep last color
      if (dist > 0) currentDistance = dist; // update if we got a reading
      setRainbowColor(currentDistance);
    }
    else {
      // 20+ seconds passed → turn off
      if (strip.getPixelColor(0) != 0) { // only print once
        Serial.println("Timeout → LEDs OFF");
      }
      strip.clear();
      strip.show();
    }
  }
}

// Returns distance in mm or -1 on error
// Uses split host/path form (much more reliable on ESP-01/ESP8266)
int getDistance() {
  if (WiFi.status() != WL_CONNECTED) return -1;

  const char* host = "willohwilloh.immenseaccumulationonline.online";
  const char* path = "/distance";

  for (int attempt = 0; attempt < 2; attempt++) {   // one automatic retry
    if (!http.begin(client, host, 80, path)) {
      Serial.println("http.begin() failed");
      delay(100);
      continue;
    }

    http.setTimeout(3000);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    int code = http.GET();
    int dist = -1;

    if (code == HTTP_CODE_OK) {
      String payload = http.getString();
      payload.trim();
      dist = payload.toInt();
      if (dist == 0 && payload != "0") dist = -1; // guard junk data
      http.end();
      return dist;
    } else if (code > 0) {
      Serial.printf("HTTP Error %d\n", code);
    } else {
      Serial.printf("Connection failed (attempt %d)\n", attempt + 1);
    }
    http.end();
    delay(150);
  }
  return -1;
}

// Map 0…900 mm → 0…360° hue (red → purple)
void setRainbowColor(int mm) {
  uint16_t hue = map(constrain(mm, 0, 900), 0, 900, 0, 360);
  hue = (hue * 300L) / 360; // compress so 900 mm = nice purple
  uint32_t color = strip.ColorHSV(hue * 182, 255, 255); // 182 ≈ 65536/360
  strip.fill(color);
  strip.show();
}

void printColorName(int mm) {
  int h = map(constrain(mm, 0, 900), 0, 900, 0, 300);
  if (h < 30) Serial.println("RED");
  else if (h < 80) Serial.println("ORANGE → YELLOW");
  else if (h < 150) Serial.println("GREEN → CYAN");
  else if (h < 220) Serial.println("BLUE");
  else Serial.println("PURPLE");
}
