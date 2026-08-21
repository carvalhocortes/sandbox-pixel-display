#pragma once

#include <Arduino.h>

class BrightnessStore {
public:
  bool begin();
  uint8_t load(uint8_t fallback) const;
  bool save(uint8_t brightness);

private:
  bool ready = false;
};
