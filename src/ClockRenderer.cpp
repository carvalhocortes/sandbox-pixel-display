#include "ClockRenderer.h"

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
const CRGB Green(0, 255, 0);
const CRGB Orange(255, 128, 0);
const CRGB Dim(8, 8, 8);

struct FibonacciSquare {
  uint8_t value;
  int x;
  int y;
  int width;
  int height;
};

const FibonacciSquare FibonacciSquares[] = {
    {5, 3, 4, 8, 8},
    {3, 11, 4, 4, 4},
    {2, 11, 8, 2, 4},
    {1, 13, 8, 2, 2},
    {1, 13, 10, 2, 2}};
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

void ClockRenderer::drawFibonacciSquare(
    int x, int y, int width, int height, const CRGB& color) {
  for (int squareY = y; squareY < y + height; squareY++) {
    for (int squareX = x; squareX < x + width; squareX++) {
      matrix.drawPixel(squareX, squareY, color);
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

void ClockRenderer::renderFibonacciClock(const DateTime& value) {
  const uint8_t hour = value.hour() % 12 == 0 ? 12 : value.hour() % 12;
  const uint8_t minuteBlock = value.minute() / 5;
  bool hourSquares[5] = {};
  bool minuteSquares[5] = {};
  uint8_t remainingHour = hour;
  uint8_t remainingMinute = minuteBlock;

  for (uint8_t index = 0; index < 5; index++) {
    const uint8_t squareValue = FibonacciSquares[index].value;
    if (squareValue <= remainingHour) {
      hourSquares[index] = true;
      remainingHour -= squareValue;
    }
    if (squareValue <= remainingMinute) {
      minuteSquares[index] = true;
      remainingMinute -= squareValue;
    }
  }

  matrix.clear();
  for (uint8_t index = 0; index < 5; index++) {
    const bool isHour = hourSquares[index];
    const bool isMinute = minuteSquares[index];
    const CRGB& color = isHour && isMinute ? Orange :
                        isHour             ? Red :
                        isMinute           ? Green : Dim;
    const FibonacciSquare& square = FibonacciSquares[index];
    drawFibonacciSquare(
        square.x, square.y, square.width, square.height, color);
  }
  matrix.show();
  logger.fibonacciClock(value, hour, minuteBlock);
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
