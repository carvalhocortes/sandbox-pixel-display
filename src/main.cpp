#include <Arduino.h>

unsigned long counter = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("Serial counter test");
}

void loop() {
  Serial.println(++counter);
  delay(500);
}
