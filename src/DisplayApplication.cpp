#include "DisplayApplication.h"

#include <Arduino.h>
#include <cstdio>
#include <cstring>

#include "DisplayConfig.h"

DisplayApplication::DisplayApplication()
    : matrix(DisplayConfig::Width, DisplayConfig::Height),
      gifs(matrix, DisplayConfig::GifDirectory, DisplayConfig::SdChipSelectPin),
      clock(matrix, logger) {}

void DisplayApplication::begin() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Display application starting");

  const bool rtcAvailable = rtc.begin(DisplayConfig::RtcSdaPin, DisplayConfig::RtcSclPin);
  if (!rtcAvailable) {
    Serial.println("RTC nao encontrado em 0x68");
  } else {
    const DateTime computerTime(F(__DATE__), F(__TIME__));
    if (rtc.needsSynchronization(computerTime)) {
      rtc.synchronizeToBuildTime(computerTime);
      Serial.println("RTC sincronizado com o horario do computador");
      Serial.printf(
          "Horario do computador: %02d/%02d/%04d %02d:%02d:%02d\n",
          computerTime.day(),
          computerTime.month(),
          computerTime.year(),
          computerTime.hour(),
          computerTime.minute(),
          computerTime.second());
    } else {
      Serial.println("RTC mantendo o horario salvo");
    }

    const DateTime rtcTime = rtc.now();
    Serial.printf(
        "Data/hora do RTC: %02d/%02d/%04d %02d:%02d:%02d\n",
        rtcTime.day(),
        rtcTime.month(),
        rtcTime.year(),
        rtcTime.hour(),
        rtcTime.minute(),
        rtcTime.second());
  }

  matrix.begin(DisplayConfig::Brightness);
  matrix.testColorsBars(5000, 64);

  if (rtcAvailable) {
    clock.renderRtcOk();
  } else {
    clock.renderRtcError();
  }
  delay(3000);

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
  handleSerialCommands();

  if (scheduler.update(now, gifs.cycleNumber())) {
    lastClockTop = -1;
    lastClockBottom = -1;
    lastClockWeekday = -1;
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
  if (scheduler.mode() == DisplayMode::Weekday) {
    const int weekday = current.dayOfTheWeek();
    if (weekday == lastClockWeekday) {
      return;
    }

    clock.renderWeekday(current);
    lastClockWeekday = weekday;
    return;
  }

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

void DisplayApplication::handleSerialCommands() {
  while (Serial.available() > 0) {
    const char character = static_cast<char>(Serial.read());

    if (character == '\r') {
      continue;
    }

    if (character == '\n') {
      if (serialCommandOverflowed) {
        Serial.println("Comando serial muito longo");
      } else if (serialCommandLength > 0) {
        serialCommand[serialCommandLength] = '\0';
        processSerialCommand();
      }

      serialCommandLength = 0;
      serialCommandOverflowed = false;
      continue;
    }

    if (serialCommandLength < sizeof(serialCommand) - 1) {
      serialCommand[serialCommandLength++] = character;
    } else {
      serialCommandOverflowed = true;
    }
  }
}

void DisplayApplication::processSerialCommand() {
  if (strcmp(serialCommand, "HELP") == 0) {
    printSerialHelp();
    return;
  }

  if (strcmp(serialCommand, "STATUS") == 0) {
    if (!rtc.isAvailable()) {
      Serial.println("RTC indisponivel");
      return;
    }

    const DateTime current = rtc.now();
    Serial.printf(
        "Data/hora do RTC: %02d/%02d/%04d %02d:%02d:%02d\n",
        current.day(),
        current.month(),
        current.year(),
        current.hour(),
        current.minute(),
        current.second());
    return;
  }

  unsigned int year = 0;
  unsigned int month = 0;
  unsigned int day = 0;
  unsigned int hour = 0;
  unsigned int minute = 0;
  unsigned int second = 0;
  char trailingCharacter = '\0';
  const int valuesRead = sscanf(
      serialCommand,
      "SYNC %u-%u-%u %u:%u:%u %c",
      &year,
      &month,
      &day,
      &hour,
      &minute,
      &second,
      &trailingCharacter);

  if (valuesRead != 6) {
    Serial.println("Comando desconhecido; envie HELP para ver os comandos");
    return;
  }

  if (year < 2000 || year > 2099 || month < 1 || month > 12 || day < 1 ||
      day > 31 || hour > 23 || minute > 59 || second > 59) {
    Serial.println("Horario invalido; use SYNC AAAA-MM-DD HH:MM:SS");
    return;
  }

  const DateTime requestedTime(
      static_cast<uint16_t>(year),
      static_cast<uint8_t>(month),
      static_cast<uint8_t>(day),
      static_cast<uint8_t>(hour),
      static_cast<uint8_t>(minute),
      static_cast<uint8_t>(second));
  if (!requestedTime.isValid()) {
    Serial.println("Horario invalido; use SYNC AAAA-MM-DD HH:MM:SS");
    return;
  }

  if (!rtc.isAvailable()) {
    Serial.println("RTC indisponivel; horario nao ajustado");
    return;
  }

  const DateTime buildTime(F(__DATE__), F(__TIME__));
  if (!rtc.setDateTime(requestedTime, buildTime)) {
    Serial.println("RTC indisponivel; horario nao ajustado");
    return;
  }

  Serial.printf(
      "RTC ajustado para: %02d/%02d/%04d %02d:%02d:%02d\n",
      requestedTime.day(),
      requestedTime.month(),
      requestedTime.year(),
      requestedTime.hour(),
      requestedTime.minute(),
      requestedTime.second());
}

void DisplayApplication::printSerialHelp() const {
  Serial.println("Comandos:");
  Serial.println("  SYNC AAAA-MM-DD HH:MM:SS  ajusta o RTC");
  Serial.println("  STATUS                    mostra o horario do RTC");
  Serial.println("  HELP                      mostra esta ajuda");
}
