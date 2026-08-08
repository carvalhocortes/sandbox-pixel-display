# Repository Guidelines

## Project overview

- PlatformIO Arduino firmware for an ESP8266 NodeMCU (`nodemcuv2`).
- Drives a 16×16 WS2812 matrix, reads GIFs from an SD card, reads an RTC over I²C, and supports OTA updates over Wi-Fi.
- `src/main.cpp` is intentionally only the Arduino bootstrap. Application composition lives in `DisplayApplication`.

## Architecture

- `DisplayApplication` — initializes hardware and coordinates the main loop.
- `DisplayScheduler` — alternates image, time, and date modes without long blocking delays.
- `GifPlayer` — decodes GIF frames and coordinates SD file selection; GIFs are selected randomly without immediate repetition.
- `LedMatrix` — owns the WS2812 buffer, serpentine coordinate mapping, clearing, and output.
- `ClockRenderer` — renders 3×5 digits for time and date.
- `RtcClock` — owns RTC access through `RTClib`.
- `OtaService` — connects to Wi-Fi and services `ArduinoOTA`.
- `DisplayLogger` — serial messages for the content currently shown.
- `FilenameFunctions` — SD-card and GIF decoder file callbacks.

## Hardware contracts

- Matrix data: `D4`.
- SD chip select: `D3`.
- RTC SDA: `D2`; RTC SCL: `D1`.
- RTC device address: `0x68`; the module EEPROM at `0x50` is expected.
- GIF directory on the SD card: `/gifs`.
- Serial monitor: `115200` baud.

Pin assignments and display dimensions are centralized in `include/DisplayConfig.h`; update that file and the README together when hardware changes.

## Build and validation

```sh
pio run -e nodemcuv2
pio run -e nodemcuv2 -t clean
```

Validate on hardware: SD initialization, RTC detection and synchronization, image ordering, two-line clock/date rendering, OTA connectivity, and the WS2812 serpentine mapping.

## Wi-Fi and OTA secrets

- Copy `platformio.local.ini.example` to `platformio.local.ini`.
- Put `WIFI_SSID` and `WIFI_PASSWORD` only in `platformio.local.ini`.
- This file is ignored by Git and must never be committed.
- The first upload is via USB. After the board connects, set `upload_protocol = espota` and `upload_port` in the local file for OTA uploads.

## Change guidelines

- Keep hardware access behind its module; keep mode policy in `DisplayScheduler`.
- Avoid long `delay()` calls in the application loop; OTA and GIF decoding need frequent service.
- Do not commit `.pio/`, `platformio.local.ini`, credentials, or generated IDE files.
- Preserve unrelated working-tree changes and check `git diff --check` before committing.
