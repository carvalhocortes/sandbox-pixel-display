#include "LedMatrix.h"

#include "DisplayConfig.h"

LedMatrix::LedMatrix(uint8_t dataPin, uint8_t width, uint8_t height)
    : dataPin(dataPin), width(width), height(height) {}

void LedMatrix::begin(uint8_t brightness) {
  FastLED.addLeds<WS2812, DisplayConfig::DataPin, GRB>(pixels, width * height);
  FastLED.setBrightness(brightness);
  clear();
}

void LedMatrix::clear() {
  FastLED.clear();
  FastLED.clearData();
  FastLED.showColor(CRGB::Black);
  FastLED.show();
}

void LedMatrix::show() {
  FastLED.show();
}

int LedMatrix::position(int x, int y) const {
  if (x < 1 || x > width || y < 1 || y > height) {
    return 0;
  }

  if (y % 2 == 0) {
    return ((y - 1) * width) + (x - 1);
  }

  return ((y - 1) * width) + (width - x);
}

void LedMatrix::setPixel(int x, int y, const CRGB& color) {
  pixels[position(x, y)] = color;
}

void LedMatrix::drawPixel(int x, int y, const CRGB& color) {
  setPixel(x, y, color);
}

void LedMatrix::drawGifPixel(
    int16_t x, int16_t y, uint8_t red, uint8_t green, uint8_t blue) {
  setPixel(x + 1, y + 1, CRGB(red, green, blue));
}

void LedMatrix::testColorsBars(unsigned long durationMs, uint8_t brightness) {
  FastLED.setBrightness(brightness);
  const CRGB colors[] = {
      CRGB(104, 104, 104), CRGB(180, 180, 180), CRGB(180, 180, 16),
      CRGB(16, 180, 180), CRGB(16, 180, 16), CRGB(180, 16, 180),
      CRGB(180, 16, 16), CRGB(16, 16, 180)};

  const int columns = 8;
  const int rows = height / 8;
  for (int y = 1; y <= height; y++) {
    for (int x = 1; x <= width; x++) {
      const int colorIndex = min(columns - 1, (x - 1) / (width / columns));
      const int rowBand = min(7, (y - 1) / rows);
      setPixel(x, y, colors[(colorIndex + rowBand) % columns]);
    }
  }

  show();
  delay(durationMs);
  clear();
}
