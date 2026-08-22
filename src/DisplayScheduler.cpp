#include "DisplayScheduler.h"

void DisplayScheduler::begin(unsigned long now) {
  (void)now;
  currentMode = DisplayMode::Image;
}

void DisplayScheduler::forceImage() {
  currentMode = DisplayMode::Image;
}

bool DisplayScheduler::move(int direction) {
  if (direction == 0) {
    return false;
  }

  int modeIndex = static_cast<int>(currentMode);
  modeIndex += direction > 0 ? 1 : -1;
  if (modeIndex < 0) {
    modeIndex = 4;
  } else if (modeIndex > 4) {
    modeIndex = 0;
  }

  currentMode = static_cast<DisplayMode>(modeIndex);
  return true;
}

DisplayMode DisplayScheduler::mode() const {
  return currentMode;
}
