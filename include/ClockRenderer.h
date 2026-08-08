#pragma once

#include <RTClib.h>

class LedMatrix;
class DisplayLogger;

class ClockRenderer {
public:
  ClockRenderer(LedMatrix& matrix, DisplayLogger& logger);
  void renderTime(const DateTime& value);
  void renderDate(const DateTime& value);
  void renderRtcError();

private:
  void drawChar(char value, int x, int y);
  void drawNumberLine(const char* text, int y);
  void drawDayMonthLine(int day, int month, int y);

  LedMatrix& matrix;
  DisplayLogger& logger;
};
