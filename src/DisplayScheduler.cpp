#include "DisplayScheduler.h"

void DisplayScheduler::begin(unsigned long now) {
  currentMode = DisplayMode::Image;
  modeStartedAt = now;
}

void DisplayScheduler::forceImage(unsigned long now) {
  currentMode = DisplayMode::Image;
  modeStartedAt = now;
}

bool DisplayScheduler::update(unsigned long now, unsigned long gifCycles) {
  DisplayMode nextMode = currentMode;

  if (currentMode == DisplayMode::Image &&
      ((now - modeStartedAt) > imageDurationMs || gifCycles > maxGifCycles)) {
    nextMode = DisplayMode::Time;
  } else if (currentMode == DisplayMode::Time &&
             (now - modeStartedAt) > timeDurationMs) {
    nextMode = DisplayMode::Date;
  } else if (currentMode == DisplayMode::Date &&
             (now - modeStartedAt) > dateDurationMs) {
    nextMode = DisplayMode::Weekday;
  } else if (currentMode == DisplayMode::Weekday &&
             (now - modeStartedAt) > weekdayDurationMs) {
    nextMode = DisplayMode::Image;
  }

  if (nextMode == currentMode) {
    return false;
  }

  currentMode = nextMode;
  modeStartedAt = now;
  return true;
}

DisplayMode DisplayScheduler::mode() const {
  return currentMode;
}
