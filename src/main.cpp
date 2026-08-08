#include <Arduino.h>
#include <RTClib.h>
#include <Wire.h>

constexpr uint8_t SDA_PIN = D2;
constexpr uint8_t SCL_PIN = D1;

RTC_DS3231 rtc;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("RTC serial test");

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!rtc.begin()) {
    Serial.println("RTC nao encontrado em 0x68");
    while (true) {
      delay(1000);
    }
  }

  // Usa a data e a hora do computador no momento da compilacao.
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  Serial.println("RTC sincronizado com o computador");
}

void loop() {
  DateTime now = rtc.now();

  Serial.printf(
      "%02d/%02d/%04d %02d:%02d:%02d\n",
      now.day(),
      now.month(),
      now.year(),
      now.hour(),
      now.minute(),
      now.second());

  delay(1000);
}
