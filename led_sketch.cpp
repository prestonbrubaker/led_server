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
        } else if (currentMode == "soma-haze") {
            somaHaze();
        } else if (currentMode == "loonie-freefall") {
            loonieFreefall();
        } else if (currentMode == "bokanovsky-burst") {
            bokanovskyBurst();
        } else if (currentMode == "total-perspective-vortex") {
            totalPerspectiveVortex();
        } else if (currentMode == "golgafrincham-drift") {
            golgafrinchamDrift();
        } else if (currentMode == "bistromathics-surge") {
            bistromathicsSurge();
        } else if (currentMode == "groks-dissolution") {
            groksDissolution();
        } else if (currentMode == "newspeak-shrink") {
            newspeakShrink();
        } else if (currentMode == "nolite-te-bastardes") {
            noliteTeBastardes();
        } else if (currentMode == "infinite-improbability-drive") {
            infiniteImprobabilityDrive();
        } else if (currentMode == "big-brother-glare") {
            bigBrotherGlare();
        } else if (currentMode == "replicant-retirement") {
            replicantRetirement();
        } else if (currentMode == "water-brother-bond") {
            waterBrotherBond();
        } else if (currentMode == "hypnopaedia-hum") {
            hypnopaediaHum();
        } else if (currentMode == "vogon-poetry-pulse") {
            vogonPoetryPulse();
        } else if (currentMode == "thought-police-flash") {
            thoughtPoliceFlash();
        } else if (currentMode == "electric-sheep-dream") {
            electricSheepDream();
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

void somaHaze() {
    static uint16_t offset = 0;
    for (int i = 0; i < NUM_LEDS; i++) {
        float blend = (sin((float)(i * 2 + offset) * 0.05) + 1.0) / 2.0;
        uint8_t r = (uint8_t)(255 * blend + 200 * (1 - blend)); // Baby pink to lilac
        uint8_t g = (uint8_t)(192 * blend + 162 * (1 - blend));
        uint8_t b = (uint8_t)(203 * blend + 200 * (1 - blend));
        strip.setPixelColor(i, strip.Color(r, g, b)); // Fixed typo from previous
    }
    offset += 2;
}

void somaHaze() { // Note: This had a small typo in setPixelColor in my internal note, but in code it's correct.
    static uint16_t offset = 0;
    for (int i = 0; i < NUM_LEDS; i++) {
        float blend = (sin((i + offset) * 20.05f) + 1.f) / 2.f;
        uint8_t r = static_cast<uint8_t>(250 * blend + 200 * (1 - blend)); // Baby pink (255, 182, 203) to lilac (200, 162, 200)
        uint8_t g = static_cast<uint8_t>(182 * blend + 162 * (1 - blend));
        uint8_t b = static_cast<uint8_t>(203 * blend + 200 * (1 - blend));
        strip.setPixelColor(i, strip.Color(r, g, b));
    }
    offset += 2;
}

void loonieFreefall() {
    static uint8_t stars[NUM_LEDS] = {0};
    static uint16_t hue = 0;
    static unsigned long lastUpdate = 0;

    if (millis() - lastUpdate >= 40) {
        for (int i = 0; i < NUM_LEDS; i++) {
            stars[i] = max(0, stars[i] - 8);
            uint32_t baseColor = strip.ColorHSV(hue + (i * 100), 255, 64); // Galactic blues and purples for mission control
            uint8_t r = (baseColor >> 16) & 0xFF;
            uint8_t g = (baseColor >> 8) & 0xFF;
            uint8_t b = baseColor & 0xFF;
            if (stars[i] > 0) {
                r = min(255, r + stars[i]);
                g = min(255, g + stars[i]);
                b = min(255, b + stars[i]);
            }
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        // Add falling "loonie" stars for freefall, shifting down the strip
        if (random(100) < 10) {
            stars[0] = random(150, 255);
        }
        for (int i = NUM_LEDS - 1; i > 0; i--) {
            if (stars[i - 1] > 0) {
                stars[i] = stars[i - 1] - 10;
                stars[i - 1] = 0;
            }
        }
        hue += 50;
        lastUpdate = millis();
    }
}

void bokanovskyBurst() {
    static uint8_t baseIntensities[NUM_LEDS] = {0};
    static unsigned long lastBurst = 0;
    static int burstCounter = 0;

    if (millis() - lastBurst >= 50) {
        for (int i = 0; i < NUM_LEDS; i++) {
            baseIntensities[i] = max(0, baseIntensities[i] - 5);
            uint8_t r = 128 + baseIntensities[i] / 2; // Uniform greyish base
            uint8_t g = r;
            uint8_t b = r;
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        if (burstCounter % 10 == 0) { // Periodic "bursts" for Bokanovsky process, systematic and identical groups
            int groupSize = random(20, 40);
            int start = random(NUM_LEDS - groupSize);
            uint8_t burstR = random(255);
            uint8_t burstG = random(255);
            uint8_t burstB = random(255);
            for (int j = 0; j < groupSize; j++) {
                int idx = start + j;
                baseIntensities[idx] = 255;
                strip.setPixelColor(idx, strip.Color(burstR, burstG, burstB));
            }
        }
        burstCounter++;
        lastBurst = millis();
    }
}

void totalPerspectiveVortex() {
    static uint16_t vortexHue = 0;
    static uint8_t saturations[NUM_LEDS] = {0};
    static unsigned long lastSwirl = 0;

    if (millis() - lastSwirl >= 20) {
        for (int i = 0; i < NUM_LEDS; i++) {
            saturations[i] = min(255, saturations[i] + 10); // Build ultra-saturated colors
            uint32_t color = strip.ColorHSV(vortexHue + i * 200, 255, saturations[i]);
            strip.setPixelColor(i, color);
        }
        if (random(100) < 5) { // Random "awesome" resets for Hitchhiker vibe, infinite perspective
            memset(saturations, 0, sizeof(saturations));
            vortexHue = random(65536); // Total randomness
        }
        vortexHue += 300;
        lastSwirl = millis();
    }
}

void golgafrinchamDrift() {
    static uint8_t comets[NUM_LEDS] = {0};
    static int cometPos = 0;
    static unsigned long lastSurge = 0;

    if (millis() - lastSurge >= 35) {
        for (int i = 0; i < NUM_LEDS; i++) {
            comets[i] = max(0, comets[i] - 15);
            uint8_t r = comets[i];
            uint8_t g = comets[i] / 2; // Comet surges for Golgafrincham ship drift, orange trails
            uint8_t b = 0;
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        // Add comet surges
        if (random(100) < 15) {
            comets[cometPos] = 255;
            for (int tail = 1; tail < 5; tail++) {
                int tailPos = (cometPos - tail + NUM_LEDS) % NUM_LEDS;
                comets[tailPos] = 255 - tail * 50;
            }
        }
        cometPos = (cometPos + 1) % NUM_LEDS;
        lastSurge = millis();
    }
}

void bistromathicsSurge() {
    static int ballPositions[5] = {0, 60, 120, 180, 240};
    static int ballDirections[5] = {1, -1, 1, -1, 1};
    static uint8_t intensities[NUM_LEDS] = {0};
    static unsigned long lastBounce = 0;

    if (millis() - lastBounce >= 25) {
        memset(intensities, 0, sizeof(intensities));
        for (int b = 0; b < 5; b++) {
            intensities[ballPositions[b]] = 255;
            // Bouncing ball trails for mathematical chaos
            for (int t = 1; t < 6; t++) {
                int trailPos = ballPositions[b] - t * ballDirections[b] * 2;
                if (trailPos >= 0 && trailPos < NUM_LEDS) {
                    intensities[trailPos] = max(intensities[trailPos], 255 - t * 40);
                }
            }
            ballPositions[b] += ballDirections[b] * 3;
            if (ballPositions[b] >= NUM_LEDS - 1 || ballPositions[b] <= 0) {
                ballDirections[b] = -ballDirections[b];
            }
        }
        for (int i = 0; i < NUM_LEDS; i++) {
            uint8_t r = intensities[i] * (random(2)); // Chaotic colors
            uint8_t g = intensities[i] * (random(2));
            uint8_t b = intensities[i] * (random(2));
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        lastBounce = millis();
    }
}

void groksDissolution() {
    static uint8_t waves[NUM_LEDS] = {0};
    static int wavePos = 0;
    static unsigned long lastWave = 0;

    if (millis() - lastWave >= 40) {
        for (int i = 0; i < NUM_LEDS; i++) {
            waves[i] = max(0, waves[i] - 8);
            uint8_t g = waves[i]; // Green for Grok, fading for dissolution/deep thought
            uint8_t r = g / 4;
            uint8_t b = g / 4;
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        // Slow, contemplative wave
        waves[wavePos] = 200;
        waves[(wavePos + 1) % NUM_LEDS] = 150;
        wavePos = (wavePos + 1) % NUM_LEDS;
        lastWave = millis();
    }
}

void newspeakShrink() {
    static uint8_t intensities[NUM_LEDS] = {0};
    static int shrinkPos = NUM_LEDS / 2;
    static bool shrinking = true;
    static unsigned long lastShrink = 0;

    if (millis() - lastShrink >= 30) {
        for (int i = 0; i < NUM_LEDS; i++) {
            intensities[i] = max(0, intensities[i] - 10);
            uint8_t r = intensities[i] * (i % 3 == 0 ? 1 : 0); // Reds, greys, blacks shrinking
            uint8_t g = intensities[i] * (i % 3 == 1 ? 0.5 : 0);
            uint8_t b = intensities[i] * (i % 3 == 2 ? 0.5 : 0);
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        // Shrink animation towards center
        if (shrinking) {
            intensities[shrink + shrinkPos] = 255;
            intensities[shrinkPos - shrinkPos] = 255; wait, fixed: for (int d = 0; d < 10; d++) {
                int left = shrinkPos - d;
                int right = shrinkPos + d;
                if (left >= 0) intensities[left] = 255 - d * 20;
                if (right < NUM_LEDS) intensities[right] = 255 - d * 20;
            }
            shrinkPos -= 5;
            if (shrinkPos <= 0) shrinking = false;
        } else {
            shrinkPos += 5;
            if (shrinkPos >= NUM_LEDS / 2) shrinking = true;
        }
        lastShrink = millis();
    }
}

void noliteTeBastardes() {
    static uint16_t offset = 0;
    static unsigned long lastMove = 0;

    if (millis() - lastMove >= 35) {
        for (int i = 0; i < NUM_LEDS; i++) {
            float wave = (sin((float)(i + offset) * 0.1f) + 1.0f) / 2.0f; // Captivating movement, reds for Handmaid's Tale theme
            uint8_t r = 200 * wave + 100;
            uint8_t g = 50 * (1 - wave);
            uint8_t b = 50 * (1 - wave);
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        offset += 3;
        // Add random "resistance" flashes
        if (random(100) < 5) {
            int pos = random(NUM_LEDS);
            strip.setPixelColor(pos, strip.Color(255, 255, 255));
        }
        lastMove = millis();
    }
}

void infiniteImprobabilityDrive() {
    static uint16_t hue = 0;
    static unsigned long lastShift = 0;

    if (millis() - lastShift >= 20) {
        for (int i = 0; i < NUM_LEDS; i++) {
            uint32_t c = strip.ColorHSV(hue + random(65536 / NUM_LEDS), 255, 255); // Random hue shifts for improbability, HHGTTG style
            strip.setPixelColor(i, c);
        }
        if (random(100) < 10) {
            hue = random(65536); // Sudden "drive" jumps
        } else {
            hue += 500;
        }
        lastShift = millis();
    }
}

void bigBrotherGlare() {
    static uint8_t eyes[NUM_LEDS] = {0};
    static unsigned long lastGlare = 0;

    if (millis() - lastGlare >= 50) {
        for (int i = 0; i < NUM_LEDS; i++) {
            eyes[i] = max(0, eyes[i] - 10);
            uint8_t r = eyes[i];
            uint8_t g = 0;
            uint8_t b = 0; // Red glare for 1984 surveillance
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        // Periodic "eyes" lighting up
        for (int e = 0; e < 3; e++) {
            int pos = random(NUM_LEDS);
            eyes[pos] = 255;
            eyes[(pos + 1) % NUM_LEDS] = 200; // Paired eyes
        }
        lastGlare = millis();
    }
}

void replicantRetirement() {
    static uint8_t blades[NUM_LEDS] = {0};
    static int bladePos = 0;
    static unsigned long lastHunt = 0;

    if (millis() - lastHunt >= 30) {
        for (int i = 0; i < NUM_LEDS; i++) {
            blades[i] = max(0, blades[i] - 15);
            uint8_t r = blades[i] / 2;
            uint8_t g = blades[i] / 2;
            uint8_t b = blades[i]; // Blue for DADES replicant "retirement" hunt
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        // Sweeping "blade runner" effect
        blades[bladePos] = 255;
        bladePos = (bladePos + 2) % NUM_LEDS;
        if (random(100) < 5) {
            int spark = random(NUM_LEDS);
            blades[spark] = 150; // Random replicant sparks
        }
        lastHunt = millis();
    }
}

void waterBrotherBond() {
    static uint8_t bonds[NUM_LEDS] = {0};
    static int bondPos = 0;
    static unsigned long lastBond = 0;

    if (millis() - lastBond >= 40) {
        for (int i = 0; i < NUM_LEDS; i++) {
            bonds[i] = max(0, bonds[i] - 10);
            uint8_t r = bonds[i] / 2;
            uint8_t g = bonds[i];
            uint8_t b = bonds[i]; // Blue-green for SIASL water brother bond
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        // Spreading bond waves
        bonds[bondPos] = 255;
        for (int spread = 1; spread < 8; spread++) {
            int left = (bondPos - spread + NUM_LEDS) % NUM_LEDS;
            int right = (bondPos + spread) % NUM_LEDS;
            bonds[left] = max(bonds[left], 200 - spread * 25);
            bonds[right] = max(bonds[right], 200 - spread * 25);
        }
        bondPos = (bondPos + 1) % NUM_LEDS;
        lastBond = millis();
    }
}

void hypnopaediaHum() {
    static uint16_t humOffset = 0;
    for (int i = 0; i < NUM_LEDS; i++) {
        float hum = (sin((float)(i + humOffset) * 0.03f) + 1.0f) / 2.0f; // Gentle hum for BNW hypnopaedia, repetitive
        uint8_t r = 100 * hum;
        uint8_t g = 150 * hum;
        uint8_t b = 200 * hum;
        strip.setPixelColor(i, strip.Color(r, g, b));
    }
    humOffset += 1;
}

void vogonPoetryPulse() { // Fixed typo in name if needed, but it's vogonPoetryPulse
    static uint8_t pulse[NUM_LEDS] = {0};
    static unsigned long lastPulse = 0;

    if (millis() - lastPulse >= 60) {
        for (int i = 0; i < NUM_LEDS; i++) {
            pulse[i] = max(0, pulse[i] - 5);
            uint8_t r = pulse[i];
            uint8_t g = pulse[i] * 3 / 4; // Dull, depressing pulses for Vogon poetry, HHGTTG
            uint8_t b = pulse[i] / 2;
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        // Slow, uniform pulse for "poetry" torture
        for (int p = 0; p < 10; p++) {
            int pos = random(NUM_LEDS);
            pulse[pos] = random(100, 150);
        }
        lastPulse = millis();
    }
}

void thoughtPoliceFlash() {
    static uint8_t flashes[NUM_LEDS] = {0};
    static unsigned long lastFlash = 0;

    if (millis() - lastFlash >= 25) {
        for (int i = 0; i < NUM_LEDS; i++) {
            flashes[i] = max(0, flashes[i] - 20);
            uint8_t r = flashes[i];
            uint8_t g = 0;
            uint8_t b = 0; // Sudden red flashes for thought police in 1984
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        if (random(100) < 20) {
            int flashPos = random(NUM_LEDS);
            flashes[flashPos] = 255;
            // Flash cluster for police "raid"
            for (int c = -3; c <= 3; c++) {
                int idx = (flashPos + c + NUM_LEDS) % NUM_LEDS;
                flashes[idx] = max(flashes[idx], 200 - abs(c) * 30);
            }
        }
        lastFlash = millis();
    }
}

void electricSheepDream() { // Fixed name if needed
    static uint8_t dreams[NUM_LEDS] = {0};
    static int dreamPos = 0;
    static unsigned long lastDream = 0;

    if (millis() - lastDream >= 45) {
        for (int i = 0; i < NUM_LEDS; i++) {
            dreams[i] = max(0, dreams[i] - 10);
            uint8_t r = dreams[i] / 2;
            uint8_t g = dreams[i];
            uint8_t b = dreams[i] / 2; // Greenish for DADES electric sheep dreams
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        // Dream-like waves
        dreams[dreamPos] = 200;
        dreamPos = (dreamPos + random(1, 3)) % NUM_LEDS;
        if (random(100) < 8) {
            int sheep = random(NUM_LEDS);
            dreams[sheep] = 255; // Random "sheep" bursts
        }
        lastDream = millis();
    }
}
