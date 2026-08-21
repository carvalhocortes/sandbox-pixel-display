#pragma once

#include <GifDecoder.h>

#include "DisplayConfig.h"

class LedMatrix;

class GifPlayer {
public:
  GifPlayer(LedMatrix& matrix, const char* directory, int chipSelectPin);

  bool begin();
  void update(unsigned long now);
  void requestCurrentImage();
  bool changeImage(int direction);
  bool saveCurrentImage();

private:
  static GifPlayer* activePlayer;
  static void clearCallback();
  static void updateCallback();
  static void drawPixelCallback(int16_t x, int16_t y, uint8_t red, uint8_t green, uint8_t blue);
  bool loadSavedImage();
  bool readSavedPath(char* path) const;

  LedMatrix& matrix;
  const char* directory;
  int chipSelectPin;
  GifDecoder<DisplayConfig::Width, DisplayConfig::Height, DisplayConfig::GifColorDepth> decoder;
  int fileCount = 0;
  int currentFileIndex = -1;
  bool playNextGif = true;
  bool eepromReady = false;
  uint32_t lastFrameDisplayedAt = 0;
  unsigned int currentFrameDelay = 0;
};
