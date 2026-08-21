#include "AnalogButtonReader.h"

#include <cstdlib>

#include "DisplayConfig.h"

AnalogButtonReader::AnalogButtonReader(uint8_t pin) : pin(pin) {}

void AnalogButtonReader::begin() {
  pinMode(pin, INPUT);
  idleValue = analogRead(pin);
  candidateValue = idleValue;

  Serial.printf("Botoes analogicos em A0; repouso ADC=%d\n", idleValue);
  Serial.println("Pressione um botao para identificar o valor ADC");
}

AnalogButton AnalogButtonReader::update() {
  const int value = analogRead(pin);
  const bool currentlyPressed = isPressed(value);

  if (currentlyPressed != pressed) {
    if (!candidateActive || std::abs(value - candidateValue) >= MinimumChange) {
      candidateValue = value;
      candidateSince = millis();
      candidateActive = true;
    }

    if (millis() - candidateSince < DebounceMs) {
      return AnalogButton::None;
    }

    pressed = currentlyPressed;
    candidateActive = false;

    if (pressed) {
      const AnalogButton button = buttonForValue(value);
      printPressed(button, value);
      return button;
    } else {
      Serial.printf("Botoes analogicos liberados; ADC=%d\n", value);
      idleValue = value;
    }
    return AnalogButton::None;
  }

  candidateActive = false;
  if (!pressed) {
    idleValue = value;
  }

  return AnalogButton::None;
}

bool AnalogButtonReader::isPressed(int value) const {
  return std::abs(value - idleValue) >= MinimumChange;
}

AnalogButton AnalogButtonReader::buttonForValue(int value) const {
  if (value >= DisplayConfig::AnalogButtonLeftMinimum &&
      value <= DisplayConfig::AnalogButtonLeftMaximum) {
    return AnalogButton::Left;
  }

  if (value >= DisplayConfig::AnalogButtonUpMinimum &&
      value <= DisplayConfig::AnalogButtonUpMaximum) {
    return AnalogButton::Up;
  }

  if (value >= DisplayConfig::AnalogButtonDownMinimum &&
      value <= DisplayConfig::AnalogButtonDownMaximum) {
    return AnalogButton::Down;
  }

  if (value >= DisplayConfig::AnalogButtonRightMinimum &&
      value <= DisplayConfig::AnalogButtonRightMaximum) {
    return AnalogButton::Right;
  }

  if (value >= DisplayConfig::AnalogButtonSelectMinimum &&
      value <= DisplayConfig::AnalogButtonSelectMaximum) {
    return AnalogButton::Select;
  }

  return AnalogButton::None;
}

const char* AnalogButtonReader::buttonName(AnalogButton button) const {
  switch (button) {
    case AnalogButton::Left:
      return "LEFT";
    case AnalogButton::Up:
      return "UP";
    case AnalogButton::Down:
      return "DOWN";
    case AnalogButton::Right:
      return "RIGHT";
    case AnalogButton::Select:
      return "SELECT";
    default:
      return "DESCONHECIDO";
  }
}

void AnalogButtonReader::printPressed(AnalogButton button, int value) const {
  Serial.printf("Botao %s pressionado; ADC=%d\n", buttonName(button), value);
}
