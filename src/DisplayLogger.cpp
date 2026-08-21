#include "DisplayLogger.h"

void DisplayLogger::image(const char *path) const
{
  Serial.print("[IMAGE] ");
  Serial.println(path);
}

void DisplayLogger::time(const DateTime &value) const
{
  Serial.printf("[TIME] %02d:%02d:%02d\n", value.hour(), value.minute(), value.second());
}

void DisplayLogger::analogClock(const DateTime &value) const
{
  Serial.printf("[ANALOG] %02d:%02d:%02d\n", value.hour(), value.minute(), value.second());
}

void DisplayLogger::date(const DateTime &value) const
{
  Serial.printf("[DATE] %02d/%02d/%04d\n", value.day(), value.month(), value.year());
}

void DisplayLogger::weekday(const DateTime &value) const
{
  static const char *const names[] = {"DOM", "SEG", "TER", "QUA", "QUI", "SEX", "SAB"};
  Serial.print("[WEEKDAY] ");
  Serial.println(names[value.dayOfTheWeek()]);
}

void DisplayLogger::rtcOk() const
{
  Serial.println("[RTC] OK");
}

void DisplayLogger::rtcError() const
{
  Serial.println("[RTC] NOK");
}
