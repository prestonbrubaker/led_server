#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>

// Wi-Fi credentials
const char* ssid = "BrubakerWifi";
const char* password = "Pre$ton01";

// Server details
IPAddress serverIP(192, 168, 1, 180);
const int serverPort = 6011;

// LED strip configuration
#define NUM_LEDS 300
#define DATA_PIN 2 // GPIO2
#define BRIGHTNESS 50 // 0-255
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, DATA_PIN, NEO_GRB + NEO_KHZ800);

// Polling timing
unsigned long lastPoll = 0;
const unsigned long pollInterval = 10; // Poll every 10ms for quick updates

// Cursor for updating LEDs
int cursor = 0;

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

    // Poll server for state updates
    if (millis() - lastPoll >= pollInterval) {
        int state = getStateFromServer();
        if (state >= 0) {
            uint32_t color;
            if (state == 1) {
                color = strip.Color(255, 0, 0); // Red for 1
            } else {
                color = strip.Color(173, 216, 230); // Light blue for 0
            }
            strip.setPixelColor(cursor, color);
            strip.show();
            cursor = (cursor + 1) % NUM_LEDS;
        }
        lastPoll = millis();
    }
}

int getStateFromServer() {
    WiFiClient client;
    if (!client.connect(serverIP, serverPort)) {
        Serial.println("Unable to connect to server");
        return -1;
    }

    // Read response
    String json = "";
    unsigned long timeout = millis() + 5000; // 5s timeout
    while (client.connected() && millis() < timeout) {
        if (client.available()) {
            char c = client.read();
            json += c;
        }
    }
    client.stop();

    if (json.length() == 0) {
        Serial.println("No response from server");
        return -1;
    }

    // Parse JSON
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        Serial.printf("JSON parse error: %s\n", error.c_str());
        return -1;
    }

    int state = doc["state"].as<int>();
    Serial.printf("Received state: %d\n", state);
    return state;
}

void setLedsOff() {
    for (int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(0, 0, 0));
    }
}
