#pragma once

#include <Arduino.h>

enum class DisplayMode { Image, Time, AnalogClock, Date, Weekday };

class DisplayScheduler {
public:
  void begin(unsigned long now);
  void forceImage();
  bool move(int direction);
  DisplayMode mode() const;

private:
  DisplayMode currentMode = DisplayMode::Image;
};
