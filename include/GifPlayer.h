#pragma once

#include <GifDecoder.h>

#include "DisplayConfig.h"

class LedMatrix;

class GifPlayer {
public:
  GifPlayer(LedMatrix& matrix, const char* directory, int chipSelectPin);

  bool begin();
  void update(unsigned long now);
  void requestNextImage();
  unsigned long cycleNumber();

private:
  static GifPlayer* activePlayer;
  static void clearCallback();
  static void updateCallback();
  static void drawPixelCallback(int16_t x, int16_t y, uint8_t red, uint8_t green, uint8_t blue);
  int chooseNextFile();

  LedMatrix& matrix;
  const char* directory;
  int chipSelectPin;
  GifDecoder<DisplayConfig::Width, DisplayConfig::Height, DisplayConfig::GifColorDepth> decoder;
  int fileCount = 0;
  int lastFileIndex = -1;
  bool playNextGif = true;
  unsigned long displayStartedAt = 0;
  uint32_t lastFrameDisplayedAt = 0;
  unsigned int currentFrameDelay = 0;
};
