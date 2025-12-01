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

// Holdings
#define BTC_HOLDINGS 0.00003484
#define RVN_HOLDINGS 0.000
#define ZEC_HOLDINGS 0.00061431
#define ETH_HOLDINGS 0.00109387
#define DOGE_HOLDINGS 20.20737449
#define PEPE_HOLDINGS 204335.86337954

// Matrix — YOUR ORIGINAL WORKING CONFIG (keep this!)
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
    matrix.setBrightness(25); // As tweaked
    matrix.setFont();         // default 8px font
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

        // Simple width advance — default font is ~6px wide on average
        x += 6 + LETTER_SPACING; // 6px char + 1px space = perfect tight spacing
    }

    textX--;
    if (textX < -((int)scrollText.length() * 7))
    {
        textX = matrix.width();
    }

    matrix.show();
    delay(10); // As tweaked
}

// -----------------------------------------------------------
// Fetch prices + holdings to nearest cent
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

    String url = "https://api.coingecko.com/api/v3/simple/price?ids=bitcoin,ravencoin,zcash,ethereum,dogecoin,tesla-xstock,pepe&vs_currencies=usd&include_24hr_change=true";
    http.begin(client, url);

    if (http.GET() == HTTP_CODE_OK)
    {
        String payload = http.getString();
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);

        float btcPrice = doc["bitcoin"]["usd"];
        float btcChange = doc["bitcoin"]["usd_24h_change"];
        float rvnPrice = doc["ravencoin"]["usd"];
        float rvnChange = doc["ravencoin"]["usd_24h_change"];
        float zecPrice = doc["zcash"]["usd"];
        float zecChange = doc["zcash"]["usd_24h_change"];
        float ethPrice = doc["ethereum"]["usd"];
        float ethChange = doc["ethereum"]["usd_24h_change"];
        float dogePrice = doc["dogecoin"]["usd"];
        float dogeChange = doc["dogecoin"]["usd_24h_change"];
        float pepePrice = doc["pepe"]["usd"];
        float pepeChange = doc["pepe"]["usd_24h_change"];

        float btcValue = btcPrice * BTC_HOLDINGS;
        float rvnValue = rvnPrice * RVN_HOLDINGS;
        float zecValue = zecPrice * ZEC_HOLDINGS;
        float ethValue = ethPrice * ETH_HOLDINGS;
        float dogeValue = dogePrice * DOGE_HOLDINGS;
        float pepeValue = pepePrice * PEPE_HOLDINGS;

        scrollText = "BTC $" + String((int)(btcPrice + 0.5)) +
                     " (" + (btcChange > 0 ? "+" : "") + String(btcChange, 1) + "%) " +
                     "RVN $" + String(rvnPrice, 4) +
                     " (" + (rvnChange > 0 ? "+" : "") + String(rvnChange, 1) + "%) " +
                     "ZEC $" + String(zecPrice, 4) +
                     " (" + (zecChange > 0 ? "+" : "") + String(zecChange, 1) + "%) " +
                     "ETH $" + String((int)(ethPrice + 0.5)) +
                     " (" + (ethChange > 0 ? "+" : "") + String(ethChange, 1) + "%) " +
                     "DOGE $" + String(dogePrice, 4) +
                     " (" + (dogeChange > 0 ? "+" : "") + String(dogeChange, 1) + "%) " +
                     "PEPE $" + String((int)(pepePrice + 0.5)) +
                     " (" + (pepeChange > 0 ? "+" : "") + String(pepeChange, 1) + "%) " +
                     "Holdings: BTC $" + String(btcValue, 2) +
                     " RVN $" + String(rvnValue, 2) +
                     " ZEC $" + String(zecValue, 2) +
                     " ETH $" + String(ethValue, 2) +
                     " DOGE $" + String(dogeValue, 2) +
                     " PEPE $" + String(pepeValue, 2) + "   ";
    }
    else
    {
        scrollText = "API Error                       ";
    }
    http.end();
}
