#pragma once

#include <FastLED.h>
#include <RTClib.h>

class LedMatrix;
class DisplayLogger;

enum class ClockEditPart { None, DateDay, DateMonth, DateYear, TimeMinute, TimeHour };

class ClockRenderer {
public:
  ClockRenderer(LedMatrix& matrix, DisplayLogger& logger);
  void renderTime(const DateTime& value);
  void renderFibonacciClock(const DateTime& value);
  void renderDate(const DateTime& value);
  void renderWeekday(const DateTime& value);
  void renderTimeEditing(const DateTime& value, ClockEditPart part, bool activeVisible);
  void renderDateEditing(const DateTime& value, ClockEditPart part, bool activeVisible);
  void renderTimeFlash(const DateTime& value, bool visible);
  void renderDateFlash(const DateTime& value, bool visible);
  void renderRtcOk();
  void renderRtcError();

private:
  void drawChar(char value, int x, int y, const CRGB& color);
  void drawNumberLine(const char* text, int y, const CRGB& color, bool visible = true);
  void drawDayMonthLine(
      int day,
      int month,
      int y,
      const CRGB& dayColor,
      const CRGB& monthColor,
      bool dayVisible = true,
      bool monthVisible = true);
  void renderTimeContent(
      const DateTime& value,
      const CRGB& color,
      ClockEditPart part,
      bool activeVisible);
  void renderDateContent(
      const DateTime& value,
      const CRGB& color,
      ClockEditPart part,
      bool activeVisible);
  void drawFibonacciSquare(
      int x, int y, int width, int height, const CRGB& color);

  LedMatrix& matrix;
  DisplayLogger& logger;
};
