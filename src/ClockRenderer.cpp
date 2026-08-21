#include "ClockRenderer.h"

#include <cmath>
#include <cstring>

#include "DisplayLogger.h"
#include "LedMatrix.h"

namespace {
const uint8_t glyphs[10][5] = {
    {0b111, 0b101, 0b101, 0b101, 0b111},
    {0b010, 0b110, 0b010, 0b010, 0b111},
    {0b111, 0b001, 0b111, 0b100, 0b111},
    {0b111, 0b001, 0b111, 0b001, 0b111},
    {0b101, 0b101, 0b111, 0b001, 0b001},
    {0b111, 0b100, 0b111, 0b001, 0b111},
    {0b111, 0b100, 0b111, 0b101, 0b111},
    {0b111, 0b001, 0b001, 0b001, 0b001},
    {0b111, 0b101, 0b111, 0b101, 0b111},
    {0b111, 0b101, 0b111, 0b001, 0b111}};

const char* const weekdays[] = {"DOM", "SEG", "TER", "QUA", "QUI", "SEX", "SAB"};
const CRGB White(255, 255, 255);
const CRGB EditGreen(128, 255, 128);
const CRGB Red(255, 0, 0);
const CRGB Blue(0, 0, 255);
constexpr float Pi = 3.14159265359f;
}

ClockRenderer::ClockRenderer(LedMatrix& matrix, DisplayLogger& logger)
    : matrix(matrix), logger(logger) {}

void ClockRenderer::drawChar(char value, int x, int y, const CRGB& color) {
  for (int row = 0; row < 5; row++) {
    uint8_t bits = 0;
    if (value >= '0' && value <= '9') {
      bits = glyphs[value - '0'][row];
    } else {
      switch (value) {
        case 'R': {
          const uint8_t glyph[5] = {0b110, 0b101, 0b110, 0b101, 0b101};
          bits = glyph[row];
          break;
        }
        case 'T': {
          const uint8_t glyph[5] = {0b111, 0b010, 0b010, 0b010, 0b010};
          bits = glyph[row];
          break;
        }
        case 'C': {
          const uint8_t glyph[5] = {0b111, 0b100, 0b100, 0b100, 0b111};
          bits = glyph[row];
          break;
        }
        case 'N': {
          const uint8_t glyph[5] = {0b101, 0b111, 0b111, 0b111, 0b101};
          bits = glyph[row];
          break;
        }
        case 'O': {
          const uint8_t glyph[5] = {0b111, 0b101, 0b101, 0b101, 0b111};
          bits = glyph[row];
          break;
        }
        case 'K': {
          const uint8_t glyph[5] = {0b101, 0b110, 0b110, 0b110, 0b101};
          bits = glyph[row];
          break;
        }
        case 'A': {
          const uint8_t glyph[5] = {0b010, 0b101, 0b111, 0b101, 0b101};
          bits = glyph[row];
          break;
        }
        case 'B': {
          const uint8_t glyph[5] = {0b110, 0b101, 0b110, 0b101, 0b110};
          bits = glyph[row];
          break;
        }
        case 'D': {
          const uint8_t glyph[5] = {0b110, 0b101, 0b101, 0b101, 0b110};
          bits = glyph[row];
          break;
        }
        case 'E': {
          const uint8_t glyph[5] = {0b111, 0b100, 0b110, 0b100, 0b111};
          bits = glyph[row];
          break;
        }
        case 'G': {
          const uint8_t glyph[5] = {0b111, 0b100, 0b101, 0b101, 0b111};
          bits = glyph[row];
          break;
        }
        case 'I': {
          const uint8_t glyph[5] = {0b111, 0b010, 0b010, 0b010, 0b111};
          bits = glyph[row];
          break;
        }
        case 'M': {
          const uint8_t glyph[5] = {0b101, 0b111, 0b111, 0b101, 0b101};
          bits = glyph[row];
          break;
        }
        case 'Q': {
          const uint8_t glyph[5] = {0b111, 0b101, 0b101, 0b111, 0b001};
          bits = glyph[row];
          break;
        }
        case 'S': {
          const uint8_t glyph[5] = {0b111, 0b100, 0b111, 0b001, 0b111};
          bits = glyph[row];
          break;
        }
        case 'U': {
          const uint8_t glyph[5] = {0b101, 0b101, 0b101, 0b101, 0b111};
          bits = glyph[row];
          break;
        }
        case 'X': {
          const uint8_t glyph[5] = {0b101, 0b101, 0b010, 0b101, 0b101};
          bits = glyph[row];
          break;
        }
      }
    }

    for (int column = 0; column < 3; column++) {
      matrix.drawPixel(
          x + column, y + row,
          (bits & (1 << (2 - column))) ? color : CRGB::Black);
    }
  }
}

void ClockRenderer::drawLine(
    int x0, int y0, int x1, int y1, const CRGB& color) {
  const int deltaX = abs(x1 - x0);
  const int stepX = x0 < x1 ? 1 : -1;
  const int deltaY = -abs(y1 - y0);
  const int stepY = y0 < y1 ? 1 : -1;
  int error = deltaX + deltaY;

  while (true) {
    matrix.drawPixel(x0, y0, color);
    if (x0 == x1 && y0 == y1) {
      return;
    }

    const int doubleError = 2 * error;
    if (doubleError >= deltaY) {
      error += deltaY;
      x0 += stepX;
    }
    if (doubleError <= deltaX) {
      error += deltaX;
      y0 += stepY;
    }
  }
}

void ClockRenderer::renderRtcError() {
  logger.rtcError();
  matrix.clear();
  drawNumberLine("RTC", 2, White);
  drawNumberLine("NOK", 10, White);
  matrix.show();
}

void ClockRenderer::renderRtcOk() {
  logger.rtcOk();
  matrix.clear();
  drawNumberLine("RTC", 2, White);
  drawNumberLine("OK", 10, White);
  matrix.show();
}

void ClockRenderer::drawNumberLine(const char* text, int y, const CRGB& color, bool visible) {
  if (!visible) {
    return;
  }

  const int length = strlen(text);
  const int startX = 16 - (length * 3 + length - 1) + 1;
  for (int index = 0; index < length; index++) {
    drawChar(text[index], startX + index * 4, y, color);
  }
}

void ClockRenderer::drawDayMonthLine(
    int day,
    int month,
    int y,
    const CRGB& dayColor,
    const CRGB& monthColor,
    bool dayVisible,
    bool monthVisible) {
  char dayText[3];
  char monthText[3];
  snprintf(dayText, sizeof(dayText), "%02d", day);
  snprintf(monthText, sizeof(monthText), "%02d", month);

  if (dayVisible) {
    drawChar(dayText[0], 1, y, dayColor);
    drawChar(dayText[1], 5, y, dayColor);
  }
  if (monthVisible) {
    drawChar(monthText[0], 10, y, monthColor);
    drawChar(monthText[1], 14, y, monthColor);
  }
}

void ClockRenderer::renderTime(const DateTime& value) {
  renderTimeContent(value, White, ClockEditPart::None, true);
  logger.time(value);
}

void ClockRenderer::renderTimeEditing(
    const DateTime& value,
    ClockEditPart part,
    bool activeVisible) {
  renderTimeContent(value, White, part, activeVisible);
}

void ClockRenderer::renderTimeFlash(const DateTime& value, bool visible) {
  renderTimeContent(value, Red, ClockEditPart::None, visible);
}

void ClockRenderer::renderAnalogClock(const DateTime& value) {
  constexpr float center = 8.5f;
  constexpr float radius = 7.5f;
  constexpr int handOrigin = 9;

  matrix.clear();

  for (int step = 0; step < 60; step++) {
    const float angle = static_cast<float>(step) * 2.0f * Pi / 60.0f;
    const int x = static_cast<int>(std::round(center + std::sin(angle) * radius));
    const int y = static_cast<int>(std::round(center - std::cos(angle) * radius));
    matrix.drawPixel(x, y, White);
  }

  const float hourAngle =
      (static_cast<float>(value.hour() % 12) + value.minute() / 60.0f) * Pi / 6.0f;
  const float minuteAngle =
      (static_cast<float>(value.minute()) + value.second() / 60.0f) * Pi / 30.0f;

  const int hourX = static_cast<int>(std::round(
      center + std::sin(hourAngle) * 4.5f));
  const int hourY = static_cast<int>(std::round(
      center - std::cos(hourAngle) * 4.5f));
  const int minuteX = static_cast<int>(std::round(
      center + std::sin(minuteAngle) * 6.0f));
  const int minuteY = static_cast<int>(std::round(
      center - std::cos(minuteAngle) * 6.0f));

  drawLine(handOrigin, handOrigin, hourX, hourY, White);
  drawLine(handOrigin, handOrigin, minuteX, minuteY, White);

  const float secondAngle = static_cast<float>(value.second()) * Pi / 30.0f;
  const int secondX = static_cast<int>(std::round(
      center + std::sin(secondAngle) * radius));
  const int secondY = static_cast<int>(std::round(
      center - std::cos(secondAngle) * radius));
  matrix.drawPixel(secondX, secondY, Blue);

  matrix.show();
  logger.analogClock(value);
}

void ClockRenderer::renderTimeContent(
    const DateTime& value,
    const CRGB& color,
    ClockEditPart part,
    bool activeVisible) {
  char hourText[4];
  char minuteText[4];
  snprintf(hourText, sizeof(hourText), "%02d", value.hour());
  snprintf(minuteText, sizeof(minuteText), "%02d", value.minute());

  matrix.clear();
  const CRGB& hourColor = part == ClockEditPart::TimeHour ? EditGreen : color;
  const CRGB& minuteColor = part == ClockEditPart::TimeMinute ? EditGreen : color;
  drawNumberLine(
      hourText,
      2,
      hourColor,
      part != ClockEditPart::TimeHour || activeVisible);
  drawNumberLine(
      minuteText,
      10,
      minuteColor,
      part != ClockEditPart::TimeMinute || activeVisible);
  matrix.show();
}

void ClockRenderer::renderDate(const DateTime& value) {
  renderDateContent(value, White, ClockEditPart::None, true);
  logger.date(value);
}

void ClockRenderer::renderDateEditing(
    const DateTime& value,
    ClockEditPart part,
    bool activeVisible) {
  renderDateContent(value, White, part, activeVisible);
}

void ClockRenderer::renderDateFlash(const DateTime& value, bool visible) {
  renderDateContent(value, Red, ClockEditPart::None, visible);
}

void ClockRenderer::renderDateContent(
    const DateTime& value,
    const CRGB& color,
    ClockEditPart part,
    bool activeVisible) {
  char yearText[5];
  snprintf(yearText, sizeof(yearText), "%04d", value.year());

  matrix.clear();
  const CRGB& dayColor = part == ClockEditPart::DateDay ? EditGreen : color;
  const CRGB& monthColor = part == ClockEditPart::DateMonth ? EditGreen : color;
  const CRGB& yearColor = part == ClockEditPart::DateYear ? EditGreen : color;
  drawDayMonthLine(
      value.day(),
      value.month(),
      3,
      dayColor,
      monthColor,
      part != ClockEditPart::DateDay || activeVisible,
      part != ClockEditPart::DateMonth || activeVisible);
  drawNumberLine(
      yearText,
      9,
      yearColor,
      part != ClockEditPart::DateYear || activeVisible);
  matrix.show();
}

void ClockRenderer::renderWeekday(const DateTime& value) {
  logger.weekday(value);
  matrix.clear();
  drawNumberLine(weekdays[value.dayOfTheWeek()], 6, White);
  matrix.show();
}
