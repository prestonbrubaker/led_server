/******************* ESP32 — Time-of-Flight Sensor (reports to central server) *******************/
#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_VL53L0X.h>

const char* ssid = "BrubakerWifi2";
const char* password = "Pre$ton01";

// Central server (via your Cloudflare Tunnel)
const char* centralUrl = "http://willohwilloh.immenseaccumulationonline.online";

#define ONBOARD_LED 2

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

unsigned long lastReading = 0;
const unsigned long readingInterval = 50;   // read sensor every 50ms
int currentDistanceMM = 9999;

unsigned long lastPost = 0;
const unsigned long postInterval = 200;     // POST to central every 200ms (non-blocking enough)

void setup() {
  Serial.begin(115200);
  pinMode(ONBOARD_LED, OUTPUT);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  if (!lox.begin()) {
    Serial.println(F("VL53L0X not found!"));
    while (1);
  }

  Serial.println("✅ Sensor ready — posting distance to central server every 200ms");
}

void loop() {
  unsigned long now = millis();

  // --- Sensor reading (keep fast for responsive LED) ---
  if (now - lastReading < readingInterval) return;
  lastReading = now;

  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false);
  currentDistanceMM = (measure.RangeStatus != 4) ? measure.RangeMilliMeter : 9999;

  // --- Onboard LED feedback (same behavior as before) ---
  if (currentDistanceMM < 1000) {
    int t = map(constrain(currentDistanceMM, 50, 1000), 50, 1000, 50, 400);
    digitalWrite(ONBOARD_LED, !digitalRead(ONBOARD_LED));
    delay(t);
  } else {
    digitalWrite(ONBOARD_LED, LOW);
  }

  // --- Report to central server (every 200ms) ---
  if (now - lastPost >= postInterval && WiFi.status() == WL_CONNECTED) {
    lastPost = now;

    HTTPClient http;
    String url = String(centralUrl) + "/update";
    http.begin(url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String postData = "distance=" + String(currentDistanceMM);

    int httpCode = http.POST(postData);
    if (httpCode != 200) {
      Serial.printf("⚠️  POST failed (code %d)\n", httpCode);
    }
    http.end();
  }

  // Debug output
  Serial.println(currentDistanceMM);
}
