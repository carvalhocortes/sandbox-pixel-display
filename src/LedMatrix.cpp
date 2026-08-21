#include "LedMatrix.h"

#include "DisplayConfig.h"

namespace {
uint8_t toFastLedBrightness(uint8_t brightnessPercent) {
  return static_cast<uint8_t>(
      (static_cast<uint16_t>(brightnessPercent) * 255U + 50U) / 100U);
}
}

LedMatrix::LedMatrix(uint8_t width, uint8_t height)
    : width(width), height(height) {}

void LedMatrix::begin(uint8_t brightnessPercent) {
  FastLED.addLeds<WS2812, DisplayConfig::DataPin, GRB>(pixels, width * height);
  currentBrightnessPercent = brightnessPercent;
  FastLED.setBrightness(toFastLedBrightness(currentBrightnessPercent));
  clear();
}

void LedMatrix::setBrightness(uint8_t brightnessPercent) {
  currentBrightnessPercent = brightnessPercent;
  FastLED.setBrightness(toFastLedBrightness(currentBrightnessPercent));
  FastLED.show();
}

uint8_t LedMatrix::brightnessPercent() const {
  return currentBrightnessPercent;
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
    return -1;
  }

  if (y % 2 == 0) {
    return ((y - 1) * width) + (x - 1);
  }

  return ((y - 1) * width) + (width - x);
}

void LedMatrix::setPixel(int x, int y, const CRGB& color) {
  const int index = position(x, y);
  if (index < 0) {
    return;
  }

  pixels[index] = color;
}

void LedMatrix::drawPixel(int x, int y, const CRGB& color) {
  setPixel(x, y, color);
}

void LedMatrix::drawGifPixel(
    int16_t x, int16_t y, uint8_t red, uint8_t green, uint8_t blue) {
  setPixel(x + 1, y + 1, CRGB(red, green, blue));
}

void LedMatrix::testColorsBars(
    unsigned long durationMs, uint8_t brightnessPercent) {
  const uint8_t normalBrightnessPercent = currentBrightnessPercent;
  FastLED.setBrightness(toFastLedBrightness(brightnessPercent));
  const CRGB lineOne[] = {
      CRGB(104, 104, 104), CRGB(180, 180, 180), CRGB(180, 180, 16),
      CRGB(16, 180, 180), CRGB(16, 180, 16), CRGB(180, 16, 180),
      CRGB(180, 16, 16), CRGB(16, 16, 180)};
  const CRGB lineTwo[] = {
      CRGB(0, 255, 255), CRGB(16, 70, 103), CRGB(180, 180, 180),
      CRGB(180, 180, 180), CRGB(180, 180, 180), CRGB(180, 180, 180),
      CRGB(180, 180, 180), CRGB(0, 0, 255)};
  const CRGB lineThree[] = {
      CRGB(255, 255, 0), CRGB(72, 16, 118), CRGB::Black,
      CRGB(64, 64, 64), CRGB(128, 128, 128), CRGB(192, 192, 192),
      CRGB(255, 255, 255), CRGB(255, 0, 0)};
  const CRGB lineFour[] = {
      CRGB(64, 64, 64), CRGB::Black, CRGB::Black, CRGB(255, 255, 255),
      CRGB(255, 255, 255), CRGB::Black, CRGB::Black, CRGB::Black};

  const int columns = 8;
  const int columnWidth = width / columns;
  const int baseRowHeight = height / 8;
  const int firstBandEnd = baseRowHeight * 5;
  const int secondBandEnd = firstBandEnd + baseRowHeight;
  const int thirdBandEnd = secondBandEnd + baseRowHeight;

  for (int y = 1; y <= height; y++) {
    const CRGB* colors = y <= firstBandEnd ? lineOne :
                         y <= secondBandEnd ? lineTwo :
                         y <= thirdBandEnd ? lineThree : lineFour;
    for (int x = 1; x <= width; x++) {
      const int column = min(columns - 1, (x - 1) / columnWidth);
      setPixel(x, y, colors[column]);
    }
  }

  show();
  delay(durationMs);
  clear();
  FastLED.setBrightness(toFastLedBrightness(normalBrightnessPercent));
}
