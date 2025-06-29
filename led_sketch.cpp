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
    static uint8_t sparkles[NUM_LEDS] = {0};
    static uint8_t sparkleColors[NUM_LEDS][3] = {{0}};
    static unsigned long lastUpdate = 0;

    unsigned long currentTime = millis();
    if (currentTime - lastUpdate >= random(15, 30)) {
        for (int i = 0; i < NUM_LEDS; i++) {
            sparkles[i] = max(0, sparkles[i] - 20);
            uint8_t r, g, b;
            if (sparkles[i] > 0) {
                r = sparkleColors[i][0];
                g = sparkleColors[i][1];
                b = sparkleColors[i][2];
            } else {
                uint16_t h = hue + (i * 65536L / NUM_LEDS) * 0.8;
                uint8_t s = random(200, 255);
                uint8_t v = 150 + (sin((hue + i * 100) * 0.01) + 1) * 52.5;
                uint32_t c = strip.ColorHSV(h, s, v);
                r = (c >> 16) & 0xFF;
                g = (c >> 8) & 0xFF;
                b = c & 0xFF;
            }
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        if (random(100) < 5) {
            for (int j = 0; j < random(1, 4); j++) {
                int spark = random(NUM_LEDS);
                sparkles[spark] = random(180, 255);
                uint8_t sparkType = random(3);
                if (sparkType == 0) {
                    sparkleColors[spark][0] = sparkles[spark];
                    sparkleColors[spark][1] = sparkles[spark];
                    sparkleColors[spark][2] = sparkles[spark];
                } else if (sparkType == 1) { // Gold
                    sparkleColors[spark][0] = sparkles[spark];
                    sparkleColors[spark][1] = sparkles[spark] * 200 / 255;
                    sparkleColors[spark][2] = sparkles[spark] * 50 / 255;
                } else { // Neon pink
                    sparkleColors[spark][0] = sparkles[spark] * 255 / 255;
                    sparkleColors[spark][1] = sparkles[spark] * 105 / 255;
                    sparkleColors[spark][2] = sparkles[spark] * 180 / 255;
                }
            }
        }
        hue += 512;
        lastUpdate = currentTime;
    }
}

void setLedsRed() {
    static uint8_t blades[NUM_LEDS] = {0};
    static int offset = 0;
    static unsigned long lastSway = 0;

    unsigned long currentTime = millis();
    if (currentTime - lastSway >= random(20, 50)) {
        for (int i = 0; i < NUM_LEDS; i++) {
            blades[i] = max(0, blades[i] - 5);
            uint8_t r = blades[i];
            uint8_t g = 0;
            uint8_t b = 0;
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        int numBursts = random(2, 5);
        for (int j = 0; j < numBursts; j++) {
            int idx = (offset + random(NUM_LEDS)) % NUM_LEDS;
            blades[idx] = random(150, 255);
        }
        offset = (offset + 1) % NUM_LEDS;
        lastSway = currentTime;
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

    unsigned long currentTime = millis();
    if (currentTime - lastCrackle >= random(30, 100)) {
        for (int i = 0; i < NUM_LEDS; i++) {
            intensities[i] = max((uint8_t)0, (uint8_t)(intensities[i] - random(5, 15)));
            uint8_t r = min(255, 255 * intensities[i] / 255);
            uint8_t g = intensities[i] / 10;
            strip.setPixelColor(i, strip.Color(r, g, 0));
        }
        for (int i = 0; i < 8; i++) {
            int led = random(NUM_LEDS);
            intensities[led] = random(50, 255);
        }
        lastCrackle = currentTime;
    }
}

void bourgeoisBrilliance() {
    static uint8_t colors[NUM_LEDS] = {0}; // 0: unclaimed, 1: bourgeois gold, 2: bourgeois emerald, 3: proletariat red, 4: proletariat orange, 5: neutral silver
    static uint8_t intensities[NUM_LEDS] = {0};
    static bool initialized = false;
    static unsigned long lastUpdate = 0;
    static unsigned long gameStart = 0;

    // Initialize game
    if (!initialized) {
        memset(colors, 0, NUM_LEDS);
        memset(intensities, 0, NUM_LEDS);
        int numPods = random(5, 9); // More pods
        int totalLeds = 0;
        int minLeds = NUM_LEDS * 0.7; // 70% coverage
        for (int i = 0; i < numPods && totalLeds < minLeds; i++) {
            int start = random(NUM_LEDS - 12);
            int size = random(6, 13); // Larger pods
            uint8_t podColor = random(1, 6); // Bourgeois, proletariat, or neutral
            for (int j = start; j < start + size && j < NUM_LEDS; j++) {
                if (colors[j] == 0) { // Avoid overlap
                    colors[j] = podColor;
                    intensities[j] = random(200, 255); // Brighter
                    totalLeds++;
                }
            }
        }
        initialized = true;
        gameStart = millis();
    }

    unsigned long currentTime = millis();
    if (currentTime - lastUpdate >= random(800, 1200)) {
        // Fade intensities
        for (int i = 0; i < NUM_LEDS; i++) {
            intensities[i] = max((uint8_t)0, (uint8_t)(intensities[i] - 3)); // Slower fade
            uint8_t r = 0, g = 0, b = 0;
            switch (colors[i]) {
                case 1: // Bourgeois gold
                    r = intensities[i];
                    g = intensities[i] * 200 / 255;
                    b = intensities[i] * 50 / 255;
                    break;
                case 2: // Bourgeois emerald
                    r = intensities[i] * 50 / 255;
                    g = intensities[i] * 150 / 255;
                    b = intensities[i] * 50 / 255;
                    break;
                case 3: // Proletariat red
                    r = intensities[i];
                    g = 0;
                    b = 0;
                    break;
                case 4: // Proletariat orange
                    r = intensities[i];
                    g = intensities[i] * 100 / 255;
                    b = 0;
                    break;
                case 5: // Neutral silver
                    r = intensities[i] * 150 / 255;
                    g = intensities[i] * 150 / 255;
                    b = intensities[i] * 160 / 255;
                    break;
            }
            strip.setPixelColor(i, strip.Color(r, g, b));
        }

        // Find pod edges
        int edges[NUM_LEDS][2]; // [0]: left edge, [1]: right edge
        int edgeCount = 0;
        for (int i = 0; i < NUM_LEDS; i++) {
            if (colors[i] != 0) {
                bool isLeftEdge = (i == 0 || colors[i - 1] != colors[i]);
                bool isRightEdge = (i == NUM_LEDS - 1 || colors[i + 1] != colors[i]);
                if (isLeftEdge || isRightEdge) {
                    edges[edgeCount][0] = isLeftEdge ? i : -1;
                    edges[edgeCount][1] = isRightEdge ? i : -1;
                    edgeCount++;
                }
            }
        }

        // Move or battle
        for (int e = 0; e < edgeCount; e++) {
            int left = edges[e][0];
            int right = edges[e][1];
            if (left >= 0) {
                int podSize = 1;
                for (int j = left; j < NUM_LEDS && colors[j] == colors[left]; j++) podSize++;
                for (int j = left - 1; j >= 0 && colors[j] == colors[left]; j--) podSize++;
                bool attack = (podSize > 5) ? random(100) < 90 : random(100) < 50; // More aggressive
                int target = (random(100) < 10) ? left + 1 : left - 1; // Less retreating
                if (target >= 0 && target < NUM_LEDS && colors[target] == 0) {
                    colors[target] = colors[left];
                    intensities[target] = intensities[left];
                } else if (target >= 0 && target < NUM_LEDS && colors[target] != colors[left]) {
                    int targetSize = 1;
                    for (int j = target; j < NUM_LEDS && colors[j] == colors[target]; j++) targetSize++;
                    for (int j = target - 1; j >= 0 && colors[j] == colors[target]; j--) targetSize++;
                    bool largerWins = (podSize > targetSize) && random(100) < 60; // Bias for larger pods
                    if (random(2) || largerWins) {
                        intensities[target] = 255; // Battle flash
                        colors[target] = colors[left];
                    } else {
                        intensities[left] = 255; // Battle flash
                    }
                }
            }
            if (right >= 0 && right != left) {
                int podSize = 1;
                for (int j = right; j >= 0 && colors[j] == colors[right]; j--) podSize++;
                for (int j = right + 1; j < NUM_LEDS && colors[j] == colors[right]; j++) podSize++;
                bool attack = (podSize > 5) ? random(100) < 90 : random(100) < 50; // More aggressive
                int target = (random(100) < 10) ? right - 1 : right + 1; // Less retreating
                if (target >= 0 && target < NUM_LEDS && colors[target] == 0) {
                    colors[target] = colors[right];
                    intensities[target] = intensities[right];
                } else if (target >= 0 && target < NUM_LEDS && colors[target] != colors[right]) {
                    int targetSize = 1;
                    for (int j = target; j < NUM_LEDS && colors[j] == colors[target]; j++) targetSize++;
                    for (int j = target - 1; j >= 0 && colors[j] == colors[target]; j--) targetSize++;
                    bool largerWins = (podSize > targetSize) && random(100) < 60; // Bias for larger pods
                    if (random(2) || largerWins) {
                        intensities[target] = 255; // Battle flash
                        colors[target] = colors[right];
                    } else {
                        intensities[right] = 255; // Battle flash
                    }
                }
            }
        }

        // Proletariat collusion
        for (int i = 1; i < NUM_LEDS - 1; i++) {
            if ((colors[i] == 3 || colors[i] == 4) && (colors[i - 1] == 3 || colors[i - 1] == 4) && random(100) < 30) {
                colors[i - 1] = colors[i]; // Merge proletariat pods
                intensities[i - 1] = intensities[i];
            }
            if ((colors[i] == 3 || colors[i] == 4) && (colors[i + 1] == 3 || colors[i + 1] == 4) && random(100) < 30) {
                colors[i + 1] = colors[i];
                intensities[i + 1] = intensities[i];
            }
        }

        // Random underground pod spawn
        if (random(100) < 10) {
            int start = random(NUM_LEDS - 4);
            if (colors[start] == 0 && colors[start + 1] == 0 && colors[start + 2] == 0) {
                for (int j = start; j < start + 3; j++) {
                    colors[j] = 5; // Neutral silver
                    intensities[j] = random(200, 255);
                }
            }
        }

        // Check for victory
        int counts[6] = {0};
        for (int i = 0; i < NUM_LEDS; i++) {
            counts[colors[i]]++;
        }
        for (int c = 1; c <= 5; c++) {
            if (counts[c] >= NUM_LEDS * 0.9) {
                uint8_t r = 0, g = 0, b = 0;
                switch (c) {
                    case 1: r = 255; g = 200; b = 50; break; // Gold
                    case 2: r = 50; g = 150; b = 50; break; // Emerald
                    case 3: r = 255; g = 0; b = 0; break; // Red
                    case 4: r = 255; g = 100; b = 0; break; // Orange
                    case 5: r = 150; g = 150; b = 160; break; // Silver
                }
                for (int i = 0; i < NUM_LEDS; i++) {
                    strip.setPixelColor(i, strip.Color(r, g, b));
                }
                strip.show();
                delay(500); // Victory flash
                initialized = false; // Reset game
            }
        }

        // Reset after ~60 seconds
        if (currentTime - gameStart >= 60000) {
            initialized = false;
        }

        lastUpdate = currentTime;
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
    static uint8_t stars[NUM_LEDS] = {0};
    static uint8_t starColors[NUM_LEDS][3] = {{0}};
    static uint16_t wavePhase = 0;
    static unsigned long lastSwirl = 0;
    static bool partyMode = false;
    static unsigned long partyStart = 0;

    unsigned long currentTime = millis();
    if (partyMode && currentTime - partyStart < 100) {
        return; // Let party flash linger
    }
    partyMode = false;

    if (currentTime - lastSwirl >= random(30, 70)) {
        // Calculate wave brightness
        uint8_t pulse = 100 + (sin(wavePhase * 0.03) + 1) * 50; // 100-200
        for (int i = 0; i < NUM_LEDS; i++) {
            stars[i] = max((uint8_t)0, (uint8_t)(stars[i] - 6)); // Slower star fade
            uint8_t r = 0, g = 0, b = 0;
            if (stars[i] > 0) {
                r = starColors[i][0];
                g = starColors[i][1];
                b = starColors[i][2];
            } else {
                // Flowing cosmic wave
                uint16_t waveOffset = (wavePhase + i * 65536L / NUM_LEDS) % 65536;
                uint8_t colorType = (waveOffset / 21845) % 3; // Divide hue space
                if (colorType == 0) { // Purple
                    r = 120 * pulse / 255;
                    g = 0;
                    b = 220 * pulse / 255;
                } else if (colorType == 1) { // Indigo
                    r = 60 * pulse / 255;
                    g = 0;
                    b = 255 * pulse / 255;
                } else { // Magenta
                    r = 220 * pulse / 255;
                    g = 0;
                    b = 220 * pulse / 255;
                }
            }
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        // Add random stars
        if (random(100) < 25) {
            for (int j = 0; j < random(3, 6); j++) {
                int spark = random(NUM_LEDS);
                stars[spark] = random(150, 255);
                uint8_t starType = random(4);
                if (starType == 0) { // White
                    starColors[spark][0] = stars[spark];
                    starColors[spark][1] = stars[spark];
                    starColors[spark][2] = stars[spark];
                } else if (starType == 1) { // Cyan
                    starColors[spark][0] = 0;
                    starColors[spark][1] = 200;
                    starColors[spark][2] = 255;
                } else if (starType == 2) { // Gold
                    starColors[spark][0] = 255;
                    starColors[spark][1] = 200;
                    starColors[spark][2] = 50;
                } else { // Neon pink
                    starColors[spark][0] = 255;
                    starColors[spark][1] = 105;
                    starColors[spark][2] = 180;
                }
            }
        }
        // Party mode flash
        if (random(100) < 5) {
            for (int j = 0; j < 10; j++) {
                int flash = random(NUM_LEDS);
                stars[flash] = 255;
                uint8_t flashType = random(4);
                if (flashType == 0) { // Magenta
                    starColors[flash][0] = 220;
                    starColors[flash][1] = 0;
                    starColors[flash][2] = 220;
                } else if (flashType == 1) { // Cyan
                    starColors[flash][0] = 0;
                    starColors[flash][1] = 200;
                    starColors[flash][2] = 255;
                } else if (flashType == 2) { // Gold
                    starColors[flash][0] = 255;
                    starColors[flash][1] = 200;
                    starColors[flash][2] = 50;
                } else { // Neon pink
                    starColors[flash][0] = 255;
                    starColors[flash][1] = 105;
                    starColors[flash][2] = 180;
                }
            }
            partyMode = true;
            partyStart = currentTime;
        }
        wavePhase += random(100, 200);
        lastSwirl = currentTime;
    }
}

void maxAquarianFlow() {
    static uint8_t waves[NUM_LEDS] = {0};
    static uint8_t sparkles[NUM_LEDS] = {0};
    static int direction = 1;
    static int pos = 0;
    static unsigned long lastWave = 0;

    unsigned long currentTime = millis();
    if (currentTime - lastWave >= random(40, 100)) {
        for (int i = 0; i < NUM_LEDS; i++) {
            waves[i] = max(0, waves[i] - 5); // Slower wave fade
            sparkles[i] = max(0, sparkles[i] - 8); // Slower sparkle fade
            uint8_t r = 0, g = 0, b = 0;
            if (sparkles[i] > waves[i]) { // Silvery sparkle overlay
                r = sparkles[i] * 200 / 255;
                g = sparkles[i] * 220 / 255;
                b = sparkles[i] * 240 / 255;
            } else if (waves[i] > 0) { // Wave colors: blue, turquoise, green
                uint8_t colorType = (i + pos) % 3;
                if (colorType == 0) { // Deep blue
                    r = 0;
                    g = waves[i] * 50 / 255;
                    b = waves[i] * 200 / 255;
                } else if (colorType == 1) { // Turquoise
                    r = 0;
                    g = waves[i] * 150 / 255;
                    b = waves[i] * 200 / 255;
                } else { // Soft green
                    r = waves[i] * 50 / 255;
                    g = waves[i] * 200 / 255;
                    b = waves[i] * 100 / 255;
                }
            }
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        // Add multiple wave peaks
        int positions[3] = {pos, (pos + NUM_LEDS / 3) % NUM_LEDS, (pos + 2 * NUM_LEDS / 3) % NUM_LEDS};
        for (int j = 0; j < 3; j++) {
            int p = positions[j];
            waves[p] = random(150, 255); // Main peak
            if (p > 0) waves[p - 1] = waves[p] * 3 / 4; // Ripple left
            if (p < NUM_LEDS - 1) waves[p + 1] = waves[p] * 3 / 4; // Ripple right
        }
        // Add random sparkles
        if (random(100) < 20) {
            for (int j = 0; j < random(1, 3); j++) {
                int spark = random(NUM_LEDS);
                sparkles[spark] = random(180, 255);
            }
        }
        pos += direction;
        if (pos >= NUM_LEDS - 1 || pos <= 0) {
            direction = -direction;
        }
        lastWave = currentTime;
    }
}

void lunarRebellionPulse() {
    static uint8_t pulses[NUM_LEDS] = {0};
    static int positions[3] = {0, NUM_LEDS / 3, 2 * NUM_LEDS / 3};
    static int targets[3] = {0};
    static int directions[3] = {1, 1, 1};
    static unsigned long lastPulse = 0;
    static bool collisionBurst = false;
    static unsigned long collisionTime = 0;

    unsigned long currentTime = millis();
    if (collisionBurst && currentTime - collisionTime < 100) {
        // Let collision burst linger
        return;
    }
    if (currentTime - lastPulse >= random(50, 150)) {
        collisionBurst = false;
        for (int i = 0; i < NUM_LEDS; i++) {
            pulses[i] = max(0, pulses[i] - 8); // Slower fade
            uint8_t r = pulses[i] * 100 / 255; // Dusty gray
            uint8_t g = pulses[i] * 100 / 255;
            uint8_t b = pulses[i] * 110 / 255;
            if (pulses[i] > 150) { // Fiery burst for high-intensity pulses
                r = pulses[i] * 255 / 255;
                g = pulses[i] * 100 / 255;
                b = 0;
            }
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        // Check for collisions and move pulses
        bool collision = false;
        for (int i = 0; i < 3; i++) {
            if (random(100) < 10) continue; // Occasional skip for erratic movement
            pulses[positions[i]] = 150; // Rebel pulse
            positions[i] += directions[i];
            if (positions[i] == targets[i]) {
                collision = true;
                targets[i] = random(NUM_LEDS); // New random target
                directions[i] = (positions[i] < targets[i]) ? 1 : -1;
            }
            if (positions[i] < 0 || positions[i] >= NUM_LEDS) {
                positions[i] = random(NUM_LEDS); // Reset to random position
                targets[i] = random(NUM_LEDS);
                directions[i] = (positions[i] < targets[i]) ? 1 : -1;
            }
        }
        // Handle collision
        for (int i = 0; i < 3; i++) {
            for (int j = i + 1; j < 3; j++) {
                if (positions[i] == positions[j] || abs(positions[i] - positions[j]) <= 2) {
                    collision = true;
                }
            }
        }
        if (collision) {
            for (int i = 0; i < 3; i++) {
                int p = positions[i];
                pulses[p] = 255; // Bright explosion
                for (int j = max(0, p - 2); j <= min(NUM_LEDS - 1, p + 2); j++) {
                    pulses[j] = 200 - 25 * abs(p - j); // Spread burst
                }
            }
            collisionBurst = true;
            collisionTime = currentTime;
        }
        // Add random background flickers
        if (random(100) < 5) {
            int spark = random(NUM_LEDS);
            pulses[spark] = random(50, 100); // Subtle lunar flicker
        }
        lastPulse = currentTime;
    }
}

void proletariatPulse() {
    static uint8_t ripples[NUM_LEDS] = {0};
    static uint8_t sparkles[NUM_LEDS] = {0};
    static uint8_t embers[NUM_LEDS] = {0};
    static int wavePos = 0;
    static int direction = 1;
    static uint8_t waveColor = 0; // 0: red, 1: orange, 2: gold
    static int emberCount = 0;
    static unsigned long lastRipple = 0;

    unsigned long currentTime = millis();
    if (currentTime - lastRipple >= random(40, 80)) {
        for (int i = 0; i < NUM_LEDS; i++) {
            ripples[i] = max(0, ripples[i] - 8); // Slower wave fade
            sparkles[i] = max(0, sparkles[i] - 10); // Slower sparkle fade
            embers[i] = max(0, embers[i] - 5); // Slow ember fade
            uint8_t r = 0, g = 0, b = 0;
            if (sparkles[i] > ripples[i] && sparkles[i] > embers[i]) { // Sparkle overlay
                uint8_t sparkType = random(10);
                if (sparkType < 7) { // Golden
                    r = sparkles[i];
                    g = sparkles[i] * 200 / 255;
                    b = sparkles[i] * 50 / 255;
                } else if (sparkType < 9) { // Crimson
                    r = sparkles[i] * 200 / 255;
                    g = sparkles[i] * 20 / 255;
                    b = sparkles[i] * 20 / 255;
                } else { // Amber
                    r = sparkles[i];
                    g = sparkles[i] * 150 / 255;
                    b = 0;
                }
            } else if (ripples[i] > embers[i]) { // Wave color
                if (waveColor == 0) { // Red
                    r = ripples[i];
                    g = 0;
                    b = 0;
                } else if (waveColor == 1) { // Orange
                    r = ripples[i];
                    g = ripples[i] * 100 / 255;
                    b = 0;
                } else { // Gold
                    r = ripples[i];
                    g = ripples[i] * 200 / 255;
                    b = ripples[i] * 50 / 255;
                }
            } else if (embers[i] > 0) { // Ember glow
                r = embers[i];
                g = embers[i] * (waveColor == 1 ? 100 : waveColor == 2 ? 200 : 0) / 255;
                b = embers[i] * (waveColor == 2 ? 50 : 0) / 255;
            }
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        // Add new ripple
        ripples[wavePos] = random(150, 255);
        // Collect embers on sparkle intersection
        if (sparkles[wavePos] > 0 && embers[wavePos] == 0) {
            embers[wavePos] = random(50, 100);
            emberCount++;
        }
        // Add random sparkles
        if (random(100) < 25) {
            for (int j = 0; j < random(1, 3); j++) {
                int spark = random(NUM_LEDS);
                sparkles[spark] = random(180, 255);
            }
        }
        // Move wave and handle bounce
        wavePos += direction;
        if (wavePos >= NUM_LEDS - 1 || wavePos <= 0) {
            direction = -direction;
            waveColor = (waveColor + 1) % 3; // Cycle color on bounce
        }
        // Check for full strip and reset
        if (emberCount >= NUM_LEDS * 0.8) {
            for (int i = 0; i < NUM_LEDS; i++) {
                strip.setPixelColor(i, strip.Color(255, 200, 50)); // Golden flash
            }
            strip.show();
            delay(100); // Brief flash
            memset(embers, 0, NUM_LEDS); // Clear embers
            memset(sparkles, 0, NUM_LEDS); // Clear sparkles
            emberCount = 0;
        }
        lastRipple = currentTime;
    }
}
