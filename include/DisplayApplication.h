#pragma once

#include "ClockRenderer.h"
#include "DisplayLogger.h"
#include "DisplayScheduler.h"
#include "GifPlayer.h"
#include "LedMatrix.h"
#include "OtaService.h"
#include "RtcClock.h"

class DisplayApplication {
public:
  DisplayApplication();
  void begin();
  void update();

private:
  LedMatrix matrix;
  RtcClock rtc;
  OtaService ota;
  DisplayLogger logger;
  GifPlayer gifs;
  ClockRenderer clock;
  DisplayScheduler scheduler;
  int lastClockTop = -1;
  int lastClockBottom = -1;
};
