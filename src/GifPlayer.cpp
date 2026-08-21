#include "GifPlayer.h"

#include <EEPROM.h>
#include <SD.h>
#include <cstring>

#include "FilenameFunctions.h"
#include "LedMatrix.h"

GifPlayer* GifPlayer::activePlayer = nullptr;

namespace {
constexpr int EepromSize = 512;
constexpr int FavoriteMagicAddress = 0;
constexpr int FavoritePathAddress = sizeof(uint32_t);
constexpr size_t FavoritePathCapacity = 256;
constexpr uint32_t FavoriteMagic = 0x47494631;
}

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

  EEPROM.begin(EepromSize);
  eepromReady = true;
  currentFileIndex = 0;
  if (!loadSavedImage()) {
    Serial.println("Nenhum GIF favorito salvo; iniciando pelo primeiro arquivo");
  }

  return true;
}

bool GifPlayer::loadSavedImage() {
  char savedPath[FavoritePathCapacity] = {};
  if (!readSavedPath(savedPath)) {
    return false;
  }

  for (int index = 0; index < fileCount; ++index) {
    char path[FavoritePathCapacity] = {};
    getGIFFilenameByIndex(directory, index, path);
    if (strcmp(path, savedPath) == 0) {
      currentFileIndex = index;
      Serial.print("GIF favorito restaurado: ");
      Serial.println(path);
      return true;
    }
  }

  Serial.print("GIF favorito nao encontrado no cartao: ");
  Serial.println(savedPath);
  return false;
}

bool GifPlayer::readSavedPath(char* path) const {
  if (!eepromReady) {
    return false;
  }

  uint32_t magic = 0;
  EEPROM.get(FavoriteMagicAddress, magic);
  if (magic != FavoriteMagic) {
    return false;
  }

  for (size_t index = 0; index < FavoritePathCapacity; ++index) {
    path[index] = static_cast<char>(EEPROM.read(FavoritePathAddress + index));
    if (path[index] == '\0') {
      return index > 0;
    }
  }

  return false;
}

void GifPlayer::update(unsigned long now) {
  if ((now - lastFrameDisplayedAt) <= currentFrameDelay) {
    return;
  }

  if (playNextGif) {
    playNextGif = false;
    if (currentFileIndex < 0 || currentFileIndex >= fileCount) {
      currentFileIndex = 0;
    }

    if (openGifFilenameByIndex(directory, currentFileIndex) >= 0) {
      if (decoder.startDecoding() < 0) {
        playNextGif = true;
        return;
      }
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

void GifPlayer::requestCurrentImage() {
  playNextGif = true;
  lastFrameDisplayedAt = 0;
  currentFrameDelay = 0;
}

bool GifPlayer::changeImage(int direction) {
  if (fileCount <= 0 || direction == 0) {
    return false;
  }

  if (currentFileIndex < 0 || currentFileIndex >= fileCount) {
    currentFileIndex = 0;
  }

  currentFileIndex += direction > 0 ? 1 : -1;
  if (currentFileIndex < 0) {
    currentFileIndex = fileCount - 1;
  } else if (currentFileIndex >= fileCount) {
    currentFileIndex = 0;
  }

  requestCurrentImage();
  return true;
}

bool GifPlayer::saveCurrentImage() {
  if (!eepromReady || currentFileIndex < 0 || currentFileIndex >= fileCount) {
    Serial.println("Nao foi possivel salvar o GIF favorito");
    return false;
  }

  char path[FavoritePathCapacity] = {};
  getGIFFilenameByIndex(directory, currentFileIndex, path);
  if (path[0] == '\0') {
    Serial.println("Nao foi possivel encontrar o GIF atual");
    return false;
  }

  EEPROM.put(FavoriteMagicAddress, FavoriteMagic);
  for (size_t index = 0; index < FavoritePathCapacity; ++index) {
    EEPROM.write(FavoritePathAddress + index, path[index]);
  }

  if (!EEPROM.commit()) {
    Serial.println("Falha ao salvar o GIF favorito");
    return false;
  }

  Serial.print("GIF favorito salvo: ");
  Serial.println(path);
  return true;
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
