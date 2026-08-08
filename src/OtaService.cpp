#include "OtaService.h"

#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>

#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD ""
#endif

void OtaService::begin(const char* hostname) {
  if (WIFI_SSID[0] == '\0' || WIFI_PASSWORD[0] == '\0') {
    Serial.println("OTA desabilitado: configure platformio.local.ini");
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.hostname(hostname);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print('.');
  }

  Serial.println();
  Serial.print("Wi-Fi conectado: ");
  Serial.println(WiFi.localIP());

  ArduinoOTA.setHostname(hostname);
  ArduinoOTA.onStart([]() { Serial.println("OTA iniciado"); });
  ArduinoOTA.onEnd([]() { Serial.println("OTA concluido"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA: %u%%\r", (progress * 100U) / total);
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA erro: %u\n", error);
  });
  ArduinoOTA.begin();
  configured = true;
  Serial.println("OTA pronto");
}

void OtaService::handle() {
  if (configured) {
    ArduinoOTA.handle();
  }
}
