#pragma once

#include <Arduino.h>

#include "DisplayConfig.h"

enum class DisplayMode { Image, Time, Date, Weekday };

class DisplayScheduler {
public:
  void begin(unsigned long now);
  void forceImage(unsigned long now);
  bool update(unsigned long now, unsigned long gifCycles);
  DisplayMode mode() const;

private:
  static constexpr unsigned long imageDurationMs = DisplayConfig::ImageDurationMs;
  static constexpr unsigned long timeDurationMs = DisplayConfig::TimeDurationMs;
  static constexpr unsigned long dateDurationMs = DisplayConfig::DateDurationMs;
  static constexpr unsigned long weekdayDurationMs = DisplayConfig::WeekdayDurationMs;
  static constexpr unsigned long maxGifCycles = DisplayConfig::MaxGifCycles;

  DisplayMode currentMode = DisplayMode::Image;
  unsigned long modeStartedAt = 0;
};
