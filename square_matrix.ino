#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#define PIN 2 // Data pin from your reference
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(16, 16, PIN,
  NEO_MATRIX_BOTTOM + NEO_MATRIX_LEFT + // Changed from RIGHT to LEFT to flip horizontally and un-mirror
  NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
  NEO_GRB + NEO_KHZ800);
String firstText = "TANSTAAFL"; // First row text
String secondText = "GOSPODIN"; // Second row text
int16_t textX = matrix.width(); // Starting position off-screen to the right
const uint8_t charWidth = 6; // 5 pixels per char + 1 spacing
// Function to generate rainbow colors (adapted from Adafruit examples)
uint16_t Wheel(byte WheelPos) {
WheelPos = 255 - WheelPos;
if (WheelPos < 85) {
return matrix.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
if (WheelPos < 170) {
WheelPos -= 85;
return matrix.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
WheelPos -= 170;
return matrix.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
void setup() {
matrix.begin();
matrix.setRotation(3); // Kept as-is; try 3 if the LEFT change alone doesn't fully fix orientation
matrix.setTextWrap(false);
matrix.setBrightness(25); // Low to avoid overpowering; increase cautiously
matrix.setFont(); // Default font (5x7 pixels)
matrix.clear();
matrix.show();
}
void loop() {
matrix.fillScreen(0); // Clear the matrix
int16_t maxTextLength = max(firstText.length(), secondText.length()) * charWidth;

// Draw first row (top, scooted up)
for (size_t i = 0; i < firstText.length(); ++i) {
int16_t charX = textX + (i * charWidth);
    // Skip drawing if the character is completely off-screen
if (charX >= matrix.width() || (charX + charWidth - 1) < 0) continue;
    // Calculate hue for this character (spreads rainbow across the whole word)
    byte hue = (i * 255) / (firstText.length() - 1);
uint16_t color = Wheel(hue);
matrix.setTextColor(color);
matrix.setCursor(charX, 0); // Scooted up to top
matrix.print(firstText[i]);
  }

// Draw second row (bottom)
for (size_t i = 0; i < secondText.length(); ++i) {
int16_t charX = textX + (i * charWidth);
    // Skip drawing if the character is completely off-screen
if (charX >= matrix.width() || (charX + charWidth - 1) < 0) continue;
    // Calculate hue for this character (spreads rainbow across the whole word)
    byte hue = (i * 255) / (secondText.length() - 1);
uint16_t color = Wheel(hue);
matrix.setTextColor(color);
matrix.setCursor(charX, 8); // Positioned below the first row
matrix.print(secondText[i]);
  }

matrix.show();
  // Scroll left by 1 pixel
textX--;
if (textX < -maxTextLength) {
textX = matrix.width(); // Reset to start again
  }
delay(50); // Adjust for scroll speed (lower = faster)
}