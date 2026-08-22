#include "DisplayApplication.h"

#include <Arduino.h>
#include <cstdio>
#include <cstring>

#include "DisplayConfig.h"

namespace {
constexpr unsigned long EditBlinkIntervalMs = 250UL;
constexpr unsigned long SaveFlashIntervalMs = 250UL;
constexpr unsigned long SaveFlashPhases = 4UL;

int wrapValue(int value, int minimum, int maximum) {
  if (value > maximum) {
    return minimum;
  }
  if (value < minimum) {
    return maximum;
  }
  return value;
}

uint8_t daysInMonth(uint16_t year, uint8_t month) {
  static const uint8_t days[] = {31, 28, 31, 30, 31, 30,
                                 31, 31, 30, 31, 30, 31};
  if (month == 2 && (year % 4 == 0)) {
    return 29;
  }
  return days[month - 1];
}
}

DisplayApplication::DisplayApplication()
    : matrix(DisplayConfig::Width, DisplayConfig::Height),
      buttons(DisplayConfig::AnalogButtonsPin),
      gifs(matrix, DisplayConfig::GifDirectory, DisplayConfig::SdChipSelectPin),
      clock(matrix, logger) {}

void DisplayApplication::begin() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Display application starting");
  buttons.begin();
  brightnessStore.begin();
  const uint8_t configuredBrightnessPercent = static_cast<uint8_t>(constrain(
      static_cast<int>(brightnessStore.load(DisplayConfig::BrightnessPercent)),
      static_cast<int>(DisplayConfig::MinimumBrightnessPercent),
      static_cast<int>(DisplayConfig::MaximumBrightnessPercent)));
  Serial.printf("Brilho carregado: %u%%\n", configuredBrightnessPercent);

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

  matrix.begin(configuredBrightnessPercent);
  matrix.testColorsBars(5000, configuredBrightnessPercent);

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

  if (saveFlashActive) {
    updateSaveFlash(now);
    return;
  }

  handleButton(buttons.update());

  if (scheduler.mode() == DisplayMode::Image) {
    gifs.update(now);
    return;
  }

  if (!rtc.isAvailable()) {
    if (scheduler.mode() != DisplayMode::Image) {
      Serial.println("RTC indisponivel; mantendo modo de imagens");
      scheduler.forceImage();
      resetRenderedContent();
      gifs.requestCurrentImage();
    }
    return;
  }

  const DateTime current = isEditing() ? editingValue : rtc.now();
  if (scheduler.mode() == DisplayMode::FibonacciClock) {
    const int hour = current.hour() % 12 == 0 ? 12 : current.hour() % 12;
    const int minuteBlock = current.minute() / 5;
    if (hour == lastClockTop && minuteBlock == lastClockBottom) {
      return;
    }

    clock.renderFibonacciClock(current);
    lastClockTop = hour;
    lastClockBottom = minuteBlock;
    return;
  }

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

  if (isEditing()) {
    const bool activeVisible = (now / EditBlinkIntervalMs) % 2 == 0;
    if (!editRenderPending && activeVisible == lastEditVisible &&
        top == lastClockTop && bottom == lastClockBottom) {
      return;
    }

    if (scheduler.mode() == DisplayMode::Time) {
      clock.renderTimeEditing(current, clockEditPart(), activeVisible);
    } else {
      clock.renderDateEditing(current, clockEditPart(), activeVisible);
    }

    lastEditVisible = activeVisible;
    editRenderPending = false;
  } else {
    if (top == lastClockTop && bottom == lastClockBottom) {
      return;
    }

    if (scheduler.mode() == DisplayMode::Time) {
      clock.renderTime(current);
    } else {
      clock.renderDate(current);
    }
  }

  lastClockTop = top;
  lastClockBottom = bottom;
}

void DisplayApplication::handleButton(AnalogButton button) {
  if (button == AnalogButton::Left || button == AnalogButton::Right) {
    if (isEditing()) {
      cancelEditing();
    }

    const int direction = button == AnalogButton::Right ? 1 : -1;
    if (scheduler.move(direction)) {
      resetRenderedContent();
      if (scheduler.mode() == DisplayMode::Image) {
        gifs.requestCurrentImage();
      }
    }
    return;
  }

  if (button == AnalogButton::Up || button == AnalogButton::Down) {
    const int direction = button == AnalogButton::Up ? 1 : -1;
    if (isEditing()) {
      adjustEditingValue(direction);
    } else if (scheduler.mode() == DisplayMode::Image) {
      gifs.changeImage(direction > 0 ? -1 : 1);
    } else if (scheduler.mode() == DisplayMode::Weekday) {
      adjustBrightness(direction);
    }
    return;
  }

  if (button != AnalogButton::Select) {
    return;
  }

  if (scheduler.mode() == DisplayMode::Date) {
    if (!rtc.isAvailable()) {
      return;
    }
    if (!isEditing()) {
      beginDateEditing();
    } else {
      advanceEditingField();
    }
  } else if (scheduler.mode() == DisplayMode::Time) {
    if (!rtc.isAvailable()) {
      return;
    }
    if (!isEditing()) {
      beginTimeEditing();
    } else {
      advanceEditingField();
    }
  } else if (scheduler.mode() == DisplayMode::Image) {
    gifs.saveCurrentImage();
  }
}

void DisplayApplication::adjustBrightness(int direction) {
  if (direction == 0) {
    return;
  }

  const int requestedBrightness =
      static_cast<int>(matrix.brightnessPercent()) +
      direction * DisplayConfig::BrightnessStepPercent;
  const uint8_t nextBrightness = static_cast<uint8_t>(constrain(
      requestedBrightness,
      static_cast<int>(DisplayConfig::MinimumBrightnessPercent),
      static_cast<int>(DisplayConfig::MaximumBrightnessPercent)));

  if (nextBrightness == matrix.brightnessPercent()) {
    return;
  }

  matrix.setBrightness(nextBrightness);
  lastClockWeekday = -1;
  if (!brightnessStore.save(nextBrightness)) {
    Serial.printf("[BRIGHTNESS] %u%% aplicado, mas nao foi salvo\n", nextBrightness);
    return;
  }

  Serial.printf("[BRIGHTNESS] %u%% salvo\n", nextBrightness);
}

void DisplayApplication::beginDateEditing() {
  editingValue = rtc.now();
  editField = EditField::DateDay;
  Serial.println("[EDIT] Data: ajustando dia; UP aumenta, DOWN diminui");
  resetRenderedContent();
}

void DisplayApplication::beginTimeEditing() {
  const DateTime current = rtc.now();
  editingValue = DateTime(
      current.year(),
      current.month(),
      current.day(),
      current.hour(),
      current.minute(),
      0);
  editField = EditField::TimeMinute;
  Serial.println("[EDIT] Hora: ajustando minutos; UP aumenta, DOWN diminui");
  resetRenderedContent();
}

void DisplayApplication::advanceEditingField() {
  switch (editField) {
    case EditField::DateDay:
      editField = EditField::DateMonth;
      Serial.println("[EDIT] Data: ajustando mes");
      break;
    case EditField::DateMonth:
      editField = EditField::DateYear;
      Serial.println("[EDIT] Data: ajustando ano");
      break;
    case EditField::DateYear:
      saveEditingValue();
      return;
    case EditField::TimeMinute:
      editField = EditField::TimeHour;
      Serial.println("[EDIT] Hora: ajustando horas");
      break;
    case EditField::TimeHour:
      saveEditingValue();
      return;
    case EditField::None:
      return;
  }

  resetRenderedContent();
}

void DisplayApplication::adjustEditingValue(int direction) {
  if (direction == 0 || !isEditing()) {
    return;
  }

  switch (editField) {
    case EditField::DateDay: {
      const int day = wrapValue(
          static_cast<int>(editingValue.day()) + direction,
          1,
          daysInMonth(editingValue.year(), editingValue.month()));
      editingValue = DateTime(
          editingValue.year(),
          editingValue.month(),
          static_cast<uint8_t>(day),
          editingValue.hour(),
          editingValue.minute(),
          editingValue.second());
      break;
    }
    case EditField::DateMonth: {
      const int month = wrapValue(static_cast<int>(editingValue.month()) + direction, 1, 12);
      const uint8_t day = editingValue.day() > daysInMonth(editingValue.year(), month)
                              ? daysInMonth(editingValue.year(), month)
                              : editingValue.day();
      editingValue = DateTime(
          editingValue.year(),
          static_cast<uint8_t>(month),
          day,
          editingValue.hour(),
          editingValue.minute(),
          editingValue.second());
      break;
    }
    case EditField::DateYear: {
      const int year = wrapValue(static_cast<int>(editingValue.year()) + direction, 2000, 2099);
      const uint8_t day = editingValue.day() > daysInMonth(year, editingValue.month())
                              ? daysInMonth(year, editingValue.month())
                              : editingValue.day();
      editingValue = DateTime(
          static_cast<uint16_t>(year),
          editingValue.month(),
          day,
          editingValue.hour(),
          editingValue.minute(),
          editingValue.second());
      break;
    }
    case EditField::TimeMinute: {
      const int minute = wrapValue(static_cast<int>(editingValue.minute()) + direction, 0, 59);
      editingValue = DateTime(
          editingValue.year(),
          editingValue.month(),
          editingValue.day(),
          editingValue.hour(),
          static_cast<uint8_t>(minute),
          editingValue.second());
      break;
    }
    case EditField::TimeHour: {
      const int hour = wrapValue(static_cast<int>(editingValue.hour()) + direction, 0, 23);
      editingValue = DateTime(
          editingValue.year(),
          editingValue.month(),
          editingValue.day(),
          static_cast<uint8_t>(hour),
          editingValue.minute(),
          editingValue.second());
      break;
    }
    case EditField::None:
      return;
  }

  resetRenderedContent();
}

void DisplayApplication::saveEditingValue() {
  const DateTime buildTime(F(__DATE__), F(__TIME__));
  if (!rtc.setDateTime(editingValue, buildTime)) {
    Serial.println("[EDIT] Falha ao salvar data/hora");
    return;
  }

  Serial.printf(
      "[EDIT] Salvo: %02d/%02d/%04d %02d:%02d:%02d\n",
      editingValue.day(),
      editingValue.month(),
      editingValue.year(),
      editingValue.hour(),
      editingValue.minute(),
      editingValue.second());
  saveFlashValue = editingValue;
  saveFlashMode = scheduler.mode();
  saveFlashStartedAt = millis();
  saveFlashActive = true;
  saveFlashRenderPending = true;
  lastSaveFlashVisible = false;
  editField = EditField::None;
}

void DisplayApplication::cancelEditing() {
  Serial.println("[EDIT] Edicao cancelada");
  editField = EditField::None;
  resetRenderedContent();
}

ClockEditPart DisplayApplication::clockEditPart() const {
  switch (editField) {
    case EditField::DateDay:
      return ClockEditPart::DateDay;
    case EditField::DateMonth:
      return ClockEditPart::DateMonth;
    case EditField::DateYear:
      return ClockEditPart::DateYear;
    case EditField::TimeMinute:
      return ClockEditPart::TimeMinute;
    case EditField::TimeHour:
      return ClockEditPart::TimeHour;
    case EditField::None:
      return ClockEditPart::None;
  }

  return ClockEditPart::None;
}

void DisplayApplication::updateSaveFlash(unsigned long now) {
  const unsigned long elapsed = now - saveFlashStartedAt;
  if (elapsed >= SaveFlashIntervalMs * SaveFlashPhases) {
    saveFlashActive = false;
    resetRenderedContent();
    return;
  }

  const bool visible = (elapsed / SaveFlashIntervalMs) % 2 == 0;
  if (!saveFlashRenderPending && visible == lastSaveFlashVisible) {
    return;
  }

  if (saveFlashMode == DisplayMode::Time) {
    clock.renderTimeFlash(saveFlashValue, visible);
  } else if (saveFlashMode == DisplayMode::Date) {
    clock.renderDateFlash(saveFlashValue, visible);
  }

  lastSaveFlashVisible = visible;
  saveFlashRenderPending = false;
}

bool DisplayApplication::isEditing() const {
  return editField != EditField::None;
}

void DisplayApplication::resetRenderedContent() {
  lastClockTop = -1;
  lastClockBottom = -1;
  lastClockWeekday = -1;
  editRenderPending = true;
  lastEditVisible = false;
  matrix.clear();
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
