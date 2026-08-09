#include "GifPlayer.h"

#include <SD.h>

#include "FilenameFunctions.h"
#include "LedMatrix.h"

GifPlayer* GifPlayer::activePlayer = nullptr;

GifPlayer::GifPlayer(LedMatrix& matrix, const char* directory, int chipSelectPin)
    : matrix(matrix), directory(directory), chipSelectPin(chipSelectPin) {
  activePlayer = this;
}

bool GifPlayer::begin() {
  decoder.setScreenClearCallback(clearCallback);
  decoder.setUpdateScreenCallback(updateCallback);
  decoder.setDrawPixelCallback(drawPixelCallback);
  decoder.setFileSeekCallback(fileSeekCallback);
  decoder.setFilePositionCallback(filePositionCallback);
  decoder.setFileReadCallback(fileReadCallback);
  decoder.setFileReadBlockCallback(fileReadBlockCallback);
  decoder.setFileSizeCallback(fileSizeCallback);

  if (initFileSystem(chipSelectPin) < 0) {
    Serial.println("No SD card");
    return false;
  }

  fileCount = enumerateGIFFiles(directory, true);
  if (fileCount <= 0) {
    Serial.println(fileCount < 0 ? "No gifs directory" : "Empty gifs directory");
    return false;
  }

  randomSeed(analogRead(A0) ^ micros());

  return true;
}

int GifPlayer::chooseNextFile() {
  if (fileCount <= 1) {
    return 0;
  }

  int nextIndex;
  do {
    nextIndex = random(fileCount);
  } while (nextIndex == lastFileIndex);

  lastFileIndex = nextIndex;
  return nextIndex;
}

void GifPlayer::update(unsigned long now) {
  if ((now - lastFrameDisplayedAt) <= currentFrameDelay) {
    return;
  }

  if (playNextGif) {
    playNextGif = false;
    if (openGifFilenameByIndex(directory, chooseNextFile()) >= 0) {
      if (decoder.startDecoding() < 0) {
        playNextGif = true;
        return;
      }
      displayStartedAt = now;
    }
  }

  beginFrame();
  const int result = decoder.decodeFrame(false);
  lastFrameDisplayedAt = now;
  currentFrameDelay = decoder.getFrameDelay_ms();
  if (result < 0) {
    playNextGif = true;
    currentFrameDelay = 0;
  }
}

void GifPlayer::requestNextImage() {
  playNextGif = true;
}

unsigned long GifPlayer::cycleNumber() {
  return decoder.getCycleNumber();
}

void GifPlayer::clearCallback() {
  activePlayer->beginGif();
}

void GifPlayer::updateCallback() {
  activePlayer->renderFrame();
}

void GifPlayer::drawPixelCallback(
    int16_t x, int16_t y, uint8_t red, uint8_t green, uint8_t blue) {
  activePlayer->capturePixel(x, y, red, green, blue);
}

void GifPlayer::beginGif() {
  backgroundColor = CRGB::Black;
  backgroundColorReady = false;
  beginFrame();
  matrix.clear();
}

void GifPlayer::beginFrame() {
  const CRGB fillColor = backgroundColorReady ? backgroundColor : CRGB::Black;
  for (uint16_t index = 0; index < DisplayConfig::LedCount; index++) {
    framePixels[index] = fillColor;
    framePixelsDrawn[index] = 0;
  }

  redTotal = 0;
  greenTotal = 0;
  blueTotal = 0;
  drawnPixelCount = 0;
}

void GifPlayer::capturePixel(
    int16_t x, int16_t y, uint8_t red, uint8_t green, uint8_t blue) {
  if (x < 0 || x >= DisplayConfig::Width || y < 0 || y >= DisplayConfig::Height) {
    return;
  }

  const uint16_t index = y * DisplayConfig::Width + x;
  framePixels[index] = CRGB(red, green, blue);

  if (framePixelsDrawn[index] == 0) {
    framePixelsDrawn[index] = 1;
    redTotal += red;
    greenTotal += green;
    blueTotal += blue;
    drawnPixelCount++;
  }
}

void GifPlayer::renderFrame() {
  if (!backgroundColorReady && drawnPixelCount > 0) {
    backgroundColor = CRGB(
        redTotal / drawnPixelCount,
        greenTotal / drawnPixelCount,
        blueTotal / drawnPixelCount);
    backgroundColorReady = true;

    if (drawnPixelCount < DisplayConfig::LedCount) {
      Serial.printf(
          "[GIF] fundo medio RGB(%u,%u,%u)\n",
          backgroundColor.r,
          backgroundColor.g,
          backgroundColor.b);
    }
  }

  if (backgroundColorReady) {
    for (uint16_t index = 0; index < DisplayConfig::LedCount; index++) {
      if (framePixelsDrawn[index] == 0) {
        framePixels[index] = backgroundColor;
      }
    }
  }

  matrix.showGifFrame(framePixels);
}
