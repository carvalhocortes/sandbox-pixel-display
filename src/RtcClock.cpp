#include "RtcClock.h"

#include <Wire.h>

bool RtcClock::begin(uint8_t sdaPin, uint8_t sclPin) {
  Wire.begin(sdaPin, sclPin);
  return rtc.begin();
}

void RtcClock::synchronizeToBuildTime() {
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

DateTime RtcClock::now() {
  return rtc.now();
}
