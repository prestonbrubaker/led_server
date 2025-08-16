#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Adafruit_NeoPixel.h>
#include <math.h>
#include <algorithm>
#include <ArduinoJson.h>

// Wi-Fi credentials
const char *ssid = "BrubakerWifi";
const char *password = "Pre$ton01";

// Flask server URL
const char *serverUrl = "http://192.168.1.126:5000/mode";

// Quantum server details for QRNG mode
IPAddress quantumServerIP(192, 168, 1, 180);
const int quantumServerPort = 6011;

// LED strip configuration
#define NUM_LEDS 300
#define DATA_PIN 2    // GPIO2
#define BRIGHTNESS 50 // 0-255
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, DATA_PIN, NEO_GRB + NEO_KHZ800);

// Current mode and timing
String currentMode = "off";
unsigned long lastPoll = 0;
const unsigned long pollInterval = 2000; // Poll every 2 seconds
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 30; // ~33 FPS

// QRNG mode variables
int quantumCursor = 0;
unsigned long quantumLastPoll = 0;
const unsigned long quantumPollInterval = 1000; // Poll every 1s for batch
const int quantumBatchSize = 10;                // Fetch 10 states at a time
int quantumStates[quantumBatchSize] = {0};      // Buffer for batched states
int quantumStateIndex = 0;                      // Current index in buffer
int quantumStatesAvailable = 0;                 // Number of states in buffer

void setup()
{
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
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    Serial.println("IP address: " + WiFi.localIP().toString());
}

void loop()
{
    // Check Wi-Fi connection
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi disconnected, reconnecting...");
        WiFi.reconnect();
        delay(2000);
        return;
    }

    // Poll server for mode updates
    if (millis() - lastPoll >= pollInterval)
    {
        String newMode = getModeFromServer();
        if (newMode != "" && newMode != currentMode)
        {
            currentMode = newMode;
            Serial.println("New mode: " + currentMode);
            setLedsOff();
            resetModeState(); // Reset mode-specific state
        }
        lastPoll = millis();
    }

    // Update LED pattern based on mode
    if (millis() - lastUpdate >= updateInterval)
    {
        if (currentMode == "rainbow-flow") {
            rainbowFlow();
        }
else if (currentMode == "constant-red") {
            setLedsRed();
        }
        else if (currentMode == "off") {
            setLedsOff();
        }
        else if (currentMode == "proletariat-crackle") {
            proletariatCrackle();
        }
        else if (currentMode == "soma-haze") {
            somaHaze();
        }
        else if (currentMode == "loonie-freefall") {
            loonieFreefall();
        }
        else if (currentMode == "bokanovsky-burst") {
            bokanovskyBurst();
        }
        else if (currentMode == "total-perspective-vortex") {
            totalPerspectiveVortex();
        }
        else if (currentMode == "golgafrincham-drift") {
            golgafrinchamDrift();
        }
        else if (currentMode == "bistromathics-surge") {
            bistromathicsSurge();
        }
        else if (currentMode == "groks-dissolution") {
            groksDissolution();
        }
        else if (currentMode == "newspeak-shrink") {
            newspeakShrink();
        }
        else if (currentMode == "nolite-te-bastardes") {
            noliteTeBastardes();
        }
        else if (currentMode == "infinite-improbability-drive") {
            infiniteImprobabilityDrive();
        }
        else if (currentMode == "big-brother-glare") {
            bigBrotherGlare();
        }
        else if (currentMode == "replicant-retirement") {
            replicantRetirement();
        }
        else if (currentMode == "water-brother-bond") {
            waterBrotherBond();
        }
        else if (currentMode == "hypnopaedia-hum") {
            hypnopaediaHum();
        }
        else if (currentMode == "vogon-poetry-pulse") {
            vogonPoetryPulse();
        }
        else if (currentMode == "thought-police-flash") {
            thoughtPoliceFlash();
        }
        else if (currentMode == "electric-sheep-dream") {
            electricSheepDream();
        }
        else if (currentMode == "QRNG") {
            quantumRandom();
        }
        strip.show();
        lastUpdate = millis();
    }
}

String getModeFromServer()
{
    WiFiClient client;
    HTTPClient http;
    String mode = "";
    http.setTimeout(5000); // 5s timeout
    int retries = 3;

    Serial.print("Attempting HTTP GET to: ");
    Serial.println(serverUrl);
    if (http.begin(client, serverUrl))
    {
        while (retries > 0)
        {
            int httpCode = http.GET();
            Serial.print("HTTP Code: ");
            Serial.println(httpCode);
            if (httpCode == HTTP_CODE_OK)
            {
                mode = http.getString();
                mode.trim();
                Serial.println("Received mode: " + mode);
                break;
            }
            else
            {
                Serial.printf("HTTP GET failed, error: %s\n", http.errorToString(httpCode).c_str());
                retries--;
                delay(1000);
            }
        }
        http.end();
    }
    else
    {
        Serial.println("Unable to connect to server");
    }
    return mode;
}

void resetModeState()
{
    // Reset mode-specific static variables to avoid glitches
    static uint8_t dummyBuffer[NUM_LEDS] = {0};
    memcpy(dummyBuffer, dummyBuffer, NUM_LEDS); // Clear buffer
    quantumCursor = 0;
    quantumStateIndex = 0;
    quantumStatesAvailable = 0;
}

void setPixel(int pixel, byte red, byte green, byte blue)
{
    strip.setPixelColor(pixel, strip.Color(red, green, blue));
}

void setAll(byte red, byte green, byte blue)
{
    for (int i = 0; i < NUM_LEDS; i++)
    {
        strip.setPixelColor(i, strip.Color(red, green, blue));
    }
}

void quantumRandom()
{
    // If buffer is empty, try to fetch new states
    if (quantumStatesAvailable == 0 && millis() - quantumLastPoll >= quantumPollInterval)
    {
        quantumStatesAvailable = getQuantumStatesFromServer();
        quantumStateIndex = 0;
        quantumLastPoll = millis();
    }

    // Apply next state if available
    if (quantumStatesAvailable > 0)
    {
        int state = quantumStates[quantumStateIndex];
        uint32_t color = (state == 1) ? strip.Color(200, 0, 0) : strip.Color(0, 0, 0); 
        strip.setPixelColor(quantumCursor, color);
        quantumCursor = (quantumCursor + 1) % NUM_LEDS;
        quantumStateIndex++;
        quantumStatesAvailable--;
    }
}

int getQuantumStatesFromServer()
{
    WiFiClient client;
    if (!client.connect(quantumServerIP, quantumServerPort))
    {
        Serial.println("Unable to connect to quantum server");
        return 0;
    }

    // Send request (assuming server expects a simple GET; modify if needed)
    client.print("GET /states HTTP/1.1\r\nHost: ");
    client.print(quantumServerIP.toString());
    client.print(":");
    client.print(quantumServerPort);
    client.print("\r\nConnection: close\r\n\r\n");

    // Read response
    String json = "";
    unsigned long timeout = millis() + 5000; // 5s timeout
    while (client.connected() && millis() < timeout)
    {
        if (client.available())
        {
            char c = client.read();
            json += c;
        }
    }
    client.stop();

    if (json.length() == 0)
    {
        Serial.println("No response from quantum server");
        return 0;
    }

    // Parse JSON
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, json);
    if (error)
    {
        Serial.printf("JSON parse error: %s\n", error.c_str());
        return 0;
    }

    JsonArray states = doc["states"].as<JsonArray>();
    int count = 0;
    for (JsonVariant v : states)
    {
        if (count < quantumBatchSize)
        {
            quantumStates[count] = v.as<int>();
            count++;
        }
    }
    Serial.printf("Received %d quantum states\n", count);
    return count;
}

void rainbowFlow()
{
    static uint16_t hue = 0;
    static uint8_t sparkles[NUM_LEDS] = {0};
    static uint8_t sparkleColors[NUM_LEDS][3] = {{0}};
    static unsigned long lastUpdate = 0;

    unsigned long currentTime = millis();
    if (currentTime - lastUpdate >= random(15, 30))
    {
        for (int i = 0; i < NUM_LEDS; i++)
        {
            sparkles[i] = max(0, sparkles[i] - 20);
            uint8_t r, g, b;
            if (sparkles[i] > 0)
            {
                r = sparkleColors[i][0];
                g = sparkleColors[i][1];
                b = sparkleColors[i][2];
            }
            else
            {
                uint16_t h = hue + (i * 65536L / NUM_LEDS);               // Full rainbow cycle
                uint8_t s = 255;                                          // Maximum saturation for deep colors
                uint8_t v = 100 + (sin((hue + i * 100) * 0.01) + 1) * 50; // Brightness 100-200
                uint32_t c = strip.ColorHSV(h, s, v);
                r = (c >> 16) & 0xFF;
                g = (c >> 8) & 0xFF;
                b = c & 0xFF;
            }
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        if (random(100) < 5)
        {
            for (int j = 0; j < random(1, 4); j++)
            {
                int spark = random(NUM_LEDS);
                sparkles[spark] = random(180, 255);
                uint8_t sparkType = random(3);
                if (sparkType == 0)
                {
                    sparkleColors[spark][0] = sparkles[spark]; // White
                    sparkleColors[spark][1] = sparkles[spark];
                    sparkleColors[spark][2] = sparkles[spark];
                }
                else if (sparkType == 1)
                { // Gold
                    sparkleColors[spark][0] = sparkles[spark];
                    sparkleColors[spark][1] = sparkles[spark] * 200 / 255;
                    sparkleColors[spark][2] = sparkles[spark] * 50 / 255;
                }
                else
                { // Neon pink
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

void setLedsRed()
{
    for (int i = 0; i < NUM_LEDS; i++)
    {
        strip.setPixelColor(i, strip.Color(255, 0, 0));
    }
}

void setLedsOff()
{
    for (int i = 0; i < NUM_LEDS; i++)
    {
        strip.setPixelColor(i, strip.Color(0, 0, 0));
    }
}

void proletariatCrackle()
{
    static uint8_t intensities[NUM_LEDS] = {0};
    static unsigned long lastCrackle = 0;

    unsigned long currentTime = millis();
    if (currentTime - lastCrackle >= random(30, 100))
    {
        for (int i = 0; i < NUM_LEDS; i++)
        {
            intensities[i] = std::max((uint8_t)0, (uint8_t)(intensities[i] - random(5, 15)));
            uint8_t r = std::min((uint8_t)255, (uint8_t)(255 * intensities[i] / 255));
            uint8_t g = intensities[i] / 10;
            strip.setPixelColor(i, strip.Color(r, g, 0));
        }
        for (int i = 0; i < 8; i++)
        {
            int led = random(NUM_LEDS);
            intensities[led] = random(50, 255);
        }
        lastCrackle = currentTime;
    }
}

void somaHaze()
{
    float time = millis() / 60000.0f; // Time in minutes for 60-second cycle
    float phase = time * 2 * M_PI;    // Full cycle every minute, continuous

    // Define super pink (hot pink)
    uint8_t pinkR = 255;
    uint8_t pinkG = 105;
    uint8_t pinkB = 180;

    // Define soft blue/purple (medium orchid)
    uint8_t blueR = 186;
    uint8_t blueG = 85;
    uint8_t blueB = 211;

    // Temporal blend oscillates smoothly between 0 (pink) and 1 (blue/purple)
    float baseBlend = (sin(phase) + 1.0f) / 2.0f; // 0 to 1, seamless cycle

    for (int i = 0; i < NUM_LEDS; i++)
    {
        // Normalized position along strip (0 to 1)
        float pos = (float)i / (NUM_LEDS - 1);

        // Sine-based spatial gradient, synchronized with temporal phase
        float spatialPhase = pos * M_PI + phase;                // Spatial wave moves with time
        float spatialBlend = (sin(spatialPhase) + 1.0f) / 2.0f; // Smooth wave across strip

        // Combine temporal and spatial blends for fluid gradient
        float blend = (1.0f - baseBlend) * (1.0f - spatialBlend) + baseBlend * spatialBlend;

        // Interpolate between pink and blue/purple
        uint8_t r = (uint8_t)(pinkR * (1.0f - blend) + blueR * blend);
        uint8_t g = (uint8_t)(pinkG * (1.0f - blend) + blueG * blend);
        uint8_t b = (uint8_t)(pinkB * (1.0f - blend) + blueB * blend);

        // Subtle ethereal pulsation for a dreamy effect
        float glow = (sin(phase * 0.3f + pos * M_PI * 0.8f) + 1.0f) / 2.0f * 0.15f + 0.85f; // Varies 0.85 to 1.0
        r = (uint8_t)(r * glow);
        g = (uint8_t)(g * glow);
        b = (uint8_t)(b * glow);

        strip.setPixelColor(i, strip.Color(r, g, b));
    }
}

void loonieFreefall()
{
    static uint8_t comets[10][3] = {0}; // pos, length, speed
    static unsigned long lastFall = 0;

    if (millis() - lastFall >= 25)
    {
        for (int i = 0; i < NUM_LEDS; i++)
        {
            strip.setPixelColor(i, strip.Color(0, 0, 20)); // Galactic background
        }
        for (int c = 0; c < 10; c++)
        {
            if (comets[c][0] == 0 && random(100) < 8)
            {
                comets[c][0] = 1;             // Start new comet
                comets[c][1] = random(5, 15); // Length
                comets[c][2] = random(2, 5);  // Speed
            }
            if (comets[c][0] > 0)
            {
                for (int t = 0; t < comets[c][1]; t++)
                {
                    int pos = comets[c][0] + t;
                    if (pos < NUM_LEDS)
                    {
                        uint8_t intensity = 255 - t * (255 / comets[c][1]);
                        uint8_t r = intensity;
                        uint8_t g = intensity / 2;
                        uint8_t b = intensity;
                        strip.setPixelColor(pos, strip.Color(r, g, b));
                    }
                }
                comets[c][0] += comets[c][2];
                if (comets[c][0] + comets[c][1] >= NUM_LEDS)
                {
                    comets[c][0] = 0;
                }
            }
        }
        lastFall = millis();
    }
}

void bokanovskyBurst()
{
    static int balls[8] = {0, 40, 80, 120, 160, 200, 240, 280};
    static int directions[8] = {2, -2, 3, -3, 2, -2, 4, -4};
    static unsigned long lastBounce = 0;

    if (millis() - lastBounce >= 20)
    {
        for (int i = 0; i < NUM_LEDS; i++)
        {
            strip.setPixelColor(i, strip.Color(50, 50, 50)); // Uniform base
        }
        for (int b = 0; b < 8; b++)
        {
            int pos = balls[b];
            if (pos >= 0 && pos < NUM_LEDS)
            {
                strip.setPixelColor(pos, strip.Color(255, 255, 0));
                for (int t = 1; t < 10; t++)
                {
                    int trail1 = pos - t * directions[b] / abs(directions[b]);
                    int trail2 = pos + t * directions[b] / abs(directions[b]);
                    uint8_t intensity = 255 - t * 25;
                    if (trail1 >= 0 && trail1 < NUM_LEDS)
                        strip.setPixelColor(trail1, strip.Color(intensity, intensity / 2, 0));
                    if (trail2 >= 0 && trail2 < NUM_LEDS)
                        strip.setPixelColor(trail2, strip.Color(intensity, intensity / 2, 0));
                }
            }
            balls[b] += directions[b];
            if (balls[b] <= 0 || balls[b] >= NUM_LEDS - 1)
            {
                directions[b] = -directions[b];
                for (int burst = -20; burst <= 20; burst++)
                {
                    int burstPos = balls[b] + burst;
                    if (burstPos >= 0 && burstPos < NUM_LEDS)
                    {
                        uint8_t br = random(200, 255);
                        uint8_t bg = random(100, 200);
                        uint8_t bb = random(0, 50);
                        strip.setPixelColor(burstPos, strip.Color(br, bg, bb));
                    }
                }
            }
        }
        lastBounce = millis();
    }
}

void totalPerspectiveVortex()
{
    static uint16_t marqueePos = 0;
    static unsigned long lastMarquee = 0;

    if (millis() - lastMarquee >= 15)
    {
        for (int i = 0; i < NUM_LEDS; i++)
        {
            uint16_t h = (marqueePos + i * 10) % 65536;
            uint32_t color = strip.ColorHSV(h, 255, 255);
            strip.setPixelColor(i, color);
        }
        marqueePos += 256;
        if (random(100) < 10)
        {
            int slingPos = random(NUM_LEDS);
            for (int s = 0; s < 50; s++)
            {
                int pos = (slingPos + s * 5) % NUM_LEDS;
                strip.setPixelColor(pos, strip.ColorHSV(random(65536), 255, 255));
            }
        }
        lastMarquee = millis();
    }
}

void golgafrinchamDrift()
{
    static uint8_t comets[NUM_LEDS] = {0};
    static int cometPositions[8] = {0, 37, 75, 112, 150, 187, 225, 262};
    static int cometSpeeds[8] = {1, 2, 1, 3, 2, 1, 4, 2};
    static int cometDirections[8] = {1, 1, 1, 1, 1, 1, 1, 1};
    static unsigned long lastSurge = 0;

    if (millis() - lastSurge >= 35)
    {
        for (int i = 0; i < NUM_LEDS; i++)
        {
            comets[i] = max(0, comets[i] - 15);
            uint8_t r = comets[i];
            uint8_t g = comets[i] / 2; // Comet surges for Golgafrincham ship drift, orange trails
            uint8_t b = 0;
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        for (int c = 0; c < 8; c++)
        {
            cometPositions[c] = (cometPositions[c] + cometSpeeds[c] * cometDirections[c] + NUM_LEDS) % NUM_LEDS;
            comets[cometPositions[c]] = 255;
            for (int tail = 1; tail < 8; tail++)
            {
                int tailPos = (cometPositions[c] - tail * cometSpeeds[c] * cometDirections[c] + NUM_LEDS) % NUM_LEDS;
                comets[tailPos] = max(comets[tailPos], (uint8_t)(255 - tail * 30));
            }
            if (random(100) < 2)
            {
                cometDirections[c] = -cometDirections[c];
                cometSpeeds[c] = random(1, 5);
            }
        }
        lastSurge = millis();
    }
}

void bistromathicsSurge()
{
    static int ballPositions[5] = {0, 60, 120, 180, 240};
    static int ballDirections[5] = {1, -1, 1, -1, 1};
    static uint8_t intensities[NUM_LEDS] = {0};
    static unsigned long lastBounce = 0;

    if (millis() - lastBounce >= 25)
    {
        memset(intensities, 0, sizeof(intensities));
        for (int b = 0; b < 5; b++)
        {
            intensities[ballPositions[b]] = 255;
            // Bouncing ball trails for mathematical chaos
            for (int t = 1; t < 6; t++)
            {
                int trailPos = ballPositions[b] - t * ballDirections[b] * 2;
                if (trailPos >= 0 && trailPos < NUM_LEDS)
                {
                    intensities[trailPos] = max(intensities[trailPos], (uint8_t)(255 - t * 40));
                }
            }
            ballPositions[b] += ballDirections[b] * 3;
            if (ballPositions[b] >= NUM_LEDS - 1 || ballPositions[b] <= 0)
            {
                ballDirections[b] = -ballDirections[b];
            }
        }
        for (int i = 0; i < NUM_LEDS; i++)
        {
            uint8_t r = intensities[i] * (random(2)); // Chaotic colors
            uint8_t g = intensities[i] * (random(2));
            uint8_t b = intensities[i] * (random(2));
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        lastBounce = millis();
    }
}

void groksDissolution()
{
    static uint8_t slings[4] = {0, 75, 150, 225};
    static int slingDirs[4] = {5, -4, 6, -5};
    static unsigned long lastSling = 0;

    if (millis() - lastSling >= 30)
    {
        for (int i = 0; i < NUM_LEDS; i++)
        {
            strip.setPixelColor(i, strip.Color(0, 20, 0)); // Deep green base
        }
        for (int s = 0; s < 4; s++)
        {
            int pos = slings[s];
            if (pos >= 0 && pos < NUM_LEDS)
            {
                strip.setPixelColor(pos, strip.Color(50, 255, 50));
                for (int t = 1; t < 15; t++)
                {
                    int trail = pos - t * slingDirs[s] / abs(slingDirs[s]);
                    if (trail >= 0 && trail < NUM_LEDS)
                    {
                        uint8_t intensity = 200 - t * 13;
                        strip.setPixelColor(trail, strip.Color(20, intensity, 20));
                    }
                }
            }
            slings[s] += slingDirs[s];
            if (slings[s] <= 0 || slings[s] >= NUM_LEDS - 1)
            {
                slingDirs[s] = -slingDirs[s];
                int rippleCenter = slings[s];
                for (int r = 0; r < 30; r++)
                {
                    int left = rippleCenter - r;
                    int right = rippleCenter + r;
                    if (left >= 0)
                        strip.setPixelColor(left, strip.Color(100, 255, 100));
                    if (right < NUM_LEDS)
                        strip.setPixelColor(right, strip.Color(100, 255, 100));
                }
            }
        }
        lastSling = millis();
    }
}

void newspeakShrink()
{
    static uint8_t intensities[NUM_LEDS] = {0};
    static int leftPos = 0;
    static int rightPos = NUM_LEDS - 1;
    static bool converging = true;
    static unsigned long lastShrink = 0;

    if (millis() - lastShrink >= 30)
    {
        for (int i = 0; i < NUM_LEDS; i++)
        {
            intensities[i] = max(0, intensities[i] - 10);
            uint8_t r = intensities[i] * (i % 3 == 0 ? 0.5 : 0);
            uint8_t g = intensities[i] * (i % 3 == 1 ? 0.5 : 0);
            uint8_t b = intensities[i]; // Blues and grays shrinking
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        if (converging)
        {
            for (int d = 0; d < 10; d++)
            {
                if (leftPos + d < NUM_LEDS)
                    intensities[leftPos + d] = (uint8_t)(255 - d * 20);
                if (rightPos - d >= 0)
                    intensities[rightPos - d] = (uint8_t)(255 - d * 20);
            }
            leftPos += 5;
            rightPos -= 5;
            if (leftPos >= rightPos)
            {
                converging = false;
            }
        }
        else
        {
            for (int d = 0; d < 10; d++)
            {
                if (leftPos - d >= 0)
                    intensities[leftPos - d] = (uint8_t)(255 - d * 20);
                if (rightPos + d < NUM_LEDS)
                    intensities[rightPos + d] = (uint8_t)(255 - d * 20);
            }
            leftPos -= 5;
            rightPos += 5;
            if (leftPos <= 0 || rightPos >= NUM_LEDS - 1)
            {
                converging = true;
            }
        }
        lastShrink = millis();
    }
}

void noliteTeBastardes()
{
    static int slingPositions[6] = {0, 50, 100, 150, 200, 250};
    static int slingSpeeds[6] = {4, -5, 6, -4, 5, -6};
    static unsigned long lastSling = 0;

    if (millis() - lastSling >= 25)
    {
        for (int i = 0; i < NUM_LEDS; i++)
        {
            strip.setPixelColor(i, strip.Color(20, 0, 0)); // Dark red base
        }
        for (int s = 0; s < 6; s++)
        {
            int pos = slingPositions[s];
            if (pos >= 0 && pos < NUM_LEDS)
            {
                strip.setPixelColor(pos, strip.Color(255, 100, 0));
                for (int t = 1; t < 12; t++)
                {
                    int trail = pos - t * slingSpeeds[s] / abs(slingSpeeds[s]);
                    if (trail >= 0 && trail < NUM_LEDS)
                    {
                        uint8_t intensity = 220 - t * 18;
                        strip.setPixelColor(trail, strip.Color(intensity, intensity / 3, 0));
                    }
                }
            }
            slingPositions[s] += slingSpeeds[s];
            if (slingPositions[s] <= 0 || slingPositions[s] >= NUM_LEDS - 1)
            {
                slingSpeeds[s] = -slingSpeeds[s];
                int burstCenter = slingPositions[s];
                for (int b = -15; b <= 15; b++)
                {
                    int burstPos = burstCenter + b;
                    if (burstPos >= 0 && burstPos < NUM_LEDS)
                    {
                        strip.setPixelColor(burstPos, strip.Color(255, random(50, 150), 0));
                    }
                }
            }
        }
        lastSling = millis();
    }
}

void infiniteImprobabilityDrive()
{
    static uint16_t hue = 0;
    static unsigned long lastShift = 0;

    if (millis() - lastShift >= 20)
    {
        for (int i = 0; i < NUM_LEDS; i++)
        {
            uint32_t c = strip.ColorHSV(hue + random(65536 / NUM_LEDS), 255, 255); // Random hue shifts for improbability, HHGTTG style
            strip.setPixelColor(i, c);
        }
        if (random(100) < 10)
        {
            hue = random(65536); // Sudden "drive" jumps
        }
        else
        {
            hue += 500;
        }
        lastShift = millis();
    }
}

void bigBrotherGlare()
{
    static uint8_t eyes[NUM_LEDS] = {0};
    static unsigned long lastGlare = 0;

    if (millis() - lastGlare >= 50)
    {
        for (int i = 0; i < NUM_LEDS; i++)
        {
            eyes[i] = max(0, eyes[i] - 10);
            uint8_t r = eyes[i];
            uint8_t g = 0;
            uint8_t b = 0; // Red glare for 1984 surveillance
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        // Periodic "eyes" lighting up
        for (int e = 0; e < 3; e++)
        {
            int pos = random(NUM_LEDS);
            eyes[pos] = 255;
            eyes[(pos + 1) % NUM_LEDS] = 200; // Paired eyes
        }
        lastGlare = millis();
    }
}

void replicantRetirement()
{
    static int pulseCenters[5] = {0, 60, 120, 180, 240};
    static uint8_t pulseRadii[5] = {0};
    static unsigned long lastPulse = 0;

    if (millis() - lastPulse >= 25)
    {
        for (int i = 0; i < NUM_LEDS; i++)
        {
            uint8_t intensity = 0;
            for (int p = 0; p < 5; p++)
            {
                int dist = abs(i - pulseCenters[p]);
                if (dist <= pulseRadii[p])
                {
                    intensity = max(intensity, (uint8_t)(255 - dist * 8));
                }
            }
            uint8_t r = intensity / 2;
            uint8_t g = intensity / 2;
            uint8_t b = intensity;
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        for (int p = 0; p < 5; p++)
        {
            pulseRadii[p] += 2;
            if (pulseRadii[p] >= 40 || random(100) < 10)
            {
                pulseCenters[p] = random(NUM_LEDS);
                pulseRadii[p] = 0;
            }
        }
        lastPulse = millis();
    }
}

void waterBrotherBond()
{
    static int balls[10] = {0};
    static int dirs[10] = {0};
    static unsigned long lastBounce = 0;

    if (millis() - lastBounce >= 20)
    {
        for (int i = 0; i < NUM_LEDS; i++)
        {
            strip.setPixelColor(i, strip.Color(0, 50, 100)); // Bond base
        }
        for (int b = 0; b < 10; b++)
        {
            if (dirs[b] == 0)
            {
                balls[b] = random(NUM_LEDS);
                dirs[b] = random(2) ? 3 : -3;
            }
            int pos = balls[b];
            strip.setPixelColor(pos, strip.Color(0, 255, 255));
            for (int t = 1; t < 8; t++)
            {
                int trail = pos - t * dirs[b] / 3;
                if (trail >= 0 && trail < NUM_LEDS)
                {
                    uint8_t intensity = 200 - t * 25;
                    strip.setPixelColor(trail, strip.Color(0, intensity, intensity));
                }
            }
            balls[b] += dirs[b];
            if (balls[b] <= 0 || balls[b] >= NUM_LEDS - 1)
            {
                dirs[b] = -dirs[b];
                int ripple = balls[b];
                for (int r = -20; r <= 20; r++)
                {
                    int rp = ripple + r;
                    if (rp >= 0 && rp < NUM_LEDS)
                    {
                        strip.setPixelColor(rp, strip.Color(100, 255, 255));
                    }
                }
            }
        }
        lastBounce = millis();
    }
}

void hypnopaediaHum()
{
    static uint16_t marqueePos = 0;
    static unsigned long lastHum = 0;

    if (millis() - lastHum >= 40)
    {
        for (int i = 0; i < NUM_LEDS; i++)
        {
            float hum = (sin((float)(i + marqueePos) * 0.05f) + 1.0f) / 2.0f;
            uint8_t r = (uint8_t)(100 * hum);
            uint8_t g = (uint8_t)(150 * hum);
            uint8_t b = (uint8_t)(200 * hum);
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        marqueePos += 2;
        if (random(100) < 10)
        {
            int slingStart = random(NUM_LEDS);
            for (int s = 0; s < 40; s++)
            {
                int pos = (slingStart + s * 4) % NUM_LEDS;
                strip.setPixelColor(pos, strip.Color(255, 255, 255));
            }
        }
        lastHum = millis();
    }
}

void vogonPoetryPulse()
{
    static uint8_t ripples[NUM_LEDS] = {0};
    static int rippleCenters[4] = {0};
    static unsigned long lastRipple = 0;

    if (millis() - lastRipple >= 60)
    {
        for (int i = 0; i < NUM_LEDS; i++)
        {
            ripples[i] = max(0, ripples[i] - 8);
            uint8_t r = ripples[i] / 2;
            uint8_t g = ripples[i] * 3 / 4;
            uint8_t b = ripples[i] / 3;
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        for (int rc = 0; rc < 4; rc++)
        {
            if (random(100) < 25)
            {
                rippleCenters[rc] = random(NUM_LEDS);
            }
            for (int d = 0; d < 20; d++)
            {
                int left = rippleCenters[rc] - d;
                int right = rippleCenters[rc] + d;
                uint8_t intensity = 150 - d * 7;
                if (left >= 0)
                    ripples[left] = max(ripples[left], intensity);
                if (right < NUM_LEDS)
                    ripples[right] = max(ripples[right], intensity);
            }
        }
        lastRipple = millis();
    }
}

void thoughtPoliceFlash()
{
    static uint8_t flashes[NUM_LEDS] = {0};
    static uint8_t flameIntensities[20] = {0};
    static unsigned long lastFlash = 0;

    if (millis() - lastFlash >= 25)
    {
        for (int i = 0; i < NUM_LEDS; i++)
        {
            flashes[i] = max(0, flashes[i] - 20);
            uint8_t r = 0;
            uint8_t g = 0;
            uint8_t b = flashes[i]; // Blue flashes
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        if (random(100) < 20)
        {
            int flashPos = random(NUM_LEDS);
            flashes[flashPos] = 255;
            // Flash cluster for police "raid"
            for (int c = -3; c <= 3; c++)
            {
                int idx = (flashPos + c + NUM_LEDS) % NUM_LEDS;
                flashes[idx] = max(flashes[idx], (uint8_t)(200 - abs(c) * 30));
            }
        }
        // Enhanced flame effects at ends with pulsing
        for (int f = 0; f < 20; f++)
        {
            flameIntensities[f] = random(150, 255) * (sin((float)millis() / 200.0f + f * 0.5f) + 1.0f) / 2.0f;
            uint8_t fr = flameIntensities[f];
            uint8_t fg = flameIntensities[f] / 2 + random(0, 50);
            uint8_t fb = random(0, 20);
            strip.setPixelColor(f, strip.Color(fr, fg, fb));
            strip.setPixelColor(NUM_LEDS - 1 - f, strip.Color(fr, fg, fb));
        }
        lastFlash = millis();
    }
}

void electricSheepDream()
{
    static uint8_t rippleCenters[5] = {0};
    static uint8_t rippleRadii[5] = {0};
    static unsigned long lastRipple = 0;

    if (millis() - lastRipple >= 50)
    {
        for (int i = 0; i < NUM_LEDS; i++)
        {
            uint8_t intensity = 0;
            for (int r = 0; r < 5; r++)
            {
                int dist = abs(i - rippleCenters[r]);
                if (dist <= rippleRadii[r])
                {
                    intensity = max(intensity, (uint8_t)(255 - dist * 10));
                }
            }
            uint8_t r = 0;
            uint8_t g = min(255, intensity * 3 / 2);
            uint8_t b = intensity / 4;
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        for (int r = 0; r < 5; r++)
        {
            rippleRadii[r] = min(30, rippleRadii[r] + 1);
            if (rippleRadii[r] >= 30 || random(100) < 5)
            {
                rippleCenters[r] = random(NUM_LEDS);
                rippleRadii[r] = 0;
            }
        }
        lastRipple = millis();
    }
}