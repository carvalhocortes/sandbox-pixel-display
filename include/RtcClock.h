#pragma once

#include <RTClib.h>

class RtcClock {
public:
  bool begin(uint8_t sdaPin, uint8_t sclPin);
  bool isAvailable() const;
  void synchronizeToBuildTime();
  DateTime now();

private:
  RTC_DS3231 rtc;
  bool available = false;
};
