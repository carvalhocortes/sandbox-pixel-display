#pragma once

#include <RTClib.h>

class DisplayLogger {
public:
  void image(const char* path) const;
  void time(const DateTime& value) const;
  void fibonacciClock(const DateTime& value, uint8_t hour, uint8_t minuteBlock) const;
  void date(const DateTime& value) const;
  void weekday(const DateTime& value) const;
  void rtcOk() const;
  void rtcError() const;
};
