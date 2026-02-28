#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// WiFi
const char *ssid = "BrubakerWifi";
const char *password = "Pre$ton01";

// Matrix — YOUR ORIGINAL WORKING CONFIG
#define PIN 2
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(32, 8, PIN,
                                               NEO_MATRIX_BOTTOM + NEO_MATRIX_RIGHT +
                                                   NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
                                               NEO_GRB + NEO_KHZ800);

// Globals
String scrollText = "Loading...                     ";
int16_t textX = matrix.width();
const uint8_t LETTER_SPACING = 1;
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 30000;

// Colors
uint16_t WHITE = matrix.Color(255, 255, 255);
uint16_t GREEN = matrix.Color(0, 255, 0);
uint16_t RED = matrix.Color(255, 0, 0);

void setup()
{
    matrix.begin();
    matrix.setRotation(4);
    matrix.setTextWrap(false);
    matrix.setBrightness(25);
    matrix.setFont();
    matrix.clear();
    matrix.show();

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
        delay(500);

    fetchPrices();
    lastUpdate = millis();
}

void loop()
{
    if (millis() - lastUpdate >= updateInterval)
    {
        fetchPrices();
        lastUpdate = millis();
    }

    matrix.fillScreen(0);
    int16_t x = textX;

    // State for coloring change blocks
    bool inChange = false;
    uint16_t changeColor = WHITE;

    for (size_t i = 0; i < scrollText.length(); i++)
    {
        char c = scrollText[i];
        uint16_t color = WHITE;

        if (c == '(')
        {
            if (i + 1 < scrollText.length())
            {
                if (scrollText[i + 1] == '+')
                {
                    changeColor = GREEN;
                }
                else if (scrollText[i + 1] == '-')
                {
                    changeColor = RED;
                }
                else
                {
                    changeColor = WHITE;
                }
            }
            color = changeColor;
            inChange = true;
        }
        else if (inChange)
        {
            color = changeColor;
            if (c == ')')
            {
                inChange = false;
            }
        }

        matrix.setTextColor(color);
        matrix.setCursor(x, 0);
        matrix.print(c);

        x += 6 + LETTER_SPACING;
    }

    textX--;
    if (textX < -((int)scrollText.length() * 7))
    {
        textX = matrix.width();
    }

    matrix.show();
    delay(1);
}

// -----------------------------------------------------------
// Fetch prices + 24h % change for TSLAx, SOL, BTC, UBER, SPYx
// (LYFT removed — no tokenized version exists on CoinGecko)
// -----------------------------------------------------------
void fetchPrices()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        scrollText = "No WiFi                         ";
        return;
    }

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    // Updated IDs — Uber now uses the working Ondo version
    String url = "https://api.coingecko.com/api/v3/simple/price?ids=tesla-xstock,solana,bitcoin,uber-ondo-tokenized-stock,sp500-xstock&vs_currencies=usd&include_24hr_change=true";
    http.begin(client, url);

    if (http.GET() == HTTP_CODE_OK)
    {
        String payload = http.getString();
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);

        float tslaPrice = doc["tesla-xstock"]["usd"];
        float tslaChange = doc["tesla-xstock"]["usd_24h_change"];
        float solPrice = doc["solana"]["usd"];
        float solChange = doc["solana"]["usd_24h_change"];
        float btcPrice = doc["bitcoin"]["usd"];
        float btcChange = doc["bitcoin"]["usd_24h_change"];
        float uberPrice = doc["uber-ondo-tokenized-stock"]["usd"];
        float uberChange = doc["uber-ondo-tokenized-stock"]["usd_24h_change"];
        float spyxPrice = doc["sp500-xstock"]["usd"];
        float spyxChange = doc["sp500-xstock"]["usd_24h_change"];

        scrollText = "TSLAx $" + String((int)(tslaPrice + 0.5)) +
                     " (" + (tslaChange > 0 ? "+" : "") + String(tslaChange, 1) + "%) " +
                     "SOL $" + String(solPrice, 2) +
                     " (" + (solChange > 0 ? "+" : "") + String(solChange, 1) + "%) " +
                     "BTC $" + String((int)(btcPrice + 0.5)) +
                     " (" + (btcChange > 0 ? "+" : "") + String(btcChange, 1) + "%) " +
                     "UBER $" + String(uberPrice, 2) +
                     " (" + (uberChange > 0 ? "+" : "") + String(uberChange, 1) + "%) " +
                     "SPYx $" + String((int)(spyxPrice + 0.5)) +
                     " (" + (spyxChange > 0 ? "+" : "") + String(spyxChange, 1) + "%)   ";
    }
    else
    {
        scrollText = "API Error                       ";
    }
    http.end();
}
