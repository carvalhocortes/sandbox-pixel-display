#pragma once

#include <Arduino.h>

enum class DisplayMode { Image, Time, Date };

class DisplayScheduler {
public:
  void begin(unsigned long now);
  bool update(unsigned long now, unsigned long gifCycles);
  DisplayMode mode() const;

private:
  static constexpr unsigned long imageDurationMs = 10000UL;
  static constexpr unsigned long timeDurationMs = 5000UL;
  static constexpr unsigned long dateDurationMs = 5000UL;
  static constexpr unsigned long maxGifCycles = 100UL;

  DisplayMode currentMode = DisplayMode::Image;
  unsigned long modeStartedAt = 0;
};
