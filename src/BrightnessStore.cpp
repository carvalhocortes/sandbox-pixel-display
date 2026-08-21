#include "BrightnessStore.h"

#include <EEPROM.h>

namespace {
constexpr int EepromSize = 512;
constexpr int BrightnessMagicAddress = 300;
constexpr int BrightnessValueAddress = 301;
constexpr uint8_t BrightnessMagic = 0xB7;
}

bool BrightnessStore::begin() {
  EEPROM.begin(EepromSize);
  ready = true;
  return ready;
}

uint8_t BrightnessStore::load(uint8_t fallback) const {
  if (!ready || EEPROM.read(BrightnessMagicAddress) != BrightnessMagic) {
    return fallback;
  }

  return EEPROM.read(BrightnessValueAddress);
}

bool BrightnessStore::save(uint8_t brightness) {
  if (!ready) {
    return false;
  }

  EEPROM.write(BrightnessMagicAddress, BrightnessMagic);
  EEPROM.write(BrightnessValueAddress, brightness);
  return EEPROM.commit();
}
