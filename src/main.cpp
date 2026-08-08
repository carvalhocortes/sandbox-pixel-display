#include <Arduino.h>
#include <RTClib.h>
#include <Wire.h>

constexpr uint8_t SDA_PIN = D2;
constexpr uint8_t SCL_PIN = D1;

RTC_DS1307 rtc;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("RTC serial test");

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!rtc.begin()) {
    Serial.println("RTC nao encontrado");
    while (true) {
      delay(1000);
    }
  }

  // Sincroniza o RTC com o horario usado na compilacao deste firmware.
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void loop() {
  DateTime now = rtc.now();

  char timestamp[24];
  snprintf(
      timestamp,
      sizeof(timestamp),
      "%02d/%02d/%04d %02d:%02d:%02d",
      now.day(),
      now.month(),
      now.year(),
      now.hour(),
      now.minute(),
      now.second());

  Serial.println(timestamp);
  delay(1000);
}
