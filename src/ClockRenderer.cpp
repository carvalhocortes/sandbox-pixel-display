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
}

ClockRenderer::ClockRenderer(LedMatrix& matrix, DisplayLogger& logger)
    : matrix(matrix), logger(logger) {}

void ClockRenderer::drawChar(char value, int x, int y) {
  for (int row = 0; row < 5; row++) {
    uint8_t bits = 0;
    if (value >= '0' && value <= '9') {
      bits = glyphs[value - '0'][row];
    }

    for (int column = 0; column < 3; column++) {
      matrix.drawPixel(
          x + column, y + row,
          (bits & (1 << (2 - column))) ? CRGB(180, 180, 180) : CRGB::Black);
    }
  }
}

void ClockRenderer::drawNumberLine(const char* text, int y) {
  const int length = strlen(text);
  const int startX = 16 - (length * 3 + length - 1) + 1;
  for (int index = 0; index < length; index++) {
    drawChar(text[index], startX + index * 4, y);
  }
}

void ClockRenderer::drawDayMonthLine(int day, int month, int y) {
  char dayText[3];
  char monthText[3];
  snprintf(dayText, sizeof(dayText), "%02d", day);
  snprintf(monthText, sizeof(monthText), "%02d", month);

  drawChar(dayText[0], 2, y);
  drawChar(dayText[1], 6, y);
  drawChar(monthText[0], 11, y);
  drawChar(monthText[1], 15, y);
}

void ClockRenderer::renderTime(const DateTime& value) {
  char hourText[3];
  char minuteText[3];
  snprintf(hourText, sizeof(hourText), "%02d", value.hour());
  snprintf(minuteText, sizeof(minuteText), "%02d", value.minute());

  logger.time(value);
  matrix.clear();
  drawNumberLine(hourText, 2);
  drawNumberLine(minuteText, 10);
  matrix.show();
}

void ClockRenderer::renderDate(const DateTime& value) {
  char yearText[5];
  snprintf(yearText, sizeof(yearText), "%04d", value.year());

  logger.date(value);
  matrix.clear();
  drawDayMonthLine(value.day(), value.month(), 3);
  drawNumberLine(yearText, 9);
  matrix.show();
}
