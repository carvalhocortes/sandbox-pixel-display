#include <Arduino.h>
#include <Wire.h>

constexpr uint8_t D1_PIN = D1;
constexpr uint8_t D2_PIN = D2;

void scanI2C(uint8_t sdaPin, uint8_t sclPin, const char* description) {
  Wire.begin(sdaPin, sclPin);
  delay(100);

  Serial.println();
  Serial.println(description);
  Serial.print("SDA=D");
  Serial.print(sdaPin == D1 ? 1 : 2);
  Serial.print(" SCL=D");
  Serial.println(sclPin == D1 ? 1 : 2);

  uint8_t devicesFound = 0;

  for (uint8_t address = 1; address < 127; address++) {
    Wire.beginTransmission(address);

    if (Wire.endTransmission() == 0) {
      Serial.print("Dispositivo encontrado em 0x");
      if (address < 16) {
        Serial.print('0');
      }
      Serial.println(address, HEX);
      devicesFound++;
    }
  }

  if (devicesFound == 0) {
    Serial.println("Nenhum dispositivo encontrado");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("Scanner I2C");

  scanI2C(D2_PIN, D1_PIN, "Teste 1: ordem normal");
  scanI2C(D1_PIN, D2_PIN, "Teste 2: ordem invertida");
}

void loop() {
  delay(5000);
}
