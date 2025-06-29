#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>

// LED strip configuration
#define NUM_LEDS 300
#define DATA_PIN 2 // GPIO2
#define BRIGHTNESS 102 // Base 40% brightness for car use (0-255)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, DATA_PIN, NEO_GRB + NEO_KHZ800);

// Pattern counter stored in EEPROM
#define EEPROM_ADDRESS 0
uint8_t currentPattern = 0;
const unsigned long updateInterval = 50; // ~20 FPS for smooth transitions
unsigned long lastUpdate = 0;

// Function declarations
void setLedsOff();
void rainbowFlow();
void austereEnlightenment();
void redBurstFlow();
void proletariatCrackle();
void cosmicRebellionPulse();

void setup() {
    // Initialize Serial for debugging (optional)
    Serial.begin(115200);
    delay(100);

    // Initialize EEPROM
    EEPROM.begin(4); // Allocate 4 bytes
    currentPattern = EEPROM.read(EEPROM_ADDRESS);
    if (currentPattern > 4) currentPattern = 0; // Ensure valid pattern
    currentPattern = (currentPattern + 1) % 5; // Cycle to next pattern
    EEPROM.write(EEPROM_ADDRESS, currentPattern);
    EEPROM.commit();

    // Initialize LED strip
    strip.begin();
    strip.setBrightness(BRIGHTNESS);
    setLedsOff();
    strip.show();
}

void loop() {
    if (millis() - lastUpdate >= updateInterval) {
        switch (currentPattern) {
            case 0:
                rainbowFlow();
                break;
            case 1:
                austereEnlightenment();
                break;
            case 2:
                redBurstFlow();
                break;
            case 3:
                proletariatCrackle();
                break;
            case 4:
                cosmicRebellionPulse();
                break;
        }
        strip.show();
        lastUpdate = millis();
    }
}

void setLedsOff() {
    for (int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(0, 0, 0));
    }
}

// Pattern 1: Rainbow Flow - Deep, saturated rainbow gradient with sparkling flickers
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
                uint16_t h = hue + (i * 65536L / NUM_LEDS); // Full rainbow cycle
                uint8_t s = 255; // Maximum saturation for deep colors
                uint8_t v = 100 + (sin((hue + i * 100) * 0.01) + 1) * 50; // Brightness 100-200
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
                    sparkleColors[spark][0] = sparkles[spark]; // White
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

// Pattern 2: Austere Enlightenment - Dynamic red, green, blue bursts
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

// Pattern 3: Red Burst Flow - Red bursts with dynamic fading
void redBurstFlow() {
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

// Pattern 4: Proletariat Crackle - Red-orange crackling effect with random bursts
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

// Pattern 5: Cosmic Rebellion Pulse - Colorful, busy pulse with full ship colors
void cosmicRebellionPulse() {
    static uint8_t pulses[NUM_LEDS] = {0};
    static int positions[5] = {0, NUM_LEDS / 5, 2 * NUM_LEDS / 5, 3 * NUM_LEDS / 5, 4 * NUM_LEDS / 5};
    static int targets[5] = {0};
    static int directions[5] = {1, 1, 1, 1, 1};
    static uint8_t colors[5][3] = {{255, 100, 0}, {0, 200, 100}, {100, 50, 255}, {255, 200, 0}, {200, 0, 200}};
    static unsigned long lastPulse = 0;
    static bool collisionBurst = false;
    static unsigned long collisionTime = 0;

    unsigned long currentTime = millis();
    if (collisionBurst && currentTime - collisionTime < 100) {
        return;
    }
    if (currentTime - lastPulse >= random(30, 100)) {
        collisionBurst = false;
        for (int i = 0; i < NUM_LEDS; i++) {
            pulses[i] = max(0, pulses[i] - 10);
            uint8_t r = pulses[i] * 80 / 255; // Dim base gradient
            uint8_t g = pulses[i] * 80 / 255;
            uint8_t b = pulses[i] * 100 / 255;
            if (pulses[i] > 150) {
                r = pulses[i] * 200 / 255;
                g = pulses[i] * 150 / 255;
                b = pulses[i] * 100 / 255;
            }
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        bool collision = false;
        for (int i = 0; i < 5; i++) {
            if (random(100) < 15) continue;
            pulses[positions[i]] = 150;
            strip.setPixelColor(positions[i], strip.Color(
                colors[i][0] * pulses[positions[i]] / 255,
                colors[i][1] * pulses[positions[i]] / 255,
                colors[i][2] * pulses[positions[i]] / 255
            ));
            positions[i] += directions[i];
            if (positions[i] == targets[i]) {
                collision = true;
                targets[i] = random(NUM_LEDS);
                directions[i] = (positions[i] < targets[i]) ? 1 : -1;
            }
            if (positions[i] < 0 || positions[i] >= NUM_LEDS) {
                positions[i] = random(NUM_LEDS);
                targets[i] = random(NUM_LEDS);
                directions[i] = (positions[i] < targets[i]) ? 1 : -1;
            }
        }
        for (int i = 0; i < 5; i++) {
            for (int j = i + 1; j < 5; j++) {
                if (positions[i] == positions[j] || abs(positions[i] - positions[j]) <= 3) {
                    collision = true;
                }
            }
        }
        if (collision) {
            for (int i = 0; i < 5; i++) {
                int p = positions[i];
                pulses[p] = 255;
                strip.setPixelColor(p, strip.Color(
                    colors[i][0] * pulses[p] / 255,
                    colors[i][1] * pulses[p] / 255,
                    colors[i][2] * pulses[p] / 255
                ));
                for (int j = max(0, p - 3); j <= min(NUM_LEDS - 1, p + 3); j++) {
                    pulses[j] = 200 - 20 * abs(p - j);
                    strip.setPixelColor(j, strip.Color(
                        colors[i][0] * pulses[j] / 255,
                        colors[i][1] * pulses[j] / 255,
                        colors[i][2] * pulses[j] / 255
                    ));
                }
            }
            collisionBurst = true;
            collisionTime = currentTime;
        }
        if (random(100) < 10) {
            int spark = random(NUM_LEDS);
            pulses[spark] = random(80, 120);
            uint8_t colorIdx = random(5);
            strip.setPixelColor(spark, strip.Color(
                colors[colorIdx][0] * pulses[spark] / 255,
                colors[colorIdx][1] * pulses[spark] / 255,
                colors[colorIdx][2] * pulses[spark] / 255
            ));
        }
        lastPulse = currentTime;
    }
}