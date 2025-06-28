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
#define BRIGHTNESS 50 // 0-255
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, DATA_PIN, NEO_GRB + NEO_KHZ800);

// Current mode and timing
String currentMode = "off";
unsigned long lastPoll = 0;
const unsigned long pollInterval = 2000; // Poll every 2 seconds
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 30; // ~33 FPS

void setup() {
    // Initialize Serial
    Serial.begin(115200);
    delay(100);

    // Initialize LED strip
    strip.begin();
    strip.setBrightness(BRIGHTNESS);
    setLedsOff();
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

    // Poll server for mode updates
    if (millis() - lastPoll >= pollInterval) {
        String newMode = getModeFromServer();
        if (newMode != "" && newMode != currentMode) {
            currentMode = newMode;
            Serial.println("New mode: " + currentMode);
            setLedsOff();
            resetModeState(); // Reset mode-specific state
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
    http.setTimeout(5000); // 5s timeout
    int retries = 3;

    Serial.print("Attempting HTTP GET to: ");
    Serial.println(serverUrl);
    if (http.begin(client, serverUrl)) {
        while (retries > 0) {
            int httpCode = http.GET();
            Serial.print("HTTP Code: ");
            Serial.println(httpCode);
            if (httpCode == HTTP_CODE_OK) {
                mode = http.getString();
                mode.trim();
                Serial.println("Received mode: " + mode);
                break;
            } else {
                Serial.printf("HTTP GET failed, error: %s\n", http.errorToString(httpCode).c_str());
                retries--;
                delay(1000);
            }
        }
        http.end();
    } else {
        Serial.println("Unable to connect to server");
    }
    return mode;
}

void resetModeState() {
    // Reset mode-specific static variables to avoid glitches
    static uint8_t dummyBuffer[NUM_LEDS] = {0};
    memcpy(dummyBuffer, dummyBuffer, NUM_LEDS); // Clear buffer
}

void rainbowFlow() {
    static uint16_t hue = 0;
    for (int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.ColorHSV(hue + (i * 65536L / NUM_LEDS)));
    }
    hue += 256;
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
            intensities[i] = max(0, intensities[i] - 10);
            uint8_t r = min(255, 200 + intensities[i]);
            uint8_t g = intensities[i] / 8;
            strip.setPixelColor(i, strip.Color(r, g, 0));
        }
        for (int i = 0; i < 5; i++) {
            int led = random(NUM_LEDS);
            intensities[led] = random(50, 255);
        }
        lastCrackle = millis();
    }
}

void bourgeoisBrilliance() {
    static uint8_t clusters[NUM_LEDS] = {0};
    static uint8_t intensities[NUM_LEDS] = {0};
    static int pos = 0;
    static unsigned long lastCascade = 0;

    if (millis() - lastCascade >= 25) {
        for (int i = 0; i < NUM_LEDS; i++) {
            clusters[i] = max(0, clusters[i] - 10); // Fade clusters
            intensities[i] = max(0, intensities[i] - 15); // Fade sparkles
            uint8_t r = clusters[i] * (i % 3 == 0); // Gold
            uint8_t g = clusters[i] * (i % 3 == 0) * 215 / 255;
            uint8_t b = clusters[i] * (i % 3 == 1) * 180 / 255; // Sapphire
            if (i % 3 == 2) { // White diamond sparkle
                r = g = b = intensities[i];
            } else if (intensities[i] > clusters[i]) {
                r = intensities[i]; // Warm white sparkle overlay
                g = intensities[i] * 240 / 255;
                b = intensities[i] * 200 / 255;
            }
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        // Add new clusters
        clusters[pos] = 255;
        clusters[(pos + NUM_LEDS / 3) % NUM_LEDS] = 200; // Secondary cluster
        clusters[(pos + 2 * NUM_LEDS / 3) % NUM_LEDS] = 200; // Tertiary cluster
        // Add random sparkles
        if (random(100) < 15) {
            int spark = random(NUM_LEDS);
            intensities[spark] = random(180, 255);
        }
        pos = (pos + 1) % NUM_LEDS;
        lastCascade = millis();
    }
}

void austereEnlightenment() {
    static uint8_t blades[NUM_LEDS] = {0};
    static int offset = 0;
    static unsigned long lastSway = 0;

    if (millis() - lastSway >= 40) {
        for (int i = 0; i < NUM_LEDS; i++) {
            blades[i] = max(0, blades[i] - 10);
            uint8_t r = blades[i] * (i % 3 == 1);
            uint8_t g = blades[i] * (i % 3 == 0);
            uint8_t b = blades[i] * (i % 3 == 2) * 238 / 255;
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        int idx = (offset + random(NUM_LEDS)) % NUM_LEDS;
        blades[idx] = random(100, 255);
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
            stars[i] = max(0, stars[i] - 20);
            uint16_t h = hue + (i * 65536L / NUM_LEDS);
            uint8_t r, g, b;
            if (stars[i] > 0) {
                r = g = b = stars[i];
            } else {
                uint32_t c = strip.ColorHSV(h);
                r = (c >> 16) & 0xFF;
                g = (c >> 8) & 0xFF;
                b = c & 0xFF;
            }
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        if (random(100) < 5) {
            stars[random(NUM_LEDS)] = 255;
        }
        hue += random(100, 200);
        lastSwirl = millis();
    }
}

void maxAquarianFlow() {
    static uint8_t waves[NUM_LEDS] = {0};
    static int direction = 1;
    static int pos = 0;
    static unsigned long lastWave = 0;

    if (millis() - lastWave >= 30) {
        for (int i = 0; i < NUM_LEDS; i++) {
            waves[i] = max(0, waves[i] - 10);
            uint8_t g = waves[i] * (i % 2);
            uint8_t b = waves[i];
            uint8_t r = waves[i] * (i % 2 == 0);
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        waves[pos] = random(150, 255);
        pos += direction;
        if (pos >= NUM_LEDS - 1 || pos <= 0) {
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
            pulses[i] = max(0, pulses[i] - 15);
            uint8_t r = pulses[i] * (i % 2);
            uint8_t g = pulses[i];
            uint8_t b = pulses[i];
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        if (converging) {
            pulses[left] = pulses[right] = 255;
            left++;
            right--;
            if (left >= right) {
                converging = false;
                left = right = NUM_LEDS / 2;
            }
        } else {
            pulses[left] = pulses[right] = 255;
            left--;
            right++;
            if (left < 0 || right >= NUM_LEDS) {
                converging = true;
                left = 0;
                right = NUM_LEDS - 1;
            }
        }
        if (random(100) < 3) {
            pulses[random(NUM_LEDS)] = random(50, 100);
        }
        lastPulse = millis();
    }
}

void proletariatPulse() {
    static uint8_t ripples[NUM_LEDS] = {0};
    static uint8_t sparkles[NUM_LEDS] = {0};
    static int wavePos = 0;
    static int direction = 1;
    static unsigned long lastRipple = 0;

    if (millis() - lastRipple >= 25) {
        for (int i = 0; i < NUM_LEDS; i++) {
            ripples[i] = max(0, ripples[i] - 12); // Fade wave
            sparkles[i] = max(0, sparkles[i] - 15); // Fade sparkles
            uint8_t r = ripples[i]; // Red wave
            uint8_t g = sparkles[i] * (i % 3 == 1) * 100 / 255; // Orange sparkles
            uint8_t b = sparkles[i] * (i % 3 == 2) * 50 / 255; // Subtle blue sparkles
            if (sparkles[i] > ripples[i]) {
                r = sparkles[i]; // Warm white sparkle overlay
                g = sparkles[i] * 240 / 255;
                b = sparkles[i] * 200 / 255;
            }
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        // Add new ripple
        ripples[wavePos] = random(150, 255);
        // Add random sparkles
        if (random(100) < 20) {
            int spark = random(NUM_LEDS);
            sparkles[spark] = random(180, 255);
        }
        wavePos += direction;
        if (wavePos >= NUM_LEDS - 1 || wavePos <= 0) {
            direction = -direction;
        }
        lastRipple = millis();
    }
}

void bourgeoisBlaze() {
    static uint8_t clusters[NUM_LEDS] = {0};
    static int pos = 0;
    static unsigned long lastWave = 0;

    if (millis() - lastWave >= 30) {
        for (int i = 0; i < NUM_LEDS; i++) {
            clusters[i] = max(0, clusters[i] - 12);
            uint8_t r = clusters[i] * (i % 3 == 0);
            uint8_t g = clusters[i] * (i % 3 == 0) * 215 / 255;
            uint8_t b = clusters[i] * (i % 3 == 1) * 180 / 255;
            if (i % 3 == 2) {
                r = clusters[i];
                g = b = clusters[i] / 4;
            }
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        clusters[pos] = 255;
        clusters[(pos + NUM_LEDS / 2) % NUM_LEDS] = 255;
        if (random(100) < 8) {
            int spark = random(NUM_LEDS);
            clusters[spark] = random(180, 255);
        }
        pos = (pos + 1) % NUM_LEDS;
        lastWave = millis();
    }
}
