#include "RtcClock.h"

#include <Arduino.h>
#include <Wire.h>

namespace {
constexpr uint8_t RtcAddress = 0x68;
constexpr uint8_t EepromAddress = 0x50;
constexpr uint8_t BuildTimestampMagic = 0xA5;
constexpr uint8_t BuildTimestampAddress = 0;
constexpr uint8_t MaxInitializationAttempts = 10;
constexpr unsigned long RetryDelayMs = 500;

uint8_t probeAddress(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission();
}

uint32_t readStoredBuildTimestamp(RTC_DS1307& rtc) {
  if (rtc.readnvram(BuildTimestampAddress) != BuildTimestampMagic) {
    return 0;
  }

  uint32_t timestamp = 0;
  for (uint8_t byteIndex = 0; byteIndex < sizeof(timestamp); byteIndex++) {
    timestamp |= static_cast<uint32_t>(
                     rtc.readnvram(BuildTimestampAddress + 1 + byteIndex))
                 << (byteIndex * 8);
  }
  return timestamp;
}

void storeBuildTimestamp(RTC_DS1307& rtc, uint32_t timestamp) {
  rtc.writenvram(BuildTimestampAddress, BuildTimestampMagic);
  for (uint8_t byteIndex = 0; byteIndex < sizeof(timestamp); byteIndex++) {
    rtc.writenvram(
        BuildTimestampAddress + 1 + byteIndex,
        static_cast<uint8_t>(timestamp >> (byteIndex * 8)));
  }
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

bool RtcClock::needsSynchronization(const DateTime& buildTime) {
  if (!available) {
    return false;
  }

  return readStoredBuildTimestamp(rtc) != buildTime.unixtime();
}

void RtcClock::synchronizeToBuildTime(const DateTime& buildTime) {
  setDateTime(buildTime, buildTime);
}

bool RtcClock::setDateTime(const DateTime& dateTime, const DateTime& buildTime) {
  if (!available) {
    return false;
  }

  rtc.adjust(dateTime);
  storeBuildTimestamp(rtc, buildTime.unixtime());
  return true;
}

DateTime RtcClock::now() {
  if (!available) {
    return DateTime(2000, 1, 1, 0, 0, 0);
  }

  return rtc.now();
}
