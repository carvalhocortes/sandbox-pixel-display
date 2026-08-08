#include "DisplayApplication.h"

#include <Arduino.h>

#include "DisplayConfig.h"

DisplayApplication::DisplayApplication()
    : matrix(DisplayConfig::DataPin, DisplayConfig::Width, DisplayConfig::Height),
      gifs(matrix, DisplayConfig::GifDirectory, DisplayConfig::SdChipSelectPin),
      clock(matrix, logger) {}

void DisplayApplication::begin() {
  Serial.begin(115200);
  Serial.println("Display application starting");

  matrix.begin(DisplayConfig::Brightness);
  matrix.testColorsBars(5000, 64);

  if (!rtc.begin(DisplayConfig::RtcSdaPin, DisplayConfig::RtcSclPin)) {
    Serial.println("RTC nao encontrado em 0x68");
  } else {
    rtc.synchronizeToBuildTime();
    Serial.println("RTC sincronizado com o horario do computador");
  }

  ota.begin(DisplayConfig::OtaHostname);

  if (!gifs.begin()) {
    while (true) {
      ota.handle();
      delay(1000);
    }
  }

  scheduler.begin(millis());
  Serial.println("Display application ready");
}

void DisplayApplication::update() {
  const unsigned long now = millis();
  ota.handle();

  if (scheduler.update(now, gifs.cycleNumber())) {
    lastClockTop = -1;
    lastClockBottom = -1;
    matrix.clear();
    if (scheduler.mode() == DisplayMode::Image) {
      gifs.requestNextImage();
    }
  }

  if (scheduler.mode() == DisplayMode::Image) {
    gifs.update(now);
    return;
  }

  if (!rtc.isAvailable()) {
    Serial.println("RTC indisponivel; mantendo modo de imagens");
    scheduler.forceImage(now);
    gifs.requestNextImage();
    return;
  }

  const DateTime current = rtc.now();
  const int top = scheduler.mode() == DisplayMode::Time ? current.hour() : current.day();
  const int bottom = scheduler.mode() == DisplayMode::Time ? current.minute() : current.month();

  if (top == lastClockTop && bottom == lastClockBottom) {
    return;
  }

  if (scheduler.mode() == DisplayMode::Time) {
    clock.renderTime(current);
  } else {
    clock.renderDate(current);
  }

  lastClockTop = top;
  lastClockBottom = bottom;
}
