#include "main.h"

Trafic_Blink traficLight;

void setup() {
  Serial.begin(115200);
  printf("Welcome IoT - ESP32\n");

  traficLight.setupPin(PIN_LED_RED, PIN_LED_YELLOW, PIN_LED_GREEN);
  traficLight.setupWaitTime(1, 1, 1);
}

void loop() {
  traficLight.run();
}
