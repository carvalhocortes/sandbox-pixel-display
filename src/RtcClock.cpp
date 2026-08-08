#include "RtcClock.h"

#include <Arduino.h>
#include <Wire.h>

bool RtcClock::begin(uint8_t sdaPin, uint8_t sclPin) {
  available = false;

  Wire.begin(sdaPin, sclPin);
  available = rtc.begin();
  Serial.printf("RTC inicializacao: %s\n", available ? "OK" : "NOK");
  return available;
}

bool RtcClock::isAvailable() const {
  return available;
}

void RtcClock::synchronizeToBuildTime() {
  if (!available) {
    return;
  }

  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

DateTime RtcClock::now() {
  if (!available) {
    return DateTime(2000, 1, 1, 0, 0, 0);
  }

  return rtc.now();
}
