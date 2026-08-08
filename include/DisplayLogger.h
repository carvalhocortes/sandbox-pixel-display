#pragma once

#include <RTClib.h>

class DisplayLogger {
public:
  void image(const char* path) const;
  void time(const DateTime& value) const;
  void date(const DateTime& value) const;
};
