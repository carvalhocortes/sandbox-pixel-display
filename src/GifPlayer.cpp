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

  return true;
}

void GifPlayer::update(unsigned long now) {
  if ((now - lastFrameDisplayedAt) <= currentFrameDelay) {
    return;
  }

  if (playNextGif) {
    playNextGif = false;
    if (openGifFilenameByIndex(directory, fileIndex) >= 0) {
      if (decoder.startDecoding() < 0) {
        playNextGif = true;
        return;
      }
      displayStartedAt = now;
    }

    if (++fileIndex >= fileCount) {
      fileIndex = 0;
    }
  }

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

unsigned long GifPlayer::cycleNumber() const {
  return decoder.getCycleNumber();
}

void GifPlayer::clearCallback() {
  activePlayer->matrix.clear();
}

void GifPlayer::updateCallback() {
  activePlayer->matrix.show();
}

void GifPlayer::drawPixelCallback(
    int16_t x, int16_t y, uint8_t red, uint8_t green, uint8_t blue) {
  activePlayer->matrix.drawGifPixel(x, y, red, green, blue);
}
