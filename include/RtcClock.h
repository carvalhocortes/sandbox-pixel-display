#pragma once

#include <RTClib.h>

class RtcClock {
public:
  bool begin(uint8_t sdaPin, uint8_t sclPin);
  void synchronizeToBuildTime();
  DateTime now();

private:
  RTC_DS3231 rtc;
};
