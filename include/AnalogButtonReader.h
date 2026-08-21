#pragma once

#include <Arduino.h>

enum class AnalogButton { None, Left, Up, Down, Right, Select };

class AnalogButtonReader {
public:
  explicit AnalogButtonReader(uint8_t pin);

  void begin();
  AnalogButton update();

private:
  static constexpr int MinimumChange = 35;
  static constexpr unsigned long DebounceMs = 60UL;

  bool isPressed(int value) const;
  AnalogButton buttonForValue(int value) const;
  const char* buttonName(AnalogButton button) const;
  void printPressed(AnalogButton button, int value) const;

  uint8_t pin;
  int idleValue = 0;
  int candidateValue = 0;
  unsigned long candidateSince = 0;
  bool candidateActive = false;
  bool pressed = false;
};
