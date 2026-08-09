#include "OtaService.h"

#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>

#include "DisplayConfig.h"

#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD ""
#endif

void OtaService::begin(const char* hostname) {
  configured = false;

  if (WIFI_SSID[0] == '\0' || WIFI_PASSWORD[0] == '\0') {
    Serial.println("OTA desabilitado: configure platformio.local.ini");
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.hostname(hostname);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Conectando ao Wi-Fi");
  const unsigned long connectionStartedAt = millis();
  while (WiFi.status() != WL_CONNECTED &&
         millis() - connectionStartedAt < DisplayConfig::OtaConnectTimeoutMs) {
    delay(250);
    Serial.print('.');
  }

  Serial.println();
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect();
    Serial.println("Wi-Fi nao conectado; OTA desabilitado");
    return;
  }

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
