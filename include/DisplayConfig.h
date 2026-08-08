#pragma once

#include <Arduino.h>

namespace DisplayConfig {
constexpr uint8_t Width = 16;
constexpr uint8_t Height = 16;
constexpr uint16_t LedCount = Width * Height;
constexpr uint8_t DataPin = D4;
constexpr uint8_t SdChipSelectPin = D3;
constexpr uint8_t RtcSdaPin = D2;
constexpr uint8_t RtcSclPin = D1;
constexpr uint8_t GifColorDepth = 12;
constexpr uint8_t Brightness = 50;
constexpr unsigned long ImageDurationMs = 10000UL;
constexpr unsigned long TimeDurationMs = 5000UL;
constexpr unsigned long DateDurationMs = 5000UL;
constexpr unsigned long MaxGifCycles = 100UL;
constexpr const char* GifDirectory = "/gifs";
constexpr const char* OtaHostname = "pixel-display";
}
