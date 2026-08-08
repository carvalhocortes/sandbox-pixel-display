#include "RtcClock.h"

#include <Arduino.h>
#include <Wire.h>

namespace {
uint8_t probeAddress(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission();
}
}

bool RtcClock::begin(uint8_t sdaPin, uint8_t sclPin) {
  available = false;

  Wire.begin(sdaPin, sclPin);
  Wire.setClock(100000);
  delay(100);

  Serial.printf(
      "I2C linhas: SDA=%s SCL=%s\n",
      digitalRead(sdaPin) == HIGH ? "HIGH" : "LOW",
      digitalRead(sclPin) == HIGH ? "HIGH" : "LOW");
  Serial.printf("I2C EEPROM 0x50: status=%u\n", probeAddress(0x50));
  Serial.printf("I2C RTC 0x68: status=%u\n", probeAddress(0x68));

  available = rtc.begin(&Wire);
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
