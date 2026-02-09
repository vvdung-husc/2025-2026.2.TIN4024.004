#include <Arduino.h>

#define LED_PIN 22

unsigned long lastTime = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  unsigned long now = millis();
  blinkNonBlocking(now, 1000);

  Serial.print("Timer: ");
  Serial.println(now);
}
void blinkNonBlocking(unsigned long now, unsigned long interval) {
  static bool ledState = false;

  if (now - lastTime < interval) return;

  lastTime = now;
  ledState = !ledState;

  digitalWrite(LED_PIN, ledState);
  Serial.println(ledState ? "LED -> ON" : "LED -> OFF");
}
