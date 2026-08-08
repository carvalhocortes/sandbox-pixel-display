#include "DisplayLogger.h"

void DisplayLogger::image(const char* path) const {
  Serial.print("[IMAGE] ");
  Serial.println(path);
}

void DisplayLogger::time(const DateTime& value) const {
  Serial.printf("[TIME] %02d:%02d\n", value.hour(), value.minute());
}

void DisplayLogger::date(const DateTime& value) const {
  Serial.printf("[DATE] %02d/%02d %04d\n", value.day(), value.month(), value.year());
}
