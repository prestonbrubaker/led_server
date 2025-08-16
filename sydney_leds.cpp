#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>

// LED strip configuration
#define NUM_LEDS 300
#define DATA_PIN 2 // GPIO2
#define BRIGHTNESS 50 // 0-255
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, DATA_PIN, NEO_GRB + NEO_KHZ800);

// Current mode index
int currentModeIndex = 0;

// Timing
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 30; // ~33 FPS

void setup() {
    // Initialize Serial for debug
    Serial.begin(115200);
    delay(100);

    // Initialize LED strip
    strip.begin();
    strip.setBrightness(BRIGHTNESS);
    setLedsOff();
    strip.show();

    // Read and update mode from EEPROM
    EEPROM.begin(1);
    byte modeIndex = EEPROM.read(0);
    modeIndex = (modeIndex + 1) % 13; // 13 modes
    EEPROM.write(0, modeIndex);
    EEPROM.commit();

    currentModeIndex = modeIndex;

    Serial.print("Current mode index: ");
    Serial.println(currentModeIndex);
}

void loop() {
    // Update LED pattern based on mode
    if (millis() - lastUpdate >= updateInterval) {
        switch (currentModeIndex) {
            case 0:
                setLedsOff();
                break;
            case 1:
                setLedsRed();
                break;
            case 2:
                setLedsGreen();
                break;
            case 3:
                setLedsBlue();
                break;
            case 4:
                setLedsMagenta();
                break;
            case 5:
                turquoiseCamo();
                break;
            case 6:
                rainbowFlow();
                break;
            case 7:
                loonieFreefall();
                break;
            case 8:
                bistromathicsSurge();
                break;
            case 9:
                groksDissolution();
                break;
            case 10:
                infiniteImprobabilityDrive();
                break;
            case 11:
                vogonPoetryPulse();
                break;
            case 12:
                electricSheepDream();
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

void setLedsRed() {
    for (int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(255, 0, 0));
    }
}

void setLedsGreen() {
    for (int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(0, 255, 0));
    }
}

void setLedsBlue() {
    for (int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(0, 0, 255));
    }
}

void setLedsMagenta() {
    for (int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(255, 0, 255));
    }
}

void turquoiseCamo() {
    static int left_pos = 0;
    static int right_pos = NUM_LEDS - 1;
    static int dir_left = 1;
    static int dir_right = -1;
    static const int section_len = 15;
    static unsigned long lastMove = 0;
    unsigned long currentTime = millis();
    if (currentTime - lastMove >= 50) { // Adjust for speed
        // Update positions
        left_pos += dir_left;
        right_pos += dir_right;

        // Check collision
        if (left_pos + section_len - 1 >= right_pos - section_len + 1) {
            dir_left = -dir_left;
            dir_right = -dir_right;
        }

        // Check walls and reverse for left
        if (left_pos < 0) {
            left_pos = 0;
            dir_left = 1;
        } else if (left_pos > NUM_LEDS - section_len) {
            left_pos = NUM_LEDS - section_len;
            dir_left = -1;
        }

        // Check walls and reverse for right
        if (right_pos < section_len - 1) {
            right_pos = section_len - 1;
            dir_right = 1;
        } else if (right_pos > NUM_LEDS - 1) {
            right_pos = NUM_LEDS - 1;
            dir_right = -1;
        }

        lastMove = currentTime;
    }
    // Set turquoise background
    uint32_t turquoise = strip.Color(10, 42, 200);
    for (int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, turquoise);
    }
    // Set camo green sections with darker greens
    uint32_t greens[3] = {
        strip.Color(10, 40, 10),   // Darker forest green
        strip.Color(30, 40, 10),   // Darker olive drab
        strip.Color(25, 30, 15)    // Darker dark olive green
    };
    // Left section
    for (int i = 0; i < section_len; i++) {
        int led = left_pos + i;
        if (led >= 0 && led < NUM_LEDS) {
            strip.setPixelColor(led, greens[i % 3]);
        }
    }
    // Right section
    for (int i = 0; i < section_len; i++) {
        int led = right_pos - i;
        if (led >= 0 && led < NUM_LEDS) {
            strip.setPixelColor(led, greens[i % 3]);
        }
    }
}

void rainbowFlow() {
    static uint16_t hue = 0;
    static uint8_t sparkles[NUM_LEDS] = {0};
    static uint8_t sparkleColors[NUM_LEDS][3] = {{0}};
    static unsigned long lastInnerUpdate = 0;
    unsigned long currentTime = millis();
    if (currentTime - lastInnerUpdate >= random(15, 30)) {
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
        lastInnerUpdate = currentTime;
    }
}

void loonieFreefall() {
    static uint8_t comets[10][3] = {0}; // pos, length, speed
    static unsigned long lastFall = 0;
    if (millis() - lastFall >= 25) {
        for (int i = 0; i < NUM_LEDS; i++) {
            strip.setPixelColor(i, strip.Color(0, 0, 20)); // Galactic background
        }
        for (int c = 0; c < 10; c++) {
            if (comets[c][0] == 0 && random(100) < 8) {
                comets[c][0] = 1; // Start new comet
                comets[c][1] = random(5, 15); // Length
                comets[c][2] = random(2, 5); // Speed
            }
            if (comets[c][0] > 0) {
                for (int t = 0; t < comets[c][1]; t++) {
                    int pos = comets[c][0] + t;
                    if (pos < NUM_LEDS) {
                        uint8_t intensity = 255 - t * (255 / comets[c][1]);
                        uint8_t r = intensity;
                        uint8_t g = intensity / 2;
                        uint8_t b = intensity;
                        strip.setPixelColor(pos, strip.Color(r, g, b));
                    }
                }
                comets[c][0] += comets[c][2];
                if (comets[c][0] + comets[c][1] >= NUM_LEDS) {
                    comets[c][0] = 0;
                }
            }
        }
        lastFall = millis();
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
                    intensities[trailPos] = max(intensities[trailPos], (uint8_t)(255 - t * 40));
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
    static uint8_t slings[4] = {0, 75, 150, 225};
    static int slingDirs[4] = {5, -4, 6, -5};
    static unsigned long lastSling = 0;
    if (millis() - lastSling >= 30) {
        for (int i = 0; i < NUM_LEDS; i++) {
            strip.setPixelColor(i, strip.Color(0, 20, 0)); // Deep green base
        }
        for (int s = 0; s < 4; s++) {
            int pos = slings[s];
            if (pos >= 0 && pos < NUM_LEDS) {
                strip.setPixelColor(pos, strip.Color(50, 255, 50));
                for (int t = 1; t < 15; t++) {
                    int trail = pos - t * slingDirs[s] / abs(slingDirs[s]);
                    if (trail >= 0 && trail < NUM_LEDS) {
                        uint8_t intensity = 200 - t * 13;
                        strip.setPixelColor(trail, strip.Color(20, intensity, 20));
                    }
                }
            }
            slings[s] += slingDirs[s];
            if (slings[s] <= 0 || slings[s] >= NUM_LEDS - 1) {
                slingDirs[s] = -slingDirs[s];
                int rippleCenter = slings[s];
                for (int r = 0; r < 30; r++) {
                    int left = rippleCenter - r;
                    int right = rippleCenter + r;
                    if (left >= 0) strip.setPixelColor(left, strip.Color(100, 255, 100));
                    if (right < NUM_LEDS) strip.setPixelColor(right, strip.Color(100, 255, 100));
                }
            }
        }
        lastSling = millis();
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

void vogonPoetryPulse() {
    static uint8_t ripples[NUM_LEDS] = {0};
    static int rippleCenters[4] = {0};
    static unsigned long lastRipple = 0;
    if (millis() - lastRipple >= 60) {
        for (int i = 0; i < NUM_LEDS; i++) {
            ripples[i] = max(0, ripples[i] - 8);
            uint8_t r = ripples[i] / 2;
            uint8_t g = ripples[i] * 3 / 4;
            uint8_t b = ripples[i] / 3;
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        for (int rc = 0; rc < 4; rc++) {
            if (random(100) < 25) {
                rippleCenters[rc] = random(NUM_LEDS);
            }
            for (int d = 0; d < 20; d++) {
                int left = rippleCenters[rc] - d;
                int right = rippleCenters[rc] + d;
                uint8_t intensity = 150 - d * 7;
                if (left >= 0) ripples[left] = max(ripples[left], intensity);
                if (right < NUM_LEDS) ripples[right] = max(ripples[right], intensity);
            }
        }
        lastRipple = millis();
    }
}

void electricSheepDream() {
    static uint8_t rippleCenters[5] = {0};
    static uint8_t rippleRadii[5] = {0};
    static unsigned long lastRipple = 0;
    if (millis() - lastRipple >= 50) {
        for (int i = 0; i < NUM_LEDS; i++) {
            uint8_t intensity = 0;
            for (int r = 0; r < 5; r++) {
                int dist = abs(i - rippleCenters[r]);
                if (dist <= rippleRadii[r]) {
                    intensity = max(intensity, (uint8_t)(255 - dist * 10));
                }
            }
            uint8_t r = 0;
            uint8_t g = min(255, intensity * 3 / 2);
            uint8_t b = intensity / 4;
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        for (int r = 0; r < 5; r++) {
            rippleRadii[r] = min(30, rippleRadii[r] + 1);
            if (rippleRadii[r] >= 30 || random(100) < 5) {
                rippleCenters[r] = random(NUM_LEDS);
                rippleRadii[r] = 0;
            }
        }
        lastRipple = millis();
    }
}
