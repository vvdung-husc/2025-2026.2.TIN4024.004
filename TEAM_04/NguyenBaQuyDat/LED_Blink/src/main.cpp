#include <Arduino.h>

#define PIN_LED_RED 23
void setup()
{

  Serial.println("He thong den giao thong da cap nhat thoi gian!");
}

void loop()
{
  digitalWrite(PIN_LED_RED, HIGH); // Turn LED ON
  delay(500);                      // Wait for 500ms
  digitalWrite(PIN_LED_RED, LOW);  // Turn LED OFF
  delay(500);                      // Wait for 500ms
}