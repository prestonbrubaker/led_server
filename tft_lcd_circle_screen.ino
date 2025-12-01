#include <WiFi.h>              // WiFi lib
#include <HTTPClient.h>        // HTTP client
#include <Adafruit_GFX.h>      // GFX
#include <Adafruit_SPITFT.h>   // SPI TFT
#include <SPI.h>               // SPI
#include <Adafruit_GC9A01A.h>  // GC9A01A driver

// Your pins (unchanged)
#define TFT_CS    22
#define TFT_DC    16
#define TFT_RST   -1
#define TFT_MOSI  23
#define TFT_SCLK  18

Adafruit_GC9A01A tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// WiFi creds (your updated ones)
const char* ssid = "BrubakerWifi";
const char* password = "Pre$ton01";

// Server (your updated port)
const char* serverIP = "192.168.1.198";
const int serverPort = 9042;
const char* imagePath = "/image";

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  Serial.println("ESP32 Random Photo Display Starting...");

  // Init display
  tft.begin();
  tft.setRotation(0);  // Portrait; adjust if needed
  tft.fillScreen(GC9A01A_BLACK);  // Black bg during first fetch

  // Connect WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected! IP: " + WiFi.localIP().toString());

  // Initial fetch
  fetchAndDrawImage();
  Serial.println("Photo cycle starting—updates every 30s.");
}

void loop() {
  delay(10000);  // 10s interval
  fetchAndDrawImage();  // Fetch & draw new random photo
  Serial.println("Next update in 10s...");
}

void fetchAndDrawImage() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "http://" + String(serverIP) + ":" + String(serverPort) + imagePath;
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
      Serial.println("HTTP OK - Fetching random photo data...");
      WiFiClient* stream = http.getStreamPtr();
      uint8_t buffer[2];  // 2 bytes per RGB565 pixel
      int x = 0, y = 0;

      tft.fillScreen(GC9A01A_BLACK);  // Clear for new image

      while (http.connected() && (y < 240)) {
        size_t size = stream->available();
        if (size >= 2) {  // Ensure full pixel
          stream->readBytes(buffer, 2);
          uint16_t color = (buffer[0] << 8) | buffer[1];  // Big-endian unpack
          tft.drawPixel(x, y, color);

          x++;
          if (x >= 240) {
            x = 0;
            y++;
          }
        }
      }
      Serial.println("Random photo drawn! (240x240 pixels)");
    } else {
      Serial.println("HTTP fail: " + String(httpCode) + " - Check server");
      tft.fillScreen(GC9A01A_RED);  // Error color
    }
    http.end();
  } else {
    Serial.println("No WiFi - can't fetch");
    tft.fillScreen(GC9A01A_RED);
  }
}