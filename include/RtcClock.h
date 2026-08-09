#pragma once

#include <RTClib.h>

class RtcClock {
public:
  bool begin(uint8_t sdaPin, uint8_t sclPin);
  bool isAvailable() const;
  bool needsSynchronization(const DateTime& buildTime);
  void synchronizeToBuildTime(const DateTime& buildTime);
  DateTime now();

private:
  RTC_DS1307 rtc;
  bool available = false;
};
