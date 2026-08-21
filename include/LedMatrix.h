#pragma once

#include <FastLED.h>

class LedMatrix {
public:
  LedMatrix(uint8_t width, uint8_t height);

  void begin(uint8_t brightnessPercent);
  void setBrightness(uint8_t brightnessPercent);
  uint8_t brightnessPercent() const;
  void clear();
  void show();
  void drawPixel(int x, int y, const CRGB& color);
  void testColorsBars(unsigned long durationMs, uint8_t brightnessPercent);
  void drawGifPixel(int16_t x, int16_t y, uint8_t red, uint8_t green, uint8_t blue);

private:
  int position(int x, int y) const;
  void setPixel(int x, int y, const CRGB& color);

  uint8_t width;
  uint8_t height;
  uint8_t currentBrightnessPercent = 0;
  CRGB pixels[256];
};
