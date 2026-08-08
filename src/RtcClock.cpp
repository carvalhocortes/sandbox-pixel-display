#include "RtcClock.h"

#include <Arduino.h>
#include <Wire.h>

bool RtcClock::begin(uint8_t sdaPin, uint8_t sclPin) {
  for (uint8_t attempt = 1; attempt <= 3; attempt++) {
    Wire.begin(sdaPin, sclPin);
    Wire.setClock(100000);
    delay(20);

    Wire.beginTransmission(0x68);
    const uint8_t addressStatus = Wire.endTransmission();
    Serial.printf("RTC I2C tentativa %u: status=%u\n", attempt, addressStatus);

    if (addressStatus == 0 && rtc.begin()) {
      return true;
    }

    delay(100);
  }

  return false;
}

void RtcClock::synchronizeToBuildTime() {
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

DateTime RtcClock::now() {
  return rtc.now();
}
