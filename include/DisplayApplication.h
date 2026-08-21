#pragma once

#include "AnalogButtonReader.h"
#include "BrightnessStore.h"
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
  enum class EditField { None, DateDay, DateMonth, DateYear, TimeMinute, TimeHour };

  void handleButton(AnalogButton button);
  void beginDateEditing();
  void beginTimeEditing();
  void advanceEditingField();
  void adjustEditingValue(int direction);
  void adjustBrightness(int direction);
  void saveEditingValue();
  void cancelEditing();
  ClockEditPart clockEditPart() const;
  void updateSaveFlash(unsigned long now);
  bool isEditing() const;
  void handleSerialCommands();
  void processSerialCommand();
  void printSerialHelp() const;
  void resetRenderedContent();

  LedMatrix matrix;
  BrightnessStore brightnessStore;
  RtcClock rtc;
  OtaService ota;
  DisplayLogger logger;
  AnalogButtonReader buttons;
  GifPlayer gifs;
  ClockRenderer clock;
  DisplayScheduler scheduler;
  int lastClockTop = -1;
  int lastClockBottom = -1;
  int lastClockWeekday = -1;
  EditField editField = EditField::None;
  DateTime editingValue = DateTime(2000, 1, 1, 0, 0, 0);
  bool editRenderPending = true;
  bool lastEditVisible = false;
  bool saveFlashActive = false;
  bool saveFlashRenderPending = false;
  bool lastSaveFlashVisible = false;
  unsigned long saveFlashStartedAt = 0;
  DateTime saveFlashValue = DateTime(2000, 1, 1, 0, 0, 0);
  DisplayMode saveFlashMode = DisplayMode::Date;
  char serialCommand[48] = {};
  uint8_t serialCommandLength = 0;
  bool serialCommandOverflowed = false;
};
