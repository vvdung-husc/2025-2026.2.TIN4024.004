#include <Arduino.h>

#define LED_PIN 2

void setup() {
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_PIN, HIGH); // bật LED
  delay(500);                  // chờ 500ms
  digitalWrite(LED_PIN, LOW);  // tắt LED
  delay(500);                  // chờ 500ms
}
