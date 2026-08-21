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
constexpr uint8_t AnalogButtonsPin = A0;
constexpr int AnalogButtonLeftMinimum = 0;
constexpr int AnalogButtonLeftMaximum = 10;
constexpr int AnalogButtonUpMinimum = 150;
constexpr int AnalogButtonUpMaximum = 160;
constexpr int AnalogButtonDownMinimum = 340;
constexpr int AnalogButtonDownMaximum = 350;
constexpr int AnalogButtonRightMinimum = 525;
constexpr int AnalogButtonRightMaximum = 535;
constexpr int AnalogButtonSelectMinimum = 770;
constexpr int AnalogButtonSelectMaximum = 780;
constexpr uint8_t GifColorDepth = 12;
constexpr uint8_t BrightnessPercent = 50;
constexpr uint8_t MinimumBrightnessPercent = 5;
constexpr uint8_t MaximumBrightnessPercent = 100;
constexpr uint8_t BrightnessStepPercent = 5;
constexpr unsigned long OtaConnectTimeoutMs = 15000UL;
constexpr const char* GifDirectory = "/gifs";
constexpr const char* OtaHostname = "pixel-display";
}
