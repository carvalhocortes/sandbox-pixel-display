#include "RtcClock.h"

#include <Arduino.h>
#include <Wire.h>

namespace {
constexpr uint8_t RtcAddress = 0x68;
constexpr uint8_t EepromAddress = 0x50;
constexpr uint8_t MaxInitializationAttempts = 10;
constexpr unsigned long RetryDelayMs = 500;

uint8_t probeAddress(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission();
}
}

bool RtcClock::begin(uint8_t sdaPin, uint8_t sclPin) {
  available = false;

  for (uint8_t attempt = 1; attempt <= MaxInitializationAttempts; attempt++) {
    Wire.begin(sdaPin, sclPin);
    Wire.setClock(100000);
    delay(100);

    if (attempt == 1) {
      Serial.printf(
          "I2C linhas: SDA=%s SCL=%s\n",
          digitalRead(sdaPin) == HIGH ? "HIGH" : "LOW",
          digitalRead(sclPin) == HIGH ? "HIGH" : "LOW");
      Serial.printf(
          "I2C EEPROM 0x50: status=%u\n", probeAddress(EepromAddress));
    }

    const uint8_t rtcStatus = probeAddress(RtcAddress);
    Serial.printf(
        "I2C RTC 0x68 tentativa %u/%u: status=%u\n",
        attempt,
        MaxInitializationAttempts,
        rtcStatus);

    if (rtcStatus == 0 && rtc.begin(&Wire)) {
      available = true;
      break;
    }

    if (attempt < MaxInitializationAttempts) {
      delay(RetryDelayMs);
    }
  }

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
