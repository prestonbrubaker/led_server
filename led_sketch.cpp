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

// Timing for non-blocking animations
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 20; // ~50 FPS

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
      setLedsOff(); // Clear strip on mode change
    }
    lastPoll = millis();
  }

  // Update LED pattern based on mode
  if (millis() - lastUpdate >= updateInterval) {
    if (currentMode == "rainbow-flow") {
      rainbowFlow();
    } else if (currentMode == "constant-red") {
      setLedsRed();
    } else if (currentMode == "off") {
      setLedsOff();
    } else if (currentMode == "proletariat-crackle") {
      proletariatCrackle();
    } else if (currentMode == "bourgeois-brilliance") {
      bourgeoisBrilliance();
    } else if (currentMode == "austere-enlightenment") {
      austereEnlightenment();
    } else if (currentMode == "zaphod-galactic-groove") {
      zaphodGalacticGroove();
    } else if (currentMode == "max-aquarian-flow") {
      maxAquarianFlow();
    } else if (currentMode == "lunar-rebellion-pulse") {
      lunarRebellionPulse();
    } else if (currentMode == "proletariat-pulse") {
      proletariatPulse();
    } else if (currentMode == "bourgeois-blaze") {
      bourgeoisBlaze();
    }
    strip.show();
    lastUpdate = millis();
  }
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

void proletariatCrackle() {
  static uint8_t intensities[NUM_LEDS] = {0};
  static unsigned long lastCrackle = 0;

  if (millis() - lastCrackle >= 50) {
    for (int i = 0; i < NUM_LEDS; i++) {
      intensities[i] = max(0, intensities[i] - 10); // Fade
      uint8_t r = min(255, 200 + intensities[i]);
      uint8_t g = intensities[i] / 8; // Subtle orange flicker
      strip.setPixelColor(i, strip.Color(r, g, 0));
    }
    for (int i = 0; i < 5; i++) { // Add 5 sparkles
      int led = random(NUM_LEDS);
      intensities[led] = random(50, 255); // Random brightness
    }
    lastCrackle = millis();
  }
}

void bourgeoisBrilliance() {
  static uint8_t clusters[NUM_LEDS] = {0};
  static int pos = 0;
  static unsigned long lastCascade = 0;

  if (millis() - lastCascade >= 30) {
    for (int i = 0; i < NUM_LEDS; i++) {
      clusters[i] = max(0, clusters[i] - 15); // Fade
      uint8_t r = clusters[i] * (i % 3 == 0); // Gold
      uint8_t g = clusters[i] * (i % 3 == 0) * 215 / 255;
      uint8_t b = clusters[i] * (i % 3 == 1) * 180 / 255; // Sapphire
      if (i % 3 == 2) { // White
        r = g = b = clusters[i];
      }
      strip.setPixelColor(i, strip.Color(r, g, b));
    }
    clusters[pos] = 255; // New cluster
    pos = (pos + 1) % NUM_LEDS; // Cascade down
    lastCascade = millis();
  }
}

void austereEnlightenment() {
  static uint8_t blades[NUM_LEDS] = {0};
  static int offset = 0;
  static unsigned long lastSway = 0;

  if (millis() - lastSway >= 40) {
    for (int i = 0; i < NUM_LEDS; i++) {
      blades[i] = max(0, blades[i] - 10); // Fade
      uint8_t r = blades[i] * (i % 3 == 1) * 255 / 255; // Yellow
      uint8_t g = blades[i] * (i % 3 == 0); // Green
      uint8_t b = blades[i] * (i % 3 == 2) * 238 / 255; // Violet
      strip.setPixelColor(i, strip.Color(r, g, b));
    }
    int idx = (offset + random(NUM_LEDS)) % NUM_LEDS;
    blades[idx] = random(100, 255); // New blade
    offset = (offset + 1) % NUM_LEDS;
    lastSway = millis();
  }
}

void zaphodGalacticGroove() {
  static uint16_t hue = 0;
  static uint8_t stars[NUM_LEDS] = {0};
  static unsigned long lastSwirl = 0;

  if (millis() - lastSwirl >= 25) {
    for (int i = 0; i < NUM_LEDS; i++) {
      stars[i] = max(0, stars[i] - 20); // Fade stars
      uint16_t h = hue + (i * 65536L / NUM_LEDS);
      uint8_t r, g, b;
      if (stars[i] > 0) { // White star
        r = g = b = stars[i];
      } else { // Galaxy swirl
        uint32_t c = strip.ColorHSV(h);
        r = (c >> 16) & 0xFF;
        g = (c >> 8) & 0xFF;
        b = c & 0xFF;
      }
      strip.setPixelColor(i, strip.Color(r, g, b));
    }
    if (random(100) < 5) { // Add star
      stars[random(NUM_LEDS)] = 255;
    }
    hue += random(100, 200); // Random swirl direction
    lastSwirl = millis();
  }
}

void maxAquarianFlow() {
  static uint8_t waves[NUM_LEDS] = {0};
  static int direction = 1; // 1 = right, -1 = left
  static int pos = 0;
  static unsigned long lastWave = 0;

  if (millis() - lastWave >= 30) {
    for (int i = 0; i < NUM_LEDS; i++) {
      waves[i] = max(0, waves[i] - 10); // Fade
      uint8_t g = waves[i] * (i % 2); // Aquamarine
      uint8_t b = waves[i]; // Blue or white
      uint8_t r = waves[i] * (i % 2 == 0); // White
      strip.setPixelColor(i, strip.Color(r, g, b));
    }
    waves[pos] = random(150, 255); // New wave peak
    pos += direction;
    if (pos >= NUM_LEDS - 1 || pos <= 0) { // Reverse at ends
      direction = -direction;
    }
    lastWave = millis();
  }
}

void lunarRebellionPulse() {
  static uint8_t pulses[NUM_LEDS] = {0};
  static int left = 0, right = NUM_LEDS - 1;
  static bool converging = true;
  static unsigned long lastPulse = 0;

  if (millis() - lastPulse >= 35) {
    for (int i = 0; i < NUM_LEDS; i++) {
      pulses[i] = max(0, pulses[i] - 15); // Fade
      uint8_t r = pulses[i] * (i % 2); // Silver
      uint8_t g = pulses[i]; // Cyan
      uint8_t b = pulses[i];
      strip.setPixelColor(i, strip.Color(r, g, b));
    }
    if (converging) {
      pulses[left] = pulses[right] = 255; // New pulses
      left++;
      right--;
      if (left >= right) { // Switch to scatter
        converging = false;
        left = right = NUM_LEDS / 2;
      }
    } else {
      pulses[left] = pulses[right] = 255; // Scatter outward
      left--;
      right++;
      if (left < 0 || right >= NUM_LEDS) { // Reset
        converging = true;
        left = 0;
        right = NUM_LEDS - 1;
      }
    }
    if (random(100) < 3) { // Lunar dust
      pulses[random(NUM_LEDS)] = random(50, 100);
    }
    lastPulse = millis();
  }
}

void proletariatPulse() {
  static uint8_t ripples[NUM_LEDS] = {0};
  static int wavePos = 0;
  static int direction = 1; // 1 = right, -1 = left
  static unsigned long lastRipple = 0;

  if (millis() - lastRipple >= 25) {
    // Fade all LEDs
    for (int i = 0; i < NUM_LEDS; i++) {
      ripples[i] = max(0, ripples[i] - 10); // Smooth fade
      strip.setPixelColor(i, strip.Color(ripples[i], 0, 0)); // Red only
    }
    // Add new ripple
    ripples[wavePos] = random(150, 255); // Bright ripple peak
    // Add random sparkles
    if (random(100) < 10) { // 10% chance per frame
      int spark = random(NUM_LEDS);
      ripples[spark] = random(200, 255); // Bright sparkle
    }
    // Move ripple
    wavePos += direction;
    if (wavePos >= NUM_LEDS - 1 || wavePos <= 0) {
      direction = -direction; // Reverse at ends
    }
    lastRipple = millis();
  }
}

void bourgeoisBlaze() {
  static uint8_t clusters[NUM_LEDS] = {0};
  static int pos = 0;
  static unsigned long lastWave = 0;

  if (millis() - lastWave >= 30) {
    // Fade all LEDs
    for (int i = 0; i < NUM_LEDS; i++) {
      clusters[i] = max(0, clusters[i] - 12); // Smooth fade
      uint8_t r = clusters[i] * (i % 3 == 0); // Gold
      uint8_t g = clusters[i] * (i % 3 == 0) * 215 / 255; // Gold
      uint8_t b = clusters[i] * (i % 3 == 1) * 180 / 255; // Emerald
      if (i % 3 == 2) { // Crimson
        r = clusters[i];
        g = b = clusters[i] / 4; // Subtle red dominance
      }
      strip.setPixelColor(i, strip.Color(r, g, b));
    }
    // Add new cluster
    clusters[pos] = 255;
    clusters[(pos + NUM_LEDS / 2) % NUM_LEDS] = 255; // Opposite side for symmetry
    // Add random sparkles
    if (random(100) < 8) { // 8% chance per frame
      int spark = random(NUM_LEDS);
      clusters[spark] = random(180, 255); // Bright sparkle
    }
    pos = (pos + 1) % NUM_LEDS; // Move cluster
    lastWave = millis();
  }
}
